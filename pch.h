#pragma once

// STL
#include <vector>
#include <string>
#include <array>
#include <algorithm>
#include <functional>
#include <memory>
#include <cstdint>
#include <chrono>
#include <filesystem>
#include <map>

// GL loader (must precede any gl.h include, incl. SDL's)
#include <GL/glew.h>

// SDL
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_OpenGL.h>

// Dear ImGui
#include <imgui.h>

// Internals
#include "utilities/logger.hpp"
#include "utilities/settings.hpp"
#include "utilities/datetimes.hpp"
#include "utilities/files.hpp"

#ifdef TRACY_ENABLE
// Profiling
#include <tracy/tracy/Tracy.hpp>
#endif