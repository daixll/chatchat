# src/util/CMakeLists.txt
add_library(util STATIC util.cpp)

# 包含 util.h 供其他模块使用
target_include_directories(util PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})