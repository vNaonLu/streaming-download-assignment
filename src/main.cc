// Copyright 2022, naon

#include <jigentec/client.h>
#include <jigentec/data_collector.h>
#include <jigentec/jigentec.h>
#include <sha256.h>

#include <chrono>
#include <iomanip>
#include <iostream>
#include <memory>
#include <thread>  // NOLINT [build/c++11]
#include <vector>

using jigentec::Client;
using jigentec::DataCollector;
using jigentec::JigenTecPacket;

using Clock = std::chrono::high_resolution_clock;
using Milli = std::chrono::milliseconds;

int main(int argc, char **argv) {
  /// downloaded data with payload only.
  DataCollector data_collection;

  /// construct a JigenTec Client object.
  /// and set the receiving callback.
  auto client = Client{[&](JigenTecPacket *pack) {
    if (nullptr != pack) {
      /// TODO: likely
      data_collection.Store(pack);
    }
  }};

  /// create a new thread and request the file download.
  auto status = client.Connect("assignment.jigentec.com", 49152);
  if (status != Client::ConnectStatus::kSuccess) {
    /// TODO: detail the failure message.
    std::cerr << "(error) failed to connect host" << std::endl;
    exit(EXIT_FAILURE);
  }

  auto    timer   = Clock::now();
  int64_t elapsed = 0;
  /// wait the download finishes.
  while (client.IsConnecting() == Client::ConnectStatus::kSuccess) {
    elapsed = std::chrono::duration_cast<Milli>(Clock::now() - timer).count();
    std::cout << "\r\e[K(client) packet downloading (" << std::fixed
              << std::setprecision(1) << (static_cast<double>(elapsed) / 1000.0)
              << "s) " << std::flush;
    std::this_thread::yield();
  }

  /// dump binary and verify the checksum.
  auto [data, length] = data_collection.Dump();
  if (nullptr != data && length > 0) {
    /// TODO: likely
    std::cout << std::endl
              << "(client) finish the file download (" << std::fixed
              << std::setprecision(1) << (static_cast<double>(elapsed) / 1000.0)
              << "s)." << std::endl;

    std::cout << "(client) received length: " << length << std::endl;
    std::cout << "(client) SHA-256 checksum: " << SHA256{}(data, length)
              << std::endl;
  } else {
    std::cout << "(error) an error occurred while downloading :(" << std::endl;
  }

  return EXIT_SUCCESS;
}
