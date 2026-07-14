cmake_minimum_required(VERSION 3.20)
project(Vrita CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

find_package(SDL3 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

set(SOURCES
    main.cpp
    debuggers/debugger.cpp
    debuggers/debugger_assembly.cpp
    debuggers/debugger_registers.cpp
    debuggers/memory_editor.cpp
    debuggers/palette_viewer.cpp
    debuggers/sprite_viewer.cpp
    debuggers/tilemap_viewer.cpp
    debuggers/tile_viewer.cpp
    emulators/agb/agb.cpp
    emulators/dmg/apu.cpp
    emulators/dmg/apu_mixer.cpp
    emulators/dmg/apu_noise.cpp
    emulators/dmg/apu_pulse.cpp
    emulators/dmg/apu_registers.cpp
    emulators/dmg/apu_wave.cpp
    emulators/dmg/cartridge.cpp
    emulators/dmg/cpu.cpp
    emulators/dmg/dmg.cpp
    emulators/dmg/cpu_extended.cpp
    emulators/dmg/cpu_normal.cpp
    emulators/dmg/interrupt.cpp
    emulators/dmg/joypad.cpp
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

target_compile_definitions(Vrita PRIVATE IMGUI_DEFINE_MATH_OPERATORS)

target_link_libraries(Vrita PRIVATE
    SDL3::SDL3
    imgui::imgui
)

if(TARGET SDL3::SDL3main)
    target_link_libraries(Vrita PRIVATE SDL3::SDL3main)
endif()

if(APPLE)
    target_link_libraries(Vrita PRIVATE "-framework Cocoa" "-framework Metal" "-framework QuartzCore")
elseif(UNIX)
    find_package(Threads REQUIRED)
    target_link_libraries(Vrita PRIVATE Threads::Threads ${CMAKE_DL_LIBS})
endif()

add_custom_command(TARGET Vrita POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/resources" "$<TARGET_FILE_DIR:Vrita>/resources"
)
