#ifndef VRITA_LOG_INCLUDES
#define VRITA_LOG_INCLUDES

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>

class Log {
public:
    void init(int positionX, int positionY, int width, int height);
    void clear();
    void addToLog(const char* fmt, ...) IM_FMTARGS(2);
    void render(bool* p_opened = nullptr);

    ImVec2 getWindowPosition();
    ImVec2 getWindowSize();

    ImGuiTextBuffer Buf;
    ImGuiTextFilter Filter;
    ImVector<int> LineOffsets;
    bool ScrollToBottom;

private:
    int positionX, positionY, width, height;
};

#endif
