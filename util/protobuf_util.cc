// Copied from
// https://github.com/google/certificate-transparency/blob/2588562/cpp/util/protobuf_util.cc
#include "util/protobuf_util.h"

#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/message.h"

namespace hcoona {

// Implements the google::protobuf::MessageLite.writeDelimitedTo() method from
// the Java proto API, which is strangely absent from the C++ library. This code
// was pinched from a response by Kenton Varda (ex Protobuf developer) to a
// question about this topic here:
//   http://stackoverflow.com/a/22927149
bool WriteDelimitedTo(const google::protobuf::MessageLite& message,
                      google::protobuf::io::ZeroCopyOutputStream* rawOutput) {
  // We create a new coded stream for each message.  Don't worry, this is fast.
  google::protobuf::io::CodedOutputStream output(rawOutput);

  // Write the size.
  const int size = message.ByteSize();
  output.WriteVarint32(size);

  google::protobuf::uint8* buffer =
      output.GetDirectBufferForNBytesAndAdvance(size);
  if (buffer != NULL) {
    // Optimization:  The message fits in one buffer, so use the faster
    // direct-to-array serialization path.
    message.SerializeWithCachedSizesToArray(buffer);
  } else {
    // Slightly-slower path when the message is multiple buffers.
    message.SerializeWithCachedSizes(&output);
    if (output.HadError()) return false;
  }

  return true;
}

bool WriteDelimitedToOstream(const google::protobuf::MessageLite& message,
                             std::ostream* os) {
  google::protobuf::io::OstreamOutputStream oos(os);
  return WriteDelimitedTo(message, &oos);
}

}  // namespace hcoona
