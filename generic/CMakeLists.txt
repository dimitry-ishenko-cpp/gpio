cmake_minimum_required(VERSION 3.1)
project(gpio++ VERSION 4.2)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_definitions(-DASIO_STANDALONE)

include_directories(../include ../base)

set(HEADERS io_cmd.hpp chip.hpp pin.hpp)
set(SOURCES chip.cpp pin.cpp)

########################
# dynamic library
add_library(gpio++ SHARED
    ${HEADERS} ${SOURCES} $<TARGET_OBJECTS:gpio++-base>
)

# install
include(GNUInstallDirs)

install(TARGETS gpio++ DESTINATION ${CMAKE_INSTALL_LIBDIR})
set_target_properties(gpio++ PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(gpio++ PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
