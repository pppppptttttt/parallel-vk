#pragma once

#include <atomic>
#include <mutex>
#include <vector>
#include "iintset.h"

namespace hw4 {

class LazyIntSet final : public IIntSet {
private:
    struct Node {
        long long key;
        std::atomic<Node *> next;
        std::atomic<bool> marked;
        mutable std::mutex mtx;

        explicit Node(long long k, Node *n = nullptr)
            : key(k), next(n), marked(false) {
        }
    };

    Node *head_;

    mutable std::mutex retired_mtx_;
    std::vector<Node *> retired_;

public:
    LazyIntSet();
    ~LazyIntSet() override;
    bool add(int key) override;
    bool remove(int key) override;
    bool contains(int key) const override;

private:
    static bool validate(const Node *pred, const Node *curr);
    void retire(Node *node);
    void clear_reachable();
    void clear_retired();
};

}  // namespace hw4
