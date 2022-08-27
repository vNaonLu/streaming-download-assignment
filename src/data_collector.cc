// Copyright 2022, naon

#include <jigentec/data_collector.h>

#include <unordered_map>
#include <vector>

namespace jigentec {

class DataCollector::Opaque {
 public:
  std::unordered_map<uint32_t, std::vector<char>> received;
  std::vector<uint32_t>                           sequence;
  std::unique_ptr<char[]>                         combined;
  size_t                                          file_length;
};

DataCollector::DataCollector() noexcept : opaque_{std::make_unique<Opaque>()} {
  if (nullptr != opaque_) {
    /// TODO: likely
    opaque_->combined    = nullptr;
    opaque_->file_length = 0;
    opaque_->received.clear();
    opaque_->sequence.clear();
  }
}

DataCollector::~DataCollector() noexcept {}

bool DataCollector::Store(JigenTecPacket *pack) noexcept {
  /// TODO: deal with large file!!
  if (nullptr == opaque_ || opaque_->combined != nullptr) {
    /// TODO: unlikely
    return false;
  }

  auto payload = std::vector<char>(pack->payload_length());
  std::memcpy(payload.data(), pack->payload(), pack->payload_length());
  [[maybe_unused]] auto [it, inserted] = opaque_->received.emplace(
      std::make_pair(pack->seqence_number(), std::move(payload)));

  if (inserted) {
    /// TODO: likely
    opaque_->file_length += pack->payload_length();
    opaque_->sequence.emplace_back(pack->seqence_number());
  }

  return inserted;
}

std::pair<char const *, size_t> DataCollector::Dump() noexcept {
  if (nullptr == opaque_ || 0 == opaque_->file_length) {
    /// TODO: unlikely
    return std::make_pair(nullptr, 0);
  }

  if (nullptr == opaque_->combined) {
    sort(opaque_->sequence.begin(), opaque_->sequence.end());
    opaque_->combined = std::make_unique<char[]>(opaque_->file_length);
    auto beg          = opaque_->sequence.begin();
    auto data         = opaque_->combined.get();

    while (beg != opaque_->sequence.end()) {
      if (beg != opaque_->sequence.begin()) {
        /// check the completeness of data
        if (*(beg) != *(beg - 1) + 1) {
          /// TODO: unlikely
          opaque_->file_length = 0;
          opaque_->combined    = 0;
          break;
        }
      }

      auto find = opaque_->received.find(*beg);
      if (find != opaque_->received.end()) {
        /// TODO: likely
        auto &buffer = find->second;
        std::memcpy(data, buffer.data(), buffer.size());
        data += buffer.size();
      } else {
        opaque_->file_length = 0;
        opaque_->combined    = 0;
        break;
      }

      ++beg;
    }

    opaque_->received.clear();
  }

  return std::make_pair(opaque_->combined.get(), opaque_->file_length);
}

}  // namespace jigentec
