# hash library
file(GLOB HASH_LIBRARY_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/hash_library/*.cpp)
add_library(
  hash_lib
  STATIC ${HASH_LIBRARY_SOURCE}
)
target_include_directories(
  hash_lib
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/hash_library/
)