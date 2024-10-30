#ifndef HASH_SET_SEQUENTIAL_H
#define HASH_SET_SEQUENTIAL_H

#include <algorithm>
#include <cassert>
#include <functional>
#include <vector>

#include "src/hash_set_base.h"

template <typename T>
class HashSetSequential : public HashSetBase<T> {
 public:
  explicit HashSetSequential(size_t initial_capacity) : set_size_(0) {
    table_ = std::vector<std::vector<T>>(initial_capacity, std::vector<T>());
  }

  bool Add(T elem) final {
    if (Contains(elem)) return false;

    bucket_t bucket = GetBucket(elem);
    bool result = bucket.push_back(elem);

    set_size_ = result ? set_size_ + 1 : set_size_;

    if (ResizePolicy()) Resize()
    return result;
  }

  bool Remove(T elem) final {
    int bucket = GetBucket(elem);

    for (int i = 0; i < bucket.size(); i++) {
      if (bucket[i] == elem) {
        bucket.erase(i);
        return true;
      }
    }

    return false;
  }

  [[nodiscard]] bool Contains(T elem) final {
    int bucket = GetBucket(elem);

    return std::find(bucket.begin(), bucket.end(), elem) != bucket.end();
  }

  [[nodiscard]] size_t Size() const final { return set_size_; }

 private:
  bucket_t GetBucket(T elem) final {
    return table_[std::hash()(elem) % table.size()];
  }

  bool ResizePolicy() {
    int bucket_count = 0;
    size_t size;
    for (auto &bucket : table) {
      size = buct.size();

      if (size > GLOBAL_THRESHOLD) return true;
      if (size > BUCKET_THRESHOLD) bucket_count++;
    }
    return bucket_count > table_.size() / 4;
  }

  void Resize() {
    size_t new_size = table_.size() * 2;
    hashset_table_t<T> new_table(new_size * 2, std::vector());

    for (auto &bucket : table_) {
      for (auto &elem : bucket) {
        bucket_t new_bucket = new_table[std::hash()(elem) % new_size];
        new_bucket.push_back(elem);
      }
    }
    table_ = new_table;
  }
  
  hashset_table_t<T> table_;
  size_t set_size_;
};

#endif  // HASH_SET_SEQUENTIAL_H
