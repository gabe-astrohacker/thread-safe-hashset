#ifndef HASH_SET_BASE_H
#define HASH_SET_BASE_H

#include <cstddef>
#include <vector>

#define GLOBAL_THRESHOLD 100;
#define BUCKET_THRESHOLD 50;

template <typename T>
using bucket_t = std::vector<T>;

template <typename T>
using hashset_table_t = std::vector<bucket_t<T>>;

template <typename T>
class HashSetBase {
 public:
  virtual ~HashSetBase() = default;

  // Adds |elem| to the hash set. Returns true if |elem| was absent, and false
  // otherwise.
  virtual bool Add(T elem) = 0;

  // Removes |elem| from the hash set. Returns true if |elem| was present, and
  // false otherwise.
  virtual bool Remove(T elem) = 0;

  // Returns true if |elem| is present in the hash set, and false otherwise.
  [[nodiscard]] virtual bool Contains(T elem) = 0;

  // Returns the size of the hash set.
  [[nodiscard]] virtual size_t Size() const = 0;
};

#endif  // HASH_SET_BASE_H
