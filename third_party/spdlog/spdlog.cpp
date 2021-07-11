// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef SPDLOG_COMPILED_LIB
#error Please define SPDLOG_COMPILED_LIB to compile this file.
#endif

#include "third_party/spdlog/spdlog-inl.h"
#include "third_party/spdlog/common-inl.h"
#include "third_party/spdlog/details/backtracer-inl.h"
#include "third_party/spdlog/details/registry-inl.h"
#include "third_party/spdlog/details/os-inl.h"
#include "third_party/spdlog/pattern_formatter-inl.h"
#include "third_party/spdlog/details/log_msg-inl.h"
#include "third_party/spdlog/details/log_msg_buffer-inl.h"
#include "third_party/spdlog/logger-inl.h"
#include "third_party/spdlog/sinks/sink-inl.h"
#include "third_party/spdlog/sinks/base_sink-inl.h"
#include "third_party/spdlog/details/null_mutex.h"

#include <mutex>

// template instantiate logger constructor with sinks init list
template SPDLOG_API spdlog::logger::logger(std::string name, sinks_init_list::iterator begin, sinks_init_list::iterator end);
template class SPDLOG_API spdlog::sinks::base_sink<std::mutex>;
template class SPDLOG_API spdlog::sinks::base_sink<spdlog::details::null_mutex>;