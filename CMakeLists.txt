project(mkvextract-gui)

find_package(PkgConfig)

pkg_check_modules(GTKMM gtkmm-2.4) # look into FindPkgConfig.cmake, 

add_subdirectory(src) 
