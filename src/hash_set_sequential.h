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
    for (size_t i = 0; i < initial_capacity; i++) {
      table_.push_back(std::vector<T>());
    }
  }

  bool Add(T elem) final {
    if (Contains(elem)) return false;

    bucket_t<T>& bucket = GetBucket(elem);
    bucket.push_back(elem);
    set_size_++;

    if (ResizePolicy()) Resize();
    return true;
  }

  bool Remove(T elem) final {
    bucket_t<T>& bucket = GetBucket(elem);

    for (long i = 0; i < bucket.size(); i++) {
      if (bucket[i] == elem) {
        bucket.erase(bucket.begin() + i);
        return true;
      }
    }

    return false;
  }

  [[nodiscard]] bool Contains(T elem) final {
    bucket_t<T>& bucket = GetBucket(elem);

    return std::find(bucket.begin(), bucket.end(), elem) != bucket.end();
  }

  [[nodiscard]] size_t Size() const final { return set_size_; }

 private:
  size_t Hash(T elem) { return std::hash<T>()(elem); }

  bucket_t<T>& GetBucket(T elem) { return table_[Hash(elem) % table_.size()]; }

  bool ResizePolicy() {
    bool cond1 = std::all_of(table_.begin(), table_.end(),
                             [](bucket_t<T> &b) { return b.size() < GLOBAL_THRESHOLD });

    int count = static_cast<int>(std::count_if(table_.begin(), table_.end(),
                              [](bucket_t<T> &b) { return b.size() < BUCKET_THRESHOLD }));
    bool cond2 = count > static_cast<int>(table_.size()) / 4;

    return cond1 || cond2;
  }

  void Resize() {
    size_t new_size = table_.size() * 2;
    hashset_table_t<T> new_table(new_size * 2, std::vector<T>());

    for (auto& bucket : table_) {
      for (auto& elem : bucket) {
        bucket_t<T> &new_bucket = new_table[Hash(elem) % new_size];
        new_bucket.push_back(elem);
      }
    }
    table_ = new_table;
  }

  hashset_table_t<T> table_;
  size_t set_size_;
};

#endif  // HASH_SET_SEQUENTIAL_H
