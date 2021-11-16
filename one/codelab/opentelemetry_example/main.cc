// Using an exporter that simply dumps span data to stdout.
#include <algorithm>
#include <iostream>
#include <map>
#include <thread>

#include "absl/strings/str_format.h"
#include "absl/strings/string_view.h"
#include "glog/logging.h"
#include "opentelemetry/context/context.h"
#include "opentelemetry/context/propagation/global_propagator.h"
#include "opentelemetry/context/propagation/text_map_propagator.h"
#include "opentelemetry/exporters/memory/in_memory_span_exporter.h"
#include "opentelemetry/sdk/resource/resource.h"
#include "opentelemetry/sdk/trace/simple_processor.h"
#include "opentelemetry/sdk/trace/tracer_provider.h"
#include "opentelemetry/trace/propagation/http_trace_context.h"
#include "opentelemetry/trace/provider.h"
#include "opentelemetry/trace/scope.h"
#include "opentelemetry/trace/span_id.h"
#include "opentelemetry/trace/span_startoptions.h"
#include "gtl/no_destructor.h"

namespace {

struct SpanExporterAndTracerProvider {
  opentelemetry::exporter::memory::InMemorySpanExporter* span_exporter{nullptr};
  std::shared_ptr<opentelemetry::sdk::trace::TracerProvider> tracer_provider{
      nullptr};
};

SpanExporterAndTracerProvider& GetSpanExporterAndTracerProvider() {
  static gtl::NoDestructor<SpanExporterAndTracerProvider>
      span_exporter_and_tracer_provider{[] {
        SpanExporterAndTracerProvider result;

        auto exporter = std::make_unique<
            opentelemetry::exporter::memory::InMemorySpanExporter>();
        result.span_exporter = exporter.get();

        auto processor =
            std::make_unique<opentelemetry::sdk::trace::SimpleSpanProcessor>(
                std::move(exporter));

        result.tracer_provider =
            std::make_shared<opentelemetry::sdk::trace::TracerProvider>(
                std::move(processor));

        return result;
      }()};
  return *span_exporter_and_tracer_provider;
}

std::shared_ptr<opentelemetry::sdk::trace::TracerProvider> GetTracerProvider() {
  return GetSpanExporterAndTracerProvider().tracer_provider;
}

opentelemetry::exporter::memory::InMemorySpanExporter* GetSpanExporter() {
  return GetSpanExporterAndTracerProvider().span_exporter;
}

void InitOpenTelemetry() {
  // Set the global trace provider
  opentelemetry::trace::Provider::SetTracerProvider(GetTracerProvider());

  opentelemetry::context::propagation::GlobalTextMapPropagator::
      SetGlobalPropagator(
          std::make_shared<
              opentelemetry::trace::propagation::HttpTraceContext>());
}

std::shared_ptr<opentelemetry::trace::Tracer> GetOrCreateLibraryTracer() {
  auto provider = opentelemetry::trace::Provider::GetTracerProvider();
  return provider->GetTracer("foo_library");
}

}  // namespace

void RunSomethingInThreads() {
  static constexpr int kThreadNum = 5;

  auto thread_span = GetOrCreateLibraryTracer()->StartSpan(__func__);

  std::vector<std::thread> threads;
  threads.reserve(kThreadNum);

  for (int thread_id = 0; thread_id < kThreadNum; ++thread_id) {
    // This shows how one can effectively use Scope objects to correctly
    // parent spans across threads.
    threads.emplace_back([=] {
      opentelemetry::trace::Scope scope(thread_span);
      auto thread_span = GetOrCreateLibraryTracer()->StartSpan(
          std::string("thread ") + std::to_string(thread_id));
      absl::PrintF("thread span scope exiting... (thread_num=%d)\n", thread_id);
    });
  }

  std::for_each(threads.begin(), threads.end(),
                [](std::thread& th) { th.join(); });
}

class SimpleTextMapCarrier
    : public opentelemetry::context::propagation::TextMapCarrier {
 public:
  absl::string_view Get(absl::string_view key) const noexcept override {
    auto it = headers_.find(std::string(key));
    if (it != headers_.end()) {
      return absl::string_view(it->second);
    }
    return "";
  }

  void Set(absl::string_view key, absl::string_view value) noexcept override {
    headers_[std::string(key)] = std::string(value);
  }

  std::map<std::string, std::string> headers_;
};

void PrintSpanData(std::ostream& sout,
                   const opentelemetry::sdk::trace::SpanData& span);

int main(int /*argc*/, char* argv[]) {
  google::InitGoogleLogging(argv[0]);

  LOG(INFO) << "Initializing.";
  google::FlushLogFiles(google::GLOG_INFO);

  InitOpenTelemetry();

  LOG(INFO) << "Initialized.";
  google::FlushLogFiles(google::GLOG_INFO);

  {
    auto root_span = GetOrCreateLibraryTracer()->StartSpan(
        __func__, {{"cluster_id", "mojito"}});
    {
      opentelemetry::trace::Scope scope(root_span);
      absl::PrintF("root_span->IsValid()=%s\n",
                   root_span->GetContext().IsValid() ? "Valid" : "Invalid");

      RunSomethingInThreads();

      SimpleTextMapCarrier carrier;
      auto current_ctx = opentelemetry::context::RuntimeContext::GetCurrent();
      auto prop = opentelemetry::context::propagation::GlobalTextMapPropagator::
          GetGlobalPropagator();
      prop->Inject(carrier, current_ctx);

      absl::PrintF("carrier header count: %d\n", carrier.headers_.size());
      for (const auto& item : carrier.headers_) {
        absl::PrintF("%s=%s\n", item.first, item.second);
      }

      char span_id_buffer[opentelemetry::trace::SpanId::kSize * 2];
      root_span->GetContext().span_id().ToLowerBase16(span_id_buffer);
      absl::PrintF("Root span. span_id=%s\n",
                   absl::string_view(span_id_buffer,
                                     opentelemetry::trace::SpanId::kSize * 2));

      absl::PrintF("Root span scope exiting...\n");
    }

    absl::PrintF("Root span exiting...\n");
  }

  for (const auto& item : GetSpanExporter()->GetData()->GetSpans()) {
    PrintSpanData(std::cout, *item);
  }
}

// Copied from OStreamSpanExporter

template <typename T>
void print_value(std::ostream& sout, const T& item) {
  sout << item;
}

template <typename T>
void print_value(std::ostream& sout, const std::vector<T>& vec) {
  sout << '[';
  size_t i = 1;
  size_t sz = vec.size();
  for (auto v : vec) {
    sout << v;
    if (i != sz) sout << ',';
    i++;
  }
  sout << ']';
}

void print_value(std::ostream& sout,
                 const opentelemetry::sdk::common::OwnedAttributeValue& value) {
  opentelemetry::nostd::visit(
      [&sout](auto&& arg) {
        /* explicit this is needed by some gcc versions (observed with v5.4.0)*/
        print_value(sout, arg);
      },
      value);
}

void printAttributes(
    std::ostream& sout,
    const std::unordered_map<
        std::string, opentelemetry::sdk::common::OwnedAttributeValue>& map,
    const std::string& prefix = "\n\t") {
  for (const auto& kv : map) {
    sout << prefix << kv.first << ": ";
    print_value(sout, kv.second);
  }
}

void printEvents(
    std::ostream& sout,
    const std::vector<opentelemetry::sdk::trace::SpanDataEvent>& events) {
  for (const auto& event : events) {
    sout << "\n\t{"
         << "\n\t  name          : " << event.GetName()
         << "\n\t  timestamp     : "
         << event.GetTimestamp().time_since_epoch().count()
         << "\n\t  attributes    : ";
    printAttributes(sout, event.GetAttributes(), "\n\t\t");
    sout << "\n\t}";
  }
}

void printLinks(
    std::ostream& sout,
    const std::vector<opentelemetry::sdk::trace::SpanDataLink>& links) {
  for (const auto& link : links) {
    char trace_id[32] = {0};
    char span_id[16] = {0};
    link.GetSpanContext().trace_id().ToLowerBase16(trace_id);
    link.GetSpanContext().span_id().ToLowerBase16(span_id);
    sout << "\n\t{"
         << "\n\t  trace_id      : " << std::string(trace_id, 32)
         << "\n\t  span_id       : " << std::string(span_id, 16)
         << "\n\t  tracestate    : "
         << link.GetSpanContext().trace_state()->ToHeader()
         << "\n\t  attributes    : ";
    printAttributes(sout, link.GetAttributes(), "\n\t\t");
    sout << "\n\t}";
  }
}

std::ostream& operator<<(std::ostream& os,
                         opentelemetry::trace::SpanKind span_kind) {
  switch (span_kind) {
    case opentelemetry::trace::SpanKind::kClient:
      return os << "Client";
    case opentelemetry::trace::SpanKind::kInternal:
      return os << "Internal";
    case opentelemetry::trace::SpanKind::kServer:
      return os << "Server";
    case opentelemetry::trace::SpanKind::kProducer:
      return os << "Producer";
    case opentelemetry::trace::SpanKind::kConsumer:
      return os << "Consumer";
  }
  return os << "";
}

void PrintSpanData(std::ostream& sout,
                   const opentelemetry::sdk::trace::SpanData& span) {
  static std::map<int, std::string> kStatusToStringMap{
      {0, "Unset"}, {1, "Ok"}, {2, "Error"}};

  char trace_id[opentelemetry::trace::TraceId::kSize * 2] = {0};
  char span_id[opentelemetry::trace::SpanId::kSize * 2] = {0};
  char parent_span_id[opentelemetry::trace::SpanId::kSize * 2] = {0};

  span.GetTraceId().ToLowerBase16(trace_id);
  span.GetSpanId().ToLowerBase16(span_id);
  span.GetParentSpanId().ToLowerBase16(parent_span_id);

  sout << "{"
       << "\n  name          : " << span.GetName()
       << "\n  trace_id      : " << std::string(trace_id, 32)
       << "\n  span_id       : " << std::string(span_id, 16)
       << "\n  tracestate    : "
       << span.GetSpanContext().trace_state()->ToHeader()
       << "\n  parent_span_id: " << std::string(parent_span_id, 16)
       << "\n  start         : "
       << span.GetStartTime().time_since_epoch().count()
       << "\n  duration(ns)  : " << span.GetDuration().count()
       << "\n  description   : " << span.GetDescription()
       << "\n  span kind     : " << span.GetSpanKind() << "\n  status        : "
       << kStatusToStringMap[static_cast<int>(span.GetStatus())]
       << "\n  attributes    : ";
  printAttributes(sout, span.GetAttributes());
  sout << "\n  events        : ";
  printEvents(sout, span.GetEvents());
  sout << "\n  links         : ";
  printLinks(sout, span.GetLinks());
  sout << "\n}\n";
}
