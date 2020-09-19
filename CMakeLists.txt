cmake_minimum_required(VERSION 3.1)
project(gpio++ VERSION 4.1)

set(HEADERS
    include/gpio++/chip.hpp
    include/gpio++/pin.hpp
    include/gpio++/types.hpp
    include/gpio++.hpp
)
set(FILES LICENSE.md README.md)

# sub-project(s)
add_subdirectory(base)
set_property(TARGET gpio++-base PROPERTY POSITION_INDEPENDENT_CODE ON)

# build generic backend
add_subdirectory(generic)

option(BUILD_PIGPIO "Build pigpio backend" ON)
if(BUILD_PIGPIO)
    add_subdirectory(pigpio)
endif(BUILD_PIGPIO)

########################
# install
install(DIRECTORY include/ DESTINATION include)
install(FILES ${FILES} DESTINATION share/doc/${PROJECT_NAME}-${VERSION})

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
set(CPACK_DEBIAN_PACKAGE_RECOMMENDS "pigpio")

include(CPack)
