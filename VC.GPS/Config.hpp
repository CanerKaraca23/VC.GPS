#pragma once
#include <string>

struct RGBA {
    unsigned char r, g, b, a;
};

class Config {
public:
    static void Init(const std::string& iniFilePath);

    static float lineWidth;
    static bool trackMovingTargets;
    static float removeRadius;

    static bool customColorsEnabled;
    static RGBA waypointColor;

    static bool enableLog;

private:
    static std::string iniPath;
    static void GenerateDefaultConfig();
    static void LoadConfig();
    static RGBA ParseColor(const std::string& colorStr, RGBA defaultColor);
};
