add_subdirectory(yamka) 

find_package(PkgConfig)

pkg_check_modules(GTKMM gtkmm-2.4) # look into FindPkgConfig.cmake, 

set(CMAKE_CXX_FLAGS "-O2 -Wall")


file(GLOB_RECURSE core_source_files	core/* )

file(GLOB_RECURSE gui_source_files gtk-gui/* )

file(GLOB_RECURSE info_source_files info/* common/*)

file(GLOB_RECURSE merge_source_files merge/* common/*)
	
link_directories(${GTKMM_LIBRARY_DIRS})
include_directories(. yamka ${GTKMM_INCLUDE_DIRS})

add_executable(
	${PROJECT_NAME}
	${core_source_files} 
	${gui_source_files})

target_link_libraries(
	${PROJECT_NAME}
	${GTKMM_LIBRARIES}
	yamka
	util # usefull for forkpty() function
	)

install(TARGETS 
  ${PROJECT_NAME}
  DESTINATION bin
  )

#set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++0x")

# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)