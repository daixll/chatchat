# src/CMakeLists.txt
add_executable(chatchat main.cpp)

target_link_libraries(chatchat PRIVATE safe util)

add_subdirectory(util)
add_subdirectory(safe)