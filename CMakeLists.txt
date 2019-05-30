project(mahjong)
cmake_minimum_required(VERSION 3.0)
#set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_BUILD_TYPE release)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-Wall")
#if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
#	set(CMAKE_CXX_FLAGS "/utf-8")
#endif()

ADD_LIBRARY(MahjongGB STATIC LibMahjongGB/MahjongGB.cpp)

#add_definitions(-DLOCAL)
add_executable(main main.cpp)
add_executable(mahjrun mahjrun.cpp)

target_link_libraries(main MahjongGB)
target_link_libraries(mahjrun MahjongGB)
