// Copyright 2022, naon

#include <assignment/compile.h>
#include <assignment/data_collector.h>

#include <algorithm>
#include <cstring>
#include <unordered_map>
#include <vector>

namespace assignment {

class DataCollector::Opaque {
 public:
  std::unordered_map<uint32_t, std::vector<char>> received;
  std::vector<uint32_t>                           sequence;
  std::unique_ptr<char[]>                         combined;
  size_t                                          file_length;
};

DataCollector::DataCollector() noexcept : opaque_{std::make_unique<Opaque>()} {
  if (LIKELY(nullptr != opaque_)) {
    opaque_->combined    = nullptr;
    opaque_->file_length = 0;
    opaque_->received.clear();
    opaque_->sequence.clear();
  }
}

DataCollector::~DataCollector() noexcept {}

bool DataCollector::Store(Packet *pack) noexcept {
  /// TODO: deal with large file!!
  if (UNLIKELY(nullptr == opaque_ || opaque_->combined != nullptr)) {
    return false;
  }

  auto payload = std::vector<char>(pack->payload_length());
  std::memcpy(payload.data(), pack->payload(), pack->payload_length());
  [[maybe_unused]] auto [it, inserted] = opaque_->received.emplace(
      std::make_pair(pack->seqence_number(), std::move(payload)));

  if (LIKELY(inserted)) {
    opaque_->file_length += pack->payload_length();
    opaque_->sequence.emplace_back(pack->seqence_number());
  }

  return inserted;
}

std::pair<char const *, size_t> DataCollector::Dump() noexcept {
  if (UNLIKELY(nullptr == opaque_ || 0 == opaque_->file_length)) {
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
        if (UNLIKELY(*(beg) != *(beg - 1) + 1)) {
          opaque_->file_length = 0;
          opaque_->combined    = nullptr;
          break;
        }
      }

      auto find = opaque_->received.find(*beg);
      if (LIKELY(find != opaque_->received.end())) {
        auto &buffer = find->second;
        std::memcpy(data, buffer.data(), buffer.size());
        data += buffer.size();
      } else {
        opaque_->file_length = 0;
        opaque_->combined    = nullptr;
        break;
      }

      ++beg;
    }

    opaque_->received.clear();
  }

  return std::make_pair(opaque_->combined.get(), opaque_->file_length);
}

}  // namespace assignment
