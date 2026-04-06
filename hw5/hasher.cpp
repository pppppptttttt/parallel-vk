#include "hasher.h"
#include <openssl/evp.h>
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

bool is_hex_char(char c) {
    return std::isxdigit(static_cast<unsigned char>(c)) != 0;
}

char nibble_to_hex(unsigned int value) {
    if (value < 10U) {
        return static_cast<char>('0' + value);
    }
    return static_cast<char>('a' + (value - 10U));
}

std::string to_hex(const std::vector<unsigned char> &bytes) {
    std::string result;
    result.reserve(bytes.size() * 2);

    for (const unsigned char byte : bytes) {
        result.push_back(nibble_to_hex((byte >> 4U) & 0x0FU));
        result.push_back(nibble_to_hex(byte & 0x0FU));
    }

    return result;
}

std::vector<unsigned char>
digest_bytes(const unsigned char *data, std::size_t size, const EVP_MD *md) {
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (ctx == nullptr) {
        throw std::runtime_error("EVP_MD_CTX_new failed");
    }

    const int md_size = EVP_MD_size(md);
    if (md_size <= 0) {
        EVP_MD_CTX_free(ctx);
        throw std::runtime_error("EVP_MD_size failed");
    }

    std::vector<unsigned char> result(static_cast<std::size_t>(md_size));
    unsigned int out_size = 0;

    const int init_ok = EVP_DigestInit_ex(ctx, md, nullptr);
    const int update_ok = EVP_DigestUpdate(ctx, data, size);
    const int final_ok = EVP_DigestFinal_ex(ctx, result.data(), &out_size);

    EVP_MD_CTX_free(ctx);

    if (init_ok != 1 || update_ok != 1 || final_ok != 1) {
        throw std::runtime_error("OpenSSL digest operation failed");
    }

    result.resize(out_size);
    return result;
}

}  // namespace

bool Hasher::is_valid_target_hash(std::string_view hash) {
    if (hash.size() != 40) {
        return false;
    }

    return std::ranges::all_of(hash, is_hex_char);
}

std::string Hasher::normalize_hash(std::string_view hash) {
    std::string result;
    result.reserve(hash.size());

    for (const char c : hash) {
        result.push_back(
            static_cast<char>(std::tolower(static_cast<unsigned char>(c)))
        );
    }

    return result;
}

std::string Hasher::sha1_of_md5_hex(std::string_view key) {
    const auto md5 = digest_bytes(
        reinterpret_cast<const unsigned char *>(key.data()), key.size(),
        EVP_md5()
    );

    const auto sha1 = digest_bytes(md5.data(), md5.size(), EVP_sha1());

    return to_hex(sha1);
}
