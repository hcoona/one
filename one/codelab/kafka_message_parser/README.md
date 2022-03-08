# Kafka Message Parser

The Apache Kafka message format is described in JSON format under https://github.com/apache/kafka/tree/3.1.0/clients/src/main/resources/common/message.

See https://github.com/apache/kafka/blob/3.1.0/clients/src/main/resources/common/message/README.md & https://cwiki.apache.org/confluence/display/KAFKA/KIP-482%3A+The+Kafka+Protocol+should+Support+Optional+Tagged+Fields#KIP482:TheKafkaProtocolshouldSupportOptionalTaggedFields-TaggedFields for how to read the JSON description files.

See https://kafka.apache.org/protocol.html for the Apache Kafka Protocol.

## Goal

This lab would launch a RPC server to serve ApiVersionRequest because it is easy enough.

This lab would help to learn the generated codes from the JSON format description files.

The referenced Java source code is under https://github.com/apache/kafka/blob/3.1.0/clients%2Fsrc%2Fmain%2Fjava%2Forg%2Fapache%2Fkafka%2Fcommon%2Frequests%2FApiVersionsRequest.java. Because the `ApiVersionsRequestData` class is automatically generated. I'll check-in the generated file in this directory.
