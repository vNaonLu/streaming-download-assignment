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
    kConnecting,
    kNotConnect,
    kFDNotEstablished,
    kDestinationNotFound,
  };

  /**
   * @brief Startup a thread to connect with specified host and port.
   *
   * @param host_name specify the target host.
   * @param port specify the target port.
   * @return indicate the status of connection.
   */
  bool Connect(std::string_view host_name, uint16_t port) noexcept;

  /**
   * @brief Close the connection from the host.
   *
   */
  void Disconnect() noexcept;

  /**
   * @brief Indicate whether the connection is alive.
   *
   * @return an enumeration of ::ConnectStatus.
   */
  ConnectStatus Status() const noexcept;

  /**
   * @brief Construct a new Client object.
   *
   * @param callback specify the receiving function and must not be nullptr.
   * Note for the argument in the callback will be destructed so ensure the data
   * is fully copied.
   */
  explicit Client(ReceiveCallback callback) noexcept;

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
