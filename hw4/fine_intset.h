#pragma once

#include <mutex>
#include "iintset.h"

namespace hw4 {

class FineIntSet final : public IIntSet {
private:
    struct Node {
        long long key;
        Node *next;
        mutable std::mutex mtx;

        explicit Node(long long k, Node *n = nullptr) : key(k), next(n) {
        }
    };

    Node *head_;

public:
    FineIntSet();
    ~FineIntSet() override;
    bool add(int key) override;
    bool remove(int key) override;
    bool contains(int key) const override;

private:
    void clear();
};

}  // namespace hw4
