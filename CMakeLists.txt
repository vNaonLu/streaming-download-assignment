cmake_minimum_required(VERSION 3.20)

project(
  streaming-coding-assignment
  LANGUAGES CXX
  VERSION 0.1
)

# compiler flags.
set(CMAKE_CXX_STANDARD      17)
set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3")
set(CMAKE_CXX_FLAGS_DEBUG   "-g")

# for vscode intellisense
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# project dependencies.
set(${PROJECT_NAME}_INCLUDES  ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(${PROJECT_NAME}_LIBRARIES )

# glob the total .cc files.
file(GLOB ${PROJECT_NAME}_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc)

# create an executable target.
add_executable(
  client
  ${${PROJECT_NAME}_SOURCE}
)

# build third party
add_subdirectory(third_party)

target_include_directories(
  client
  PRIVATE ${${PROJECT_NAME}_INCLUDES}
)

target_link_libraries(
  client
  PRIVATE ${${PROJECT_NAME}_LIBRARIES}
          pthread
          hash_lib
)

target_compile_options(
  client
  PRIVATE "-Wall" "-Wextra" "-Werror"
)