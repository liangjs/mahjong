project(mahjong)
cmake_minimum_required(VERSION 2.8)
#set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_BUILD_TYPE debug)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-Wall -Wextra")

ADD_LIBRARY(MahjongGB STATIC LibMahjongGB/MahjongGB.cpp)

add_definitions(-DLOCAL)
add_executable(main main.cpp)

target_link_libraries(main MahjongGB)
