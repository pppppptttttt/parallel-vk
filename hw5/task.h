#pragma once

#include <string>

struct TaskMessage {
    std::string target_hash;
    std::string left;
    std::string right;
};

enum class BruteForceStatus : std::uint8_t { kFound, kNotFound, kAborted };

struct BruteForceResult {
    BruteForceStatus status = BruteForceStatus::kNotFound;
    std::string key;
};
