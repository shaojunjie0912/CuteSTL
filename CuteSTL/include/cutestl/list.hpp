#pragma once

#include "allocator.hpp"

template <typename T>
struct ListNode {
    ListNode<T>* prev_;
    ListNode<T>* next_;
    T data_;
};

template <typename T>
class ListIterator {
public:
    using value_type = T;
    using pointer = T*;
    using reference = T&;
    // using const_reference = T const&;
    using iterator_category = std::bidirectional_iterator_tag;

    using Node = ListNode<T>;
    using iterator = ListIterator<T>;

private:
    Node* node_;

public:
    ListIterator() : node_(nullptr) {}
    ListIterator(Node* const node) : node_(node) {}

    ListIterator(iterator const& other) : node_(other.node_) {}

    reference operator*() const { return node_->data_; }
    pointer operator->() const { return node_; }

    bool operator==(iterator const& other) const { return node_ == other.node_; }
    bool operator!=(iterator const& other) const { return !(*this == other); }

    iterator& operator++() {
        node_ = node_->next_;
        return *this;
    }

    iterator operator++(int) {
        Node* tmp{node_};
        node_ = node_->next_;
        return tmp;
    }

    iterator& operator--() {
        node_ = node_->prev_;
        return *this;
    }

    iterator operator--(int) {
        Node* tmp{node_};
        node_ = node_->prev_;
        return tmp;
    }
};

template <typename T, typename Alloc = Allocator<ListNode<T>>>
class List {
public:
    using value_type = T;
    using pointer = T*;
    using reference = T&;

    using Node = ListNode<T>;
    using iterator = ListIterator<T>;

private:
    Node* dummy_;  // 哨兵
};
