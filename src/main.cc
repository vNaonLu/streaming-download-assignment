// Copyright 2022, naon

#include <jigentec/client.h>
#include <sha256.h>

#include <chrono>
#include <iomanip>
#include <iostream>
#include <memory>
#include <thread>  // NOLINT [build/c++11]
#include <vector>

using jigentec::Client;
using jigentec::JigenTecPacket;

int main(int argc, char **argv) {
  /// downloaded data with payload only.
  auto data_collection = std::vector<char>();

  /// construct a JigenTec Client object.
  /// and set the receiving callback.
  auto client = Client{[&](JigenTecPacket *pack) {
    std::cout << "seq: " << pack->seqence_number
              << ", length: " << pack->payload_length << std::endl;
    if (pack->seqence_number + pack->payload_length >= data_collection.size()) {
      data_collection.resize(pack->seqence_number + pack->payload_length + 1);
    }

    /// payload is immediately after JigenTecPacket since it is just a header
    /// structure.
    /// TODO: make clearer pointer to copy.
    std::memcpy(data_collection.data() + pack->seqence_number, pack->payload(),
                pack->payload_length);
  }};

  /// create a new thread and request the file download.
  auto status = client.Connect("assignment.jigentec.com", 49152);
  if (status != Client::ConnectStatus::kSuccess) {
    /// TODO: detail the failure message.
    std::cerr << "(error) failed to connect host" << std::endl;
    exit(EXIT_FAILURE);
  }

  /// wait the download finishes.
  using namespace std::chrono;
  auto    timer   = high_resolution_clock::now();
  int64_t elapsed = 0;
  while (client.IsConnecting() == Client::ConnectStatus::kSuccess) {
    /// TODO: make data_collection be message queue can acheive coroutine in
    /// downloading and combining.
    elapsed = duration_cast<milliseconds>(high_resolution_clock::now() - timer)
                  .count();
    // std::cout << "\r\e[K(client) packet downloading (" << std::fixed
    //           << std::setprecision(1) << (static_cast<double>(elapsed) / 1000.0)
    //           << "s) " << std::flush;
    std::this_thread::yield();
  }
  std::cout << std::endl
            << "(client) finish the file download (" << std::fixed
            << std::setprecision(1) << (static_cast<double>(elapsed) / 1000.0)
            << "s)." << std::endl;

  std::cout << "(client) received length: " << data_collection.size()
            << std::endl;
  std::cout << "(client) SHA-256 checksum: "
            << SHA256{}(data_collection.data(), data_collection.size())
            << std::endl;

  return EXIT_SUCCESS;
}
