cmake_minimum_required(VERSION 3.9)

project(static_containers)

set(CMAKE_CXX_STANDARD 17)

add_library(static_containers INTERFACE)

include(GNUInstallDirs)

target_include_directories(
    static_containers INTERFACE
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

install(
    TARGETS static_containers
    EXPORT static_containers-config)

install(
    EXPORT static_containers-config
    NAMESPACE static_containers::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/static_containers)

install(
    FILES    static_containers/include/stc/common.hpp    static_containers/include/stc/static_map.hpp    static_containers/include/stc/static_vector.hpp
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/stc)
