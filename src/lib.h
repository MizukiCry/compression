#ifndef COMPRESSION_LIB_H_
#define COMPRESSION_LIB_H_

// #define COMPRESSION_DEBUG

#include <cassert>
#include <iostream>
#include <type_traits>

namespace compression {
void TODO() {
  std::cerr << "[TODO] - Unimplemented!" << std::endl;
  assert(false);
}

template <class T>
class HashTable {
  static const size_t kDefaultTableSize = 1 << 12;

 private:
  class Node {
   private:
    Node *prev_, *next_;
    T key_, value_;

   public:
    Node(const T key, const T value, Node* prev = nullptr, Node* next = nullptr)
        : key_(key), value_(value), prev_(prev), next_(next) {}

    const Node* prev() const { return prev_; }

    const Node* next() const { return next_; }

    const T key() const { return key_; }

    const T value() const { return value_; }
  };

  Node** head;

 public:
  class iterator {
   private:
    int key_;

   public:
    using iterator_category = std::input_iterator_tag;
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using size_type = size_t;
    using difference_type = ptrdiff_t;

    iterator() {}

    ~iterator() {}
  };

  HashTable(const size_t table_size = kDefaultTableSize) {
    head = new Node*[table_size];
    TODO();
  }

  ~HashTable() { delete[] head; }
};
}  // namespace compression

class BitArray {
 private:
 public:
  BitArray() {}
  ~BitArray() {}
};

#endif