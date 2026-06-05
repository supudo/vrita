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

#pragma once

#include <string>
#include <unordered_map>
#include <fstream>

class Settings {
public:
    explicit Settings(const std::string& filename)
        : m_filename(filename) {
        Load();
    }

    // -------------------------
    // SETTERS
    // -------------------------
    inline void Set(const std::string& section, const std::string& key, bool value) {
        m_values[section][key] = value ? "1" : "0";
    }

    inline void Set(const std::string& section, const std::string& key, int value) {
        m_values[section][key] = std::to_string(value);
    }

    inline void Set(const std::string& section, const std::string& key, float value) {
        m_values[section][key] = std::to_string(value);
    }

    inline void Set(const std::string& section, const std::string& key, const std::string& value) {
        m_values[section][key] = value;
    }

    // -------------------------
    // GETTERS
    // -------------------------
    inline bool GetBool(const std::string& section, const std::string& key, bool defaultValue = false) const {
        auto sit = m_values.find(section);
        if (sit == m_values.end()) return defaultValue;

        auto it = sit->second.find(key);
        if (it == sit->second.end()) return defaultValue;

        return it->second == "1";
    }

    inline int GetInt(const std::string& section, const std::string& key, int defaultValue = 0) const {
        auto sit = m_values.find(section);
        if (sit == m_values.end()) return defaultValue;

        auto it = sit->second.find(key);
        if (it == sit->second.end()) return defaultValue;

        return std::stoi(it->second);
    }

    inline float GetFloat(const std::string& section, const std::string& key, float defaultValue = 0.0f) const {
        auto sit = m_values.find(section);
        if (sit == m_values.end()) return defaultValue;

        auto it = sit->second.find(key);
        if (it == sit->second.end()) return defaultValue;

        return std::stof(it->second);
    }

    inline std::string GetString(const std::string& section, const std::string& key,
                                 const std::string& defaultValue = "") const {
        auto sit = m_values.find(section);
        if (sit == m_values.end()) return defaultValue;

        auto it = sit->second.find(key);
        if (it == sit->second.end()) return defaultValue;

        return it->second;
    }

    // -------------------------
    // SAVE
    // -------------------------
    inline void Save() const {
        std::ofstream file(m_filename);

        for (const auto& [section, keys] : m_values) {
            file << "[" << section << "]\n";

            for (const auto& [key, value] : keys) {
                file << key << "=" << value << "\n";
            }

            file << "\n";
        }
    }

private:
    // -------------------------
    // LOAD
    // -------------------------
    inline void Load() {
        std::ifstream file(m_filename);
        if (!file.is_open())
            return;

        std::string line;
        std::string currentSection;

        while (std::getline(file, line)) {
            if (line.empty())
                continue;

            // [Section]
            if (line.front() == '[' && line.back() == ']') {
                currentSection = line.substr(1, line.size() - 2);
                continue;
            }

            auto pos = line.find('=');
            if (pos == std::string::npos)
                continue;

            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);

            m_values[currentSection][key] = value;
        }
    }

private:
    std::string m_filename;

    std::unordered_map<
        std::string,
        std::unordered_map<std::string, std::string>
    > m_values;
};

#endif