# Streaming Download Assignment

This project is about the coding assignment which requires a download from specified host and verify the checksum of file.

## Getting Started

To get started. see [Requirement](#requirement), [Build](#build) and the [Usage](#usage).

## Requirement

The following minimum versions are required to build the library.

- C++17 supported compiler
- CMake 3.20 or above
- Unix-like os

## Build

This describes the build process using cmake. As pre-requisites, you'll need git and cmake installed.

``` sh
# Check out the porject.
$ git clone https://github.com/vNaonLu/streaming-download-assignment.git assignment
# Go to the project root directory.
$ cd assignment
# Check out the submodules.
$ git submodule update --init --recursive
# Generate build system files with cmake
$ cmake -S . -B "build" -DCMAKE_BUILD_TYPE=Release
# Build the project
$ cmake --build "build"
```

The build directory should now look something like this:

```
+/streaming-assignment
|
+-+/build
  |
  +-/client
  |
  ...
```

## Usage

There is exact one executable `client` in the building directory, just type the below command in the project root directory:

``` sh
# you may need to execute with root permission.
$ ./build/client <host> <port>
```

And the output may looks like:

``` sh
$ ./build/client some.url.com 12345
(client) connecting to the host: some.url.com:12345
(client) established a connection to the host: some.url.com:12345
(client) packet downloading... (14.3s) 
(client) finish the download (14.3s).
(client) received length: 2906111 byte(s)
(client) SHA-256 checksum: <sha...>
```