# CMakeLists.txt
cmake_minimum_required(VERSION 3.25)

project(chatchat VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 外部包
find_package(Qt6 REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(src)