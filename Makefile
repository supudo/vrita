CONFIG ?= Debug
PRESET ?=

ifeq ($(PRESET),)
ifeq ($(OS),Windows_NT)
PRESET := windows
else ifeq ($(shell uname -s),Darwin)
PRESET := macos-arm64
else
PRESET := linux
endif
endif

.PHONY: configure build rebuild all clean

configure:
	cmake --preset $(PRESET)

build: configure
	cmake --build --preset $(PRESET) --config $(CONFIG)

rebuild: clean build

all: configure
	cmake --build --preset $(PRESET) --config Debug
	cmake --build --preset $(PRESET) --config Release

clean:
	cmake -E rm -rf build/$(PRESET)
