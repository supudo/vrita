cmake_minimum_required(VERSION 3.20)
project(Vrita CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL3 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

set(SOURCES
    main.cpp
    emulators/agb/agb.cpp
    emulators/debuggers/memory_editor.cpp
    emulators/debuggers/palette_viewer.cpp
    emulators/debuggers/sprite_viewer.cpp
    emulators/debuggers/tilemap_viewer.cpp
    emulators/debuggers/tile_viewer.cpp
    emulators/dmg/apu.cpp
    emulators/dmg/cartridge.cpp
    emulators/dmg/cpu.cpp
    emulators/dmg/dmg.cpp
    emulators/dmg/cpu_extended.cpp
    emulators/dmg/cpu_normal.cpp
    emulators/dmg/interrupt.cpp
    emulators/dmg/mmu.cpp
    emulators/dmg/ppu.cpp
    emulators/dmg/timer.cpp
    emulators/emulators.cpp
    eyecandy/dots.cpp
    gui/filebrowser.cpp
    gui/log.cpp
)

add_executable(Vrita ${SOURCES})

target_include_directories(Vrita PRIVATE ${CMAKE_SOURCE_DIR})

target_link_libraries(Vrita PRIVATE
    SDL3::SDL3
    imgui::imgui
)

if(TARGET SDL3::SDL3main)
    target_link_libraries(Vrita PRIVATE SDL3::SDL3main)
endif()
