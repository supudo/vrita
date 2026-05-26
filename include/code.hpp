#ifndef VRITA_CODE_INCLUDES
#define VRITA_CODE_INCLUDES

#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "SDL3/SDL.h"

int* gFrameBuffer;
SDL_Window* gSDLWindow;
SDL_Renderer* gSDLRenderer;
SDL_Texture* gSDLTexture;
int gDone;
const int WINDOW_WIDTH = 1920 / 2;
const int WINDOW_HEIGHT = 1080 / 2;
const char* AppTitle = "Vrita";

int runVrita();

#endif