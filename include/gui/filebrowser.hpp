#ifndef VRITA_FILEBROWSER_INCLUDES
#define VRITA_FILEBROWSER_INCLUDES

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <functional>
#include <map>
#include <stdint.h>
#include <string>
#include <unordered_set>
#include <vector>

#include "../files.hpp"

class FileBrowser {
public:
    void init(const std::function<void(const char*)>& processFile);
    void render(bool* p_opened = nullptr, std::string const& emulatorType = "dmg");

private:
    std::map<std::string, VritaUtils::FBEntity> getFolderContents(std::string const& filePath, std::string const& emulatorType);
    std::function<void(const char*)> processFile;
    void drawFiles(const std::string& fPath, std::string const& emulatorType);
    const std::string convertToString(double num) const;
    const std::string convertSize(size_t size) const;

    std::string currentFolder;

    std::unordered_map<std::string, std::unordered_set<std::string>> allowedExtensions = {
        { "dmg", { ".gb", ".gbc" } }, // Game Boy / Color
        { "agb", { ".gba", ".agb" } },   // Game Boy Advance
    };
};

#endif
