#ifndef VRITA_LOG_INCLUDES
#define VRITA_LOG_INCLUDES

#include <SDL2/SDL.h>
#include <imgui.h>
#include "third_party/imgui/imgui_impl_sdl2.h"

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
