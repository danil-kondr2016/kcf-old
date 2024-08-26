add_defines("_FILE_OFFSET_BITS=64")

package("io")
	add_files("src/*.c")
	add_headerfiles("include/io/*.h")
	add_includedirs("include")
