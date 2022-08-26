// Copyright 2022, naon

#include <jigentec/client.h>

#include <iostream>
#include <memory>
#include <thread>  // NOLINT [build/c++11]
#include <vector>

using jigentec::Client;
using jigentec::JigenTecPacket;

int main(int argc, char **argv) {
  /// downloaded data with payload only.
  auto data_collection = std::vector<std::vector<char>>();

  /// construct a JigenTec Client object.
  /// and set the receiving callback.
  auto client = Client{[&](JigenTecPacket *pack) {
    if (pack->seqence_number >= data_collection.size()) {
      data_collection.resize(pack->seqence_number + 1);
    }
    auto &slot = data_collection[pack->seqence_number];
    slot.resize(pack->payload_length);
    /// payload is immediately after JigenTecPacket since it is just a header
    /// structure.
    /// TODO: make clearer pointer to copy.
    std::memcpy(slot.data(), pack + sizeof(JigenTecPacket),
                pack->payload_length);
  }};

  /// create a new thread and request the file download.
  auto status = client.Connect("test.jigentec.com", 49152);
  if (status != Client::ConnectStatus::kSuccess) {
    /// TODO: detail the failure message.
    std::cerr << "(error) failed to connect host" << std::endl;
    exit(EXIT_FAILURE);
  }

  /// wait the download finishes.
  while (client.IsConnecting() == Client::ConnectStatus::kSuccess) {
    /// TODO: make data_collection be message queue can acheive coroutine in
    /// downloading and combining.
    std::this_thread::yield();
  }

  /// TODO: check file is downloaded completely.

  return EXIT_SUCCESS;
}
