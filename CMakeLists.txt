cmake_minimum_required(VERSION 3.31)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(game VERSION 0.0.1 LANGUAGES CXX)

include(cmake/Dependencies.cmake)

add_subdirectory(game)
add_subdirectory(gamelib)
add_subdirectory(tests)