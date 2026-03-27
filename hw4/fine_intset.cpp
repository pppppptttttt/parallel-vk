#include "fine_intset.h"

namespace hw4 {

FineIntSet::FineIntSet() {
    auto *tail = new Node(std::numeric_limits<long long>::max());
    head_ = new Node(std::numeric_limits<long long>::min(), tail);
}

FineIntSet::~FineIntSet() {
    clear();
}

bool FineIntSet::add(int key) {
    const long long k = key;

    Node *pred = head_;
    pred->mtx.lock();

    Node *curr = pred->next;
    curr->mtx.lock();

    while (curr->key < k) {
        pred->mtx.unlock();
        pred = curr;
        curr = curr->next;
        curr->mtx.lock();
    }

    if (curr->key == k) {
        curr->mtx.unlock();
        pred->mtx.unlock();
        return false;
    }

    pred->next = new Node(k, curr);

    curr->mtx.unlock();
    pred->mtx.unlock();
    return true;
}

bool FineIntSet::remove(int key) {
    const long long k = key;

    Node *pred = head_;
    pred->mtx.lock();

    Node *curr = pred->next;
    curr->mtx.lock();

    while (curr->key < k) {
        pred->mtx.unlock();
        pred = curr;
        curr = curr->next;
        curr->mtx.lock();
    }

    if (curr->key != k) {
        curr->mtx.unlock();
        pred->mtx.unlock();
        return false;
    }

    Node *succ = curr->next;
    succ->mtx.lock();

    pred->next = succ;

    succ->mtx.unlock();
    curr->mtx.unlock();
    pred->mtx.unlock();

    delete curr;
    return true;
}

bool FineIntSet::contains(int key) const {
    const long long k = key;

    const Node *pred = head_;
    pred->mtx.lock();

    const Node *curr = pred->next;
    curr->mtx.lock();

    while (curr->key < k) {
        pred->mtx.unlock();
        pred = curr;
        curr = curr->next;
        curr->mtx.lock();
    }

    const bool result = (curr->key == k);

    curr->mtx.unlock();
    pred->mtx.unlock();
    return result;
}

void FineIntSet::clear() {
    const Node *curr = head_;
    while (curr) {
        const Node *next = curr->next;
        delete curr;
        curr = next;
    }
    head_ = nullptr;
}

}  // namespace hw4