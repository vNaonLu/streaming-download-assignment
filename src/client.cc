// Copyright 2022, naon

#include <arpa/inet.h>
#include <errno.h>
#include <jigentec/client.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cassert>
#include <iostream>
#include <thread>

namespace jigentec {

class Client::Opaque {
 public:
  bool StartupTcpSocket() noexcept {
    fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (!IsValid()) {
      /// Socket failed to be established.
      return false;
    }
    return true;
  }

  bool Connect(struct sockaddr_in *addr) {
    return is_connection_alive_ =
               (IsValid() && ::connect(fd_, (sockaddr *)addr, sizeof(addr)));
  }

  void Close() noexcept {
    if (fd_ >= 0) {
      close(fd_);
      fd_                  = -1;
      is_connection_alive_ = false;
    }
  }

  bool IsValid() const noexcept { return fd_ >= 0; }

  bool Receive() noexcept {
    JigenTecPacket peak;
    if (::recv(fd_, &peak, sizeof(peak), MSG_PEEK | MSG_DONTWAIT) < 0) {
      switch (errno) {
        case EWOULDBLOCK:  /// or EAGAIN
          return true;
        default:
          is_connection_alive_ = false;
          return false;
      }
    }

    auto tot_bytes = sizeof(JigenTecPacket) + peak.payload_length;
    auto buffer    = std::make_unique<char[]>(tot_bytes);
    if (::recv(fd_, buffer.get(), tot_bytes, 0) < 0) {
      /// TODO: receiving failure
      return false;
    }

    assert(nullptr != cb_);
    cb_((JigenTecPacket *)buffer.get());
    return true;
  }

  void BindReceivingFunc(ReceiveCallback cb) { cb_ = cb; }

  bool is_connection_alive() const noexcept { return is_connection_alive_; }

  Opaque() noexcept : is_connection_alive_{false}, fd_{-1} {}

  ~Opaque() noexcept { Close(); }

 private:
  bool            is_connection_alive_;
  int             fd_;
  ReceiveCallback cb_;
};

Client::Client(ReceiveCallback callback) noexcept
    : opaque_{std::make_unique<Opaque>()} {
  if (nullptr != opaque_) {
    /// TODO: likely
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

  /// print out the ip to required hostname
  /// std::cout << inet_ntoa(*(in_addr *)he->h_addr_list[0]) << std::endl;

  sockaddr_in addr;
  std::memset(&addr, 0, sizeof(addr));
  std::memcpy(&addr.sin_addr, he->h_addr_list[0], he->h_length);
  addr.sin_family = AF_INET;
  addr.sin_port   = htons(port);

  if (!opaque_->Connect(&addr)) {
    return ConnectStatus::kFailedToConnect;
  }

  std::thread([=]() {
    while (this->opaque_->is_connection_alive()) {
      this->opaque_->Receive();
    }
    this->opaque_->Close();
  }).detach();

  return ConnectStatus::kSuccess;
}

void Client::Disconnect() noexcept {
  // re-startup the file descriptor
  if (nullptr != opaque_) {
    /// TODO: likely
    opaque_->Close();
  }
}

Client::ConnectStatus Client::IsConnecting() const noexcept {
  if (nullptr == opaque_) {
    /// TODO: unlikely
    return ConnectStatus::kFDNotEstablished;
  }
  return opaque_->is_connection_alive() ? ConnectStatus::kSuccess
                                        : ConnectStatus::kDisconnected;
}

}  // namespace jigentec
