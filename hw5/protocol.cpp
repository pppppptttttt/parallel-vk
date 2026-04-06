#include "protocol.h"
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

namespace {

std::vector<std::string> split_ws(const std::string &line) {
    std::istringstream input(line);
    std::vector<std::string> parts;
    std::string part;

    while (input >> part) {
        parts.push_back(part);
    }

    return parts;
}

bool parse_positive_uint(const std::string &text, unsigned int &value) {
    if (text.empty()) {
        return false;
    }

    for (const char c : text) {
        if (std::isdigit(static_cast<unsigned char>(c)) == 0) {
            return false;
        }
    }

    try {
        const unsigned long parsed = std::stoul(text);
        if (parsed == 0) {
            return false;
        }

        value = static_cast<unsigned int>(parsed);
        return true;
    } catch (...) {
        return false;
    }
}

void set_error(std::string *error_description, const std::string &text) {
    if (error_description != nullptr) {
        *error_description = text;
    }
}

}  // namespace

std::string Protocol::make_hello(std::uint64_t hashrate) {
    return "HELLO " + std::to_string(hashrate);
}

std::string Protocol::make_found(const std::string &key) {
    return "FOUND " + key;
}

std::string Protocol::make_not_found() {
    return "NOT_FOUND";
}

std::string
Protocol::make_error(const std::string &code, const std::string &description) {
    if (description.empty()) {
        return "ERROR " + code;
    }

    return "ERROR " + code + " " + description;
}

bool Protocol::parse_server_message(
    const std::string &line,
    ServerMessage &out_message,
    std::string *error_description
) {
    out_message = ServerMessage{};
    set_error(error_description, "");

    const auto parts = split_ws(line);
    if (parts.empty()) {
        set_error(error_description, "empty_message");
        return false;
    }

    const std::string &cmd = parts[0];

    if (cmd == "TASK") {
        if (parts.size() != 4) {
            set_error(error_description, "TASK expects 3 arguments");
            return false;
        }

        out_message.type = ServerMessageType::kTask;
        out_message.task.target_hash = parts[1];
        out_message.task.left = parts[2];
        out_message.task.right = parts[3];
        return true;
    }

    if (cmd == "WAIT") {
        if (parts.size() != 2) {
            set_error(error_description, "WAIT expects 1 argument");
            return false;
        }

        unsigned int seconds = 0;
        if (!parse_positive_uint(parts[1], seconds)) {
            set_error(
                error_description, "WAIT seconds must be positive integer"
            );
            return false;
        }

        out_message.type = ServerMessageType::kWait;
        out_message.wait.seconds = seconds;
        return true;
    }

    if (cmd == "DONE") {
        if (parts.size() != 1) {
            set_error(error_description, "DONE expects no arguments");
            return false;
        }

        out_message.type = ServerMessageType::kDone;
        return true;
    }

    if (cmd == "ERROR") {
        if (parts.size() < 2) {
            set_error(error_description, "ERROR expects at least 1 argument");
            return false;
        }

        out_message.type = ServerMessageType::kError;
        out_message.error_code = parts[1];

        if (parts.size() >= 3) {
            std::ostringstream description;
            for (std::size_t i = 2; i < parts.size(); ++i) {
                if (i > 2) {
                    description << ' ';
                }
                description << parts[i];
            }
            out_message.error_description = description.str();
        }

        return true;
    }

    out_message.type = ServerMessageType::kUnknown;
    set_error(error_description, "unknown_command");
    return false;
}
