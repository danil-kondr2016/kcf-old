workspace "TheKCFProject"
	configurations { "Debug", "Release" }

project "kcflib"
	kind "StaticLib"
	language "C"
	files {
		"archive.h",
		"bytepack.h",
		"crc32c.h",
		"errors.h",
		"kcf_impl.h",
		"pack.h",
		"record.h",
		"unpack.h",
		"utils.h",
		"archive.c",
		"archhdr.c",
		"bytepack.c",
		"crc32c.c",
		"errors.c",
		"filehdr.c",
		"marker.c",
		"pack.c",
		"read.c",
		"record.c",
		"skip.c",
		"unpack.c",
		"write.c",
	}

	filter { "configurations:Debug" }
		defines { "DEBUG", "_KCF_TRACE" }
		symbols "On"
	
	filter { "configurations:Release" }
		defines { "NDEBUG" }
		optimize "On"

project "kcf"
	kind "ConsoleApp"
	language "C"
	files { "main.c" }
	links { "kcflib" }

	filter { "configurations:Debug" }
		defines { "DEBUG" }
		symbols "On"
	
	filter { "configurations:Release" }
		defines { "NDEBUG" }
		optimize "On"


