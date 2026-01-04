#pragma once

#ifdef __EMSCRIPTEN__

#include "../IGenericVideo.h"
#include <emscripten/emscripten.h>
#include <string>
#include <memory>
#include <functional>

/**
 * @brief WASM implementation of Video Player using WebCodecs API
 */
class WasmVideoPlayer : public IGenericVideo {
public:
    WasmVideoPlayer();
    ~WasmVideoPlayer() override;

    bool Load(const std::string& source) override;
    void Play() override;
    void Pause() override;
    void Stop() override;
    void Seek(double timeSeconds) override;
    void Update(float deltaTime) override;
    
    std::shared_ptr<ITexture> GetFrameTexture() override;

    bool IsPlaying() const override { return m_isPlaying; }
    double GetDuration() const override { return m_duration; }
    double GetCurrentTime() const override { return m_currentTime; }
    int GetWidth() const override { return m_width; }
    int GetHeight() const override { return m_height; }

    // Callbacks from JS
    void OnVideoLoaded(double duration, int width, int height);
    void OnVideoEnded();
    void OnFrameDecoded(int textureId, int width, int height); // For WebGL path
    // void OnFrameDecodedWebGPU(void* externalTexture); // For WebGPU path (todo)

private:
    bool m_isPlaying;
    double m_duration;
    double m_currentTime;
    int m_width;
    int m_height;
    
    std::string m_currentSource;
    std::shared_ptr<ITexture> m_currentFrameTexture;
    
    // JS object handle (int) to the VideoDecoder wrapper in JS
    int m_jsPlayerHandle;
};

#endif // __EMSCRIPTEN__
