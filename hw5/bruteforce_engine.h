#pragma once

#include <atomic>
#include <cstddef>
#include <exception>
#include <functional>
#include <mutex>
#include <string>
#include "task.h"

class BruteForceEngine {
public:
    using StopPredicate = std::function<bool()>;

    explicit BruteForceEngine(
        StopPredicate external_stop = {},
        std::size_t worker_count = 0
    );

    BruteForceResult run(const TaskMessage &task);
    void request_stop();

private:
    struct Chunk {
        std::string start;
        std::string end;
    };

    static constexpr std::size_t kDefaultChunkSize = 4096;
    static constexpr std::size_t kDefaultWorkerCount = 4;

    bool stop_requested() const;
    std::size_t effective_worker_count() const;

    bool acquire_next_chunk(Chunk &out_chunk, const std::string &global_right);
    void worker_loop(
        const std::string &target_hash,
        const std::string &global_right
    );

    std::atomic<bool> stop_requested_ = false;
    std::atomic<bool> solution_found_ = false;

    StopPredicate external_stop_;
    std::size_t worker_count_ = 0;

    std::mutex chunk_mutex_;
    std::string next_chunk_start_;
    bool has_more_chunks_ = false;

    std::mutex result_mutex_;
    BruteForceResult result_{};

    std::mutex exception_mutex_;
    std::exception_ptr worker_exception_{};
};
