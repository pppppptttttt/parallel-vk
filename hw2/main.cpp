#include <chrono>
#include <iostream>
#include <mutex>
#include <random>
#include <semaphore>
#include <string>
#include <thread>
#include <vector>

namespace {

class BallRoom {
public:
    void leader(int id) {
        bool must_wait = false;
        bool matcher = false;

        {
            std::lock_guard lock(mtx_);
            if (followers_ > 0) {
                --followers_;
                matcher = true;
                follower_queue_.release();
                log("Leader " + std::to_string(id) + " found a follower");
            } else {
                ++leaders_;
                must_wait = true;
                log("Leader " + std::to_string(id) + " is waiting");
            }
        }

        if (must_wait) {
            leader_queue_.acquire();
            rendezvous_.release();
        } else if (matcher) {
            rendezvous_.acquire();
        }

        dance("Leader", id);
    }

    void follower(int id) {
        bool must_wait = false;
        bool matcher = false;

        {
            std::lock_guard lock(mtx_);
            if (leaders_ > 0) {
                --leaders_;
                matcher = true;
                leader_queue_.release();
                log("Follower " + std::to_string(id) + " found a leader");
            } else {
                ++followers_;
                must_wait = true;
                log("Follower " + std::to_string(id) + " is waiting");
            }
        }

        if (must_wait) {
            follower_queue_.acquire();
            rendezvous_.release();
        } else if (matcher) {
            rendezvous_.acquire();
        }

        dance("Follower", id);
    }

private:
    void dance(const std::string &role, int id) {
        {
            std::lock_guard lock(io_mutex_);
            std::cout << role << " " << id << " is dancing on thread "
                      << std::this_thread::get_id() << '\n';
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    void log(const std::string &msg) {
        std::lock_guard lock(io_mutex_);
        std::cout << msg << '\n';
    }

    std::mutex mtx_;
    std::mutex io_mutex_;

    int leaders_ = 0;
    int followers_ = 0;

    std::counting_semaphore<> leader_queue_{0};
    std::counting_semaphore<> follower_queue_{0};
    std::counting_semaphore<> rendezvous_{0};
};

}  // namespace

int main() {
    BallRoom ballroom;

    std::vector<std::jthread> threads;

    constexpr int kLeadersCount = 5;
    constexpr int kFollowersCount = 5;

    for (int i = 0; i < kLeadersCount; ++i) {
        threads.emplace_back([&ballroom, i] {
            std::this_thread::sleep_for(
                std::chrono::milliseconds(50 * (i % 3))
            );
            ballroom.leader(i);
        });
    }

    for (int i = 0; i < kFollowersCount; ++i) {
        threads.emplace_back([&ballroom, i] {
            std::this_thread::sleep_for(
                std::chrono::milliseconds(70 * (i % 3))
            );
            ballroom.follower(i);
        });
    }
}
