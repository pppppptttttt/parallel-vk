#pragma once

#include <string>
#include <string_view>

class Keyspace {
public:
    static constexpr std::size_t kMinKeyLength = 1;
    static constexpr std::size_t kMaxKeyLength = 32;
    static constexpr std::size_t kAlphabetSize = 62;

    static std::string_view alphabet();

    static bool is_valid_key(const std::string &key);
    static bool
    is_valid_range(const std::string &left, const std::string &right);

    static int compare(const std::string &lhs, const std::string &rhs);

    static bool next_key(std::string &key);

private:
    static int char_to_index(char c);
    static char index_to_char(int index);
};
