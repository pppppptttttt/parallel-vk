#include <cassert>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <random>
#include <thread>

namespace {

std::mutex mtx;
std::condition_variable cv;

std::size_t total_threads = 0;
std::size_t arrived_threads = 0;

constexpr int kAtomsInMolecule = 3;
constexpr int kHSlots = 2;
constexpr int kOSlots = 1;

int h_count = 0;
int o_count = 0;

int molecules_count() {
    assert(0 <= h_count && h_count <= kHSlots && "invalid h_count!");
    assert(0 <= o_count && o_count <= kOSlots && "invalid o_count!");
    return h_count + o_count;
}

void reset_if_needed() {
    if (molecules_count() == kAtomsInMolecule) {
        h_count = 0;
        o_count = 0;
        std::cout << ' ';
    }
}

int total_h = 0;
int total_o = 0;

int remaining_h = 0;
int remaining_o = 0;

bool counting_done = false;

void handle_thread_arrived(std::unique_lock<std::mutex> &l) {
    if (arrived_threads == total_threads) {
        const int full_molecules =
            std::min(total_h / kHSlots, total_o / kOSlots);
        remaining_h = full_molecules * kHSlots;
        remaining_o = full_molecules * kOSlots;
        counting_done = true;
        cv.notify_all();
    } else {
        cv.wait(l, [] { return counting_done; });
    }
}

void hydrogen() {
    std::unique_lock lock(mtx);

    ++arrived_threads;
    ++total_h;

    handle_thread_arrived(lock);
    cv.wait(lock, [] { return remaining_h == 0 || h_count < kHSlots; });

    if (remaining_h == 0) {
        return;
    }

    std::cout << "H";
    h_count++;
    --remaining_h;

    reset_if_needed();
    cv.notify_all();
}

void oxygen() {
    std::unique_lock lock(mtx);

    ++arrived_threads;
    ++total_o;

    handle_thread_arrived(lock);
    cv.wait(lock, [] { return remaining_o == 0 || o_count < kOSlots; });

    if (remaining_o == 0) {
        return;
    }

    std::cout << "O";
    o_count++;
    --remaining_o;

    reset_if_needed();
    cv.notify_all();
}

}  // namespace

int main() {
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> thread_dist(1, 24);
    std::uniform_int_distribution<std::mt19937::result_type> coin(0, 1);

    total_threads = thread_dist(rng);
    std::cout << "Threads: " << total_threads << std::endl;
    std::vector<std::thread> threads(total_threads);

    std::cout << "Initial molecules: ";
    int hydrogen_molecule_count = 0;

    for (std::thread &t : threads) {
        constexpr char molecule_to_char[] = {'O', 'H'};
        const bool is_hydrogen = coin(rng);
        hydrogen_molecule_count += is_hydrogen;
        std::cout << molecule_to_char[is_hydrogen];
        t = std::thread(is_hydrogen ? hydrogen : oxygen);
    }
    std::cout << " (" << hydrogen_molecule_count << "H, "
              << total_threads - hydrogen_molecule_count << "O)" << std::endl;

    for (std::thread &t : threads) {
        t.join();
    }

    std::cout << std::endl;
}
