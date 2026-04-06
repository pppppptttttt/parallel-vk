#include "keyspace.h"
#include <algorithm>
#include <stdexcept>

namespace {

constexpr char kAlphabet[] =
    "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
}

std::string_view Keyspace::alphabet() {
    return kAlphabet;
}

int Keyspace::char_to_index(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'z') {
        return 10 + (c - 'a');
    }
    if (c >= 'A' && c <= 'Z') {
        return 36 + (c - 'A');
    }
    return -1;
}

char Keyspace::index_to_char(int index) {
    if (index < 0 || static_cast<std::size_t>(index) >= kAlphabetSize) {
        throw std::out_of_range("alphabet index out of range");
    }

    return kAlphabet[index];
}

bool Keyspace::is_valid_key(const std::string &key) {
    if (key.size() < kMinKeyLength || key.size() > kMaxKeyLength) {
        return false;
    }

    return std::ranges::all_of(key, [](char c) {
        return char_to_index(c) >= 0;
    });
}

int Keyspace::compare(const std::string &lhs, const std::string &rhs) {
    if (!is_valid_key(lhs) || !is_valid_key(rhs)) {
        throw std::invalid_argument("compare() received invalid key");
    }

    if (lhs.size() != rhs.size()) {
        return lhs.size() < rhs.size() ? -1 : 1;
    }

    for (std::size_t i = 0; i < lhs.size(); ++i) {
        const int lhs_index = char_to_index(lhs[i]);
        const int rhs_index = char_to_index(rhs[i]);

        if (lhs_index != rhs_index) {
            return lhs_index < rhs_index ? -1 : 1;
        }
    }

    return 0;
}

bool Keyspace::is_valid_range(
    const std::string &left,
    const std::string &right
) {
    if (!is_valid_key(left) || !is_valid_key(right)) {
        return false;
    }

    return compare(left, right) <= 0;
}

bool Keyspace::next_key(std::string &key) {
    if (!is_valid_key(key)) {
        return false;
    }

    for (std::size_t pos = key.size(); pos > 0; --pos) {
        const std::size_t index = pos - 1;
        const int current_index = char_to_index(key[index]);

        if (current_index + 1 < static_cast<int>(kAlphabetSize)) {
            key[index] = index_to_char(current_index + 1);
            return true;
        }

        key[index] = index_to_char(0);
    }

    if (key.size() == kMaxKeyLength) {
        return false;
    }

    key.assign(key.size() + 1, index_to_char(0));
    return true;
}
