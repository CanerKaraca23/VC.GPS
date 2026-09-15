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

    staticruntime "On"
    multiprocessorcompile "On"
    warnings "Extra"
    conformancemode "On"
    rtti "Off"
    exceptionhandling "Off"
    vectorextensions "SSE2"

    defines {
        "NOMINMAX",
        "WIN32_LEAN_AND_MEAN",
        "_USE_MATH_DEFINES"
    }

    buildoptions {
        "/utf-8",
        "/Zc:__cplusplus",
        "/Zc:preprocessor",
        "/Zc:strictStrings",
        "/Zc:inline",
        "/Zc:throwingNew",
        "/Zc:lambda",
        "/Zc:enumTypes",
        "/Zc:templateScope",
        "/Zc:ternary",
        "/Zc:rvalueCast",
        "/Zc:nrvo",
        "/Zc:sizedDealloc",
        "/Zc:checkGwOdr",
        "/Zc:alignedNew",
        "/volatile:iso",
        "/Oi"
    }

    filter "configurations:Debug"
        defines { "_DEBUG" }
        symbols "On"
        optimize "Off"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "Speed"
        stringpooling "On"
        functionlevellinking "On"
        linktimeoptimization "On"
        symbols "Off"
        omitframepointer "On"
        
        buildoptions { 
            "/Gw", 
            "/Ob3", 
            "/Ot" 
        }
        
        linkoptions { 
            "/OPT:REF", 
            "/OPT:ICF", 
            "/INCREMENTAL:NO" 
        }
