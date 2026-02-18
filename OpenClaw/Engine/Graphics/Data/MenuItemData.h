#pragma once

#include <string>

// Menu item state enumeration
enum class GraphicsMenuItemState {
    Inactive,
    Active,
    Focused,
    Pressed
};

// Menu item data structure
struct MenuItemData {
    std::string name;                // Item name/ID
    std::string inactiveTexturePath; // Inactive state texture
    std::string activeTexturePath;   // Active state texture
    float x, y;                      // Position
    float width, height;             // Size
    float alpha;                     // Transparency (0.0 - 1.0)
    bool visible;                    // Visibility flag
    GraphicsMenuItemState state;     // Current state
    bool canBeFocused;               // Can receive focus
    
    // Constructor with defaults
    MenuItemData() 
        : x(0.0f), y(0.0f), width(100.0f), height(50.0f), 
          alpha(1.0f), visible(true), state(GraphicsMenuItemState::Inactive), 
          canBeFocused(true) {}
    
    MenuItemData(const std::string& itemName, float x, float y, float w, float h)
        : name(itemName), x(x), y(y), width(w), height(h), 
          alpha(1.0f), visible(true), state(GraphicsMenuItemState::Inactive), 
          canBeFocused(true) {}
};
