cmake_minimum_required(VERSION 3.10)

project(isomorph)

set (CMAKE_CXX_STANDARD 20)

set(LIB_SOURCES
    deck.cc
    deck.h
    hand_index-impl.h
    hand_index.cc
    hand_index.h
)

add_library(isomorph ${LIB_SOURCES})

add_executable(isomorphtest check_main.cc)
target_link_libraries(isomorphtest isomorph)
