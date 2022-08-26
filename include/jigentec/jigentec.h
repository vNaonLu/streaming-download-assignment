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
  int32_t seqence_number;
  int16_t payload_length;
};

}  // namespace jigentec

#endif  // INCLUDE_JIGENTEC_JIGENTEC_H_
