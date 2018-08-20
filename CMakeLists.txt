cmake_minimum_required(VERSION 3.8)
project(gpio VERSION 1.0)

set(CMAKE_BUILD_TYPE Release)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(include)

set(HEADERS
    include/gpio/gpio.hpp
    include/gpio/chip.hpp
    include/gpio/pin.hpp
    include/gpio/types.hpp
)
set(SOURCES lib/gpio.cpp)

add_library(gpio STATIC ${HEADERS} ${SOURCES})

install(TARGETS gpio DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)

########################
option(BUILD_GPIOD "Build gpiod backend library" ON)
if(BUILD_GPIOD)
    add_subdirectory(gpiod)
endif(BUILD_GPIOD)
