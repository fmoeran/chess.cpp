cmake_minimum_required(VERSION 3.10)

project(Chess VERSION 0.9)

target_include_directories(Chess ${CMAKE_CURRENT_SOURCE_DIR}/include)
target

add_executable(Chess src/* include/*)

target_link_directory(Chess PUBLIC dependencies/SFML/lib)

