#include "../include/code.hpp"

bool update() {
    SDL_Event e;
    if (SDL_PollEvent(&e)) {
        if (e.type == SDL_EVENT_QUIT)
            return false;
        if (e.type == SDL_EVENT_KEY_UP && e.key.key == SDLK_ESCAPE)
            return false;
    }

    char* pix;
    int pitch;

    SDL_LockTexture(gSDLTexture, NULL, (void**)&pix, &pitch);
    for (int i = 0, sp = 0, dp = 0; i < WINDOW_HEIGHT; i++, dp += WINDOW_WIDTH, sp += pitch)
        memcpy(pix + sp, gFrameBuffer + dp, WINDOW_WIDTH * 4);

    SDL_UnlockTexture(gSDLTexture);
    SDL_RenderTexture(gSDLRenderer, gSDLTexture, NULL, NULL);
    SDL_RenderPresent(gSDLRenderer);
    SDL_Delay(1);
    return true;
}

void render(Uint64 aTicks) {
    for (int i = 0, c = 0; i < WINDOW_HEIGHT; i++) {
        for (int j = 0; j < WINDOW_WIDTH; j++, c++) {
            gFrameBuffer[c] = (int)(i * i + j * j + aTicks) | 0xff000000;
        }
    }
}

void loop() {
    if (!update())
        gDone = 1;
    else
        render(SDL_GetTicks());
}

void destroyVrita() {
    SDL_DestroyTexture(gSDLTexture);
    SDL_DestroyRenderer(gSDLRenderer);
    SDL_DestroyWindow(gSDLWindow);
    SDL_Quit();
}

int runVrita() {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
        return -1;

    gFrameBuffer = new int[WINDOW_WIDTH * WINDOW_HEIGHT];
    gSDLWindow = SDL_CreateWindow(AppTitle, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    gSDLRenderer = SDL_CreateRenderer(gSDLWindow, NULL);
    gSDLTexture = SDL_CreateTexture(gSDLRenderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, WINDOW_WIDTH, WINDOW_HEIGHT);

    if (!gFrameBuffer || !gSDLWindow || !gSDLRenderer || !gSDLTexture)
        return -1;

    gDone = 0;
    while (!gDone) {
        loop();
    }

    destroyVrita();

    return 0;
}