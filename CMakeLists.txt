cmake_minimum_required(VERSION 3.0.0)
project(Orderbook VERSION 0.1.0 LANGUAGES C CXX)

# Set C++20 as the standard for all C++ targets
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)
enable_testing()

add_executable(Orderbook main.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
