workspace "TheKCFProject"
	configurations { "Debug", "Release" }
	platforms { "x86", "x86_64" }

	filter "platforms:x86"
		architecture "x86"
	filter "platforms:x86_64"
		architecture "x86_64"

	filter "system:windows"
		defines { "KCF_WINDOWS" }
	filter "system:linux"
		defines { "KCF_LINUX", "KCF_UNIX" }
	filter "system:android"
		defines { "KCF_ANDROID", "KCF_LINUX", "KCF_UNIX" }
	filter "system:bsd"
		defines { "KCF_BSD", "KCF_UNIX" }
	filter "system:macosx"
		defines { "KCF_MACOSX", "KCF_UNIX" }
	filter "system:solaris"
		defines { "KCF_SOLARIS", "KCF_UNIX" }

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


