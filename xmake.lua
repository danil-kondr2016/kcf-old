add_defines("_FILE_OFFSET_BITS=64")

target("io")
	set_kind("static")
	add_files("io/*.c")
	add_headerfiles("include/(io/*.h)")
	add_includedirs("include", {public = true})

target("kcflib")
	set_kind("static")
	add_files("kcf/*.c")
	add_headerfiles("include/(kcf/*.h)")
	add_includedirs("include", {public = true})
	add_deps("io")

target("kcf")
	set_kind("binary")
	add_files("cmd/kcf/*.c")
	add_deps("kcflib")
