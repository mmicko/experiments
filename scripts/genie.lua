solution "experiments"
	configurations {
		"Debug",
		"Release",
	}

	if _ACTION == "xcode4" then
		platforms {
			"Universal",
		}
	else
		platforms {
			"x32",
			"x64",
			"Native", -- for targets where bitness is not specified
		}
	end

language "C++"
startproject "experiments"

-- Big project specific
premake.make.makefile_ignore = true
premake._checkgenerate = false
premake.check_paths = true

msgcompile ("Compiling $(subst ../,,$<)...")

msgcompile_objc ("Objective-C compiling $(subst ../,,$<)...")

msgresource ("Compiling resources $(subst ../,,$<)...")

msglinking ("Linking $(notdir $@)...")

msgarchiving ("Archiving $(notdir $@)...")

msgprecompile ("Precompiling $(subst ../,,$<)...")

messageskip { "SkipCreatingMessage", "SkipBuildingMessage", "SkipCleaningMessage" }

MODULE_DIR = path.getabsolute("../")
SRC_DIR = path.getabsolute("../src")
BGFX_DIR   = path.getabsolute("../../bgfx")
BX_DIR     = path.getabsolute("../../bx")

local BGFX_BUILD_DIR = path.join("../", "build")
local BGFX_THIRD_PARTY_DIR = path.join(BGFX_DIR, "3rdparty")

defines {
	"BX_CONFIG_ENABLE_MSVC_LEVEL4_WARNINGS=1"
}

dofile (path.join(BX_DIR, "scripts/toolchain.lua"))
if not toolchain(BGFX_BUILD_DIR, BGFX_THIRD_PARTY_DIR) then
	return -- no action specified
end

function copyLib()
end

function mainProject(name_)

	project (name_)
		uuid (os.uuid(name_))
		kind "WindowedApp"

	targetdir(MODULE_DIR)
	targetsuffix ""
	
	removeflags {
		"NoExceptions",
	} 
	configuration {}

	includedirs {
		path.join(BX_DIR,   "include"),
		path.join(BGFX_DIR, "include"),
		path.join(BGFX_DIR, "3rdparty"),
		path.join(BGFX_DIR, "examples/common"),
		path.join(SRC_DIR,  ""),
	}

	files {
		path.join(SRC_DIR, "main.cpp"),
	}

	links {
		"bgfx",
		"example-common",
	}
	configuration { "mingw*" }
		targetextension ".exe"
		links {
			"gdi32",
			"psapi",
		}

	configuration { "vs20*", "x32 or x64" }
		links {
			"gdi32",
			"psapi",
		}

	configuration { "mingw-clang" }
		kind "ConsoleApp"

	configuration { "linux-*" }
		links {
			"X11",
			"GL",
			"pthread",
		}

	configuration { "osx" }
		linkoptions {
			"-framework Cocoa",
			"-framework QuartzCore",
			"-framework OpenGL",
			"-weak_framework Metal",
		}

	configuration {}

	strip()
end

dofile (path.join(BGFX_DIR, "scripts", "bgfx.lua"))

group "common"
dofile (path.join(BGFX_DIR, "scripts", "example-common.lua"))

group "libs"
bgfxProject("", "StaticLib", {})

group "main"
mainProject("experiments")

