cmake_minimum_required(VERSION 3.1)
project(gpio++ VERSION 1.1)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_definitions(-DASIO_STANDALONE)

include_directories(include)

set(HEADERS
    include/gpio++/chip.hpp
    include/gpio++/gpio.hpp
    include/gpio++/pin.hpp
    include/gpio++/types.hpp
)
set(SOURCES lib/gpio.cpp)
set(FILES LICENSE.md README.md)

# sub-project(s)
add_subdirectory(util)
set_property(TARGET gpio++-util PROPERTY POSITION_INDEPENDENT_CODE ON)

option(BUILD_CHIP "Build generic chip backend" ON)
if(BUILD_CHIP)
    add_subdirectory(generic)
endif(BUILD_CHIP)

########################
# static library
add_library(gpio++ STATIC ${HEADERS} ${SOURCES})

# install
install(TARGETS gpio++ DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)
install(FILES ${FILES} DESTINATION share/doc/${PROJECT_NAME})

########################
# package
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_STRIP_FILES ON)
set(CPACK_PACKAGE_FILE_NAME
    ${PROJECT_NAME}_${PROJECT_VERSION}_${CMAKE_SYSTEM_NAME}_${CMAKE_SYSTEM_PROCESSOR}
)

# source
set(CPACK_SOURCE_GENERATOR TBZ2)
set(CPACK_SOURCE_IGNORE_FILES /\.git.*;/build/;\.user$)

# binary
set(CPACK_GENERATOR DEB)
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "C++ GPIO Library for Linux")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Dimitry Ishenko <dimitry (dot) ishenko (at) (gee) mail (dot) com>")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libasio-dev (>= 1:1.10.10-1)")

include(CPack)
