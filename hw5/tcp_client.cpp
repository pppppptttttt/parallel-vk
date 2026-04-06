#include "tcp_client.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <string>

TcpClient::~TcpClient() {
    close();
}

bool TcpClient::connect_to(const std::string &host, std::uint16_t port) {
    close();

    addrinfo hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    addrinfo *result = nullptr;
    const std::string port_text = std::to_string(port);

    const int rc =
        getaddrinfo(host.c_str(), port_text.c_str(), &hints, &result);
    if (rc != 0) {
        last_error_ = std::string("getaddrinfo failed: ") + gai_strerror(rc);
        return false;
    }

    bool connected = false;
    int last_errno = 0;

    for (const addrinfo *current = result; current != nullptr;
         current = current->ai_next) {
        const int fd = ::socket(
            current->ai_family, current->ai_socktype, current->ai_protocol
        );
        if (fd < 0) {
            last_errno = errno;
            continue;
        }

        if (::connect(fd, current->ai_addr, current->ai_addrlen) == 0) {
            socket_fd_ = fd;
            connected = true;
            break;
        }

        last_errno = errno;
        ::close(fd);
    }

    freeaddrinfo(result);

    if (!connected) {
        last_error_ =
            std::string("connect failed: ") + std::strerror(last_errno);
        return false;
    }

    last_error_.clear();
    read_buffer_.clear();
    return true;
}

bool TcpClient::send_line(const std::string &line) {
    if (!is_connected()) {
        last_error_ = "socket is not connected";
        return false;
    }

    std::string data = line;
    data.push_back('\n');

    std::size_t sent_total = 0;
    while (sent_total < data.size()) {
        const ssize_t sent = ::send(
            socket_fd_, data.data() + sent_total, data.size() - sent_total, 0
        );

        if (sent < 0) {
            if (errno == EINTR) {
                continue;
            }

            last_error_ = std::string("send failed: ") + std::strerror(errno);
            return false;
        }

        if (sent == 0) {
            last_error_ = "send returned 0";
            return false;
        }

        sent_total += static_cast<std::size_t>(sent);
    }

    return true;
}

bool TcpClient::read_line_interruptible(
    std::string &out_line,
    const StopPredicate &stop_predicate,
    int timeout_ms
) {
    out_line.clear();

    if (!is_connected()) {
        last_error_ = "socket is not connected";
        return false;
    }

    while (true) {
        if (stop_predicate && stop_predicate()) {
            last_error_ = "operation interrupted";
            return false;
        }

        const std::size_t newline_pos = read_buffer_.find('\n');
        if (newline_pos != std::string::npos) {
            out_line = read_buffer_.substr(0, newline_pos);
            read_buffer_.erase(0, newline_pos + 1);

            if (!out_line.empty() && out_line.back() == '\r') {
                out_line.pop_back();
            }

            return true;
        }

        pollfd descriptor{};
        descriptor.fd = socket_fd_;
        descriptor.events = POLLIN | POLLERR | POLLHUP | POLLNVAL;

        const int poll_rc = ::poll(&descriptor, 1, timeout_ms);
        if (poll_rc < 0) {
            if (errno == EINTR) {
                continue;
            }

            last_error_ = std::string("poll failed: ") + std::strerror(errno);
            return false;
        }

        if (poll_rc == 0) {
            continue;
        }

        if ((descriptor.revents & POLLNVAL) != 0) {
            last_error_ = "poll reported invalid socket";
            return false;
        }

        if ((descriptor.revents & POLLERR) != 0) {
            last_error_ = "poll reported socket error";
            return false;
        }

        if ((descriptor.revents & (POLLIN | POLLHUP)) == 0) {
            continue;
        }

        char chunk[4096];
        const ssize_t received = ::recv(socket_fd_, chunk, sizeof(chunk), 0);

        if (received < 0) {
            if (errno == EINTR) {
                continue;
            }

            last_error_ = std::string("recv failed: ") + std::strerror(errno);
            return false;
        }

        if (received == 0) {
            last_error_ = "connection closed by peer";
            return false;
        }

        read_buffer_.append(chunk, static_cast<std::size_t>(received));
    }
}

void TcpClient::close() {
    if (socket_fd_ >= 0) {
        ::shutdown(socket_fd_, SHUT_RDWR);
        ::close(socket_fd_);
        socket_fd_ = -1;
    }
}

bool TcpClient::is_connected() const {
    return socket_fd_ >= 0;
}

const std::string &TcpClient::last_error() const {
    return last_error_;
}
