// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

#include "third_party/arrow/src/arrow/dataset/file_ipc.h"

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "third_party/arrow/src/arrow/dataset/dataset_internal.h"
#include "third_party/arrow/src/arrow/dataset/file_base.h"
#include "third_party/arrow/src/arrow/dataset/scanner.h"
#include "third_party/arrow/src/arrow/ipc/reader.h"
#include "third_party/arrow/src/arrow/ipc/writer.h"
#include "third_party/arrow/src/arrow/util/checked_cast.h"
#include "third_party/arrow/src/arrow/util/iterator.h"
#include "third_party/arrow/src/arrow/util/logging.h"

namespace arrow {

using internal::checked_pointer_cast;

namespace dataset {

static inline ipc::IpcReadOptions default_read_options() {
  auto options = ipc::IpcReadOptions::Defaults();
  options.use_threads = false;
  return options;
}

static inline Result<std::shared_ptr<ipc::RecordBatchFileReader>> OpenReader(
    const FileSource& source,
    const ipc::IpcReadOptions& options = default_read_options()) {
  ARROW_ASSIGN_OR_RAISE(auto input, source.Open());

  std::shared_ptr<ipc::RecordBatchFileReader> reader;

  auto status =
      ipc::RecordBatchFileReader::Open(std::move(input), options).Value(&reader);
  if (!status.ok()) {
    return status.WithMessage("Could not open IPC input source '", source.path(),
                              "': ", status.message());
  }
  return reader;
}

static inline Result<std::vector<int>> GetIncludedFields(
    const Schema& schema, const std::vector<std::string>& materialized_fields) {
  std::vector<int> included_fields;

  for (FieldRef ref : materialized_fields) {
    ARROW_ASSIGN_OR_RAISE(auto match, ref.FindOneOrNone(schema));
    if (match.indices().empty()) continue;

    included_fields.push_back(match.indices()[0]);
  }

  return included_fields;
}

/// \brief A ScanTask backed by an Ipc file.
class IpcScanTask : public ScanTask {
 public:
  IpcScanTask(std::shared_ptr<FileFragment> fragment,
              std::shared_ptr<ScanOptions> options)
      : ScanTask(std::move(options), fragment), source_(fragment->source()) {}

  Result<RecordBatchIterator> Execute() override {
    struct Impl {
      static Result<RecordBatchIterator> Make(const FileSource& source,
                                              FileFormat* format,
                                              const ScanOptions* scan_options) {
        ARROW_ASSIGN_OR_RAISE(auto reader, OpenReader(source));

        ARROW_ASSIGN_OR_RAISE(
            auto ipc_scan_options,
            GetFragmentScanOptions<IpcFragmentScanOptions>(
                kIpcTypeName, scan_options, format->default_fragment_scan_options));
        auto options = ipc_scan_options->options ? *ipc_scan_options->options
                                                 : default_read_options();
        options.memory_pool = scan_options->pool;
        options.use_threads = false;
        if (!options.included_fields.empty()) {
          // Cannot set them here
          ARROW_LOG(WARNING) << "IpcFragmentScanOptions.options->included_fields was set "
                                "but will be ignored; included_fields are derived from "
                                "fields referenced by the scan";
        }
        ARROW_ASSIGN_OR_RAISE(
            options.included_fields,
            GetIncludedFields(*reader->schema(), scan_options->MaterializedFields()));

        ARROW_ASSIGN_OR_RAISE(reader, OpenReader(source, options));
        return RecordBatchIterator(Impl{std::move(reader), 0});
      }

      Result<std::shared_ptr<RecordBatch>> Next() {
        if (i_ == reader_->num_record_batches()) {
          return nullptr;
        }

        return reader_->ReadRecordBatch(i_++);
      }

      std::shared_ptr<ipc::RecordBatchFileReader> reader_;
      int i_;
    };

    return Impl::Make(
        source_, internal::checked_pointer_cast<FileFragment>(fragment_)->format().get(),
        options_.get());
  }

 private:
  FileSource source_;
};

class IpcScanTaskIterator {
 public:
  static Result<ScanTaskIterator> Make(std::shared_ptr<ScanOptions> options,
                                       std::shared_ptr<FileFragment> fragment) {
    return ScanTaskIterator(IpcScanTaskIterator(std::move(options), std::move(fragment)));
  }

  Result<std::shared_ptr<ScanTask>> Next() {
    if (once_) {
      // Iteration is done.
      return nullptr;
    }

    once_ = true;
    return std::shared_ptr<ScanTask>(new IpcScanTask(fragment_, options_));
  }

 private:
  IpcScanTaskIterator(std::shared_ptr<ScanOptions> options,
                      std::shared_ptr<FileFragment> fragment)
      : options_(std::move(options)), fragment_(std::move(fragment)) {}

  bool once_ = false;
  std::shared_ptr<ScanOptions> options_;
  std::shared_ptr<FileFragment> fragment_;
};

Result<bool> IpcFileFormat::IsSupported(const FileSource& source) const {
  RETURN_NOT_OK(source.Open().status());
  return OpenReader(source).ok();
}

Result<std::shared_ptr<Schema>> IpcFileFormat::Inspect(const FileSource& source) const {
  ARROW_ASSIGN_OR_RAISE(auto reader, OpenReader(source));
  return reader->schema();
}

Result<ScanTaskIterator> IpcFileFormat::ScanFile(
    const std::shared_ptr<ScanOptions>& options,
    const std::shared_ptr<FileFragment>& fragment) const {
  return IpcScanTaskIterator::Make(options, fragment);
}

//
// IpcFileWriter, IpcFileWriteOptions
//

std::shared_ptr<FileWriteOptions> IpcFileFormat::DefaultWriteOptions() {
  std::shared_ptr<IpcFileWriteOptions> ipc_options(
      new IpcFileWriteOptions(shared_from_this()));

  ipc_options->options =
      std::make_shared<ipc::IpcWriteOptions>(ipc::IpcWriteOptions::Defaults());
  return ipc_options;
}

Result<std::shared_ptr<FileWriter>> IpcFileFormat::MakeWriter(
    std::shared_ptr<io::OutputStream> destination, std::shared_ptr<Schema> schema,
    std::shared_ptr<FileWriteOptions> options) const {
  if (!Equals(*options->format())) {
    return Status::TypeError("Mismatching format/write options.");
  }

  auto ipc_options = checked_pointer_cast<IpcFileWriteOptions>(options);

  // override use_threads to avoid nested parallelism
  ipc_options->options->use_threads = false;

  ARROW_ASSIGN_OR_RAISE(auto writer,
                        ipc::MakeFileWriter(destination, schema, *ipc_options->options,
                                            ipc_options->metadata));

  return std::shared_ptr<FileWriter>(
      new IpcFileWriter(std::move(destination), std::move(writer), std::move(schema),
                        std::move(ipc_options)));
}

IpcFileWriter::IpcFileWriter(std::shared_ptr<io::OutputStream> destination,
                             std::shared_ptr<ipc::RecordBatchWriter> writer,
                             std::shared_ptr<Schema> schema,
                             std::shared_ptr<IpcFileWriteOptions> options)
    : FileWriter(std::move(schema), std::move(options), std::move(destination)),
      batch_writer_(std::move(writer)) {}

Status IpcFileWriter::Write(const std::shared_ptr<RecordBatch>& batch) {
  return batch_writer_->WriteRecordBatch(*batch);
}

Status IpcFileWriter::FinishInternal() { return batch_writer_->Close(); }

}  // namespace dataset
}  // namespace arrow
