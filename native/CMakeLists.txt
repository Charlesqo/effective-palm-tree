cmake_minimum_required(VERSION 3.16)
project(dlss_snake_native LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(snake_core
  src/core.cpp
  src/streamline_bridge.cpp
)

target_include_directories(snake_core PUBLIC include)

add_executable(snake_native src/main.cpp)
target_link_libraries(snake_native PRIVATE snake_core)

add_executable(snake_core_tests tests/core_tests.cpp)
target_link_libraries(snake_core_tests PRIVATE snake_core)

add_executable(streamline_bridge_tests tests/streamline_bridge_tests.cpp)
target_link_libraries(streamline_bridge_tests PRIVATE snake_core)

enable_testing()
add_test(NAME snake_core_tests COMMAND snake_core_tests)
add_test(NAME streamline_bridge_tests COMMAND streamline_bridge_tests)
