-- TODO replace with my IO
add_requires('openssl3')

add_defines("_FILE_OFFSET_BITS=64")

target("io")
	set_kind("static")
	add_files("src/io/*.c")
	add_headerfiles("include/(io/*.h)")
	add_includedirs("include", {public = true})

target("kcflib")
	set_kind("static")
	add_packages("openssl3")
	add_files("src/kcflib/*.c")
	add_headerfiles("include/(kcf/*.h)")
	add_includedirs("include", {public = true})
	add_rules("utils.install.cmake_importfiles")
	add_rules("utils.install.pkgconfig_importfiles")

