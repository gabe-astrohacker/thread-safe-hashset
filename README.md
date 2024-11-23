# thread-safe-hashset
• A sequential hash set, i.e. one that is not thread safe.
• A coarse-grained hash set, which achieves thread safety via a “big global lock” that protects the whole hash set.
• A striped hash set, which achieves thread safety at a finer level of granularity via a separate lock for each initial bucket of the hash set, but which keeps the set of locks fixed for the lifetime of the hash set.
• A refinable hash set, which is like a striped hash set except that there is always a separate lock per bucket of the hash set. That is, when the hash set is resized to use a larger number of buckets, the set of locks is resized correspondingly.

This exercise was started as a coursework of a Theory and Practice of Concurrent Programming module and continued independently.
