#include "timer.hpp"

void DMGTimer::reset() {
    perfFreq = SDL_GetPerformanceFrequency();
    lastTime = SDL_GetPerformanceCounter();
    accumulator = 0.0;
}

uint32_t DMGTimer::tickFrame() {
    uint64_t now = SDL_GetPerformanceCounter();
    double elapsed = (double)(now - lastTime) / (double)perfFreq;
    lastTime = now;

    // cap to 50ms: prevents runaway catch-up after window drag or debugger pause
    if (elapsed > 0.05) elapsed = 0.05;

    accumulator += elapsed * CPU_HZ;
    uint32_t cycles = (uint32_t)accumulator;
    accumulator -= cycles;
    return cycles;
}
