#ifndef VRITA_LOG_INCLUDES
#define VRITA_LOG_INCLUDES

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>

class Log {
public:
    void clear();
    void addToLog(const char* fmt, ...) IM_FMTARGS(2);
    void render(bool* p_opened = nullptr);

    ImGuiTextBuffer Buf;
    ImGuiTextFilter Filter;
    ImVector<int> LineOffsets;
    bool ScrollToBottom;
};

#endif
