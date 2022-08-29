// Copyright 2022, naon

#ifndef INCLUDE_ASSIGNMENT_DATA_COLLECTOR_H_
#define INCLUDE_ASSIGNMENT_DATA_COLLECTOR_H_

#include <memory>
#include <utility>

#include "./protocol.h"

namespace assignment {

/**
 * @brief The data collector receives packets and combines them to
 * a single buffer.
 *
 */
class DataCollector {
 private:
  class Opaque;
  std::unique_ptr<Opaque> opaque_;

 public:
  /**
   * @brief Collect a packet. Note that the ::Store() function will be
   * unavailable after ::Dump() called.
   *
   * @param packet specify the target packet.
   * @return ture if the store operation run successfully, false otherwise.
   */
  bool Store(Packet *packet) noexcept;

  /**
   * @brief Combine the packets which are stored before and dump the binary.
   * Note that ::Dump will make ::Store() function be no longer available.
   *
   * @return a combined data and its length.
   */
  std::pair<char const *, size_t> Dump() noexcept;

  /**
   * @brief Construct a new Data Collector object.
   *
   */
  DataCollector() noexcept;

  /**
   * @brief Destroy the Data Collector object.
   *
   */
  ~DataCollector() noexcept;
};

}  // namespace assignment

#endif  // INCLUDE_ASSIGNMENT_DATA_COLLECTOR_H_
