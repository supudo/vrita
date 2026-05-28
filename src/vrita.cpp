#include "../include/code.hpp"

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <memory>

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>

#include "../include/emulators.hpp"
#include "../include/dots.hpp"

std::shared_ptr<Emulators> managerEmulators;
std::shared_ptr<Dots> eyeCandy_Dots;
bool SHOW_DOTS = false;

void initEmulatorsManager() {
    managerEmulators = std::make_shared<Emulators>();
    managerEmulators->init();

    eyeCandy_Dots = std::make_shared<Dots>();
}

void ShowMainMenu() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Exit"))
                gDone = true;

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
        ImGui::EndMainMenuBar();
    }
}

int runVrita() {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        printf("Error: SDL_Init(): %s\n", SDL_GetError());
        return 1;
    }

    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    SDL_WindowFlags window_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    SDL_Window* window = SDL_CreateWindow(AppTitle, (int)(WINDOW_WIDTH * main_scale), (int)(WINDOW_HEIGHT * main_scale), window_flags);
    if (window == nullptr) {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return 1;
    }
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(window);

    SDL_GPUDevice* gpu_device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL | SDL_GPU_SHADERFORMAT_METALLIB, true, nullptr);

    if (gpu_device == nullptr) {
        printf("Error: SDL_CreateGPUDevice(): %s\n", SDL_GetError());
        return 1;
    }

    if (!SDL_ClaimWindowForGPUDevice(gpu_device, window)) {
        printf("Error: SDL_ClaimWindowForGPUDevice(): %s\n", SDL_GetError());
        return 1;
    }

    SDL_SetGPUSwapchainParameters(gpu_device, window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_VSYNC);

    if (!managerEmulators->createTexture(gpu_device)) {
        printf("Error: Cannot create emulator texture\n");
        return 1;
    }
    if (!eyeCandy_Dots->createTexture(gpu_device)) {
        printf("Error: Cannot create dots texture\n");
        return 1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);
    style.FontScaleDpi = main_scale;

    ImGui_ImplSDL3_InitForSDLGPU(window);

    ImGui_ImplSDLGPU3_InitInfo init_info = {};
    init_info.Device = gpu_device;
    init_info.ColorTargetFormat = SDL_GetGPUSwapchainTextureFormat(gpu_device, window);
    init_info.MSAASamples = SDL_GPU_SAMPLECOUNT_1;
    init_info.SwapchainComposition = SDL_GPU_SWAPCHAINCOMPOSITION_SDR;
    init_info.PresentMode = SDL_GPU_PRESENTMODE_VSYNC;
    ImGui_ImplSDLGPU3_Init(&init_info);

    ImVec4 clear_color = ImVec4(188.0f / 255.0f, 190.0f / 255.0f, 194.0f / 255.0f, 1.00f);

    while (!gDone) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT)
                gDone = true;
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
                event.window.windowID == SDL_GetWindowID(window)) {
                gDone = true;
            }
        }

        if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED) {
            SDL_Delay(10);
            continue;
        }

        float emulatorTime = (float)SDL_GetTicks() / 1000.0f;

        managerEmulators->generateTestPattern(emulatorTime);

        ImGui_ImplSDLGPU3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
        
        ShowMainMenu();
        
        managerEmulators->run();

        if (SHOW_DOTS)
            eyeCandy_Dots->run();
        
        ImGui::Render();

        ImDrawData* draw_data = ImGui::GetDrawData();
        bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
        SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(gpu_device);
        managerEmulators->uploadFramebufferToTexture(gpu_device, command_buffer);
        eyeCandy_Dots->uploadFramebufferToTexture(gpu_device, command_buffer);
        SDL_GPUTexture* swapchain_texture;
        SDL_WaitAndAcquireGPUSwapchainTexture(command_buffer, window, &swapchain_texture, nullptr, nullptr);

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

    ImGui_ImplSDL3_Shutdown();
    ImGui_ImplSDLGPU3_Shutdown();
    ImGui::DestroyContext();
    SDL_ReleaseWindowFromGPUDevice(gpu_device, window);
    SDL_DestroyGPUDevice(gpu_device);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}