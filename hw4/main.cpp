#include <barrier>
#include <iostream>
#include <random>
#include <string>
#include <thread>
#include "fine_intset.h"
#include "lazy_intset.h"
#include "naive_intset.h"
#include "optimistic_intset.h"

namespace hw4 {

struct BenchResult {
    std::string name;
    double seconds = 0.0;
    std::uint64_t operations = 0;
    double ops_per_sec = 0.0;
};

// 20% writes / 80% reads
// writes = 10% add + 10% remove
template <class SetFactory>
BenchResult run_benchmark(
    const std::string &name,
    SetFactory &&make_set,
    int threads,
    int seconds,
    int key_range = 10000,
    int prefill = 5000
) {
    auto set = make_set();

    for (int i = 0; i < prefill; ++i) {
        set->add(i * 2);
    }

    std::barrier sync_point(threads + 1);
    std::atomic<bool> stop = false;
    std::vector<std::thread> workers;
    std::vector<std::uint64_t> counters(threads, 0);

    for (int tid = 0; tid < threads; ++tid) {
        workers.emplace_back([&, tid] {
            std::mt19937_64 rng(
                std::chrono::system_clock::now().time_since_epoch().count()
            );
            std::uniform_int_distribution op_dist(0, 99);
            std::uniform_int_distribution key_dist(0, key_range - 1);

            sync_point.arrive_and_wait();

            std::uint64_t local_ops = 0;
            while (!stop.load(std::memory_order_relaxed)) {
                const int key = key_dist(rng);

                if (const int op = op_dist(rng); op < 10) {
                    set->add(key);  // 10%
                } else if (op < 20) {
                    set->remove(key);  // 10%
                } else {
                    set->contains(key);  // 80%
                }

                ++local_ops;
            }

            counters[tid] = local_ops;
        });
    }

    sync_point.arrive_and_wait();
    const auto t0 = std::chrono::steady_clock::now();
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
    stop.store(true, std::memory_order_relaxed);

    for (auto &th : workers) {
        th.join();
    }

    const auto t1 = std::chrono::steady_clock::now();
    const double elapsed = std::chrono::duration<double>(t1 - t0).count();

    std::uint64_t total_ops = 0;
    for (auto x : counters) {
        total_ops += x;
    }

    return {name, elapsed, total_ops, static_cast<double>(total_ops) / elapsed};
}

}  // namespace hw4

int main() {
    using namespace hw4;

    const auto threads =
        static_cast<int>(std::max(2u, std::thread::hardware_concurrency()));
    constexpr int kSeconds = 3;

    std::vector<BenchResult> results;
    results.push_back(run_benchmark(
        "Naive     ", [] { return std::make_unique<NaiveIntSet>(); }, threads,
        kSeconds
    ));
    results.push_back(run_benchmark(
        "Fine      ", [] { return std::make_unique<FineIntSet>(); }, threads, kSeconds
    ));
    results.push_back(run_benchmark(
        "Optimistic", [] { return std::make_unique<OptimisticIntSet>(); },
        threads, kSeconds
    ));
    results.push_back(run_benchmark(
        "Lazy      ", [] { return std::make_unique<LazyIntSet>(); }, threads, kSeconds
    ));

    std::cout << "Threads: " << threads << "\n";
    std::cout << "10% add, 10% remove, 80% contains\n\n";

    for (const auto &[name, seconds, operations, ops_per_sec] : results) {
        std::cout << name << ": ops=" << operations << ", seconds=" << seconds
                  << ", ops/sec=" << static_cast<std::uint64_t>(ops_per_sec)
                  << '\n';
    }

    return 0;
}
