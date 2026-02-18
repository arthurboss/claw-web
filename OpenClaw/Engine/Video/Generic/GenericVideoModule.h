#pragma once

#include "IGenericVideo.h"
#include <memory>
#include <string>
#include <vector>

/**
 * @brief Generic Video Module - Cross-platform video playback solution
 * 
 * Provides a unified interface for playing video content using platform-specific
 * implementations (e.g., WebCodecs on WASM).
 */
class GenericVideoModule {
public:
    GenericVideoModule();
    ~GenericVideoModule();

    /**
     * @brief Initialize the video module
     * @return true if successful
     */
    bool Initialize();

    /**
     * @brief Shutdown the module
     */
    void Shutdown();

    /**
     * @brief Create a video player instance
     * @return Shared pointer to generic video player
     */
    std::shared_ptr<IGenericVideo> CreateVideoPlayer();

    /**
     * @brief Get module version
     */
    static std::string GetVersion();

private:
    std::string DetectPlatform() const;

    bool m_initialized;
    std::string m_platform;
};
