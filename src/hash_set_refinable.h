#ifndef HASH_SET_REFINABLE_H
#define HASH_SET_REFINABLE_H

#include <algorithm>
#include <atomic>
#include <cassert>
#include <functional>
#include <mutex>
#include <vector>

#include "./hash_set_base.h"

template <typename T>
class HashSetRefinable : public HashSetBase<T> {
 public:
  explicit HashSetRefinable(size_t initial_capacity)
      : being_resized_(false), set_size_(0) {
    for (size_t i = 0; i < initial_capacity; i++) {
      table_.push_back(std::vector<T>());
    }
    std::vector<std::mutex> mutexes(initial_capacity);
    mutexes_.swap(mutexes);
  }

  bool Add(T elem) final {
    while (being_resized_) {
    }
    std::mutex* curr_mutex = NULL;
    do {
      curr_mutex = &GetMutex(elem);
      curr_mutex->lock();
    } while (curr_mutex != &GetMutex(elem));

    if (ContainsElem(elem)) return false;

    bucket_t<T> bucket = GetBucket(elem);
    bucket.push_back(elem);
    set_size_++;

    curr_mutex->unlock();
    if (ResizePolicy()) Resize();
    return true;
  }

  bool Remove(T elem) final {
    while (being_resized_) {
    }
    std::mutex* curr_mutex = NULL;
    do {
      curr_mutex = &GetMutex(elem);
      curr_mutex->lock();
    } while (curr_mutex != &GetMutex(elem));

    bucket_t<T>& bucket = GetBucket(elem);

    for (size_t i = 0; i < bucket.size(); i++) {
      if (bucket[i] == elem) {
        bucket.erase(bucket.begin() + i);
        return true;
      }
    }

    curr_mutex->unlock();

    return false;
  }

  [[nodiscard]] bool Contains(T elem) final {
    while (being_resized_) {
    }
    std::mutex* curr_mutex = NULL;
    do {
      curr_mutex = &GetMutex(elem);
      curr_mutex->lock();
    } while (curr_mutex != &GetMutex(elem));
    curr_mutex->unlock();
    return ContainsElem(elem);
  }

  [[nodiscard]] size_t Size() const final {
    return set_size_;  // Data race read
  }

 private:
  bool ContainsElem(T elem) {
    bucket_t<T>& bucket = GetBucket(elem);
    return std::find(bucket.begin(), bucket.end(), elem) != bucket.end();
  }

  size_t Hash(T elem) { return std::hash<T>()(elem); }

  bucket_t<T>& GetBucket(T elem) { return table_[Hash(elem) % table_.size()]; }

  std::mutex& GetMutex(T elem) {
    return mutexes_[Hash(elem) % mutexes_.size()];
  }

  bool ResizePolicy() {
    bool cond1 = std::all_of(table_.begin(), table_.end(),
                             [](bucket_t<T> &b) { return b.size() < GLOBAL_THRESHOLD });

    int count = static_cast<int>(std::count_if(table_.begin(), table_.end(),
                              [](bucket_t<T> &b) { return b.size() < BUCKET_THRESHOLD }));
    bool cond2 = count > static_cast<int>(table_.size()) / 4;

    return cond1 || cond2;
  }

  void Resize() {
    being_resized_ = true;
    for (auto& mutex : mutexes_) {
      mutex.lock();
    }

    size_t new_size = table_.size() * 2;
    hashset_table_t<T> new_table(new_size * 2, std::vector<T>());

    for (auto& bucket : table_) {
      for (auto& elem : bucket) {
        bucket_t<T> &new_bucket = new_table[Hash(elem) % new_size];
        new_bucket.push_back(elem);
      }
    }
    table_ = new_table;

    std::vector<std::mutex> new_mutexes(new_size);
    mutexes_.swap(new_mutexes);

    for (auto& mutex : mutexes_) {
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
