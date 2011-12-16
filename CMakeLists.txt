cmake_minimum_required(VERSION 2.6)

project(mkvextract-gtk)

find_package(PkgConfig)

pkg_check_modules(GTKMM gtkmm-2.4) # look into FindPkgConfig.cmake, 

set(CMAKE_CXX_FLAGS "-O2 -Wall")

add_subdirectory(src) 
