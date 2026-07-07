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

    try {
        if (std::getline(ss, token, ',')) color.r = (unsigned char)std::stoi(token);
        else return defaultColor;

        if (std::getline(ss, token, ',')) color.g = (unsigned char)std::stoi(token);
        else return defaultColor;

        if (std::getline(ss, token, ',')) color.b = (unsigned char)std::stoi(token);
        else return defaultColor;

        if (std::getline(ss, token, ',')) color.a = (unsigned char)std::stoi(token);
        else return defaultColor;
    } catch (...) {
        return defaultColor;
    }

    return color;
}

void Config::LoadConfig() {
    char buffer[256];

    GetPrivateProfileStringA("Navigation", "LineWidth", "2.5", buffer, sizeof(buffer), iniPath.c_str());
    try { lineWidth = std::stof(buffer); } catch (...) { lineWidth = 2.5f; }


    GetPrivateProfileStringA("Navigation", "RemoveRadiusDistance", "15.0", buffer, sizeof(buffer), iniPath.c_str());
    try { removeRadius = std::stof(buffer); } catch (...) { removeRadius = 15.0f; }



    GetPrivateProfileStringA("Custom Colors", "LineColor", "255, 77, 210, 255", buffer, sizeof(buffer), iniPath.c_str());
    LineColor = ParseColor(buffer, {255, 77, 210, 255});
}
