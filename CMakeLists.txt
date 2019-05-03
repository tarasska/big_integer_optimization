cmake_minimum_required(VERSION 3.12)
project(big_integer_optimization)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -lpthread")

add_executable(big_integer_optimization big_integer.h big_integer.cpp gtest/gtest.h gtest/gtest-all.cc gtest/gtest_main.cc big_integer_testing.cpp my_vector.cpp my_vector.h)

target_link_libraries(big_integer_optimization pthread)