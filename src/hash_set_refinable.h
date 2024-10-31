#ifndef HASH_SET_REFINABLE_H
#define HASH_SET_REFINABLE_H

#include <algorithm>
#include <atomic>
#include <cassert>
#include <functional>
#include <mutex>
#include <vector>
#include "src/hash_set_base.h"

template <typename T>
class HashSetRefinable : public HashSetBase<T> {
 public:
  explicit HashSetStriped(size_t initial_capacity) : set_size_(0), being_resized_(false) {
    table_ = std::vector<std::vector<T>>(initial_capacity, std::vector<T>());
    mutexes_ = std::vector<std::mutexes>(initial_capacity, std::mutex());
  }

  bool Add(T elem) final {
    while (being_resized_) {}
    do {
      std::mutex &mutex = GetMutex(elem);
      std::scoped_lock<std::mutex> lock(mutex);
    } while (mutex != GetMutex(elem));

    if (ContainsElem(elem)) return false;

    bucket_t bucket = GetBucket(elem);
    bucket.push_back(elem);
    set_size_++;

    lock.unlock();
    if (ResizePolicy()) Resize();
    return true;
  }

  bool Remove(T elem) final {
    while (being_resized_) {}
    do {
      std::mutex &mutex = GetMutex(elem);
      std::scoped_lock<std::mutex> lock(mutex);
    } while (mutex != GetMutex(elem));

    bucket_t &bucket = GetBucket(elem);

    for (int i = 0; i < bucket.size(); i++) {
      if (bucket[i] == elem) {
        bucket.erase(i);
        return true;
      }
    }

    return false;
  }

  [[nodiscard]] bool Contains(T elem) final {
    while (being_resized_) {}
    do {
      std::mutex &mutex = GetMutex(elem);
      std::scoped_lock<std::mutex> lock(mutex);
    } while (mutex != GetMutex(elem));
    return ContainsElem(elem);
  }

  [[nodiscard]] size_t Size() const final {
    return set_size_;  // Data race read
  }

 private:
  bool ContainElem(T elem) const final {
    bucket_t &bucket = GetBucket(elem);
    return std::find(bucket.begin(), bucket.end(), elem) != bucket.end();
  }

  bucket_t &GetBucket(T elem) const final {
    return table_[std::hash{}(elem) % table.size()];
  }

  std::mutex &GetMutex(T elem) const final {
    return mutexes_[std::hash{}(elem) % mutexes_.size()];
  }

  bool LessThanGlobalThreshold(bucket_t bucket) const final {
    return bucket.size() < GLOBAL_THRESHOLD;
  }

  bool LessThanBucketThreshold(bucket_t bucket) const final {
    return bucket.size() < BUCKET_THRESHOLD;
  }

  bool ResizePolicy() const final {
    bool cond1 =
        std::all_of(table_.begin(), table_.end(), LessThanGlobalThreshold);

    int count =
        std::count_if(table_.begin(), table_.end(), LessThanBucketThreshold);
    bool cond2 = count > table_.size() / 4;

    return cond1 || cond2;
  }

  void Resize() final {
    being_resized_ = true;
    for (auto &mutex : mutexes_) {
      mutex.lock();
    }

    size_t new_size = table_.size() * 2;
    hashset_table_t<T> new_table(new_size * 2, std::vector());

    for (auto& bucket : table_) {
      for (auto& elem : bucket) {
        bucket_t new_bucket = new_table[std::hash()(elem) % new_size];
        new_bucket.push_back(elem);
      }
    }
    table_ = new_table;

    std::vector<std::mutex> new_mutexes(new_size, std::mutex());
    std::vector<std::mutex> &old_mutexes = &mutexes_;
    mutexes_ = new_mutexes;

    for (auto &mutex : mutexes_) {
      mutex.unlock();
    }
    being_resized_ = false;
  }

  hashset_table_t<T> table_;
  std::vector<std::mutex> mutexes_;
  std::atomic_bool being_resized_;
  size_t set_size_;
};

#endif  // HASH_SET_REFINABLE_H
