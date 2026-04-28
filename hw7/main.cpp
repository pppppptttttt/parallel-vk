#include <chrono>
#include <cstdint>
#include <functional>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace {

using Matrix = std::vector<std::vector<int>>;

int64_t sum_rows(const Matrix &m) {
    int64_t sum = 0;

    for (const auto &row : m) {
        for (const int value : row) {
            sum += value;
        }
    }

    return sum;
}

int64_t sum_cols(const Matrix &m) {
    if (m.empty()) {
        return 0;
    }

    int64_t sum = 0;
    const std::size_t rows = m.size();
    const std::size_t cols = m[0].size();

    for (std::size_t col = 0; col < cols; ++col) {
        for (std::size_t row = 0; row < rows; ++row) {
            sum += m[row][col];
        }
    }

    return sum;
}

std::vector<int64_t> sum_each_col(const Matrix &m) {
    if (m.empty()) {
        return {};
    }

    const std::size_t cols = m[0].size();
    std::vector<int64_t> col_sums(cols, 0);

    for (const auto &row : m) {
        for (std::size_t col = 0; col < cols; ++col) {
            col_sums[col] += row[col];
        }
    }

    return col_sums;
}

int64_t sum_cols_opt(const Matrix &m) {
    const auto col_sums = sum_each_col(m);

    int64_t total = 0;
    for (const int64_t value : col_sums) {
        total += value;
    }

    return total;
}

template <typename... Ts>
std::pair<std::size_t, int64_t> timed_invoke(Ts &&...invoke_args) {
    const auto start = std::chrono::steady_clock::now();
    const auto s = std::invoke(std::forward<Ts>(invoke_args)...);
    const auto end = std::chrono::steady_clock::now();

    return {
        std::chrono::duration_cast<std::chrono::nanoseconds>(end - start)
            .count(),
        s
    };
}

std::random_device random_device;
std::mt19937 generator(random_device());
std::uniform_int_distribution<int> distribution(0, 1000);

Matrix generate_matrix(int rows, int cols) {
    Matrix m(rows, std::vector<int>(cols, 0));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            m[i][j] = distribution(generator);
        }
    }
    return m;
}

int64_t run(const std::string &mode, const Matrix &m) {
    if (mode == "rows") {
        return sum_rows(m);
    }

    if (mode == "cols") {
        return sum_cols(m);
    }
    if (mode == "cols_opt") {
        return sum_cols_opt(m);
    }

    throw std::runtime_error("unknown mode: " + mode);
}

}  // namespace

int main(int argc, const char *argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0]
                  << " rows|cols|cols_opt MATRIX_SIZE REPEATS\n";
        return 1;
    }

    const std::string mode = argv[1];
    const std::size_t size = std::stoull(argv[2]);
    const int repeats = std::stoi(argv[3]);

    const Matrix m = generate_matrix(size, size);

    const auto [ns, checksum] =
        timed_invoke([&m, &mode] { return run(mode, m); });

    std::cout << "mode: " << mode << '\n';
    std::cout << "size: " << size << "x" << size << '\n';
    std::cout << "repeats: " << repeats << '\n';
    std::cout << "time_ns: " << ns << '\n';
    std::cout << "time_ms: " << ns / 1'000'000.0 << '\n';
    std::cout << "checksum: " << checksum << '\n';
}
