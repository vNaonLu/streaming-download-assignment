// Copyright 2022, naon

#include <arpa/inet.h>
#include <assignment/client.h>
#include <assignment/compile.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <cassert>
#include <cstring>
#include <iostream>
#include <thread>  // NOLINT [build/c++11]

namespace assignment {

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

    int flag = ::fcntl(fd_, F_GETFL, 0);
    ::fcntl(fd_, F_SETFL, flag | O_NONBLOCK);
    ::connect(fd_, addr, length);

    fd_set wfds;
    FD_ZERO(&wfds);
    FD_SET(fd_, &wfds);
    /// wait up to 5 seconds.
    auto tv    = timeval{0, 0};
    tv.tv_sec  = 5;
    tv.tv_usec = 0;

    switch (::select(fd_ + 1, nullptr, &wfds, nullptr, &tv)) {
      case -1:
        status_.store(ConnectStatus::kUnknownError, std::memory_order_release);
        break;

      case 0:
        status_.store(ConnectStatus::kConnectTimeout,
                      std::memory_order_release);
        break;

      default:
        status_.store(ConnectStatus::kConnecting, std::memory_order_release);
        break;
    }
    ::fcntl(fd_, F_SETFL, flag);
    return status_.load(std::memory_order_relaxed) ==
           ConnectStatus::kConnecting;
  }

  void Close() noexcept {
    if (fd_ >= 0) {
      close(fd_);
      fd_ = -1;
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
        return false;

      default:
        break;
    }

    auto pack = Packet{};
    if (FD_ISSET(fd_, &rfds) &&
        ::recv(fd_, pack.data(), sizeof(pack), MSG_PEEK) > 0) {
      uint32_t recv_length = 0;
      uint32_t tot_bytes   = pack.payload_length() + Packet::kHeaderLength;
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

  ConnectStatus status() const noexcept {
    return status_.load(std::memory_order_acquire);
  }

  explicit Opaque(ReceiveCallback cb) noexcept
      : fd_{-1}, cb_{cb}, status_{ConnectStatus::kFDNotEstablished} {}

  ~Opaque() noexcept { Close(); }

 private:
  int                        fd_;
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

}  // namespace assignment
