cmake_minimum_required(VERSION 3.12)
project(big_integer_optimization)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -lpthread -O3")


add_executable(big_integer_optimization shared.h shared.cpp big_integer.h big_integer.cpp  big_integer_testing.cpp my_vector.h
        my_vector.cpp test.cpp test.h gtest/gtest.h gtest/gtest-all.cc gtest/gtest_main.cc)

target_link_libraries(big_integer_optimization pthread)