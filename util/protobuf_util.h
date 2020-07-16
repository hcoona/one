#ifndef UTIL_PROTOBUF_UTIL_H_
#define UTIL_PROTOBUF_UTIL_H_

#include <iosfwd>

// Copied from
// https://github.com/google/certificate-transparency/blob/2588562fd306a447958471b6f06c1069619c1641/cpp/util/protobuf_util.h
namespace google {
namespace protobuf {
namespace io {
class ZeroCopyOutputStream;
}  // namespace io

class MessageLite;
}  // namespace protobuf
}  // namespace google

namespace hcoona {

// Implements the MessageLite.writeDelimitedTo() method from the Java proto
// API, which is strangly absent from the C++ library.
// This code was pinched from a response by Kenton Varda (ex Protobuf
// developer) to a question about this topic here:
//   http://stackoverflow.com/a/22927149
bool WriteDelimitedTo(const google::protobuf::MessageLite& message,
                      google::protobuf::io::ZeroCopyOutputStream* rawOutput);

bool WriteDelimitedToOstream(const google::protobuf::MessageLite& message,
                             std::ostream* os);

}  // namespace hcoona

#endif  // UTIL_PROTOBUF_UTIL_H_
