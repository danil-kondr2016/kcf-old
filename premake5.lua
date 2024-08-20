workspace "TheKCFProject"
	configurations { "Debug", "Release" }

	filter "system:windows"
		defines { "KCF_WINDOWS" }

project "kcflib"
	kind "StaticLib"
	language "C"
	includedirs { "include/kcflib" }
	files {
		"include/kcflib/*.h",
		"src/kcflib/*.h",
		"src/kcflib/*.c"
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
	files { "src/kcf/main.c" }
	includedirs { "include/kcflib" }
	links { "kcflib" }

	filter { "configurations:Debug" }
		defines { "DEBUG" }
		symbols "On"
	
	filter { "configurations:Release" }
		defines { "NDEBUG" }
		optimize "On"


