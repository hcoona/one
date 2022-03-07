# Base

## Container

* ARC: [ARC: A Self-Tuning, Low Overhead Replacement Cache](https://www.usenix.org/conference/fast-03/arc-self-tuning-low-overhead-replacement-cache), [one up on LRU](https://www.usenix.org/publications/login/august-2003-volume-28-number-4/one-lru), not started yet.
* CircularBuffer: A port from Boost or Chromium, not started yet.
* LRU: a cache which evicts the least recently used item when it is full, not started yet.
* Trie: [An Implementation of Double-Array Trie](https://linux.thai.net/~thep/datrie/datrie.html), [Compressed double-array tries for string dictionaries supporting fast lookup](https://link.springer.com/article/10.1007/s10115-016-0999-8), not started yet.

## Retry Manager & Rate limiter

Port from [resilience4j](https://resilience4j.readme.io/), and [Polly](https://github.com/App-vNext/Polly).

Rate limiter also reference [Guava RateLimiter](https://github.com/google/guava/blob/v31.1/guava/src/com/google/common/util/concurrent/RateLimiter.java).
