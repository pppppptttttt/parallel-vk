#include <limits>
#include "lazy_intset.h"

namespace hw4 {

LazyIntSet::LazyIntSet() {
    auto *tail = new Node(std::numeric_limits<long long>::max());
    head_ = new Node(std::numeric_limits<long long>::min(), tail);
}

LazyIntSet::~LazyIntSet() {
    clear_reachable();
    clear_retired();
}

bool LazyIntSet::add(int key) {
    const long long k = key;

    while (true) {
        Node *pred = head_;
        Node *curr = pred->next.load(std::memory_order_acquire);

        while (curr->key < k) {
            pred = curr;
            curr = curr->next.load(std::memory_order_acquire);
        }

        std::unique_lock lock_pred(pred->mtx);
        std::unique_lock lock_curr(curr->mtx);

        if (!validate(pred, curr)) {
            continue;
        }

        if (curr->key == k) {
            return false;
        }

        auto *node = new Node(k, curr);
        pred->next.store(node, std::memory_order_release);
        return true;
    }
}

bool LazyIntSet::remove(int key) {
    const long long k = key;

    while (true) {
        Node *pred = head_;
        Node *curr = pred->next.load(std::memory_order_acquire);

        while (curr->key < k) {
            pred = curr;
            curr = curr->next.load(std::memory_order_acquire);
        }

        std::unique_lock lock_pred(pred->mtx);
        std::unique_lock lock_curr(curr->mtx);

        if (!validate(pred, curr)) {
            continue;
        }

        if (curr->key != k) {
            return false;
        }

        curr->marked.store(true, std::memory_order_release);

        Node *succ = curr->next.load(std::memory_order_acquire);
        pred->next.store(succ, std::memory_order_release);

        retire(curr);
        return true;
    }
}

bool LazyIntSet::contains(int key) const {
    const long long k = key;

    const Node *curr = head_;
    while (curr->key < k) {
        curr = curr->next.load(std::memory_order_acquire);
    }

    return (curr->key == k) && !curr->marked.load(std::memory_order_acquire);
}

bool LazyIntSet::validate(const Node *pred, const Node *curr) {
    return !pred->marked.load(std::memory_order_acquire) &&
           !curr->marked.load(std::memory_order_acquire) &&
           pred->next.load(std::memory_order_acquire) == curr;
}

void LazyIntSet::retire(Node *node) {
    std::lock_guard lock(retired_mtx_);
    retired_.push_back(node);
}

void LazyIntSet::clear_reachable() {
    const Node *curr = head_;
    while (curr) {
        const Node *next = curr->next.load(std::memory_order_relaxed);
        delete curr;
        curr = next;
    }
    head_ = nullptr;
}

void LazyIntSet::clear_retired() {
    for (const Node *node : retired_) {
        delete node;
    }
    retired_.clear();
}

}  // namespace hw4