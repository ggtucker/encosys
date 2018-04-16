workspace "encosys"
    configurations { "Debug", "Release" }
    platforms { "Win32", "Win64" }
    location "build"

project "encosys"
    kind "StaticLib"
    language "C++"
    location "build"
    targetdir "bin/%{cfg.buildcfg}"
    includedirs { "include/encosys/" }
    defines { "ENCOSYS_DISABLE_INCLUDE_ECSCONFIG_H" }

    files { "include/**.h", "source/**.cpp" }

    filter "configurations:Debug"
        symbols "On"
        defines { "DEBUG" }

    filter "configurations:Release"
        optimize "On"
        defines { "NDEBUG" }

    filter { "platforms:Win32" }
        system "Windows"
        architecture "x32"

    filter { "platforms:Win64" }
        system "Windows"
        architecture "x64"
