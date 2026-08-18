cmake_minimum_required(VERSION 3.20)
project(Vrita CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

find_package(SDL2 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

if(NOT APPLE)
    find_package(imgui CONFIG REQUIRED)
else()
    # No vcpkg on macOS build machines (just cmake + compilers) - fetch imgui
    # source at configure time instead of vendoring the full tree in-repo.
    include(FetchContent)
    FetchContent_Declare(imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG v1.92.8
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(imgui)

    add_library(imgui_vendored STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
    )
    target_include_directories(imgui_vendored PUBLIC ${imgui_SOURCE_DIR})
    add_library(imgui::imgui ALIAS imgui_vendored)
endif()

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
    third_party/imgui/imgui_impl_sdl2.cpp
    third_party/imgui/imgui_impl_opengl3.cpp
    third_party/ImGuiColorTextEdit/TextEditor.cpp
    third_party/ImGuiColorTextEdit/TextDiff.cpp
)

if(WIN32)
    enable_language(RC)
    list(APPEND SOURCES resources/Vrita.rc)
elseif(APPLE)
    list(APPEND SOURCES resources/Vrita.icns)
    set_source_files_properties(resources/Vrita.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
endif()

add_executable(Vrita ${SOURCES})

target_precompile_headers(Vrita PRIVATE pch.h)

if(APPLE)
    set_target_properties(Vrita PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_ICON_FILE Vrita.icns
        MACOSX_BUNDLE_BUNDLE_NAME Vrita
        MACOSX_BUNDLE_GUI_IDENTIFIER com.vrita.app
    )
endif()

target_include_directories(Vrita PRIVATE ${CMAKE_SOURCE_DIR})

target_compile_definitions(Vrita PRIVATE IMGUI_DEFINE_MATH_OPERATORS IMGUI_IMPL_OPENGL_LOADER_CUSTOM)

target_link_libraries(Vrita PRIVATE
    SDL2::SDL2
    OpenGL::GL
    GLEW::GLEW
    imgui::imgui
)

if(TARGET SDL2::SDL2main)
    target_link_libraries(Vrita PRIVATE SDL2::SDL2main)
endif()

if(APPLE)
    target_link_libraries(Vrita PRIVATE "-framework Cocoa" "-framework QuartzCore")
elseif(UNIX)
    find_package(Threads REQUIRED)
    target_link_libraries(Vrita PRIVATE Threads::Threads ${CMAKE_DL_LIBS})
endif()

add_custom_command(TARGET Vrita POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/resources" "$<TARGET_FILE_DIR:Vrita>/resources"
)
