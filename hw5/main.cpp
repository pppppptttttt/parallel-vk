#include <chrono>
#include <cstdint>
#include <exception>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <thread>
#include "bruteforce_engine.h"
#include "hasher.h"
#include "hashrate_reader.h"
#include "keyspace.h"
#include "protocol.h"
#include "signal_manager.h"
#include "tcp_client.h"

namespace {

std::uint16_t parse_port(const std::string &text) {
    std::size_t pos = 0;
    const unsigned long value = std::stoul(text, &pos, 10);

    if (pos != text.size()) {
        throw std::invalid_argument("port must be a decimal number");
    }

    if (value == 0 || value > std::numeric_limits<std::uint16_t>::max()) {
        throw std::invalid_argument("port must be in range [1, 65535]");
    }

    return static_cast<std::uint16_t>(value);
}

bool sleep_interruptible(unsigned int seconds) {
    for (unsigned int i = 0; i < seconds; ++i) {
        if (SignalManager::stop_requested()) {
            return false;
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return !SignalManager::stop_requested();
}

bool validate_task_message(
    const TaskMessage &task,
    std::string &error_code,
    std::string &error_description
) {
    if (!Hasher::is_valid_target_hash(task.target_hash)) {
        error_code = "BAD_TASK";
        error_description = "invalid_target_hash";
        return false;
    }

    if (!Keyspace::is_valid_key(task.left)) {
        error_code = "BAD_TASK";
        error_description = "invalid_left_boundary";
        return false;
    }

    if (!Keyspace::is_valid_key(task.right)) {
        error_code = "BAD_TASK";
        error_description = "invalid_right_boundary";
        return false;
    }

    if (!Keyspace::is_valid_range(task.left, task.right)) {
        error_code = "BAD_TASK";
        error_description = "invalid_range";
        return false;
    }

    return true;
}

bool send_error_and_close(
    TcpClient &client,
    const std::string &code,
    const std::string &description
) {
    const bool sent = client.send_line(Protocol::make_error(code, description));
    client.close();
    return sent;
}

}  // namespace

int main(int argc, char *argv[]) {
    try {
        if (argc != 4) {
            throw std::invalid_argument(
                "usage: ./app <host> <port> <path_to_hashrate_file>"
            );
        }

        const std::string host = argv[1];
        const std::uint16_t port = parse_port(argv[2]);
        const std::string hashrate_file = argv[3];

        if (host.empty()) {
            throw std::invalid_argument("host must not be empty");
        }

        if (hashrate_file.empty()) {
            throw std::invalid_argument(
                "path_to_hashrate_file must not be empty"
            );
        }

        const std::uint64_t hashrate =
            HashrateReader::read_from_file(hashrate_file);

        SignalManager::install_handlers();

        TcpClient client;
        if (!client.connect_to(host, port)) {
            std::cerr << "[ERROR] " << client.last_error() << '\n';
            return 1;
        }

        std::cout << "[INFO] connected to server\n";

        if (!client.send_line(Protocol::make_hello(hashrate))) {
            std::cerr << "[ERROR] " << client.last_error() << '\n';
            client.close();
            return 1;
        }

        std::cout << "[INFO] HELLO sent\n";

        while (!SignalManager::stop_requested()) {
            std::string line;
            const bool read_ok = client.read_line_interruptible(line, [] {
                return SignalManager::stop_requested();
            });

            if (!read_ok) {
                if (SignalManager::stop_requested()) {
                    std::cout << "[INFO] interrupted by signal "
                              << SignalManager::last_signal() << '\n';
                    client.close();
                    return 0;
                }

                std::cerr << "[ERROR] " << client.last_error() << '\n';
                client.close();
                return 1;
            }

            if (line.empty()) {
                continue;
            }

            ServerMessage message;
            std::string parse_error;
            if (!Protocol::parse_server_message(line, message, &parse_error)) {
                std::cerr << "[ERROR] invalid server message: " << line << '\n';
                send_error_and_close(client, "BAD_MESSAGE", parse_error);
                return 1;
            }

            switch (message.type) {
                case ServerMessageType::kTask: {
                    std::cout << "[INFO] received TASK hash="
                              << message.task.target_hash << " range=["
                              << message.task.left << "; " << message.task.right
                              << "]\n";

                    std::string error_code;
                    std::string error_description;
                    if (!validate_task_message(
                            message.task, error_code, error_description
                        )) {
                        std::cerr
                            << "[ERROR] invalid task: " << error_description
                            << '\n';
                        send_error_and_close(
                            client, error_code, error_description
                        );
                        return 1;
                    }

                    try {
                        BruteForceEngine engine([] {
                            return SignalManager::stop_requested();
                        });

                        const BruteForceResult result =
                            engine.run(message.task);

                        if (result.status == BruteForceStatus::kFound) {
                            std::cout << "[INFO] key found: " << result.key
                                      << '\n';
                            if (!client.send_line(
                                    Protocol::make_found(result.key)
                                )) {
                                std::cerr << "[ERROR] " << client.last_error()
                                          << '\n';
                                client.close();
                                return 1;
                            }
                        } else if (result.status ==
                                   BruteForceStatus::kNotFound) {
                            std::cout << "[INFO] key not found\n";
                            if (!client.send_line(Protocol::make_not_found())) {
                                std::cerr << "[ERROR] " << client.last_error()
                                          << '\n';
                                client.close();
                                return 1;
                            }
                        } else {
                            std::cout << "[INFO] bruteforce aborted\n";
                            client.close();
                            return 0;
                        }
                    } catch (const std::invalid_argument &ex) {
                        std::cerr << "[ERROR] bad task: " << ex.what() << '\n';
                        send_error_and_close(client, "BAD_TASK", ex.what());
                        return 1;
                    } catch (const std::exception &ex) {
                        std::cerr << "[ERROR] internal failure: " << ex.what()
                                  << '\n';
                        send_error_and_close(
                            client, "INTERNAL", "bruteforce_failed"
                        );
                        return 1;
                    }

                    break;
                }

                case ServerMessageType::kWait:
                    std::cout << "[INFO] received WAIT " << message.wait.seconds
                              << '\n';
                    if (!sleep_interruptible(message.wait.seconds)) {
                        std::cout << "[INFO] interrupted while waiting\n";
                        client.close();
                        return 0;
                    }
                    break;

                case ServerMessageType::kDone:
                    std::cout << "[INFO] received DONE\n";
                    client.close();
                    return 0;

                case ServerMessageType::kError:
                    std::cerr << "[ERROR] server ERROR " << message.error_code;
                    if (!message.error_description.empty()) {
                        std::cerr << ": " << message.error_description;
                    }
                    std::cerr << '\n';
                    client.close();
                    return 1;

                case ServerMessageType::kUnknown:
                    std::cerr << "[ERROR] unknown message type\n";
                    send_error_and_close(
                        client, "BAD_MESSAGE", "unknown_message_type"
                    );
                    return 1;
            }
        }

        std::cout << "[INFO] termination signal received\n";
        client.close();
        return 0;
    } catch (const std::exception &ex) {
        std::cerr << "[ERROR] " << ex.what() << '\n';
        return 1;
    }
}
