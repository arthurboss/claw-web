#pragma once

#include <string>

// Menu background data structure
struct MenuBackgroundData {
    std::string texturePath;        // Path to background texture
    float x, y;                     // Position
    float width, height;            // Size
    float alpha;                    // Transparency (0.0 - 1.0)
    bool visible;                   // Visibility flag
    
    // Constructor with defaults
    MenuBackgroundData() 
        : x(0.0f), y(0.0f), width(1280.0f), height(960.0f), 
          alpha(1.0f), visible(true) {}
    
    MenuBackgroundData(const std::string& path, float x, float y, float w, float h)
        : texturePath(path), x(x), y(y), width(w), height(h), 
          alpha(1.0f), visible(true) {}
};
