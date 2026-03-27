#pragma once

#include <mutex>
#include "iintset.h"

namespace hw4 {

class NaiveIntSet final : public IIntSet {
private:
    struct Node {
        long long key;
        Node *next;

        explicit Node(long long k, Node *n = nullptr) : key(k), next(n) {
        }
    };

    Node *head_;
    mutable std::mutex mtx_;

public:
    NaiveIntSet();
    ~NaiveIntSet() override;
    bool add(int key) override;
    bool remove(int key) override;
    bool contains(int key) const override;

private:
    void clear();
};


}