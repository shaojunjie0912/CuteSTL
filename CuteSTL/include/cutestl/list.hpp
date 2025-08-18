#pragma once

#include <iostream>

#include "allocator.hpp"

namespace cutestl {
template <typename T>
struct _ListNode {
    _ListNode<T>* prev_;
    _ListNode<T>* next_;
    T data_;
};

template <typename T>
class _ListIterator {
public:
    using value_type = T;
    using pointer = value_type*;
    using reference = value_type&;
    using iterator_category = std::bidirectional_iterator_tag;  // 双向迭代器 (不能随机访问)
    using Node = _ListNode<T>;
    using iterator = _ListIterator<T>;

public:
    Node* node_;  // NOTE: 关键: 指向 ListNode 的指针

public:
    // 构造函数
    _ListIterator() : node_(nullptr) {}
    _ListIterator(Node* const node) : node_(node) {}
    _ListIterator(iterator const& other) : node_(other.node_) {}

    _ListIterator& operator=(_ListIterator const& other) {
        if (this != &other) {
            node_ = other.node_;
        }
        return *this;
    }

    // 解引用运算符 返回当前节点的数据
    reference operator*() const { return node_->data_; }
    // 获取指针运算符 返回当前节点的指针
    Node* operator->() const { return node_; }

    // 相等运算符 比较两个迭代器是否相等
    bool operator==(iterator const& other) const { return node_ == other.node_; }
    bool operator!=(iterator const& other) const { return !(*this == other); }

    // ++前置
    iterator& operator++() {
        node_ = node_->next_;
        return *this;
    }

    // 后置++
    iterator operator++(int) {
        Node* tmp{node_};
        node_ = node_->next_;
        return tmp;  // iterator 构造函数(Node*)
    }

    // --前置
    iterator& operator--() {
        node_ = node_->prev_;
        return *this;
    }

    // 后置--
    iterator operator--(int) {
        Node* tmp{node_};
        node_ = node_->prev_;
        return tmp;  // iterator 构造函数(Node*)
    }
};

template <typename T, typename Alloc = Allocator<_ListNode<T>>>
class List {
public:
    using value_type = T;
    using pointer = value_type*;
    using reference = value_type&;
    using size_type = std::size_t;

    using Node = _ListNode<value_type>;
    using iterator = _ListIterator<value_type>;

private:
    Node* dummy_;  // 哨兵节点指针

private:
    static Node* AllocateNode() { return Alloc::Allocate(1); }
    static void DeallocateNode(Node* p) { Alloc::Deallocate(p); }
    static Node* CreateNode(value_type const& value) {
        Node* node{AllocateNode()};
        new (&node->data_) value_type(value);  // NOTE: placement new, 在 &node->data 内存上构造
        return node;
    }

public:
    // 默认构造函数: dummy 的 next prev 指向自己
    List() : dummy_(AllocateNode()) {
        dummy_->prev_ = dummy_;
        dummy_->next_ = dummy_;
    }

public:
    iterator Insert(iterator pos, value_type const& value) {
        Node* new_node{CreateNode(value)};
        new_node->prev_ = pos->prev_;
        new_node->next_ = pos.node_;
        pos->prev_->next_ = new_node;
        pos->prev_ = new_node;
        return new_node;
    }

    iterator Insert(iterator pos, size_type count, value_type const& value) {
        iterator new_pos{pos};
        while (count--) {
            new_pos = Insert(new_pos, value);
        }
        return new_pos;
    }
    // TODO:
    template <typename InputIt>
    iterator Insert(iterator pos, InputIt first, InputIt last) {}

    void Show() const {
        for (iterator it = Begin(); it != End(); ++it) {
            std::cout << *it << ' ';
        }
        std::cout << '\n';
    }

    // NOTE: begin(): dummy 下一个
    iterator Begin() {
        return dummy_->next_;  // iterator 构造函数(Node*)
    }
    iterator Begin() const {
        return dummy_->next_;  // iterator 构造函数(Node*)
    }

    // NOTE: end() 指向尾迭代器的后一个位置!!!
    iterator End() {
        return dummy_;  // iterator 构造函数(Node*)
    }
    iterator End() const {
        return dummy_;  // iterator 构造函数(Node*)
    }
};
}  // namespace cutestl