# Code Lab

As name indicated, this project is for some experimental idea POCs.

## Projects

* libevent_http: HTTP server example supported by libevent library.
* jinduo_http: HTTP server example supported by jinduo library (our port from muduo library).
* minikafka: Serving as a Apache Kafka broker, AKA. support Apache Kafka protocol in a sub-set. But the idea is similar to Apache Pulsar, which stores the data on remote storage system. The architecture is similar to [SEDA: An Architecture for Well-Conditioned, Scalable Internet Services](http://sosp.org/2001/papers/welsh.pdf). This is a PoC project for writing such a distributed message queue system with high performance & good isolation requirements.
