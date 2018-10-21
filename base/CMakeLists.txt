cmake_minimum_required(VERSION 3.1)
project(gpio++-base VERSION 2.0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_definitions(-DASIO_STANDALONE)

include_directories(../include)

set(HEADERS chip_base.hpp pin_base.hpp type_id.hpp)
set(SOURCES chip_base.cpp pin_base.cpp)

########################
# object files
add_library(gpio++-base OBJECT ${HEADERS} ${SOURCES})
