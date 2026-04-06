#include "bruteforce_engine.h"
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>
#include "hasher.h"
#include "keyspace.h"

BruteForceEngine::BruteForceEngine(
    StopPredicate external_stop,
    std::size_t worker_count
)
    : external_stop_(std::move(external_stop)), worker_count_(worker_count) {
}

void BruteForceEngine::request_stop() {
    stop_requested_.store(true, std::memory_order_relaxed);
}

bool BruteForceEngine::stop_requested() const {
    if (stop_requested_.load(std::memory_order_relaxed)) {
        return true;
    }

    if (external_stop_) {
        return external_stop_();
    }

    return false;
}

std::size_t BruteForceEngine::effective_worker_count() const {
    if (worker_count_ > 0) {
        return worker_count_;
    }

    const unsigned int hardware_threads = std::thread::hardware_concurrency();
    if (hardware_threads == 0U) {
        return kDefaultWorkerCount;
    }

    return hardware_threads;
}

bool BruteForceEngine::acquire_next_chunk(
    Chunk &out_chunk,
    const std::string &global_right
) {
    std::lock_guard lock(chunk_mutex_);

    if (!has_more_chunks_) {
        return false;
    }

    out_chunk.start = next_chunk_start_;
    out_chunk.end = next_chunk_start_;

    std::string current = out_chunk.start;

    for (std::size_t i = 1; i < kDefaultChunkSize; ++i) {
        if (current == global_right) {
            break;
        }

        std::string next = current;
        if (!Keyspace::next_key(next)) {
            break;
        }

        if (Keyspace::compare(next, global_right) > 0) {
            break;
        }

        current = std::move(next);
    }

    out_chunk.end = current;

    if (current == global_right) {
        has_more_chunks_ = false;
        next_chunk_start_.clear();
        return true;
    }

    std::string next_start = current;
    if (!Keyspace::next_key(next_start)) {
        has_more_chunks_ = false;
        next_chunk_start_.clear();
        return true;
    }

    if (Keyspace::compare(next_start, global_right) > 0) {
        has_more_chunks_ = false;
        next_chunk_start_.clear();
        return true;
    }

    next_chunk_start_ = std::move(next_start);
    return true;
}

void BruteForceEngine::worker_loop(
    const std::string &target_hash,
    const std::string &global_right
) {
    try {
        while (!stop_requested()) {
            Chunk chunk;
            if (!acquire_next_chunk(chunk, global_right)) {
                return;
            }

            std::string current = chunk.start;

            while (true) {
                if (stop_requested()) {
                    return;
                }

                const std::string candidate_hash =
                    Hasher::sha1_of_md5_hex(current);
                if (candidate_hash == target_hash) {
                    if (bool expected = false;
                        solution_found_.compare_exchange_strong(
                            expected, true, std::memory_order_relaxed
                        )) {
                        std::lock_guard lock(result_mutex_);
                        result_.status = BruteForceStatus::kFound;
                        result_.key = current;
                    }

                    stop_requested_.store(true, std::memory_order_relaxed);
                    return;
                }

                if (current == chunk.end) {
                    break;
                }

                if (!Keyspace::next_key(current)) {
                    return;
                }
            }
        }
    } catch (...) {
        {
            std::lock_guard lock(exception_mutex_);
            if (!worker_exception_) {
                worker_exception_ = std::current_exception();
            }
        }

        stop_requested_.store(true, std::memory_order_relaxed);
    }
}

BruteForceResult BruteForceEngine::run(const TaskMessage &task) {
    if (!Hasher::is_valid_target_hash(task.target_hash)) {
        throw std::invalid_argument("invalid target hash");
    }

    if (!Keyspace::is_valid_range(task.left, task.right)) {
        throw std::invalid_argument("invalid key range");
    }

    stop_requested_.store(false, std::memory_order_relaxed);
    solution_found_.store(false, std::memory_order_relaxed);

    {
        std::lock_guard lock(chunk_mutex_);
        next_chunk_start_ = task.left;
        has_more_chunks_ = true;
    }

    {
        std::lock_guard lock(result_mutex_);
        result_ = {BruteForceStatus::kNotFound, ""};
    }

    {
        std::lock_guard lock(exception_mutex_);
        worker_exception_ = nullptr;
    }

    const std::string target_hash = Hasher::normalize_hash(task.target_hash);
    const std::size_t worker_count = effective_worker_count();

    std::vector<std::thread> workers;
    workers.reserve(worker_count);

    for (std::size_t i = 0; i < worker_count; ++i) {
        workers.emplace_back([this, &target_hash, &task]() {
            worker_loop(target_hash, task.right);
        });
    }

    for (std::thread &worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }

    {
        std::lock_guard lock(exception_mutex_);
        if (worker_exception_) {
            std::rethrow_exception(worker_exception_);
        }
    }

    if (solution_found_.load(std::memory_order_relaxed)) {
        std::lock_guard lock(result_mutex_);
        return result_;
    }

    if (stop_requested()) {
        return {BruteForceStatus::kAborted, ""};
    }

    return {BruteForceStatus::kNotFound, ""};
}
