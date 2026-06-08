#include "log.hpp"

void Log::clear() {
    this->Buf.clear();
    this->LineOffsets.clear();
}

void Log::addToLog(const char* fmt, ...) {
    int old_size = this->Buf.size();
    va_list args;
    va_start(args, fmt);
    this->Buf.appendfv(fmt, args);
    va_end(args);
    for (int new_size = this->Buf.size(); old_size < new_size; old_size++)
        if (this->Buf[old_size] == '\n')
            this->LineOffsets.push_back(old_size);
    this->ScrollToBottom = true;
}

void Log::render(bool* p_opened) {
    ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(40, 40), ImGuiCond_FirstUseEver);

    ImGui::Begin("Log", p_opened);

    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    ImGui::Separator();

    if (ImGui::Button("Clear"))
        this->clear();
    ImGui::SameLine();
    bool copy = ImGui::Button("Copy");
    ImGui::SameLine();
    this->Filter.Draw("Filter", -100.0f);
    ImGui::Separator();
    ImGui::BeginChild("scrolling");
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 1));
    if (copy)
        ImGui::LogToClipboard();

    if (this->Filter.IsActive()) {
        const char* buf_begin = this->Buf.begin();
        const char* line = buf_begin;
        for (int line_no = 0; line != NULL; line_no++) {
            const char* line_end = (line_no < this->LineOffsets.Size) ? buf_begin + this->LineOffsets[line_no] : NULL;
            if (this->Filter.PassFilter(line, line_end))
                ImGui::TextUnformatted(line, line_end);
            line = line_end && line_end[1] ? line_end + 1 : NULL;
        }
    }
    else
        ImGui::TextUnformatted(this->Buf.begin());

    if (this->ScrollToBottom)
        ImGui::SetScrollHereY(1.0f);
    this->ScrollToBottom = false;
    ImGui::PopStyleVar();
    ImGui::EndChild();

    ImGui::End();
}
