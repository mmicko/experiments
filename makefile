#
# Copyright 2011-2016 Branimir Karadzic. All rights reserved.
# License: http://www.opensource.org/licenses/BSD-2-Clause
#

UNAME := $(shell uname)
ifeq ($(UNAME),$(filter $(UNAME),Linux Darwin FreeBSD GNU/kFreeBSD))
ifeq ($(UNAME),$(filter $(UNAME),Darwin))
OS=darwin
else
ifeq ($(UNAME),$(filter $(UNAME),FreeBSD GNU/kFreeBSD))
OS=bsd
else
OS=linux
endif
endif
else
OS=windows

help: projgen

endif

# $(info $(OS))

BX_DIR?=../bx
GENIE?=$(BX_DIR)/tools/bin/$(OS)/genie

.PHONY: clean projgen build help

clean: ## Clean all intermediate files.
	@echo Cleaning...
	-@rm -rf build
	@mkdir build

projgen: ## Generate project files for all configurations.
	$(GENIE) vs2015
	$(GENIE) --gcc=mingw-gcc gmake
	$(GENIE) --gcc=linux-gcc gmake
	$(GENIE) --gcc=osx gmake

.PHONY: build/projects/gmake-linux
build/projects/gmake-linux:
	$(GENIE) --gcc=linux-gcc gmake
linux-debug32: build/projects/gmake-linux ## Build - Linux x86 Debug
	$(MAKE) -R -C build/projects/gmake-linux config=debug32
linux-release32: build/projects/gmake-linux ## Build - Linux x86 Release
	$(MAKE) -R -C build/projects/gmake-linux config=release32
linux-debug64: build/projects/gmake-linux ## Build - Linux x64 Debug
	$(MAKE) -R -C build/projects/gmake-linux config=debug64
linux-release64: build/projects/gmake-linux ## Build - Linux x64 Release
	$(MAKE) -R -C build/projects/gmake-linux config=release64
linux: linux-debug32 linux-release32 linux-debug64 linux-release64 ## Build - Linux x86/x64 Debug and Release

.PHONY: build/projects/gmake-mingw-gcc
build/projects/gmake-mingw-gcc:
	$(GENIE) --gcc=mingw-gcc gmake
mingw-gcc-debug32: build/projects/gmake-mingw-gcc ## Build - MinGW GCC x86 Debug
	$(MAKE) -R -C build/projects/gmake-mingw-gcc config=debug32
mingw-gcc-release32: build/projects/gmake-mingw-gcc ## Build - MinGW GCC x86 Release
	$(MAKE) -R -C build/projects/gmake-mingw-gcc config=release32
mingw-gcc-debug64: build/projects/gmake-mingw-gcc ## Build - MinGW GCC x64 Debug
	$(MAKE) -R -C build/projects/gmake-mingw-gcc config=debug64
mingw-gcc-release64: build/projects/gmake-mingw-gcc ## Build - MinGW GCC x64 Release
	$(MAKE) -R -C build/projects/gmake-mingw-gcc config=release64
mingw-gcc: mingw-gcc-debug32 mingw-gcc-release32 mingw-gcc-debug64 mingw-gcc-release64 ## Build - MinGW GCC x86/x64 Debug and Release

.PHONY: build/projects/vs2015
build/projects/vs2015:
	$(GENIE) vs2015
vs2015-debug32: build/projects/vs2015 ## Build - VS2015 x86 Debug
	devenv build/projects/vs2015/bgfx.sln /Build "Debug|Win32"
vs2015-release32: build/projects/vs2015 ## Build - VS2015 x86 Release
	devenv build/projects/vs2015/bgfx.sln /Build "Release|Win32"
vs2015-debug64: build/projects/vs2015 ## Build - VS2015 x64 Debug
	devenv build/projects/vs2015/bgfx.sln /Build "Debug|x64"
vs2015-release64: build/projects/vs2015 ## Build - VS2015 x64 Release
	devenv build/projects/vs2015/bgfx.sln /Build "Release|x64"
vs2015: vs2015-debug32 vs2015-release32 vs2015-debug64 vs2015-release64 ## Build - VS2015 x86/x64 Debug and Release

.PHONY: build/projects/gmake-osx
build/projects/gmake-osx:
	$(GENIE) --gcc=osx gmake
osx-debug32: build/projects/gmake-osx ## Build - OSX x86 Debug
	$(MAKE) -C build/projects/gmake-osx config=debug32
osx-release32: build/projects/gmake-osx ## Build - OSX x86 Release
	$(MAKE) -C build/projects/gmake-osx config=release32
osx-debug64: build/projects/gmake-osx ## Build - OSX x64 Debug
	$(MAKE) -C build/projects/gmake-osx config=debug64
osx-release64: build/projects/gmake-osx ## Build - OSX x64 Release
	$(MAKE) -C build/projects/gmake-osx config=release64
osx: osx-debug32 osx-release32 osx-debug64 osx-release64 ## Build - OSX x86/x64 Debug and Release

build-darwin: osx-release64

build-linux: linux-release64

build-windows: mingw-gcc-release64

build: build-$(OS)

