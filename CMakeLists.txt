cmake_minimum_required(VERSION 3.22)
project(IMC)

set(CMAKE_CXX_STANDARD 20)

include_directories(include)

add_executable(main source/main.cpp)

find_package(Threads REQUIRED)
target_link_libraries(main ${CMAKE_THREAD_LIBS_INIT})