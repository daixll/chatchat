# src/safe/CMakeLists.txt
add_library(safe STATIC safe.cpp decrypt.cpp encrypt.cpp)

target_link_libraries(safe PUBLIC util)

target_link_libraries(safe PRIVATE ${OPENSSL_LIBRARIES})