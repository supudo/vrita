#include "SDL3/SDL_main.h"
#include "utilities/settings.hpp"

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <memory>

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>

#include "emulators/emulators.hpp"
#include "eyecandy/dots.hpp"
#include "gui/filebrowser.hpp"
#include "gui/log.hpp"

#include "utilities/logger.hpp"

SDL_Window* appWindow;

Settings settings("app_settings.ini");

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
    guiFileBrowserVisible = true;
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

void loadGui() {
    // set gui visibility flags
    guiMetricsVisible = settings.GetBool("guiMetricsVisible", false);
    guiLogVisible = settings.GetBool("guiLogVisible", false);

    guiLog = std::make_shared<Log>();
    int windowLogX = settings.GetInt("windowLogX", 40);
    int windowLogY = settings.GetInt("windowLogY", 40);
    int windowLogWidth = settings.GetInt("windowLogWidth", 400);
    int windowLogHeight = settings.GetInt("windowLogHeight", 200);
    guiLog->init(windowLogX, windowLogY, windowLogWidth, windowLogHeight);

    logger = std::make_shared<Logger>([] (const char* msg) {
        guiLog->addToLog("%s\n", msg);
    });

    managerEmulators = std::make_shared<Emulators>(*logger);
    managerEmulators->init();
    managerEmulators->EMULATORS_SHOW_DMG = settings.GetBool("EMULATORS_SHOW_DMG", false);
    managerEmulators->EMULATORS_SHOW_AGB = settings.GetBool("EMULATORS_SHOW_AGB", false);

    guiFileBrowser = std::make_shared<FileBrowser>();
    guiFileBrowser->init(std::bind(&loadROM, std::placeholders::_1));
}

void saveAppSettings() {
    settings.Set("guiMetricsVisible", guiMetricsVisible);
    settings.Set("guiLogVisible", guiLogVisible);
    settings.Set("EMULATORS_SHOW_DMG", managerEmulators->EMULATORS_SHOW_DMG);
    settings.Set("EMULATORS_SHOW_AGB", managerEmulators->EMULATORS_SHOW_AGB);

    //settings.Set("windowLogX", guiLog->getWindowPosition().x);
    //settings.Set("windowLogY", guiLog->getWindowPosition().y);
    //settings.Set("windowLogWidth", guiLog->getWindowSize().x);
    //settings.Set("windowLogHeight", guiLog->getWindowSize().y);

    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    int width, height;
    SDL_GetWindowSize(appWindow, &width, &height);
    settings.Set("mainWindowWidth", (int)(width / main_scale));
    settings.Set("mainWindowHeight", (int)(height / main_scale));
    int x, y;
    SDL_GetWindowPosition(appWindow, &x, &y);
    settings.Set("mainWindowHasPos", true);
    settings.Set("mainWindowX", x);
    settings.Set("mainWindowY", y);

    settings.Save();
}

int main(int argc, char** argv) {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        printf("[VRITA] Error: SDL_Init(): %s\n", SDL_GetError());
        return 1;
    }

    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    SDL_WindowFlags window_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    int windowWidth = settings.GetInt("mainWindowWidth", WINDOW_WIDTH);
    int windowHeight = settings.GetInt("mainWindowHeight", WINDOW_HEIGHT);
    appWindow = SDL_CreateWindow(AppTitle, (int)(windowWidth * main_scale), (int)(windowHeight * main_scale), window_flags);
    if (appWindow == nullptr) {
        printf("[VRITA] Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return 1;
    }
    bool hasSavedPos = settings.GetBool("mainWindowHasPos", false);
    if (hasSavedPos) {
        int windowX = settings.GetInt("mainWindowX", 0);
        int windowY = settings.GetInt("mainWindowY", 0);
        SDL_SetWindowPosition(appWindow, windowX, windowY);
    }
    else
        SDL_SetWindowPosition(appWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(appWindow);

    SDL_GPUDevice* gpu_device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL | SDL_GPU_SHADERFORMAT_METALLIB, true, nullptr);

    if (gpu_device == nullptr) {
        printf("[VRITA] Error: SDL_CreateGPUDevice(): %s\n", SDL_GetError());
        return 1;
    }

    if (!SDL_ClaimWindowForGPUDevice(gpu_device, appWindow)) {
        printf("[VRITA] Error: SDL_ClaimWindowForGPUDevice(): %s\n", SDL_GetError());
        return 1;
    }

    SDL_SetGPUSwapchainParameters(gpu_device, appWindow, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_VSYNC);

    loadGui();

    eyeCandy_Dots = std::make_shared<Dots>();

    if (!managerEmulators->createTexture(gpu_device)) {
        logger->log("[VRITA] Error: Cannot create emulator texture");
        return 1;
    }
    if (!eyeCandy_Dots->createTexture(gpu_device)) {
        logger->log("[VRITA] Error: Cannot create dots texture");
        return 1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.IniFilename = "gui_options.ini";

    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);
    style.FontScaleDpi = main_scale;

    ImGui_ImplSDL3_InitForSDLGPU(appWindow);

    ImGui_ImplSDLGPU3_InitInfo init_info = {};
    init_info.Device = gpu_device;
    init_info.ColorTargetFormat = SDL_GetGPUSwapchainTextureFormat(gpu_device, appWindow);
    init_info.MSAASamples = SDL_GPU_SAMPLECOUNT_1;
    init_info.SwapchainComposition = SDL_GPU_SWAPCHAINCOMPOSITION_SDR;
    init_info.PresentMode = SDL_GPU_PRESENTMODE_VSYNC;
    ImGui_ImplSDLGPU3_Init(&init_info);

    ImVec4 clear_color = ImVec4(188.0f / 255.0f, 190.0f / 255.0f, 194.0f / 255.0f, 1.00f);

    while (!vritaRunning) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT)
                vritaRunning = true;
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
                event.window.windowID == SDL_GetWindowID(appWindow)) {
                vritaRunning = true;
            }
        }

        if (SDL_GetWindowFlags(appWindow) & SDL_WINDOW_MINIMIZED) {
            SDL_Delay(10);
            continue;
        }

        float emulatorTime = (float)SDL_GetTicks() / 1000.0f;

        managerEmulators->generateTestPattern(emulatorTime);

        ImGui_ImplSDLGPU3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        ShowMainMenu();

        renderGUIComponents();

        managerEmulators->run(std::bind(&loadROM, std::placeholders::_1), std::bind(&showFileBrowser, std::placeholders::_1), [] (const char* type) { emulatorType = type; });

        if (SHOW_DOTS)
            eyeCandy_Dots->run();

        ImGui::Render();

        ImDrawData* draw_data = ImGui::GetDrawData();
        bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
        SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(gpu_device);
        managerEmulators->uploadFramebufferToTexture(gpu_device, command_buffer);
        eyeCandy_Dots->uploadFramebufferToTexture(gpu_device, command_buffer);
        SDL_GPUTexture* swapchain_texture;
        SDL_WaitAndAcquireGPUSwapchainTexture(command_buffer, appWindow, &swapchain_texture, nullptr, nullptr);

        if (swapchain_texture != nullptr && !is_minimized) {
            ImGui_ImplSDLGPU3_PrepareDrawData(draw_data, command_buffer);
            SDL_GPUColorTargetInfo target_info = {};
            target_info.texture = swapchain_texture;
            target_info.clear_color = SDL_FColor{ clear_color.x, clear_color.y, clear_color.z, clear_color.w };
            target_info.load_op = SDL_GPU_LOADOP_CLEAR;
            target_info.store_op = SDL_GPU_STOREOP_STORE;
            SDL_GPURenderPass* render_pass = SDL_BeginGPURenderPass(command_buffer, &target_info, 1, nullptr);
            ImGui_ImplSDLGPU3_RenderDrawData(draw_data, command_buffer, render_pass);
            SDL_EndGPURenderPass(render_pass);
        }

        SDL_SubmitGPUCommandBuffer(command_buffer);
    }

    SDL_WaitForGPUIdle(gpu_device);

    managerEmulators->release(gpu_device);
    eyeCandy_Dots->release(gpu_device);

    ImGui::SaveIniSettingsToDisk("gui_options.ini"); 
    saveAppSettings();

    ImGui_ImplSDL3_Shutdown();
    ImGui_ImplSDLGPU3_Shutdown();
    ImGui::DestroyContext();
    SDL_ReleaseWindowFromGPUDevice(gpu_device, appWindow);
    SDL_DestroyGPUDevice(gpu_device);
    SDL_DestroyWindow(appWindow);
    SDL_Quit();

    return 0;
}