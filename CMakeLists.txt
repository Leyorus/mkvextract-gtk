project(mkvextract-gui)

find_package(PkgConfig)

pkg_check_modules(GTKMM gtkmm-2.4) # look into FindPkgConfig.cmake, 

#set(CMAKE_CXX_FLAGS "-g -Wall")

add_subdirectory(src) 
