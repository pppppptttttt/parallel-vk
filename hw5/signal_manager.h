#pragma once

class SignalManager {
public:
    static void install_handlers();
    static bool stop_requested();
    static int last_signal();
};
