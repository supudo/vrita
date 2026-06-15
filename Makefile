CONFIG   ?= Debug
PLATFORM ?= x64
PROJECT  := Vrita.vcxproj

MSBUILD := $(shell cmd /c ""%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe")
OUTDIR      = build\$(1)\$(2)\\
INTDIR      = build\intermediate\$(1)\$(2)\\
MSFLAGS     = /m /nologo

define msbuild_build
	"$(MSBUILD)" $(PROJECT) /p:Configuration=$(1) /p:Platform=$(2) \
		/p:OutDir=$(call OUTDIR,$(1),$(2)) \
		/p:IntDir=$(call INTDIR,$(1),$(2)) \
		$(MSFLAGS)
endef

define msbuild_clean
	"$(MSBUILD)" $(PROJECT) /t:Clean /p:Configuration=$(1) /p:Platform=$(2) \
		/p:OutDir=$(call OUTDIR,$(1),$(2)) \
		/p:IntDir=$(call INTDIR,$(1),$(2)) \
		$(MSFLAGS)
endef

.PHONY: build build-win64 build-win32 all clean

build:
	$(call msbuild_build,$(CONFIG),$(PLATFORM))

build-win64:
	$(call msbuild_build,$(CONFIG),x64)

buil-wind32:
	$(call msbuild_build,$(CONFIG),Win32)

all:
	$(call msbuild_build,Debug,x64)
	$(call msbuild_build,Release,x64)
	$(call msbuild_build,Debug,Win32)
	$(call msbuild_build,Release,Win32)

clean:
	$(call msbuild_clean,Debug,x64)
	$(call msbuild_clean,Release,x64)
	$(call msbuild_clean,Debug,Win32)
	$(call msbuild_clean,Release,Win32)
	if exist build rmdir /s /q build
