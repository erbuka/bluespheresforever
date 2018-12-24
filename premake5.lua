workspace "BlueSpheresForever"
    configurations { "Debug", "Release" }
    architecture "x86_64"
    system "windows"
    startproject "Launcher"

    
    filter "configurations:Debug"
        symbols "On"
        optimize "Off"

    filter "configurations:Release"
        optimize "On"
        symbols "Off"


project "LevelEditor"
    kind "WindowedApp"
    language "C#"
    location "LevelEditor"

    objdir "bin-int/%{cfg.buildcfg}/%{prj.name}"
    targetdir "bin/%{cfg.buildcfg}/BlueSpheresForever"
    debugdir "bin/%{cfg.buildcfg}/BlueSpheresForever"    

    links { 
        "System", 
        "System.Windows.Forms",
        "System.Drawing", 
        "System.Data"
    }    

    files { "%{prj.name}/**" }

    filter "files:%{prj.name}/Resource/**"
        buildaction "Resource"
    

project "Launcher"
    kind "WindowedApp"
    language "C#"
    location "Launcher"

    objdir "bin-int/%{cfg.buildcfg}/%{prj.name}"
    targetdir "bin/%{cfg.buildcfg}/BlueSpheresForever"
    debugdir "bin/%{cfg.buildcfg}/BlueSpheresForever"

    links { 
        "System", 
        "System.Windows.Forms",
        "System.Drawing"
    }    

    files { "%{prj.name}/**" }

    filter "files:%{prj.name}/Resource/**"
        buildaction "Resource"

project "BlueSpheresForever"
    kind "WindowedApp"
    language "C++"
    location "BlueSpheresForever"
    characterset "MBCS"

    objdir "bin-int/%{cfg.buildcfg}/%{prj.name}"
    targetdir "bin/%{cfg.buildcfg}/%{prj.name}"
    debugdir "bin/%{cfg.buildcfg}/%{prj.name}"

    files { "%{prj.name}/**.h", "%{prj.name}/**.cpp" }
    includedirs { "vendor/glew/include", "vendor/bass/include" }
    libdirs { "vendor/glew/lib", "vendor/bass/lib" }

    links { "glew32", "bass", "bass_fx", "opengl32" }

    postbuildcommands {
        "{COPY} res/ ../bin/%{cfg.buildcfg}/%{prj.name}/res",
        "{COPY} ../vendor/glew/bin/glew32.dll  ../bin/%{cfg.buildcfg}/%{prj.name}/", 
        "{COPY} ../vendor/bass/bin/bass.dll  ../bin/%{cfg.buildcfg}/%{prj.name}/", 
        "{COPY} ../vendor/bass/bin/bass_fx.dll  ../bin/%{cfg.buildcfg}/%{prj.name}/", 
    }

    filter "system:windows"
        systemversion "latest"    

    filter "configurations:Debug"
        defines "DEBUG"
    
    filter "configurations:Release"
        defines "NDEBUG"
    
    
