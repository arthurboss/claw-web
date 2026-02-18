#include "WebGLRenderer.h"
#include "../../Logger/Logger.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

// Constructor
WebGLRenderer::WebGLRenderer()
    : sdlRenderer(nullptr)
    , defaultFont(nullptr)
    , isInitialized(false)
    , ownsRenderer(false)
    , frameTime(0.0f)
    , drawCalls(0)
    , frameStartTime(0)
    , viewportX(0), viewportY(0), viewportWidth(1280), viewportHeight(960)
{
}

// Destructor
WebGLRenderer::~WebGLRenderer() {
    Shutdown();
}

// Initialize WebGL renderer
bool WebGLRenderer::Initialize() {
    LOG("Initializing WebGL Renderer...");
    
    // Get the main window from SDL
    SDL_Window* window = SDL_GL_GetCurrentWindow();
    if (!window) {
        LOG_ERROR("No SDL window available for WebGL renderer");
        return false;
    }
    
    // Create SDL renderer
    sdlRenderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!sdlRenderer) {
        LOG_ERROR("Failed to create SDL renderer: " + std::string(SDL_GetError()));
        return false;
    }
    
    ownsRenderer = true; // We created the renderer
    return InitializeRenderer();
}

// Initialize WebGL renderer with existing SDL renderer
bool WebGLRenderer::Initialize(SDL_Renderer* existingRenderer) {
    LOG("Initializing WebGL Renderer with existing SDL renderer...");
    
    if (!existingRenderer) {
        LOG_ERROR("No existing SDL renderer provided");
        return false;
    }
    
    sdlRenderer = existingRenderer;
    ownsRenderer = false; // We don't own the renderer
    return InitializeRenderer();
}

// Common initialization logic
bool WebGLRenderer::InitializeRenderer() {
    // Set renderer properties
    SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_BLEND);
    
    // Initialize SDL_image
    int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        LOG_ERROR("Failed to initialize SDL_image: " + std::string(IMG_GetError()));
        return false;
    }
    
    // Initialize SDL_ttf
    if (TTF_Init() == -1) {
        LOG_ERROR("Failed to initialize SDL_ttf: " + std::string(TTF_GetError()));
        return false;
    }
    
    // Load default font
    defaultFont = TTF_OpenFont("clacon.ttf", 16);
    if (!defaultFont) {
        LOG_WARNING("Failed to load default font, using system font");
        defaultFont = TTF_OpenFont("arial.ttf", 16);
    }
    
    isInitialized = true;
    ResetPerformanceStats();
    
    LOG("WebGL Renderer initialized successfully");
    return true;
}

// Shutdown WebGL renderer
void WebGLRenderer::Shutdown() {
    ClearTextureCache();
    
    if (defaultFont) {
        TTF_CloseFont(defaultFont);
        defaultFont = nullptr;
    }
    
    if (sdlRenderer && ownsRenderer) {
        SDL_DestroyRenderer(sdlRenderer);
        sdlRenderer = nullptr;
    } else if (sdlRenderer) {
        // We don't own the renderer, just clear our reference
        sdlRenderer = nullptr;
    }
    
    TTF_Quit();
    IMG_Quit();
    isInitialized = false;
    
    LOG("WebGL Renderer shutdown complete");
}

// Begin frame
void WebGLRenderer::BeginFrame() {
    if (!isInitialized) return;
    
    frameStartTime = SDL_GetTicks();
    drawCalls = 0;
    
    // Clear the screen
    Clear(0.0f, 0.0f, 0.0f, 1.0f);
}

// End frame
void WebGLRenderer::EndFrame() {
    if (!isInitialized) return;
    
    // Present the frame
    Present();
    
    // Calculate frame time
    uint32_t frameEndTime = SDL_GetTicks();
    frameTime = static_cast<float>(frameEndTime - frameStartTime);
}

// Render menu background
void WebGLRenderer::RenderMenuBackground(const MenuBackgroundData& data) {
    if (!isInitialized || !data.visible) return;
    
    RenderTexture(data.texturePath, data.x, data.y, data.width, data.height, data.alpha);
    drawCalls++;
}

// Render menu item
void WebGLRenderer::RenderMenuItem(const MenuItemData& data) {
    if (!isInitialized || !data.visible) return;
    
    // Choose texture based on state
        std::string texturePath = (data.state == GraphicsMenuItemState::Active) ? 
                              data.activeTexturePath : data.inactiveTexturePath;
    
    RenderTexture(texturePath, data.x, data.y, data.width, data.height, data.alpha);
    drawCalls++;
}

// Render menu text
void WebGLRenderer::RenderMenuText(const MenuTextData& data) {
    if (!isInitialized || !data.visible) return;
    
    SDL_Texture* textTexture = CreateTextTexture(data.text, data);
    if (textTexture) {
        RenderTexture(textTexture, data.x, data.y, 0, 0, data.a);
        SDL_DestroyTexture(textTexture);
        drawCalls++;
    }
}

// Set viewport
void WebGLRenderer::SetViewport(int x, int y, int width, int height) {
    viewportX = x;
    viewportY = y;
    viewportWidth = width;
    viewportHeight = height;
    
    if (sdlRenderer) {
        SDL_Rect viewport = { x, y, width, height };
        SDL_RenderSetViewport(sdlRenderer, &viewport);
    }
}

// Clear screen
void WebGLRenderer::Clear(float r, float g, float b, float a) {
    if (!sdlRenderer) return;
    
    SDL_SetRenderDrawColor(sdlRenderer, 
        static_cast<Uint8>(r * 255), 
        static_cast<Uint8>(g * 255), 
        static_cast<Uint8>(b * 255), 
        static_cast<Uint8>(a * 255));
    SDL_RenderClear(sdlRenderer);
}

// Present frame
void WebGLRenderer::Present() {
    if (sdlRenderer) {
        SDL_RenderPresent(sdlRenderer);
    }
}

// Check feature support
bool WebGLRenderer::SupportsFeature(RendererFeature feature) {
    switch (feature) {
        case RendererFeature::WebGL2:
        case RendererFeature::WebGL1:
        case RendererFeature::TextureCompression:
        case RendererFeature::ShaderSupport:
            return true;
        case RendererFeature::WebGPU:
        case RendererFeature::MultiSampling:
            return false;
        default:
            return false;
    }
}

// Get renderer name
std::string WebGLRenderer::GetRendererName() const {
    return "WebGL (SDL2)";
}

// Reset performance stats
void WebGLRenderer::ResetStats() {
    ResetPerformanceStats();
}

// Reset performance statistics
void WebGLRenderer::ResetPerformanceStats() {
    frameTime = 0.0f;
    drawCalls = 0;
    frameStartTime = 0;
}

// Load texture with caching
SDL_Texture* WebGLRenderer::LoadTexture(const std::string& path) {
    // Check cache first
    auto it = textureCache.find(path);
    if (it != textureCache.end()) {
        return it->second;
    }
    
    // Load new texture
    SDL_Surface* surface = IMG_Load(path.c_str());
    if (!surface) {
        LOG_ERROR("Failed to load texture: " + path + " - " + IMG_GetError());
        return nullptr;
    }
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(sdlRenderer, surface);
    SDL_FreeSurface(surface);
    
    if (!texture) {
        LOG_ERROR("Failed to create texture from surface: " + path + " - " + SDL_GetError());
        return nullptr;
    }
    
    // Cache the texture
    textureCache[path] = texture;
    LOG("Loaded and cached texture: " + path);
    
    return texture;
}

// Render texture with position and size
void WebGLRenderer::RenderTexture(SDL_Texture* texture, float x, float y, float width, float height, float alpha) {
    if (!texture || !sdlRenderer) return;
    
    // Set alpha modulation
    SDL_SetTextureAlphaMod(texture, static_cast<Uint8>(alpha * 255));
    
    // Create destination rectangle
    SDL_Rect destRect = {
        static_cast<int>(x),
        static_cast<int>(y),
        static_cast<int>(width > 0 ? width : 100),  // Default width if not specified
        static_cast<int>(height > 0 ? height : 100) // Default height if not specified
    };
    
    // If width/height not specified, use texture's natural size
    if (width <= 0 || height <= 0) {
        int texWidth, texHeight;
        SDL_QueryTexture(texture, nullptr, nullptr, &texWidth, &texHeight);
        destRect.w = texWidth;
        destRect.h = texHeight;
    }
    
    SDL_RenderCopy(sdlRenderer, texture, nullptr, &destRect);
}

// Render texture by path
void WebGLRenderer::RenderTexture(const std::string& texturePath, float x, float y, float width, float height, float alpha) {
    SDL_Texture* texture = LoadTexture(texturePath);
    if (texture) {
        RenderTexture(texture, x, y, width, height, alpha);
    }
}

// Clear texture cache
void WebGLRenderer::ClearTextureCache() {
    for (auto& pair : textureCache) {
        if (pair.second) {
            SDL_DestroyTexture(pair.second);
        }
    }
    textureCache.clear();
}

// Create text texture
SDL_Texture* WebGLRenderer::CreateTextTexture(const std::string& text, const MenuTextData& textData) {
    if (!defaultFont || text.empty()) return nullptr;
    
    // Set font color
    SDL_Color color = {
        static_cast<Uint8>(textData.r * 255),
        static_cast<Uint8>(textData.g * 255),
        static_cast<Uint8>(textData.b * 255),
        static_cast<Uint8>(textData.a * 255)
    };
    
    // Set font style
    int style = TTF_STYLE_NORMAL;
    if (textData.bold) style |= TTF_STYLE_BOLD;
    if (textData.italic) style |= TTF_STYLE_ITALIC;
    TTF_SetFontStyle(defaultFont, style);
    
    // Set font size
    // TTF_SetFontSize is not available in older SDL_ttf versions, we'll use the font as is
    // In a real implementation, you'd need to load the font at the correct size
    
    // Render text surface
    SDL_Surface* surface = TTF_RenderText_Blended(defaultFont, text.c_str(), color);
    if (!surface) {
        LOG_ERROR("Failed to render text: " + text + " - " + TTF_GetError());
        return nullptr;
    }
    
    // Create texture from surface
    SDL_Texture* texture = SDL_CreateTextureFromSurface(sdlRenderer, surface);
    SDL_FreeSurface(surface);
    
    if (!texture) {
        LOG_ERROR(std::string("Failed to create text texture: ") + SDL_GetError());
        return nullptr;
    }
    
    return texture;
}
