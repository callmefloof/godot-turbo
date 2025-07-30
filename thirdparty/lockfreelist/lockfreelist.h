#pragma once

#include <atomic>
#include <iostream>
#include <memory>
#include <vector>
#include <thread>
#include <cassert>

namespace ut {

struct Node {

  struct Tag {
    static constexpr uintptr_t version_mask = 0x4;
    static constexpr uintptr_t ptr_mask = 0xFFFFFFFFFFFFFFF0;

    Tag() noexcept = default;

    explicit Tag(uintptr_t ptr, uintptr_t version) noexcept
      : m_ptr(ptr & ptr_mask) {
      assert(version < (1u << 4) - 1);
      m_ptr |= version;
    }

    explicit Tag(Node* ptr, uintptr_t version) noexcept
      : Tag(reinterpret_cast<uintptr_t>(ptr), version) {}

    bool operator==(const Tag& rhs) const noexcept {
      return m_ptr == rhs.m_ptr;
    }

    operator Node*() const noexcept {
      return reinterpret_cast<Node*>(m_ptr & ptr_mask);
    }

    uint32_t version() const {
      return m_ptr & version_mask;
    }

    Tag next_version() const noexcept {
      const auto v = version();
      assert(v < (1u << 4) - 1);  

      return Tag(m_ptr, v + 1);
    }

    uintptr_t m_ptr{};
  };

  static_assert(sizeof(Tag) == 8, "Tag must be 8 bytes");
    
  Node() = default;

  virtual ~Node() = default;

  void init() {
    Tag null_tag{};

    m_next.store(null_tag, std::memory_order_relaxed);
    m_prev.store(null_tag, std::memory_order_relaxed);
  }

  void prefetch_next() const {
    /* Read, high temporal locality */
    if (auto next = (Node*)(m_next.load(std::memory_order_acquire))) {
      __builtin_prefetch(next, 0, 3);
    }
  }
  
  void prefetch_prev() const {
    /* Read, high temporal locality */
    if (auto prev = (Node*)m_prev.load(std::memory_order_acquire)) {
      __builtin_prefetch(prev, 0, 3);
    }
  }

  std::atomic<Tag> m_next{};
  std::atomic<Tag> m_prev{};
};

template <typename T>
struct Lock_free_list {

  class iterator;
  class const_iterator;
    
  struct iterator {
    using value_type = T;
    using pointer = T*;
    using reference = T&;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::bidirectional_iterator_tag;
        
    iterator() noexcept : m_node(nullptr) {}

    explicit iterator(Node* node, Node* prev = nullptr) noexcept 
      : m_node(node), m_prev(prev) {}
        
    reference operator*() const {
      if (m_node == nullptr) {
        throw std::runtime_error("Dereferencing null iterator");
      }
      return *static_cast<T*>(m_node);
    }
        
    pointer operator->() const {
      if (m_node == nullptr) {
        throw std::runtime_error("Dereferencing null iterator");
      }
      return static_cast<T*>(m_node);
    }
        
    iterator& operator++() {
      if (m_node == nullptr) {
        throw std::runtime_error("Incrementing null iterator");
      }
            
      /* Load next node with acquire semantics */
      auto next = (Node*)m_node->m_next.load(std::memory_order_acquire);
            
      /* Verify the node hasn't been removed */
      if ((Node*)m_node->m_prev.load(std::memory_order_acquire) != m_prev) {
        /* Node was removed, try to recover by finding next valid node */
        while (m_node != nullptr && (Node*)m_node->m_prev.load(std::memory_order_acquire) != m_prev) {
          m_node = (Node*)m_node->m_next.load(std::memory_order_acquire);
          if (m_node != nullptr) {
            m_prev = (Node*)m_node->m_prev.load(std::memory_order_acquire);
          }
        }
      } else {
        m_prev = m_node;
        m_node = next;
      }
            
      return *this;
    }
        
    iterator operator++(int) {
      auto tmp = *this;

      ++(*this);
      return tmp;
    }
        
    iterator& operator--() {
      if (m_node == nullptr) {
        throw std::runtime_error("Decrementing begin iterator");
      }
            
      /* Load prev node with acquire semantics */
      auto prev = (Node*)m_prev->m_prev.load(std::memory_order_acquire);
            
      /* Verify the node hasn't been removed */
      if (m_prev->m_next.load(std::memory_order_acquire) != m_node) {
        /* Node was removed, try to recover by finding previous valid node */
        while (m_prev != nullptr && (Node*)m_prev->m_next.load(std::memory_order_acquire) != m_node) {
          m_prev = (Node*)m_prev->m_prev.load(std::memory_order_acquire);
          if (m_prev != nullptr) {
            m_node = (Node*)m_prev->m_next.load(std::memory_order_acquire);
          }
        }
      } else {
        m_node = m_prev;
        m_prev = prev;
      }
            
      return *this;
    }
        
    iterator operator--(int) {
      auto tmp = *this;
      --(*this);
      return tmp;
    }
        
    bool operator==(const iterator& rhs) const noexcept {
      return m_node == rhs.m_node;
    }
        
    bool operator!=(const iterator& rhs) const noexcept {
      return !(*this == rhs);
    }
        
    Node* m_node;

    /* Keep track of previous node for bidirectional iteration */
    Node* m_prev;
  };
    
  struct const_iterator {
    using value_type = const T;
    using pointer = const T*;
    using reference = const T&;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::bidirectional_iterator_tag;
        
    const_iterator() noexcept
      : m_node(nullptr), m_prev(nullptr) {}

    const_iterator(const iterator& other) noexcept 
      : m_node(other.m_node), m_prev(other.m_prev) {}

    explicit const_iterator(const Node* node, const Node* prev = nullptr) noexcept 
      : m_node(node), m_prev(prev) {}
        
    reference operator*() const {
      if (m_node == nullptr) {
        throw std::runtime_error("Dereferencing null iterator");
      }
      return *static_cast<const T*>(m_node);
    }
        
    pointer operator->() const {
      if (m_node == nullptr) {
        throw std::runtime_error("Dereferencing null iterator");
      }
      return static_cast<const T*>(m_node);
    }
        
    const_iterator& operator++() {
      if (m_node == nullptr) {
        throw std::runtime_error("Incrementing null iterator");
      }
            
      auto next = (Node*)m_node->m_next.load(std::memory_order_acquire);
            
      if ((Node*)m_node->m_prev.load(std::memory_order_acquire) != m_prev) {
        while (m_node != nullptr && (Node*)m_node->m_prev.load(std::memory_order_acquire) != m_prev) {
          m_node = (Node*)m_node->m_next.load(std::memory_order_acquire);
          if (m_node != nullptr) {
            m_prev = (Node*)m_node->m_prev.load(std::memory_order_acquire);
          }
        }
      } else {
        m_prev = m_node;
        m_node = next;
      }
            
      return *this;
    }
        
    const_iterator operator++(int) {
      auto tmp = *this;
      ++(*this);
      return tmp;
    }
        
    const_iterator& operator--() {
      if (m_prev == nullptr) {
        throw std::runtime_error("Decrementing begin iterator");
      }
            
      auto prev = (Node*)m_prev->m_prev.load(std::memory_order_acquire);
            
      if ((Node*)m_prev->m_next.load(std::memory_order_acquire) != m_node) {
        while (m_prev != nullptr && (Node*)m_prev->m_next.load(std::memory_order_acquire) != m_node) {
          m_prev = (Node*)m_prev->m_prev.load(std::memory_order_acquire);
          if (m_prev != nullptr) {
            m_node = (Node*)m_prev->m_next.load(std::memory_order_acquire);
          }
        }
      } else {
        m_node = m_prev;
        m_prev = prev;
      }
            
      return *this;
    }
        
    const_iterator operator--(int) {
      auto tmp = *this;
      --(*this);
      return tmp;
    }
        
    bool operator==(const const_iterator& other) const noexcept {
      return m_node == other.m_node;
    }
        
    bool operator!=(const const_iterator& other) const noexcept {
      return !(*this == other);
    }
        
    const Node* m_node;
    const Node* m_prev;
  };
    
  Lock_free_list() {
    Node::Tag null_tag{}; 

    m_head.store(null_tag, std::memory_order_relaxed);
    m_tail.store(null_tag, std::memory_order_relaxed);
  }

  ~Lock_free_list() {
    clear();
  }

  /* Add a node to the front */
  void push_front(Node* node) {
    assert(node != nullptr);
        
    typename Node::Tag null_ptr{};

    node->init();
        
    for (;;) {
      auto old_head = m_head.load(std::memory_order_acquire);
            
      /* Setup new node's pointers */
      node->m_next.store(typename Node::Tag{old_head.m_ptr, 0}, std::memory_order_relaxed);
            
      /* Try to set as new head with incremented version */
      typename Node::Tag new_head{node, old_head.version() + 1};
            
      if (m_head.compare_exchange_weak(old_head, new_head, std::memory_order_release, std::memory_order_relaxed)) {
        if (old_head != nullptr) {
          /* Update old head's prev pointer */
          auto old_prev = ((Node*) old_head)->m_prev.load(std::memory_order_acquire);
          typename Node::Tag new_prev{node, old_prev.version() + 1};

          ((Node*) old_head)->m_prev.store(new_prev, std::memory_order_release);
        } else {
          // Empty list case - update tail
          m_tail.store(new_head, std::memory_order_release);
        }
        return;
      }
    }
  }

  /* Remove a specific node */
  void remove(Node* node) {
    for (;;) {
      /* Load both links with their versions */
      auto prev = node->m_prev.load(std::memory_order_acquire);
      auto next = node->m_next.load(std::memory_order_acquire);
            
      Node *prev_ptr = prev;
      Node *next_ptr = next;
            
      if (prev_ptr != nullptr) {
        /* Read prev's current next pointer and version */
        auto expected = ((Node*)prev)->m_next.load(std::memory_order_acquire);

        if (expected != node) {
          /* Node already removed or list changed */
          continue;
        }
                
        /* Prepare new tagged pointer with incremented version */
        Node::Tag new_tag{next_ptr, expected.version() + 1};
                
        /* Try to update with new version */
        if (((Node*)prev)->m_next.compare_exchange_strong(expected, new_tag, std::memory_order_release, std::memory_order_relaxed)) {
          /* Successfully unlinked from prev side */
          if (next != nullptr) {
            auto next_expected = ((Node*)next)->m_prev.load(std::memory_order_acquire);
            /* Increment version */
            Node::Tag next_new{prev_ptr, next_expected.version() + 1};

            ((Node*)next)->m_prev.compare_exchange_strong(next_expected, next_new, std::memory_order_release);
          }
          return;
        }
      } else {
        /* Handle head case */
        auto expected = m_head.load(std::memory_order_acquire);

        if (expected != node) {
          continue;
        }
                
        Node::Tag new_tag{next_ptr, expected.version() + 1};
                
        if (m_head.compare_exchange_strong(expected, new_tag, std::memory_order_release, std::memory_order_relaxed)) {
          if (next_ptr != nullptr) {
            auto next_expected = ((Node*)next)->m_prev.load(std::memory_order_acquire);
            Node::Tag next_new{nullptr, next_expected.version() + 1};

            ((Node*)next)->m_prev.compare_exchange_strong(next_expected, next_new, std::memory_order_release);
          }
          return;
        }
      }
    }
  }

  /* Add a node to the back */
  void push_back(Node* node) {
    assert(node != nullptr);

    node->init();

    typename Node::Tag null_tag{};
        
    for (;;) {
      auto old_tail = m_tail.load(std::memory_order_acquire);
            
      if (old_tail == nullptr) {
        /* Empty list case - try to set both head and tail */
        auto old_head = m_head.load(std::memory_order_acquire);

        if (old_head == nullptr) {
          typename Node::Tag new_tag{node, old_head.version() + 1};

          if (m_head.compare_exchange_weak(old_head, new_tag, std::memory_order_release, std::memory_order_relaxed)) {
            m_tail.store(new_tag, std::memory_order_release);
            return;
          }
        }
        continue;
      }
            
      /* Setup new node's prev pointer */
      node->m_prev.store(typename Node::Tag{old_tail.m_ptr, 0}, std::memory_order_relaxed);
            
      /* Try to update old tail's next pointer */
      auto old_next = ((Node*)old_tail)->m_next.load(std::memory_order_acquire);

      /* Tail was incorrect */
      if (old_next != nullptr) {
        continue;
      }
            
      /* Prepare new tagged pointer with incremented version */
      typename Node::Tag new_next{node, old_next.version() + 1};

      if (((Node*)old_tail)->m_next.compare_exchange_weak(old_next, new_next, std::memory_order_release, std::memory_order_relaxed)) {
        /* Update tail pointer */
        typename Node::Tag new_tail{node, old_tail.version() + 1};

        m_tail.compare_exchange_strong(old_tail, new_tail, std::memory_order_release, std::memory_order_relaxed);
        return;
      }
    }    
  }

  /* Insert a node after a specific node */
  bool insert_after(Node* node, Node* new_node) {
    assert(node != nullptr);
    assert(new_node != nullptr);
        
    typename Node::Tag null_ptr{};

    new_node->init();
        
    for (;;) {
      auto next_tagged = node->m_next.load(std::memory_order_acquire);

      new_node->m_prev.store(typename Node::Tag{node, 0}, std::memory_order_relaxed);
      new_node->m_next.store(typename Node::Tag{next_tagged.m_ptr, 0}, std::memory_order_relaxed);
            
      typename Node::Tag new_next{new_node, next_tagged.version() + 1};

      if (node->m_next.compare_exchange_weak(next_tagged, new_next, std::memory_order_release, std::memory_order_relaxed)) {
        if (next_tagged != nullptr) {
          auto next_prev = ((Node*)next_tagged)->m_prev.load(std::memory_order_acquire);
          typename Node::Tag new_prev{new_node, next_prev.version() + 1};

          ((Node*)next_tagged)->m_prev.store(new_prev, std::memory_order_release);
        } else {
          /* New node becomes the tail */
          auto old_tail = m_tail.load(std::memory_order_acquire);

          if (old_tail == node) {
            typename Node::Tag new_tail{new_node, old_tail.version() + 1};

            m_tail.compare_exchange_strong(old_tail, new_tail, std::memory_order_release, std::memory_order_relaxed);
          }
        }
        return true;
      }
    }
  }

  /* Find a node with a specific value */
  template<typename Predicate>
  Node* find_if(Predicate pred) {
    for (;;) {
      auto current = m_head.load(std::memory_order_acquire);
        
      while (current != nullptr) {
        /* Check if current node matches predicate */
        if (pred(static_cast<T*>((Node*)current))) {
          /* Verify node is still in list by checking its links */
          auto next = ((Node*)current)->m_next.load(std::memory_order_acquire);
          auto prev = ((Node*)current)->m_prev.load(std::memory_order_acquire);
                
          /* If node's next->prev points back to node, it's still valid */
          if (next != nullptr) {
            if (((Node*)next)->m_prev.load(std::memory_order_acquire) != current) {
              /* Node was removed, restart search */
              break;
            }
          } else if (m_tail.load(std::memory_order_acquire) != current) {
            /* Node was tail but no longer is */
            break;
          }
                
          /* If node's prev->next points to node, it's still valid */
          if (prev != nullptr) {
            if (((Node*)prev)->m_next.load(std::memory_order_acquire) != current) {
              /* Node was removed, restart search */
              break;
            }
          } else if (m_head.load(std::memory_order_acquire) != current) {
            /* Node was head but no longer is */
            break;
          }
                
          return current;
        }
        current = ((Node*)current)->m_next.load(std::memory_order_acquire);
      }
        
      /* If we completed iteration without finding a match, return nullptr */
      if (current == nullptr) {
        return nullptr;
      }
        
      /* If we broke out of the loop due to concurrent modification, retry */
    }
  }

  /* Convenience method to find by value */
  Node* find(const typename T::value_type& value) {
    return find_if([&value](const T* node) {
      return node->m_value == value;
    });
  }

  /* Clear the list */
  void clear() {
    Node::Tag null_tag{}; 

    /* The containing node should be deleted by the list owne*/
    m_head.store(null_tag, std::memory_order_relaxed);
    m_tail.store(null_tag, std::memory_order_relaxed);
  }

  iterator begin() noexcept {
    return iterator(m_head.load(std::memory_order_acquire), nullptr);
  }
    
  const_iterator begin() const noexcept {
    return const_iterator(m_head.load(std::memory_order_acquire), nullptr);
  }
    
  const_iterator cbegin() const noexcept {
    return const_iterator(m_head.load(std::memory_order_acquire), nullptr);
  }
  
  iterator end() noexcept {
    return iterator(nullptr, m_tail.load(std::memory_order_acquire));
  }
  
  const_iterator end() const noexcept {
    return const_iterator(nullptr, m_tail.load(std::memory_order_acquire));
  }
  
  const_iterator cend() const noexcept {
    return const_iterator(nullptr, m_tail.load(std::memory_order_acquire));
  }
  
  /* Print the list */
  void print() {
    auto current = m_head.load(std::memory_order_acquire);

    while (current != nullptr) {
      std::cout << static_cast<T*>(current)->m_value << " ";
      current = ((Node*)current)->m_next.load(std::memory_order_acquire);
    }

    std::cout << std::endl;
  }

  std::atomic<Node::Tag> m_head{};
  std::atomic<Node::Tag> m_tail{};

};

} // namespace ut
