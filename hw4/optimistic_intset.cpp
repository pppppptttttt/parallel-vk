#include "optimistic_intset.h"
#include <limits>

namespace hw4 {

OptimisticIntSet::OptimisticIntSet() {
    auto *tail = new Node(std::numeric_limits<long long>::max());
    head_ = new Node(std::numeric_limits<long long>::min(), tail);
}

OptimisticIntSet::~OptimisticIntSet() {
    clear_reachable();
    clear_retired();
}

bool OptimisticIntSet::add(int key) {
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

bool OptimisticIntSet::remove(int key) {
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

        Node *succ = curr->next.load(std::memory_order_acquire);
        pred->next.store(succ, std::memory_order_release);
        retire(curr);
        return true;
    }
}

bool OptimisticIntSet::contains(int key) const {
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

        return curr->key == k;
    }
}

bool OptimisticIntSet::validate(Node *pred, Node *curr) const {
    const Node *node = head_;
    while (node->key <= pred->key) {
        if (node == pred) {
            return pred->next.load(std::memory_order_acquire) == curr;
        }
        node = node->next.load(std::memory_order_acquire);
    }
    return false;
}

void OptimisticIntSet::retire(Node *node) {
    std::lock_guard lock(retired_mtx_);
    retired_.push_back(node);
}

void OptimisticIntSet::clear_reachable() {
    const Node *curr = head_;
    while (curr) {
        const Node *next = curr->next.load(std::memory_order_relaxed);
        delete curr;
        curr = next;
    }
    head_ = nullptr;
}

void OptimisticIntSet::clear_retired() {
    for (const Node *node : retired_) {
        delete node;
    }
    retired_.clear();
}

}  // namespace hw4