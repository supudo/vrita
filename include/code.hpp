#ifndef VRITA_CODE_INCLUDES
#define VRITA_CODE_INCLUDES

#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "SDL3/SDL.h"

inline int gDone = 0;
inline constexpr int WINDOW_WIDTH = 1920 / 2;
inline constexpr int WINDOW_HEIGHT = 1080 / 2;
inline const char* AppTitle = "Vrita";

void initEmulatorsManager();
int runDots();
int runVrita();

#endif