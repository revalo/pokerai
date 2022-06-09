cmake_minimum_required(VERSION 3.10)

project(ompeval)

include_directories(
  libdivide
  omp
)

set (CMAKE_CXX_STANDARD 11)

set(ROOT_DIR omp)

set(LIB_SOURCES
    ${ROOT_DIR}/CardRange.cpp
    ${ROOT_DIR}/CardRange.h
    ${ROOT_DIR}/CombinedRange.cpp
    ${ROOT_DIR}/CombinedRange.h
    ${ROOT_DIR}/Constants.h
    ${ROOT_DIR}/EquityCalculator.cpp
    ${ROOT_DIR}/EquityCalculator.h
    ${ROOT_DIR}/Hand.h
    ${ROOT_DIR}/HandEvaluator.cpp
    ${ROOT_DIR}/HandEvaluator.h
    ${ROOT_DIR}/OffsetTable.hxx
    ${ROOT_DIR}/Random.h
    ${ROOT_DIR}/Util.h
    libdivide/libdivide.h
)

add_library(omp ${LIB_SOURCES})
