add_subdirectory(yamka) 

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
	
#SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

target_link_libraries(
	${PROJECT_NAME}
	${GTKMM_LIBRARIES}
	yamka
	util # usefull for forkpty() function
	)

#set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++0x")