#include <cassert>
#include <atomic>
#include <thread>

constexpr int kMagicValue = 30;
constexpr int kIters = 100'000;

namespace very_unsafe {

bool ready;
int value;

// uncomment for kaboom on any arch
void run() {
    // ready = false;
    // value = 0;
    //
    // std::thread t1([] { value = kMagicValue; ready = true; });
    // std::thread t2([] { while (!ready); assert(value == kMagicValue); });
    // t1.join();
    // t2.join();
}

}

namespace atomic {

std::atomic<bool> ready;
int value;

void run() {
    ready.store(false, std::memory_order_relaxed);
    value = 0;

    std::thread t1([] { value = kMagicValue; ready.store(true, std::memory_order_relaxed); });
    std::thread t2([] { while (!ready); assert(value == kMagicValue); });
    t1.join();
    t2.join();
}

}

int main(int argc, char **argv) {
    if (argc != 2) { return 1; }

    const enum Mode : std::uint8_t { kVeryUnsafe, Atomic }
        mode = std::atoi(argv[1]) == 0 ? kVeryUnsafe : Atomic;

    switch (mode) {
        case kVeryUnsafe:
            for (int i = 0; i < kIters; ++i) {
                very_unsafe::run();
            }
            break;

        case Atomic:
            for (int i = 0; i < kIters; ++i) {
                atomic::run();
            }
            break;

        default:
            assert(false);
    }
}