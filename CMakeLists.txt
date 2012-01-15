cmake_minimum_required(VERSION 2.6)

project(mkvextract-gtk)

find_package(PkgConfig)

pkg_check_modules(GTKMM gtkmm-3.0) # look into FindPkgConfig.cmake, 

set(CMAKE_CXX_FLAGS "-O2 -Wall")

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

add_subdirectory(src) 
