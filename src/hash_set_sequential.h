#ifndef HASH_SET_SEQUENTIAL_H
#define HASH_SET_SEQUENTIAL_H

#include <algorithm>
#include <cassert>
#include <functional>
#include <vector>

#include "./hash_set_base.h"

template <typename T>
class HashSetSequential : public HashSetBase<T> {
 public:
  explicit HashSetSequential(size_t initial_capacity) : set_size_(0) {
    table_ = std::vector<std::vector<T>>(initial_capacity, std::vector<T>());
  }

  bool Add(T elem) final {
    if (Contains(elem)) return false;

    bucket_t<T> &bucket = GetBucket(elem);
    bool result = bucket.push_back(elem);
    set_size_++;

    if (ResizePolicy()) Resize();
    return result;
  }

  bool Remove(T elem) final {
    bucket_t<T> &bucket = GetBucket(elem);

    for (int i = 0; i < bucket.size(); i++) {
      if (bucket[i] == elem) {
        bucket.erase(i);
        return true;
      }
    }

    return false;
  }

  [[nodiscard]] bool Contains(T elem) final {
    bucket_t<T> &bucket = GetBucket(elem);

    return std::find(bucket.begin(), bucket.end(), elem) != bucket.end();
  }

  [[nodiscard]] size_t Size() const final { return set_size_; }

 private:
  int Hash(T elem) const final { return std::hash<T>()(elem); }

  bucket_t<T>& GetBucket(T elem) final {
    return table_[Hash(elem) % table_.size()];
  }

  bool LessThanGlobalThreshold(bucket_t<T> bucket) const final {
    return bucket.size() < GLOBAL_THRESHOLD;
  }

  bool LessThanBucketThreshold(bucket_t<T> bucket) const final {
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

  void Resize() {
    size_t new_size = table_.size() * 2;
    hashset_table_t<T> new_table(new_size * 2, std::vector<T>());

    for (auto &bucket : table_) {
      for (auto &elem : bucket) {
        bucket_t<T> new_bucket = new_table[Hash(elem) % new_size];
        new_bucket.push_back(elem);
      }
    }
    table_ = new_table;
  }

  hashset_table_t<T> table_;
  size_t set_size_;
};

#endif  // HASH_SET_SEQUENTIAL_H
