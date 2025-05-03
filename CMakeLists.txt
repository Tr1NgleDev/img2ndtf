cmake_minimum_required(VERSION 3.10)
project(img2ndtf)

add_subdirectory("libs/ndtf")

set(CMAKE_CXX_STANDARD 20)

add_executable(img2ndtf main.cpp)

target_link_libraries(img2ndtf PRIVATE ndtf)
