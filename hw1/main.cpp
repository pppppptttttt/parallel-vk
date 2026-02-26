#include <immintrin.h>
#include <cassert>
#include <chrono>
#include <functional>
#include <iostream>
#include <random>
#include <string_view>
#include <utility>

namespace {

constexpr std::size_t kSize = 1'000'000;
float v1[kSize]{};
float v2[kSize]{};
float out_naive[kSize]{};
float out_sse[kSize]{};
float out_avx[kSize]{};

extern "C" void NaiveSum() {
    for (std::size_t i = 0; i < kSize; ++i) {
        out_naive[i] = v1[i] + v2[i];
    }
}

extern "C" void SseSum() {
    std::size_t i = 0;
    for (; i + 4 <= kSize; i += 4) {
        __m128 va = _mm_loadu_ps(v1 + i);
        __m128 vb = _mm_loadu_ps(v2 + i);
        __m128 vc = _mm_add_ps(va, vb);
        _mm_storeu_ps(out_sse + i, vc);
    }

    for (; i < kSize; ++i) {
        out_sse[i] = v1[i] + v2[i];
    }
}

extern "C" void AvxSum() {
    std::size_t i = 0;
    for (; i + 8 <= kSize; i += 8) {
        __m256 va = _mm256_loadu_ps(v1 + i);
        __m256 vb = _mm256_loadu_ps(v2 + i);
        __m256 vc = _mm256_add_ps(va, vb);
        _mm256_storeu_ps(out_avx + i, vc);
    }

    for (; i < kSize; ++i) {
        out_avx[i] = v1[i] + v2[i];
    }
}

void FillRandom(auto &arr) {
    static std::default_random_engine gen;
    static std::uniform_real_distribution dist(0.0f, 1.0f);

    for (auto &el : arr) {
        el = dist(gen);
    }
}

void FillAll() {
    FillRandom(v1);
    FillRandom(v2);
}

void AssertSame() {
    for (std::size_t i = 0; i < kSize; ++i) {
        assert(std::abs(out_naive[i] - out_sse[i]) < 1e-6);
        assert(std::abs(out_naive[i] - out_avx[i]) < 1e-6);
    }
}

template <typename... Ts>
std::size_t TimedInvoke(Ts &&...invoke_args) {
    const auto start = std::chrono::steady_clock::now();
    std::invoke(std::forward<Ts>(invoke_args)...);
    const auto end = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start)
        .count();
}

template <typename S>
void RunSingle(S &&sum_functor, std::string_view description) {
    const std::size_t elapsed = TimedInvoke(std::forward<S>(sum_functor));
    std::cout << description << ":\t" << elapsed << "ns\n";
}

void RunAll() {
    FillAll();
    RunSingle(NaiveSum, "Naive");
    RunSingle(SseSum, "SSE");
    RunSingle(AvxSum, "AVX");
    AssertSame();
}

}  // namespace

int main() {
    RunAll();
}
