// Copyright 2022, naon

#include <arpa/inet.h>
#include <errno.h>
#include <jigentec/client.h>
#include <jigentec/compile.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <cassert>
#include <iostream>
#include <thread>  // NOLINT [build/c++11]

namespace jigentec {

class Client::Opaque {
 public:
  bool StartupTcpSocket() noexcept {
    if (IsValid()) {
      /// socket is already established
      return true;
    }

    fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (IsValid()) {
      status_.store(ConnectStatus::kNotConnect, std::memory_order_release);
      return true;
    } else {
      return false;
    }
  }

  bool ConnectByIP(std::string_view host, uint16_t port) {
    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_addr.s_addr = ::inet_addr(host.data());
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(port);
    return Connect(reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
  }

  bool ConnectByHostname(std::string_view hostname, uint16_t port) {
    hostent *he = ::gethostbyname(hostname.data());
    if (nullptr == he) {
      status_.store(ConnectStatus::kDestinationNotFound,
                    std::memory_order_release);
      return false;
    }
    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    std::memcpy(&addr.sin_addr, he->h_addr, he->h_length);
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);
    return Connect(reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
  }

  bool Connect(struct sockaddr *addr, socklen_t length) {
    if (!IsValid()) {
      return false;
    }
    if ((client_fd_ = ::connect(fd_, addr, length)) >= 0) {
      status_.store(ConnectStatus::kConnecting, std::memory_order_release);
      return true;
    } else {
      status_.store(ConnectStatus::kDestinationNotFound,
                    std::memory_order_release);
      return false;
    }
  }

  void Close() noexcept {
    if (client_fd_ >= 0) {
      close(client_fd_);
      client_fd_ = -1;
      status_.store(ConnectStatus::kNotConnect, std::memory_order_release);
    }
  }

  bool IsValid() const noexcept { return fd_ >= 0; }

  bool Receive() noexcept {
    /// wait up to 1 second.
    auto tv    = timeval{0, 0};
    tv.tv_sec  = 1;
    tv.tv_usec = 0;

    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(fd_, &rfds);

    switch (::select(fd_ + 1, &rfds, nullptr, nullptr, &tv)) {
      case -1:
        [[fallthrough]];
      case 0:
        /// timeout
        /// regard as the finish of download.
        return false;

      default:
        break;
    }

    auto pack = JigenTecPacket{};
    if (FD_ISSET(fd_, &rfds) &&
        ::recv(fd_, pack.data(), sizeof(pack), MSG_PEEK) > 0) {
      uint32_t recv_length = 0;
      uint32_t tot_bytes =
          pack.payload_length() + JigenTecPacket::kHeaderLength;
      while (recv_length != tot_bytes) {
        auto len =
            ::read(fd_, pack.data() + recv_length, (tot_bytes - recv_length));
        if (len <= 0) {
          return false;
        }
        recv_length += len;
      }
      cb_(&pack);
      return true;
    } else {
      return false;
    }
  }

  bool is_connection_alive() const noexcept { return client_fd_ >= 0; }

  ConnectStatus status() const noexcept {
    return status_.load(std::memory_order_acquire);
  }

  explicit Opaque(ReceiveCallback cb) noexcept
      : fd_{-1},
        client_fd_{-1},
        cb_{cb},
        status_{ConnectStatus::kFDNotEstablished} {}

  ~Opaque() noexcept { Close(); }

 private:
  int                        fd_;
  int                        client_fd_;
  ReceiveCallback            cb_;
  std::atomic<ConnectStatus> status_;
};

Client::Client(ReceiveCallback callback) noexcept
    : opaque_{std::make_unique<Opaque>(callback)} {
  if (LIKELY(nullptr != opaque_)) {
    opaque_->StartupTcpSocket();
  }
}

Client::~Client() noexcept {}

bool Client::Connect(std::string_view host_name, uint16_t port) noexcept {
  if (nullptr == opaque_ ||
      (!opaque_->IsValid() && !opaque_->StartupTcpSocket())) {
    return false;
  }

  if (!opaque_->ConnectByHostname(host_name, port)) {
    return false;
  }

  std::thread([=]() {
    while (this->opaque_->status() == ConnectStatus::kConnecting) {
      if (!this->opaque_->Receive()) {
        break;
      }
    }
    this->opaque_->Close();
  }).detach();

  return true;
}

void Client::Disconnect() noexcept {
  if (LIKELY(nullptr != opaque_)) {
    opaque_->Close();
  }
}

Client::ConnectStatus Client::Status() const noexcept {
  if (UNLIKELY(nullptr == opaque_)) {
    return ConnectStatus::kFDNotEstablished;
  }
  return opaque_->status();
}

}  // namespace jigentec
