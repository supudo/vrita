#ifndef VRITA_CODE_INCLUDES
#define VRITA_CODE_INCLUDES

#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "SDL3/SDL.h"

inline int vritaRunning = 0;
inline constexpr int WINDOW_WIDTH = 1920 / 2;
inline constexpr int WINDOW_HEIGHT = 1080 / 2;
inline const char* AppTitle = "Vrita";

void renderGUIComponents();
void showFileBrowser(const char* emulatorType);
void loadROM(const char* romFilePath);
void initEmulatorsManager();
int runVrita();

#endif