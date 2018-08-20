cmake_minimum_required(VERSION 3.8)
project(gpio-gpiod)

set(CMAKE_BUILD_TYPE Release)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(.. ../include)
set(gpiod_SOURCES gpiod_chip.cpp)
set(posix_SOURCES ../posix/error.cpp ../posix/resource.cpp)

add_library(gpio-gpiod SHARED ${gpiod_SOURCES} ${posix_SOURCES})
install(TARGETS gpio-gpiod DESTINATION lib)

