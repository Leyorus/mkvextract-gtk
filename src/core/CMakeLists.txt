
file(
	GLOB_RECURSE
	core_source_files
	./* )
	
add_library(mkvextract_core STATIC ${core_source_files})