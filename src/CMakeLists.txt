file(GLOB_RECURSE core_source_files	core/* )

file(GLOB_RECURSE gui_source_files gui/* )
	
link_directories(${GTKMM_LIBRARY_DIRS})
include_directories(core ${GTKMM_INCLUDE_DIRS})

add_executable(
	${PROJECT_NAME}
	${core_source_files} 
	${gui_source_files})
	
#SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

target_link_libraries(
	${PROJECT_NAME}
	${GTKMM_LIBRARIES}
	util # usefull for forkpty() function
	)

# add_subdirectory(tests)