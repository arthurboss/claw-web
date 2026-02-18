#pragma once

#include <string>

// Text alignment enumeration
enum class TextAlignment {
    Left,
    Center,
    Right
};

// Menu text data structure
struct MenuTextData {
    std::string text;               // Text content
    std::string fontPath;           // Font file path
    float fontSize;                 // Font size
    float x, y;                     // Position
    float r, g, b, a;               // Color (RGBA)
    TextAlignment alignment;        // Text alignment
    bool visible;                   // Visibility flag
    bool bold;                      // Bold text
    bool italic;                    // Italic text
    
    // Constructor with defaults
    MenuTextData() 
        : fontSize(16.0f), x(0.0f), y(0.0f), r(1.0f), g(1.0f), b(1.0f), a(1.0f),
          alignment(TextAlignment::Left), visible(true), bold(false), italic(false) {}
    
    MenuTextData(const std::string& textContent, float x, float y, float size)
        : text(textContent), fontSize(size), x(x), y(y), r(1.0f), g(1.0f), b(1.0f), a(1.0f),
          alignment(TextAlignment::Left), visible(true), bold(false), italic(false) {}
};
