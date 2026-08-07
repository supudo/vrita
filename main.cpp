#define SDL_MAIN_HANDLED

#include "utilities/settings.hpp"

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <memory>

#include <imgui.h>
#include <imgui_internal.h>
#include "imgui/imgui_impl_sdl2.h"
#include "imgui/imgui_impl_opengl3.h"

#ifdef TRACY_ENABLE
#define TRACY_OPENGL_AUTO_CALIBRATION
#include <tracy/tracy/TracyOpenGL.hpp>
#endif

#include "utilities/logger.hpp"
#include "emulators/emulators.hpp"
#include "eyecandy/dots.hpp"
#include "gui/filebrowser.hpp"
#include "gui/log.hpp"
#include "utilities/iconfonts/IconsFontAwesome7.h"

#ifdef  TRACY_ENABLE
void* operator new(size_t size) {
    void* ptr = malloc(size);
    TracyAlloc(ptr, size);
    return ptr;
}
void operator delete(void* ptr) noexcept {
    TracyFree(ptr);
    free(ptr);
}
#endif 

SDL_Window* appWindow;
SDL_GLContext glContext;

Settings appSettings("app_settings.ini");

std::shared_ptr<Dots> eyeCandy_Dots;
bool SHOW_DOTS = false;

std::shared_ptr<Logger> logger;

std::shared_ptr<Emulators> managerEmulators;
std::string emulatorType = "dmg";
std::string romLoadError;

std::shared_ptr<FileBrowser> guiFileBrowser;
bool guiFileBrowserVisible = false;

std::shared_ptr<Log> guiLog;
bool guiLogVisible = false;

bool guiStyleOptionsVisible = false;
bool guiMetricsVisible = false;

void ShowMainMenu() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Show Log"))
                guiLogVisible = !guiLogVisible;
            ImGui::Separator();
            if (ImGui::MenuItem("Exit"))
                vritaRunning = true;

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Emulators")) {
            if (ImGui::MenuItem("GameBoy (DMG)", NULL, managerEmulators->EMULATORS_SHOW_DMG))
                managerEmulators->EMULATORS_SHOW_DMG = !managerEmulators->EMULATORS_SHOW_DMG;
            if (ImGui::MenuItem("GameBoy Advance (AGB)", NULL, managerEmulators->EMULATORS_SHOW_AGB))
                managerEmulators->EMULATORS_SHOW_AGB = !managerEmulators->EMULATORS_SHOW_AGB;
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Debuggers")) {
            if (ImGui::MenuItem("Memory Editor", NULL, managerEmulators->debuggersMemoryEditorVisible))
                managerEmulators->debuggersMemoryEditorVisible = !managerEmulators->debuggersMemoryEditorVisible;
            if (ImGui::MenuItem("Tile Viewer (Tiles)", NULL, managerEmulators->debuggerTileViewerVisible))
                managerEmulators->debuggerTileViewerVisible = !managerEmulators->debuggerTileViewerVisible;
            if (ImGui::MenuItem("Tilemap Viewer (BG)", NULL, managerEmulators->debuggerTilemapViewerVisible))
                managerEmulators->debuggerTilemapViewerVisible = !managerEmulators->debuggerTilemapViewerVisible;
            if (ImGui::MenuItem("Sprite Viewer (OAM)", NULL, managerEmulators->debuggerSpriteViewerVisible))
                managerEmulators->debuggerSpriteViewerVisible = !managerEmulators->debuggerSpriteViewerVisible;
            if (ImGui::MenuItem("Palette Viewer (Palettes)", NULL, managerEmulators->debuggerPaletteViewerVisible))
                managerEmulators->debuggerPaletteViewerVisible = !managerEmulators->debuggerPaletteViewerVisible;
            if (ImGui::MenuItem("Debugger", NULL, managerEmulators->debuggerDebuggerVisible))
                managerEmulators->debuggerDebuggerVisible = !managerEmulators->debuggerDebuggerVisible;
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Eyecandy")) {
            if (ImGui::MenuItem("Dots", NULL, SHOW_DOTS))
                SHOW_DOTS = !SHOW_DOTS;
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Tools")) {
            if (ImGui::MenuItem("Style", NULL, guiStyleOptionsVisible))
                guiStyleOptionsVisible = !guiStyleOptionsVisible;
            if (ImGui::MenuItem("Metrics", NULL, guiMetricsVisible))
                guiMetricsVisible = !guiMetricsVisible;
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void showFileBrowser(const char* type) {
    guiFileBrowserVisible = !guiFileBrowserVisible;
    emulatorType = type;
}

void renderGUIComponents() {
    if (guiFileBrowserVisible)
        guiFileBrowser->render(&guiFileBrowserVisible, emulatorType);
    if (guiLogVisible)
        guiLog->render(&guiLogVisible);
    if (guiStyleOptionsVisible)
        ImGui::ShowStyleEditor(&ImGui::GetStyle());
    if (guiMetricsVisible)
        ImGui::ShowMetricsWindow(&guiMetricsVisible);
}

void loadROM(const char* romFilePath) {
    guiFileBrowserVisible = false;
    std::string errorMessage = managerEmulators->loadROM(romFilePath);
    if (errorMessage != "") {
        ImGui::OpenPopup("ROM Load Error");
        ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        if (ImGui::BeginPopupModal("ROM Load Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("%s", errorMessage.c_str());
            ImGui::Separator();
            ImGui::SetCursorPosX((ImGui::GetWindowSize().x - 120) * 0.5f);
            if (ImGui::Button("OK", ImVec2(120, 0))) {
                errorMessage.clear();
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }
}

void initComponents() {
    guiMetricsVisible = appSettings.GetBool("Visibility", "gui_metrics_visible", false);
    guiLogVisible = appSettings.GetBool("Visibility", "gui_log_visible", false);

    guiLog = std::make_shared<Log>();

    logger = std::make_shared<Logger>([] (const char* msg) {
        guiLog->addToLog("%s\n", msg);
    });

    managerEmulators = std::make_shared<Emulators>(*logger);
    managerEmulators->init(appSettings);

    guiFileBrowser = std::make_shared<FileBrowser>(appSettings);
    guiFileBrowser->init(std::bind(&loadROM, std::placeholders::_1));
}

void saveAppSettings() {
    appSettings.Set("Visibility", "gui_metrics_visible", guiMetricsVisible);
    appSettings.Set("Visibility", "gui_log_visible", guiLogVisible);

    int width, height;
    SDL_GetWindowSizeInPixels(appWindow, &width, &height);
    appSettings.Set("MainWindow", "width", width);
    appSettings.Set("MainWindow", "height", height);
    int x, y;
    SDL_GetWindowPosition(appWindow, &x, &y);
    appSettings.Set("MainWindow", "has_position", true);
    appSettings.Set("MainWindow", "x", x);
    appSettings.Set("MainWindow", "y", y);

    appSettings.Save();
}

void loadFonts() {
    ImGuiIO& io = ImGui::GetIO();
    float baseFontSize = 13.0f;
    
    ImFontConfig base_config;
    base_config.SizePixels = baseFontSize;
    io.Fonts->AddFontDefault(&base_config);

    float iconFontSize = baseFontSize * 1.5f;
    static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    icons_config.GlyphMinAdvanceX = iconFontSize;
    icons_config.GlyphOffset = ImVec2(0.0f, 4.0f);
    io.Fonts->AddFontFromFileTTF("./resources/fonts/fa-regular-400.ttf", iconFontSize, &icons_config, icons_ranges);
    io.Fonts->AddFontFromFileTTF("./resources/fonts/fa-solid-900.ttf", iconFontSize, &icons_config, icons_ranges);
}

bool initBackend() {
    bool initialized = true;
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("[VRITA] Error: SDL_Init(): %s\n", SDL_GetError());
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#ifdef Def_Kuplung_DEBUG_BUILD
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

    SDL_SetHint(SDL_HINT_MAC_CTRL_CLICK_EMULATE_RIGHT_CLICK, "1");
    SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");

#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

    float main_scale = ImGui_ImplSDL2_GetContentScaleForDisplay(0);
    Uint32 window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    int windowWidth = appSettings.GetInt("MainWindow", "width", WINDOW_WIDTH);
    int windowHeight = appSettings.GetInt("MainWindow", "height", WINDOW_HEIGHT);
    appWindow = SDL_CreateWindow(AppTitle, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, (int)(windowWidth * main_scale), (int)(windowHeight * main_scale), window_flags);
    if (appWindow == nullptr) {
        printf("[VRITA] Error: Window could not be created! SDL Error: %s\n", SDL_GetError());
        initialized = false;
    }
    else {
        glContext = SDL_GL_CreateContext(appWindow);
        if (!glContext) {
            printf("[VRITA] Error: Unable to create OpenGL context! SDL Error: %s\n", SDL_GetError());
            initialized = false;
        }
        else {
            if (SDL_GL_MakeCurrent(appWindow, glContext) != 0) {
                printf("[VRITA] Warning: Unable to set current context! SDL Error: %s\n", SDL_GetError());
                initialized = false;
            }
            else {
                if (SDL_GL_SetSwapInterval(1) != 0) {
                    printf("[VRITA] Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
                    initialized = false;
                }
#ifdef _WIN32
                const GLenum glewInitCode = glewInit();
                if (glewInitCode != GLEW_OK) {
                    printf("[VRITA] Cannot initialize GLEW.\n");
                    initialized = false;
                }
#endif
#ifdef TRACY_ENABLE
                TracyGpuContext;
#endif
                printf("[VRITA] GL_VERSION:  %s\n", (const char*)glGetString(GL_VERSION));
                printf("[VRITA] GL_VENDOR:   %s\n", (const char*)glGetString(GL_VENDOR));
                printf("[VRITA] GL_RENDERER: %s\n", (const char*)glGetString(GL_RENDERER));
                int glMajor = 0, glMinor = 0, glProfile = 0;
                glGetIntegerv(GL_MAJOR_VERSION, &glMajor);
                glGetIntegerv(GL_MINOR_VERSION, &glMinor);
                glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &glProfile);
                printf("[VRITA] GL_MAJOR_VERSION.GL_MINOR_VERSION: %d.%d, CONTEXT_PROFILE_MASK: 0x%X (core=0x1)\n", glMajor, glMinor, glProfile);
            }
        }
    }
    return initialized;
}

int main(int argc, char** argv) {
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    bool backendInitialized = initBackend();
    if (!backendInitialized) {
        printf("[VRITA] Error: Backend not initialized.\n");
        exit(EXIT_FAILURE);
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.IniFilename = "gui_options.ini";
    ImGui_ImplSDL2_InitForOpenGL(appWindow, glContext);
    ImGui_ImplOpenGL3_Init("#version 410 core");

    ImGui::StyleColorsDark();

    loadFonts();

    bool hasSavedPos = appSettings.GetBool("MainWindow", "has_position", false);
    if (hasSavedPos) {
        int windowX = appSettings.GetInt("MainWindow", "x", 0);
        int windowY = appSettings.GetInt("MainWindow", "y", 0);
        SDL_SetWindowPosition(appWindow, windowX, windowY);
    }
    else
        SDL_SetWindowPosition(appWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    initComponents();

    eyeCandy_Dots = std::make_shared<Dots>();

    if (!managerEmulators->createTexture()) {
        logger->log("[VRITA] Error: Cannot create emulator texture");
        return 1;
    }
    if (!eyeCandy_Dots->createTexture()) {
        logger->log("[VRITA] Error: Cannot create dots texture");
        return 1;
    }

    ImVec4 clear_color = ImVec4(145.0f / 255.0f, 145.0f / 255.0f, 145.0f / 255.0f, 1.00f);

    std::vector<std::string> droppedFiles;
    bool showDropCountError = false;
    while (!vritaRunning) {
#ifdef TRACY_ENABLE
        ZoneScoped;
#endif
        SDL_Event event;
        {
#ifdef TRACY_ENABLE
            ZoneScopedN("PollEvents");
#endif
            while (SDL_PollEvent(&event)) {
                ImGui_ImplSDL2_ProcessEvent(&event);
                if (event.type == SDL_QUIT)
                    vritaRunning = true;
                if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(appWindow))
                    vritaRunning = true;
                if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
                    managerEmulators->handleKey(event.type, event.key.keysym.sym);
                if (event.type == SDL_DROPFILE) {
                    logger->log("[VRITA] File dropped: %s", event.drop.file);
                    droppedFiles.emplace_back(event.drop.file);
                    SDL_free(event.drop.file);
                }
                if (event.type == SDL_DROPCOMPLETE) {
                    if (droppedFiles.size() == 1)
                        loadROM(droppedFiles[0].c_str());
                    else if (droppedFiles.size() > 1)
                        showDropCountError = true;
                    droppedFiles.clear();
                }
            }
        }

        if (SDL_GetWindowFlags(appWindow) & SDL_WINDOW_MINIMIZED) {
            SDL_Delay(10);
            continue;
        }

        int fbW, fbH;
        SDL_GetWindowSizeInPixels(appWindow, &fbW, &fbH);
        glViewport(0, 0, fbW, fbH);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        float emulatorTime = (float)SDL_GetTicks() / 1000.0f;

        managerEmulators->generateTestPattern(emulatorTime);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        if (showDropCountError) {
            ImGui::OpenPopup("Error");
            showDropCountError = false;
        }
        ImVec2 dropErrorCenter = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(dropErrorCenter, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        if (ImGui::BeginPopupModal("Error", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Please, drop only one file at a time.");
            ImGui::Dummy(ImVec2(0.0f, 10.0f));
            float okButtonWidth = 120.0f;
            ImGui::SetCursorPosX((ImGui::GetWindowSize().x - okButtonWidth) * 0.5f);
            if (ImGui::Button("OK", ImVec2(okButtonWidth, 0)))
                ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }

        ShowMainMenu();

        renderGUIComponents();

        {
#ifdef TRACY_ENABLE
            ZoneScopedN("Emulators::Run");
#endif
            managerEmulators->run(std::bind(&loadROM, std::placeholders::_1), std::bind(&showFileBrowser, std::placeholders::_1), [] (const char* type) { emulatorType = type; });
        }

        if (SHOW_DOTS)
            eyeCandy_Dots->run();

        {
#ifdef TRACY_ENABLE
            ZoneScopedN("ImGui::Render");
#endif
            ImGui::Render();
        }

        {
#ifdef TRACY_ENABLE
            TracyGpuZone("Framebuffer Upload");
#endif
            managerEmulators->uploadFramebufferToTexture();
            eyeCandy_Dots->uploadFramebufferToTexture();
        }

        ImDrawData* draw_data = ImGui::GetDrawData();
        {
#ifdef TRACY_ENABLE
            TracyGpuZone("ImGui Draw");
#endif
            ImGui_ImplOpenGL3_RenderDrawData(draw_data);
        }

        {
#ifdef TRACY_ENABLE
            ZoneScopedN("SwapWindow (vsync wait)");
#endif
            SDL_GL_SwapWindow(appWindow);
        }

#ifdef TRACY_ENABLE
        TracyGpuCollect;
        FrameMark;
#endif
    }

    managerEmulators->release(appSettings);
    eyeCandy_Dots->release();
    saveAppSettings();

    ImGui::SaveIniSettingsToDisk("gui_options.ini"); 

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(appWindow);
    SDL_Quit();

    return 0;
}