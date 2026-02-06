-- ORCA Framework Build Configuration
-- This premake5 script replaces both the Makefile and Xcode project

workspace "orca"
    configurations { "Debug", "Release" }
    platforms { "native" }
    location "build/premake"
    
    -- Global settings
    language "C"
    architecture "x86_64"
    
    -- Build directories
    targetdir "build/bin"
    objdir "build/obj/%{prj.name}"
    
    -- Compiler settings
    filter "system:macosx"
        toolset "clang"
    filter "system:linux"
        toolset "gcc"
    filter {}
    
    -- Common flags
    buildoptions { 
        "-fpic",
        "-I.",
        "`pkg-config --cflags libjpeg freetype2 libxml-2.0 2>/dev/null`"
    }
    
    -- Configuration specific settings
    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"
        optimize "Off"
        
    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "Speed"
        symbols "Off"
        
    filter {}

-- Geometry module
project "orca.geometry"
    kind "SharedLib"
    targetname "orca.geometry"
    targetextension ".so"
    targetprefix "lib"
    targetdir "build/lib"
    files {
        "source/geometry/geom_ops.c",
        "source/geometry/geom_export.c",
        "source/geometry/geom.h"
    }

-- Filesystem module
project "orca.filesystem"
    kind "SharedLib"
    targetname "orca.filesystem"
    targetextension ".so"
    targetprefix "lib"
    targetdir "build/lib"
    files {
        "source/filesystem/**.c",
        "source/filesystem/**.h"
    }
    links { "orca.geometry" }

-- Renderer module
project "orca.renderer"
    kind "SharedLib"
    targetname "orca.renderer"
    targetextension ".so"
    targetprefix "lib"
    targetdir "build/lib"
    files {
        "source/renderer/**.c",
        "source/renderer/**.h"
    }
    excludes {
        "source/renderer/builtin/**"
    }
    includedirs {
        "source/renderer",
        "source/renderer/api"
    }
    links { 
        "orca.filesystem",
        "orca.geometry"
    }
    linkoptions {
        "`pkg-config --libs freetype2 libjpeg libpng 2>/dev/null`"
    }
    
    filter "system:macosx"
        links { "OpenGL.framework", "IOSurface.framework" }
    filter "system:linux"
        links { "GL" }
    filter {}

-- Core module
project "orca.core"
    kind "SharedLib"
    targetname "orca.core"
    targetextension ".so"
    targetprefix "lib"
    targetdir "build/lib"
    files {
        "source/core/**.c",
        "source/core/**.h"
    }
    links { "orca.geometry" }

-- Platform module
project "orca.platform"
    kind "SharedLib"
    targetname "orca.platform"
    targetextension ".so"
    targetprefix "lib"
    targetdir "build/lib"
    files {
        "source/platform/**.c",
        "source/platform/**.h"
    }

-- System utilities module  
project "orca.sysutil"
    kind "SharedLib"
    targetname "orca.sysutil"
    targetextension ".so"
    targetprefix "lib"
    targetdir "build/lib"
    files {
        "source/sysutil/**.c",
        "source/sysutil/**.h"
    }
    links { "orca.platform" }

-- Backend module
project "orca.backend"
    kind "SharedLib"
    targetname "orca.backend"
    targetextension ".so"
    targetprefix "lib"
    targetdir "build/lib"
    files {
        "source/backend/**.c",
        "source/backend/**.h"
    }

-- Parsers module
project "orca.parsers"
    kind "SharedLib"
    targetname "orca.parsers"
    targetextension ".so"
    targetprefix "lib"
    targetdir "build/lib"
    files {
        "source/parsers/**.c",
        "source/parsers/**.h"
    }
    links { "orca.core" }
    linkoptions {
        "`pkg-config --libs libxml-2.0 2>/dev/null`"
    }

-- Network module
project "orca.network"
    kind "SharedLib"
    targetname "orca.network"
    targetextension ".so"
    targetprefix "lib"
    targetdir "build/lib"
    files {
        "source/network/**.c",
        "source/network/**.h"
    }
    links { "curl" }

-- Localization module
project "orca.localization"
    kind "SharedLib"
    targetname "orca.localization"
    targetextension ".so"
    targetprefix "lib"
    targetdir "build/lib"
    files {
        "source/localization/**.c",
        "source/localization/**.h"
    }
    links { "orca.filesystem" }

-- SceneKit module (3D scene)
project "orca.scene"
    kind "SharedLib"
    targetname "orca.scene"
    targetextension ".so"
    targetprefix "lib"
    targetdir "build/lib"
    files {
        "source/SceneKit/**.c",
        "source/SceneKit/**.h"
    }
    includedirs { "source/SceneKit" }
    links { 
        "orca.geometry",
        "orca.renderer"
    }

-- UIKit module (2D UI)
project "orca.ui"
    kind "SharedLib"
    targetname "orca.ui"
    targetextension ".so"
    targetprefix "lib"
    targetdir "build/lib"
    files {
        "source/UIKit/**.c",
        "source/UIKit/**.h"
    }
    links { 
        "orca.core",
        "orca.geometry",
        "orca.renderer"
    }

-- Debug module
project "orca.debug"
    kind "SharedLib"
    targetname "orca.debug"
    targetextension ".so"
    targetprefix "lib"
    targetdir "build/lib"
    files {
        "source/debug/**.c",
        "source/debug/**.h"
    }

-- VSomeIP module
project "orca.vsomeip"
    kind "SharedLib"
    targetname "orca.vsomeip"
    targetextension ".so"
    targetprefix "lib"
    targetdir "build/lib"
    files {
        "source/vsomeip/**.c",
        "source/vsomeip/**.h"
    }
    links { "orca.filesystem" }

-- Server module
project "orca.server"
    kind "SharedLib"
    targetname "orca.server"
    targetextension ".so"
    targetprefix "lib"
    targetdir "build/lib"
    files {
        "source/server/**.c",
        "source/server/**.h"
    }

-- Editor module
project "orca.editor"
    kind "SharedLib"
    targetname "orca.editor"
    targetextension ".so"
    targetprefix "lib"
    targetdir "build/lib"
    files {
        "source/editor/**.c",
        "source/editor/**.h"
    }
    links { 
        "orca.filesystem",
        "orca.renderer",
        "orca.backend",
        "orca.geometry",
        "orca.core",
        "orca.ui",
        "orca.localization"
    }

-- Main executable
project "orca"
    kind "ConsoleApp"
    targetname "orca"
    targetdir "build/bin"
    files {
        "source/orca.c"
    }
    includedirs {
        ".",
        "source"
    }
    links {
        "orca.geometry",
        "orca.platform",
        "orca.sysutil",
        "orca.core",
        "orca.filesystem",
        "orca.renderer",
        "orca.ui",
        "orca.scene",
        "orca.backend",
        "orca.editor",
        "orca.localization",
        "orca.parsers",
        "orca.network",
        "orca.vsomeip",
        "orca.debug",
        "orca.server",
        "dl",
        "pthread",
        "z",
        "lz4",
        "curl"
    }
    linkoptions {
        "`pkg-config --libs lua5.4 freetype2 libjpeg libpng libxml-2.0 2>/dev/null`"
    }
    
    filter "system:macosx"
        links { "OpenGL.framework", "IOSurface.framework" }
        linkoptions { "-Wl,-rpath,@executable_path/../../build/lib" }
    filter "system:linux"
        links { "GL" }
        linkoptions { "-Wl,-rpath,'$$ORIGIN/../../build/lib'" }
    filter {}

-- Lua library (optional)
project "orcalib"
    kind "SharedLib"
    targetname "orca"
    targetextension ".so"
    targetprefix "lib"
    targetdir "build/lib/lua/5.4"
    files {
        "source/orca/orcalib.c"
    }
    links {
        "orca.geometry",
        "orca.core",
        "orca.filesystem",
        "orca.renderer"
    }
    linkoptions {
        "`pkg-config --libs lua5.4 2>/dev/null`"
    }

-- pz2 tool (optional)
project "pz2"
    kind "ConsoleApp"
    targetname "pz2"
    targetdir "build/bin"
    files {
        "source/tools/pz2.c"
    }
    links {
        "orca.filesystem",
        "z",
        "lz4"
    }
    linkoptions {
        "`pkg-config --libs lua5.4 2>/dev/null`"
    }

-- FBX importer tool (optional, requires FBX SDK)
project "orca-fbx"
    kind "ConsoleApp"
    targetname "orca-fbx"
    targetdir "build/bin"
    language "C++"
    files {
        "source/tools/fbx-import.cpp"
    }
    includedirs {
        "/Applications/Autodesk/FBX SDK/2020.2/include"
    }
    libdirs {
        "/Applications/Autodesk/FBX SDK/2020.2/lib/clang/release"
    }
    links {
        "fbxsdk"
    }
    
-- Helper targets
newaction {
    trigger = "run",
    description = "Build and run the application",
    execute = function()
        os.execute("make config=debug && build/bin/orca data")
    end
}

newaction {
    trigger = "modules",
    description = "Generate module bindings",
    execute = function()
        os.execute([[python3 tools/conv-module.py \
            source/geometry/geom.xml \
            source/core/core.xml \
            source/renderer/api/renderer.xml \
            source/UIKit/UIKit.xml \
            source/SceneKit/SceneKit.xml \
            source/filesystem/filesystem.xml]])
    end
}

newaction {
    trigger = "clean-all",
    description = "Clean all build artifacts",
    execute = function()
        os.execute("rm -rf build")
    end
}