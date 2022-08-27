// Copyright 2022, naon

#ifndef INCLUDE_JIGENTEC_JIGENTEC_H_
#define INCLUDE_JIGENTEC_JIGENTEC_H_

#include <arpa/inet.h>

#include <cstdint>
#include <limits>
#include <memory>

#include <iostream>

namespace jigentec {

/**
 * @brief The JigenTec's file transfer protocol
 *
 */
class JigenTecPacket {
 public:
  inline constexpr static size_t kHeaderLength =
      sizeof(uint32_t) + sizeof(uint16_t);

 private:
  char buffer_[std::numeric_limits<uint16_t>::max() + kHeaderLength];

 public:
  inline char *data() noexcept { return buffer_; }

  inline uint32_t seqence_number() noexcept {
    return ntohl(*reinterpret_cast<uint32_t *>(buffer_));
  }

  inline uint16_t payload_length() noexcept {
    return ntohs(*reinterpret_cast<uint16_t *>(buffer_ + sizeof(uint32_t)));
  }

  inline char *payload() noexcept {
    return reinterpret_cast<char *>(buffer_ + sizeof(uint32_t) + sizeof(uint16_t));
  }
};

}  // namespace jigentec

#endif  // INCLUDE_JIGENTEC_JIGENTEC_H_
