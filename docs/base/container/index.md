# Container

* `blocking_bounded_queue.h`: fix-size queue which would blocking wait if inserting to a fulled queue or popping from an empty queue. The underlying data structure is a circular buffer to avoid over-commit memory.
* `fixed_ring_buffer.h`: similar to `boost:circular_buffer`. Write from clean house, no other dependencies.
* LRU: a cache which evicts the least recently used item when it is full.

## Not started yet

* ARC: [ARC: A Self-Tuning, Low Overhead Replacement Cache](https://www.usenix.org/conference/fast-03/arc-self-tuning-low-overhead-replacement-cache), [one up on LRU](https://www.usenix.org/publications/login/august-2003-volume-28-number-4/one-lru).
* Trie: [An Implementation of Double-Array Trie](https://linux.thai.net/~thep/datrie/datrie.html), [Compressed double-array tries for string dictionaries supporting fast lookup](https://link.springer.com/article/10.1007/s10115-016-0999-8).
