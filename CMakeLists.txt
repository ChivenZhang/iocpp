cmake_minimum_required (VERSION 3.8)
project(iocpp)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(IOCPP_ENABLE_TESTS ON CACHE BOOL "Enable tests")

add_library(iocpp STATIC
	iocpp.h
	iocpp.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

 if(IOCPP_ENABLE_TESTS)
 add_executable(iocpp_test
	tests/tests.cpp
 )
 target_link_libraries(iocpp_test iocpp)
 endif()