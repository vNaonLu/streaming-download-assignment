// Copyright 2022, naon

#include <arpa/inet.h>
#include <errno.h>
#include <jigentec/client.h>
#include <jigentec/compile.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

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
    return IsValid();
  }

  bool Connect(struct sockaddr_in *addr) {
    if (!IsValid()) {
      return false;
    }
    client_fd_ =
        ::connect(fd_, reinterpret_cast<sockaddr *>(addr), sizeof(sockaddr_in));

    return client_fd_ >= 0;
  }

  void Close() noexcept {
    if (client_fd_ >= 0) {
      close(client_fd_);
      client_fd_ = -1;
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

    /// TODO: select handle
    switch (::select(fd_ + 1, &rfds, nullptr, nullptr, &tv)) {
      case -1:
        /// select failure
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
          /// TODO: receiving failure or disconnect
          return false;
        }
        recv_length += len;
      }

      assert(nullptr != cb_);
      cb_(&pack);
      return true;
    } else {
      return false;
    }
  }

  void BindReceivingFunc(ReceiveCallback cb) { cb_ = cb; }

  bool is_connection_alive() const noexcept { return client_fd_ >= 0; }

  Opaque() noexcept : fd_{-1}, client_fd_{-1} {}

  ~Opaque() noexcept { Close(); }

 private:
  int             fd_;
  int             client_fd_;
  ReceiveCallback cb_;
};

Client::Client(ReceiveCallback callback) noexcept
    : opaque_{std::make_unique<Opaque>()} {
  if (LIKELY(nullptr != opaque_)) {
    opaque_->StartupTcpSocket();
    opaque_->BindReceivingFunc(callback);
  }
}

Client::~Client() noexcept {}

bool Client::IsReady() const noexcept { return nullptr != opaque_; }

Client::ConnectStatus Client::Connect(std::string_view host_name,
                                      uint16_t         port) noexcept {
  if (nullptr == opaque_ ||
      (!opaque_->IsValid() && !opaque_->StartupTcpSocket())) {
    return ConnectStatus::kFDNotEstablished;
  }

  hostent *he = ::gethostbyname(host_name.data());
  if (nullptr == he) {
    return ConnectStatus::kNoSuchHostname;
  }

  sockaddr_in addr;
  std::memset(&addr, 0, sizeof(addr));
  std::memcpy(&addr.sin_addr, he->h_addr, he->h_length);
  addr.sin_family = AF_INET;
  addr.sin_port   = htons(port);

  if (!opaque_->Connect(&addr)) {
    return ConnectStatus::kFailedToConnect;
  }

  std::thread([=]() {
    while (this->opaque_->is_connection_alive()) {
      if (!this->opaque_->Receive()) {
        /// TODO: failed to receive
        break;
      }
    }
    this->opaque_->Close();
  }).detach();

  return ConnectStatus::kSuccess;
}

void Client::Disconnect() noexcept {
  if (LIKELY(nullptr != opaque_)) {
    opaque_->Close();
  }
}

Client::ConnectStatus Client::IsConnecting() const noexcept {
  if (UNLIKELY(nullptr == opaque_)) {
    return ConnectStatus::kFDNotEstablished;
  }
  return opaque_->is_connection_alive() ? ConnectStatus::kSuccess
                                        : ConnectStatus::kDisconnected;
}

}  // namespace jigentec
