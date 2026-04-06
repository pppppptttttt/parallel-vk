#include "hashrate_reader.h"
#include <cctype>
#include <fstream>
#include <stdexcept>
#include <string>

std::uint64_t HashrateReader::read_from_file(const std::string &path) {
    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error("failed to open hashrate file: " + path);
    }

    std::string text;
    std::getline(input, text);

    if (text.empty()) {
        throw std::runtime_error("hashrate file is empty");
    }

    std::size_t pos = 0;
    const unsigned long long value = std::stoull(text, &pos, 10);

    while (pos < text.size() &&
           std::isspace(static_cast<unsigned char>(text[pos])) != 0) {
        ++pos;
    }

    if (pos != text.size()) {
        throw std::runtime_error("hashrate file contains invalid characters");
    }

    if (value == 0ULL) {
        throw std::runtime_error("hashrate must be positive");
    }

    return value;
}
