cmake_minimum_required(VERSION 3.1)
project(gpio++-gpiod VERSION 1.1)

set(CMAKE_BUILD_TYPE Release)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(../include ../util)

set(HEADERS gpiod_chip.hpp gpiod_pin.hpp)
set(SOURCES gpiod_chip.cpp gpiod_pin.cpp)

add_subdirectory(../util util)
set_property(TARGET gpio++-util PROPERTY POSITION_INDEPENDENT_CODE ON)

add_library(gpio++-gpiod SHARED
    ${HEADERS} ${SOURCES} $<TARGET_OBJECTS:gpio++-util>
)

install(TARGETS gpio++-gpiod DESTINATION lib)
set_target_properties(gpio++-gpiod PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(gpio++-gpiod PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
