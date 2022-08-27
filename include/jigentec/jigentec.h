// Copyright 2022, naon

#ifndef INCLUDE_JIGENTEC_JIGENTEC_H_
#define INCLUDE_JIGENTEC_JIGENTEC_H_

#include <cstdint>

namespace jigentec {

/**
 * @brief The packet header defined by the assignment.
 *
 */
struct JigenTecPacket {
  uint32_t seqence_number;
  uint16_t payload_length;

  inline char *payload() noexcept {
    return reinterpret_cast<char *>(this + sizeof(JigenTecPacket));
  }
};

}  // namespace jigentec

#endif  // INCLUDE_JIGENTEC_JIGENTEC_H_
