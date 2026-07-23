workspace "VC.GPS"
    architecture "x86"
    configurations { "Debug", "Release" }
    
    startproject "VC.GPS"

project "VC.GPS"
    kind "SharedLib"
    language "C++"
    cppdialect "C++latest"
    targetextension ".asi"
    
    targetdir ("bin/%{cfg.buildcfg}")
    objdir ("obj/%{cfg.buildcfg}")

    files { "VC.GPS/**.cpp", "VC.GPS/**.hpp" }
    
    multiprocessorcompile "On"
    warnings "Extra"
    buildoptions { "/sdl" }
    
    filter "configurations:Debug"
        defines { "_DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
        staticruntime "On"
        floatingpoint "Fast"
