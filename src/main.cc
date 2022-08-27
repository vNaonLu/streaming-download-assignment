// Copyright 2022, naon

#include <jigentec/client.h>
#include <jigentec/compile.h>
#include <jigentec/data_collector.h>
#include <jigentec/jigentec.h>
#include <sha256.h>

#include <chrono>  // NOLINT [build/c++11]
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

struct Optional {
  std::string_view host;
  uint16_t         port;
};

std::unique_ptr<Optional> Parse(int argc, char **argv);
void                      Usage() noexcept;

int main(int argc, char **argv) {
  auto opts = Parse(argc, argv);
  if (nullptr == opts) {
    exit(EXIT_FAILURE);
  }

  DataCollector data_collection;
  /// construct a JigenTec Client object.
  /// and set the receiving callback.
  auto client = Client{[&](JigenTecPacket *pack) {
    if (LIKELY(nullptr != pack)) {
      data_collection.Store(pack);
    }
  }};

  /// create a new thread and request the file download.
  std::cout << "(client) connecting to the host: " << opts->host << ":"
            << opts->port << std::endl;
  if (!client.Connect(opts->host, opts->port)) {
    switch (client.Status()) {
      case Client::ConnectStatus::kDestinationNotFound:
        std::cout << "(error) cannot resolve the host: " << opts->host << ":"
                  << opts->port << std::endl;
        break;

      case Client::ConnectStatus::kFDNotEstablished:
        std::cout << "(error) cannot establish client." << std::endl;
        break;

      case Client::ConnectStatus::kConnectTimeout:
        std::cout << "(error) timeout." << std::endl;
        break;

      default:
        std::cout << "(error) failed to connect host." << std::endl;
        break;
    }
    exit(EXIT_FAILURE);
  }
  std::cout << "(client) established a connection to the host: " << opts->host
            << ":" << opts->port << std::endl;

  /// wait the download finishes.
  auto    timer   = Clock::now();
  int64_t elapsed = 0;
  while (client.Status() == Client::ConnectStatus::kConnecting) {
    elapsed = std::chrono::duration_cast<Milli>(Clock::now() - timer).count();
    std::cout << "\r\e[K(client) packet downloading... (" << std::fixed
              << std::setprecision(1) << (static_cast<double>(elapsed) / 1000.0)
              << "s) " << std::flush;
    std::this_thread::yield();
  }

  /// dump binary and verify the checksum.
  auto [data, length] = data_collection.Dump();
  if (nullptr != data && length > 0) {
    std::cout << std::endl
              << "(client) finish the download (" << std::fixed
              << std::setprecision(1) << (static_cast<double>(elapsed) / 1000.0)
              << "s)." << std::endl;

    std::cout << "(client) received length: " << length << " byte(s)"
              << std::endl;
    std::cout << "(client) SHA-256 checksum: " << SHA256()(data, length)
              << std::endl;
  } else {
    std::cout << "(error) an error occurred while downloading :(" << std::endl;
  }

  return EXIT_SUCCESS;
}

std::unique_ptr<Optional> Parse(int argc, char **argv) {
  if (argc == 3) {
    auto opts  = std::make_unique<Optional>();
    opts->host = argv[1];

    try {
      auto port = std::stoul(argv[2]);
      if (port < std::numeric_limits<uint16_t>::max()) {
        opts->port = port;
        return opts;
      }
    } catch (std::exception const &e) {
    }
    std::cout << "(error) cannot resolve the port: " << argv[2] << std::endl;
  }
  Usage();
  return nullptr;
}

void Usage() noexcept {
  std::cout << "Usage: jigentec-client <host> <port>" << std::endl;
}
