cmake_minimum_required(VERSION 3.10)
project(colors)

find_package(OpenCV REQUIRED)

# set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -Wextra -Wstrict-aliasing -pedantic -Wunreachable-code")

include_directories(include)
add_subdirectory(source)
