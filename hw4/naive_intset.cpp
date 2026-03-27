#include "naive_intset.h"

namespace hw4 {

NaiveIntSet::NaiveIntSet() {
    auto *tail = new Node(std::numeric_limits<long long>::max());
    head_ = new Node(std::numeric_limits<long long>::min(), tail);
}

NaiveIntSet::~NaiveIntSet() {
    clear();
}

bool NaiveIntSet::add(int key) {
    const long long k = key;
    std::lock_guard lock(mtx_);

    Node *pred = head_;
    Node *curr = head_->next;

    while (curr->key < k) {
        pred = curr;
        curr = curr->next;
    }

    if (curr->key == k) {
        return false;
    }

    pred->next = new Node(k, curr);
    return true;
}

bool NaiveIntSet::remove(int key) {
    const long long k = key;
    std::lock_guard lock(mtx_);

    Node *pred = head_;
    Node *curr = head_->next;

    while (curr->key < k) {
        pred = curr;
        curr = curr->next;
    }

    if (curr->key != k) {
        return false;
    }

    pred->next = curr->next;
    delete curr;
    return true;
}

bool NaiveIntSet::contains(int key) const {
    const long long k = key;
    std::lock_guard lock(mtx_);

    const Node *curr = head_->next;
    while (curr->key < k) {
        curr = curr->next;
    }
    return curr->key == k;
}

void NaiveIntSet::clear() {
    const Node *curr = head_;
    while (curr) {
        const Node *next = curr->next;
        delete curr;
        curr = next;
    }
    head_ = nullptr;
}

}  // namespace hw4
