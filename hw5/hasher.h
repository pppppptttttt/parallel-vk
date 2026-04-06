#pragma once

#include <string>
#include <string_view>

class Hasher {
public:
    static bool is_valid_target_hash(std::string_view hash);
    static std::string normalize_hash(std::string_view hash);
    static std::string sha1_of_md5_hex(std::string_view key);
};
