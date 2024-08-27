add_defines("_FILE_OFFSET_BITS=64")

target("kcflib")
	set_kind("static")
	add_packages("io")
	add_files("src/kcflib/*.c")
	add_headerfiles("include/(kcf/*.h)")
	add_includedirs("include", {public = true})
	add_includedirs("io/include")
	add_rules("utils.install.cmake_importfiles")
	add_rules("utils.install.pkgconfig_importfiles")

