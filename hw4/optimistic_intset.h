#pragma once

#include <atomic>
#include <mutex>
#include <vector>
#include "iintset.h"

namespace hw4 {

class OptimisticIntSet final : public IIntSet {
private:
    struct Node {
        long long key;
        std::atomic<Node *> next;
        mutable std::mutex mtx;

        explicit Node(long long k, Node *n = nullptr) : key(k), next(n) {
        }
    };

    Node *head_;

    mutable std::mutex retired_mtx_;
    std::vector<Node *> retired_;

public:
    OptimisticIntSet();
    ~OptimisticIntSet() override;
    bool add(int key) override;
    bool remove(int key) override;
    bool contains(int key) const override;

private:
    bool validate(Node *pred, Node *curr) const;
    void retire(Node *node);
    void clear_reachable();
    void clear_retired();
};


}  // namespace hw4

