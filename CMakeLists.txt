cmake_minimum_required(VERSION 2.6)
project(ProjectName)

#include_directories("include")

include_directories(${CURSES_INCLUDE_DIR})

add_library(app net_com.cpp)

add_executable(main Safe_data.cpp)

target_link_libraries(main app wiringPi ${CURSES_LIBRARIES})

