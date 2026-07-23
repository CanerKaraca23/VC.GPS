workspace "VC.GPS"
    architecture "x86"
    configurations { "Debug", "Release" }

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

    filter "configurations:Debug"
        defines { "_DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "Speed"
        floatingpoint "Fast"
        linktimeoptimization "On"
        vectorextensions "AVX2"
        largeaddressaware "On"
        rtti "Off"
        exceptionhandling "Off"
        symbols "Off"
        omitframepointer "On"
        buildoptions { "/Gw", "/Qpar", "/Zc:preprocessor" }
