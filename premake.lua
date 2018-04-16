local function prequire(m) 
    local ok, err = pcall(require, m) 
    if not ok then return nil, err end
    return err
end

local config, err = prequire "premake_config"
if not config then
    print("Error: could not find premake_config.lua. See README.md for setup information.")
	print(err)
    os.exit()
end
print("Loaded premake_config module")

workspace "encosys"
    configurations { "Debug", "Release" }
    platforms { "Win32", "Win64" }
    location "build"

project "encosys"
    kind "StaticLib"
    language "C++"
    location "build"
    targetdir "bin/%{cfg.buildcfg}"
    includedirs { config.includedirs, "include/encosys/" }
    defines { config.defines }

    files { "include/**.h", "source/**.cpp" }

    filter "configurations:Debug"
        symbols "On"
        defines { "DEBUG", "SFML_STATIC" }
        links { "sfml-graphics-s-d", "sfml-window-s-d", "sfml-system-s-d", "opengl32", "freetype", "jpeg", "winmm", "gdi32" }

    filter "configurations:Release"
        optimize "On"
        defines { "NDEBUG", "SFML_STATIC" }
        links { "sfml-graphics-s", "sfml-window-s", "sfml-system-s", "opengl32", "freetype", "jpeg", "winmm", "gdi32" }

    filter { "platforms:Win32" }
        system "Windows"
        architecture "x32"
        syslibdirs { config.syslibdirs32 }

    filter { "platforms:Win64" }
        system "Windows"
        architecture "x64"
        syslibdirs { config.syslibdirs64 }