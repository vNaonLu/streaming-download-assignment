// Copyright 2022, naon

#ifndef INCLUDE_JIGENTEC_CLIENT_H_
#define INCLUDE_JIGENTEC_CLIENT_H_

#include <cstdint>
#include <functional>
#include <memory>
#include <string_view>

#include "./jigentec.h"

namespace jigentec {

/**
 * @brief The unix TCP socket client class.
 *
 */
class Client {
 public:
  using ReceiveCallback = std::function<void(JigenTecPacket*)>;

  /**
   * @brief The return value of function ::Connect.
   *
   */
  enum class ConnectStatus {
    kSuccess,
    kFDNotEstablished,
    kNoSuchHostname,
    kFailedToConnect,
    kDisconnected
  };
  /**
   * @brief Startup a thread to connect with specified host and port.
   *
   * @param host_name specify the target host.
   * @param port specify the target port.
   * @return indicate the status of connection.
   */
  ConnectStatus Connect(std::string_view host_name, uint16_t port) noexcept;

  /**
   * @brief Close the connection from the host.
   *
   */
  void Disconnect() noexcept;

  /**
   * @brief Indicate whether the Client is ready. Should be invoked before
   * connect the server.
   *
   * @return true if the socket is established, false otherwise.
   */
  bool IsReady() const noexcept;

  /**
   * @brief Indicate whether the connection is alive between host.
   *
   * @return One of ConnectStatus::kSuccess, ConnectStatus::kFDNotEstablished or
   * ConnectStatus::kDisconnected.
   */
  ConnectStatus IsConnecting() const noexcept;

  /**
   * @brief Construct a new Client object.
   *
   * @param callback specify the receiving function and must not be nullptr.
   * Note for the argument in the callback will be destructed so ensure the data
   * is fully copied.
   */
  Client(ReceiveCallback callback) noexcept;

  /**
   * @brief Destroy the Client object.
   *
   */
  ~Client() noexcept;

 private:
  class Opaque;
  std::unique_ptr<Opaque> opaque_;
};

}  // namespace jigentec

#endif  // INCLUDE_JIGENTEC_CLIENT_H_