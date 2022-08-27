# JigenTec Streaming Coding Assignement

This project is about the JigenTec's coding assignment which requires a download from specified host and verify the checksum of file.

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
$ git clone https://github.com/vNaonLu/JigenTec-coding-assignment.git jigentec-assignment
# Go to the project root directory.
$ cd jigentec-assignment
# Check out the submodules.
$ git submodule update --init --recursive
# Generate build system files with cmake
$ cmake -S . -B "build" -DCMAKE_BUILD_TYPE=Release
# Build the project
$ cmake --build "build"
```

The build directory should now look something like this:

```
+/jigentec-assignment
|
+-+/build
  |
  +-/jigentec-client
  |
  ...
```

## Usage

There is exact one executable `jigentec-client` in the building directory, just type the below command in the project root directory:

``` sh
# you may need to execute with root permission.
$ ./build/jigentec-client <host> <port>
```

And the output may looks like:

``` sh
$ ./build/jigentec-client assignment.jigentec.com 49152
(client) connecting to the host: assignment.jigentec.com:49152
(client) established a connection to the host: assignment.jigentec.com:49152
(client) packet downloading... (14.3s) 
(client) finish the download (14.3s).
(client) received length: 2906111 byte(s)
(client) SHA-256 checksum: 093afcce35604b2ef9119f39981073e7ccd3530b569325fc9f6b2c40925b4e6d
```