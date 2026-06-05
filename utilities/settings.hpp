#ifndef VRITA_SETTINGS_INCLUDES
#define VRITA_SETTINGS_INCLUDES

#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>

#include <imgui.h>

inline int vritaRunning = 0;
inline constexpr int WINDOW_WIDTH = 900;
inline constexpr int WINDOW_HEIGHT = 500;
inline const char* AppTitle = "Vrita";

class Settings {
public:
    explicit Settings(const std::string& filename) : m_filename(filename) { Load(); }
    inline void Set(const std::string& key, bool value) { m_values[key] = value ? "1" : "0"; }
    inline void Set(const std::string& key, int value) { m_values[key] = std::to_string(value); }
    inline void Set(const std::string& key, float value) { m_values[key] = std::to_string(value); }
    inline void Set(const std::string& key, const std::string& value) { m_values[key] = value; }

    inline bool GetBool(const std::string& key, bool defaultValue = false) const {
        auto it = m_values.find(key);
        if (it == m_values.end()) return defaultValue;
        return it->second == "1";
    }

    inline int GetInt(const std::string& key, int defaultValue = 0) const {
        auto it = m_values.find(key);
        if (it == m_values.end()) return defaultValue;
        return std::stoi(it->second);
    }

    inline float GetFloat(const std::string& key, float defaultValue = 0.0f) const {
        auto it = m_values.find(key);
        if (it == m_values.end()) return defaultValue;
        return std::stof(it->second);
    }

    inline std::string GetString(const std::string& key, const std::string& defaultValue = "") const {
        auto it = m_values.find(key);
        if (it == m_values.end()) return defaultValue;
        return it->second;
    }

    inline void Save() const {
        std::ofstream file(m_filename);
        for (const auto& [key, value] : m_values)
            file << key << "=" << value << '\n';
    }

private:
    inline void Load() {
        std::ifstream file(m_filename);
        std::string line;
        while (std::getline(file, line)) {
            auto pos = line.find('=');
            if (pos == std::string::npos) continue;
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            m_values[key] = value;
        }
    }

private:
    std::string m_filename;
    std::unordered_map<std::string, std::string> m_values;
};

#endif