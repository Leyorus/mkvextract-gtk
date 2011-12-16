
file(
	GLOB_RECURSE
	source_files
	./* )
	
add_executable(
	tests_pty
	${source_files})
	
target_link_libraries(
	tests_pty
	util
)