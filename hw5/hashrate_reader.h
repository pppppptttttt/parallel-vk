#pragma once

#include <cstdint>
#include <string>

class HashrateReader {
public:
    static std::uint64_t read_from_file(const std::string &path);
};
