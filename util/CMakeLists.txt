cmake_minimum_required(VERSION 3.1)
project(gpio++-util VERSION 4.0)

set(CMAKE_BUILD_TYPE Release)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(../include)

set(HEADERS chip_base.hpp pin_base.hpp type_id.hpp)
set(SOURCES chip_base.cpp pin_base.cpp)

add_library(gpio++-util OBJECT ${HEADERS} ${SOURCES})
