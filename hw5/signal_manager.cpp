#include "signal_manager.h"
#include <csignal>
#include <stdexcept>

namespace {

volatile std::sig_atomic_t stop_requested_ = 0;
volatile std::sig_atomic_t last_signal_ = 0;

extern "C" void handle_termination_signal(int signum) {
    last_signal_ = signum;
    stop_requested_ = 1;
}

void install_one_handler(int signum) {
    struct sigaction action {};

    action.sa_handler = handle_termination_signal;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    if (sigaction(signum, &action, nullptr) != 0) {
        throw std::runtime_error("failed to install signal handler");
    }
}

}  // namespace

void SignalManager::install_handlers() {
    install_one_handler(SIGINT);
    install_one_handler(SIGTERM);
    install_one_handler(SIGHUP);
    install_one_handler(SIGQUIT);
}

bool SignalManager::stop_requested() {
    return stop_requested_ != 0;
}

int SignalManager::last_signal() {
    return last_signal_;
}
