#pragma once

#include <cstdint>
#include <string>
#include "task.h"

enum class ServerMessageType : std::uint8_t {
    kTask,
    kWait,
    kDone,
    kError,
    kUnknown
};

struct WaitMessage {
    unsigned int seconds = 0;
};

struct ServerMessage {
    ServerMessageType type = ServerMessageType::kUnknown;
    TaskMessage task;
    WaitMessage wait;
    std::string error_code;
    std::string error_description;
};

class Protocol {
public:
    static std::string make_hello(std::uint64_t hashrate);
    static std::string make_found(const std::string &key);
    static std::string make_not_found();
    static std::string
    make_error(const std::string &code, const std::string &description);

    static bool parse_server_message(
        const std::string &line,
        ServerMessage &out_message,
        std::string *error_description = nullptr
    );
};
