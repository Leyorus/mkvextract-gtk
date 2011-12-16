link_directories(
	${GTKMM_LIBRARY_DIRS}
	../core)

include_directories(
	${GTKMM_INCLUDE_DIRS}
	../core)

file(
	GLOB_RECURSE
	source_files
	./* )
	
add_executable(
	mkvextract-gtk
	${source_files})

target_link_libraries(
	mkvextract-gtk
	mkvextract_core
	util # for forkpty()
	${GTKMM_LIBRARIES})