cmake_minimum_required(VERSION 3.8)
project(gpio++-gpiod VERSION 1.0)

set(CMAKE_BUILD_TYPE Release)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(.. ../include)

set(gpiod_HEADERS gpiod_chip.hpp gpiod_pin.hpp)
set(gpiod_SOURCES gpiod_chip.cpp gpiod_pin.cpp)

set(posix_HEADERS ../posix/error.hpp ../posix/resource.hpp)
set(posix_SOURCES ../posix/error.cpp ../posix/resource.cpp)

add_library(gpio++-gpiod SHARED
    ${gpiod_HEADERS} ${gpiod_SOURCES}
    ${posix_HEADERS} ${posix_SOURCES}
)

install(TARGETS gpio++-gpiod DESTINATION lib)
set_target_properties(gpio++-gpiod PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(gpio++-gpiod PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
