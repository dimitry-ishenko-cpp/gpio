cmake_minimum_required(VERSION 3.1)
project(gpio++ VERSION 4.2)

set(CMAKE_BUILD_TYPE Release)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(include)

set(HEADERS
    include/gpio++/gpio.hpp
    include/gpio++/chip.hpp
    include/gpio++/pin.hpp
    include/gpio++/types.hpp
)
set(SOURCES lib/gpio.cpp)

add_definitions(-DASIO_STANDALONE)
add_library(gpio++ STATIC ${HEADERS} ${SOURCES})

install(TARGETS gpio++ DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)

########################
add_subdirectory(util)
set_property(TARGET gpio++-util PROPERTY POSITION_INDEPENDENT_CODE ON)

option(BUILD_GPIOD "Build gpiod backend" ON)
if(BUILD_GPIOD)
    add_subdirectory(gpiod)
endif(BUILD_GPIOD)

########################
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})

set(CPACK_GENERATOR DEB)
set(CPACK_STRIP_FILES ON)
set(CPACK_PACKAGE_FILE_NAME
    ${PROJECT_NAME}_${PROJECT_VERSION}_${CMAKE_SYSTEM_NAME}_${CMAKE_SYSTEM_PROCESSOR}
)
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "C++ GPIO Library for Linux")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Dimitry Ishenko <dimitry (dot) ishenko (at) (gee) mail (dot) com>")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libasio-dev (>= 1:1.12.1-1)")

set(CPACK_SOURCE_GENERATOR ZIP)
set(CPACK_SOURCE_IGNORE_FILES /\.git.*;/build/;\.user$)

include(CPack)
