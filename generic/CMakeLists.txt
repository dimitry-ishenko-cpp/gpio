cmake_minimum_required(VERSION 3.1)
project(gpio++-chip VERSION 1.1)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_definitions(-DASIO_STANDALONE)

include_directories(../include ../util)

set(HEADERS io_cmd.hpp chip.hpp pin.hpp)
set(SOURCES chip.cpp pin.cpp)

########################
# dynamic library
add_library(gpio++-chip SHARED
    ${HEADERS} ${SOURCES} $<TARGET_OBJECTS:gpio++-util>
)

# install
install(TARGETS gpio++-chip DESTINATION lib)
set_target_properties(gpio++-chip PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(gpio++-chip PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
