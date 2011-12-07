link_directories(
	${GTKMM_LIBRARY_DIRS}  )

include_directories(
	${GTKMM_INCLUDE_DIRS}  )

file(
	GLOB_RECURSE
	source_files
	./* )

add_executable(mkvextract-cli
	${source_files})

target_link_libraries(mkvextract-cli 
	${GTKMM_LIBRARIES}  )
