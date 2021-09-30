#ifndef CODELAB_FEATURE_STORE_BENCHMARK_ENCODING_NULL_OUTPUT_STREAM_H_
#define CODELAB_FEATURE_STORE_BENCHMARK_ENCODING_NULL_OUTPUT_STREAM_H_

#include <memory>

#include "arrow/api.h"
#include "arrow/io/api.h"

namespace hcoona {
namespace codelab {
namespace feature_store {

class NullOutputStream : public arrow::io::OutputStream {
 public:
  NullOutputStream() : closed_(false), position_(0) {}
  ~NullOutputStream() override = default;

  /// \brief Close the stream cleanly
  ///
  /// For writable streams, this will attempt to flush any pending data
  /// before releasing the underlying resource.
  ///
  /// After Close() is called, closed() returns true and the stream is not
  /// available for further operations.
  arrow::Status Close() override {
    closed_ = true;
    return arrow::Status::OK();
  }

  /// \brief Close the stream abruptly
  ///
  /// This method does not guarantee that any pending data is flushed.
  /// It merely releases any underlying resource used by the stream for
  /// its operation.
  ///
  /// After Abort() is called, closed() returns true and the stream is not
  /// available for further operations.
  arrow::Status Abort() override {
    closed_ = true;
    return arrow::Status::OK();
  }

  /// \brief Return the position in this stream
  arrow::Result<int64_t> Tell() const override { return position_; }

  /// \brief Return whether the stream is closed
  bool closed() const override { return closed_; }

  /// \brief Write the given data to the stream
  ///
  /// This method always processes the bytes in full.  Depending on the
  /// semantics of the stream, the data may be written out immediately,
  /// held in a buffer, or written asynchronously.  In the case where
  /// the stream buffers the data, it will be copied.  To avoid potentially
  /// large copies, use the Write variant that takes an owned Buffer.
  arrow::Status Write(const void* data, int64_t nbytes) override {
    position_ += nbytes;
    return arrow::Status::OK();
  }

  /// \brief Write the given data to the stream
  ///
  /// Since the Buffer owns its memory, this method can avoid a copy if
  /// buffering is required.  See Write(const void*, int64_t) for details.
  arrow::Status Write(const std::shared_ptr<arrow::Buffer>& data) override {
    position_ += data->size();
    return arrow::Status::OK();
  }

  /// \brief Flush buffered bytes, if any
  arrow::Status Flush() override { return arrow::Status::OK(); }

 private:
  bool closed_;
  int64_t position_;
};

}  // namespace feature_store
}  // namespace codelab
}  // namespace hcoona

#endif  // CODELAB_FEATURE_STORE_BENCHMARK_ENCODING_NULL_OUTPUT_STREAM_H_
