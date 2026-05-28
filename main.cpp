#include "SDL3/SDL_main.h"
#include "include/code.hpp"

int main(int argc, char** argv) {
    initEmulatorsManager();
    return runVrita();
}