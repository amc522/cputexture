print("Generating gpuformat project...")
os.execute(_PREMAKE_COMMAND .. " --file=../../thirdparty/gpuformat/projects/premake/premake5.lua " .. _ACTION)
    
workspace "cputex"
    language "C++"
    cppdialect "c++latest"

    warnings "Extra"

    configurations {
        "Debug",
        "Release",
    }

    platforms {
        "x64"
    }

    flags {
        "NoMinimalRebuild",
        "MultiProcessorCompile"
    }

    characterset "Unicode"

    location("../" .. _ACTION)

    filter {"action:vs*"}
        disablewarnings {"4201"} -- nonstandard extension used : nameless struct/union

    filter {"action:vs*" , "configurations:Debug*"}
        buildoptions {"/JMC"}
    
    filter {"platforms:x64"}
        architecture "x64"

    filter {"configurations:Debug*"}
        runtime "Debug"
        optimize "Off"
        symbols "FastLink"
        
    filter {"configurations:Release*"}
        defines {
            "NDEBUG",
            "_NDEBUG",
        }

        flags {
            "LinkTimeOptimization",
        }

        runtime "Release"
        optimize "Speed"
        symbols "Off"

    filter {"system:windows"}
        defines {
            "WIN32_LEAN_AND_MEAN",
            "NOMINMAX"
        }

    filter {}

    local outputTriplet = "%{cfg.system}_%{cfg.shortname}"
    local binaryPath = "../../build/bin/" .. outputTriplet .. "/"
    local intermediateDir = "../../build/intermediate/" .. outputTriplet

    externalproject "gpufmt"
        location "../../thirdparty/gpuformat/projects/vs2019"
        uuid "B8FE4E00-A4A0-79D6-8D5B-8D2A799C0027"
        kind "StaticLib"
        language "C++"

    project "cputex"
        kind "StaticLib"

        files {
            "../../include/**.h",
            "../../src/*.cpp",
        }
        
        includedirs {
            "../../include",
            "../../thirdparty/gl",
            "../../thirdparty/glm/include",
            "../../thirdparty/gpuformat/include",
            "../../thirdparty/khr",
            "../../thirdparty/span/include"
        }

        filter {"configurations:Debug*"}
            libdirs {
                "../../thirdparty/gpuformat/build/bin/windows_debug_x64"
            }

        filter {"configurations:Release*"}
            libdirs {
                "../../thirdparty/gpuformat/build/bin/windows_release_x64"
            }

        filter {}

        links {
            "gpufmt",
        }

    project "cputex_test"
        kind "ConsoleApp"

        files {
            "../../test/*.cpp",
        }

        includedirs {
            "../../include",
            "../../thirdparty/gl",
            "../../thirdparty/glm/include",
            "../../thirdparty/gpuformat/include",
            "../../thirdparty/khr",
            "../../thirdparty/span/include"
        }

        filter {}

        links {
            "cputex",
            "gpufmt",
        }

        targetdir(binaryPath)
        objdir(intermediateDir)

        filter{}