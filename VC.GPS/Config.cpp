#include "Config.hpp"
#include <windows.h>
#include <sstream>

float Config::lineWidth = 2.5f;
float Config::removeRadius = 15.0f;

RGBA Config::LineColor = {255, 77, 210, 255};
std::string Config::iniPath = "";

void Config::Init(const std::string& iniFilePath) {
    iniPath = iniFilePath;
    DWORD attrs = GetFileAttributesA(iniPath.c_str());
    if (attrs == INVALID_FILE_ATTRIBUTES) {
        GenerateDefaultConfig();
    }
    LoadConfig();
}

void Config::GenerateDefaultConfig() {
    WritePrivateProfileStringA("Navigation", "LineWidth", "2.5", iniPath.c_str());
    WritePrivateProfileStringA("Navigation", "RemoveRadiusDistance", "15.0", iniPath.c_str());

    WritePrivateProfileStringA("Custom Colors", "LineColor", "255, 77, 210, 255", iniPath.c_str());

}

RGBA Config::ParseColor(const std::string& colorStr, RGBA defaultColor) {
    RGBA color = defaultColor;
    std::stringstream ss(colorStr);
    std::string token;

    auto parseComponent = [](const std::string& t, unsigned char& out) -> bool {
        try {
            int val = std::stoi(t);
            if (val < 0 || val > 255) return false;
            out = static_cast<unsigned char>(val);
            return true;
        } catch (...) {
            return false;
        }
    };

    if (std::getline(ss, token, ',') && parseComponent(token, color.r) &&
        std::getline(ss, token, ',') && parseComponent(token, color.g) &&
        std::getline(ss, token, ',') && parseComponent(token, color.b) &&
        std::getline(ss, token, ',') && parseComponent(token, color.a)) {
        return color;
    }

    return defaultColor;
}

void Config::LoadConfig() {
    char buffer[256];

    GetPrivateProfileStringA("Navigation", "LineWidth", "2.5", buffer, sizeof(buffer), iniPath.c_str());
    try {
        lineWidth = std::stof(buffer);
        if (lineWidth <= 0.0f) lineWidth = 2.5f;
    } catch (...) {
        lineWidth = 2.5f;
    }

    GetPrivateProfileStringA("Navigation", "RemoveRadiusDistance", "15.0", buffer, sizeof(buffer), iniPath.c_str());
    try {
        removeRadius = std::stof(buffer);
        if (removeRadius <= 0.0f) removeRadius = 15.0f;
    } catch (...) {
        removeRadius = 15.0f;
    }

    GetPrivateProfileStringA("Custom Colors", "LineColor", "255, 77, 210, 255", buffer, sizeof(buffer), iniPath.c_str());
    LineColor = ParseColor(buffer, {255, 77, 210, 255});


}
