#pragma once

#include <cstdint>
#include <functional>
#include <string>

class TcpClient {
public:
    using StopPredicate = std::function<bool()>;

    TcpClient() = default;
    ~TcpClient();

    TcpClient(const TcpClient &) = delete;
    TcpClient &operator=(const TcpClient &) = delete;

    bool connect_to(const std::string &host, std::uint16_t port);
    bool send_line(const std::string &line);

    bool read_line_interruptible(
        std::string &out_line,
        const StopPredicate &stop_predicate,
        int timeout_ms = 200
    );

    void close();
    bool is_connected() const;
    const std::string &last_error() const;

private:
    int socket_fd_ = -1;
    std::string read_buffer_;
    std::string last_error_;
};
