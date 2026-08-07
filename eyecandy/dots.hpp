#ifndef VRITA_DOTS_INCLUDES
#define VRITA_DOTS_INCLUDES

#include <SDL2/SDL.h>
#ifdef _WIN32
#include <GL/glew.h>
#endif
#include <stdint.h>
#include <vector>

class Dots {
public:
    bool createTexture();
    void generateTestPattern(float fwidth, float fheight, float time);
    void uploadFramebufferToTexture();
    void run();
    void release();

private:
    uint32_t width = 0;
    uint32_t height = 0;
    std::vector<uint32_t> framebuffer;
    GLuint gTexture = 0;
};

#endif
