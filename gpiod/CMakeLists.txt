cmake_minimum_required(VERSION 3.1)
project(gpio++-gpiod VERSION 1.0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_definitions(-DASIO_STANDALONE)

include_directories(../include ../util)

set(HEADERS command.hpp gpiod_chip.hpp gpiod_pin.hpp)
set(SOURCES gpiod_chip.cpp gpiod_pin.cpp)

########################
# dynamic library
add_library(gpio++-gpiod SHARED
    ${HEADERS} ${SOURCES} $<TARGET_OBJECTS:gpio++-util>
)

# install
install(TARGETS gpio++-gpiod DESTINATION lib)
set_target_properties(gpio++-gpiod PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(gpio++-gpiod PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
