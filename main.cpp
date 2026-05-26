#include "SDL3/SDL_main.h"
#include "include/code.hpp"

//#define VRITA_DOTS

int main(int argc, char** argv) {
    initEmulatorsManager();

#ifdef VRITA_DOTS
    return runDots();
#else
    return runVrita();
#endif
}