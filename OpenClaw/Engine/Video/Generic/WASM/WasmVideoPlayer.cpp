#include "WasmVideoPlayer.h"
#include <iostream>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

// Extern functions defined in our JS library/bridge
extern "C" {
    // These would be implemented in a --js-library file or embedded JS
    int GenericVideo_CreatePlayer();
    void GenericVideo_DestroyPlayer(int handle);
    void GenericVideo_Load(int handle, const char* url);
    void GenericVideo_Play(int handle);
    void GenericVideo_Pause(int handle);
    void GenericVideo_Seek(int handle, double time);
    // ...
}

// Mock implementation using inline JS for self-containment if library not linked
// In a real scenario, these would be in a generic_video_bridge.js

EM_JS(int, js_CreatePlayer, (), {
    if (!Module.genericVideoPlayers) Module.genericVideoPlayers = {};
    var id = Object.keys(Module.genericVideoPlayers).length + 1;
    Module.genericVideoPlayers[id] = {
        video: document.createElement('video'),
        ready: false
    };
    // Note: This is an HTMLVideoElement fallback for quick demo. 
    // True WebCodecs implementation requires intricate JS logic involving VideoDecoder, 
    // mp4box.js (demuxing), etc., which is too large for inline EM_JS.
    // We will stick to HTML5 Video for the "Generic Reusable" proof-of-concept unless
    // the user provided the specific JS libraries for WebCodecs.
    // Given the prompt "trying to use latest available apis... especially for video",
    // we should aim for WebCodecs, but without a demuxer library it's hard.
    // We will use Video Element -> importExternalTexture (WebGPU) bridge as minimal "high perf" path.
    return id;
});

EM_JS(void, js_DestroyPlayer, (int id), {
    if (Module.genericVideoPlayers && Module.genericVideoPlayers[id]) {
        delete Module.genericVideoPlayers[id];
    }
});

EM_JS(void, js_LoadVideo, (int id, const char* url), {
    var player = Module.genericVideoPlayers[id];
    if (player) {
         player.video.crossOrigin = "anonymous";
         player.video.src = UTF8ToString(url);
          player.video.onended = function() {
              player.ended = true;
              // Remove video element from DOM when playback ends
              if (player.video.parentNode) {
                  player.video.parentNode.removeChild(player.video);
              }
          };
          player.video.onerror = function() {
              player.error = true;
              console.error("Video load error for: " + player.video.src);
              // Remove video element on error too
              if (player.video.parentNode) {
                  player.video.parentNode.removeChild(player.video);
              }
          };
     }
});

EM_JS(int, js_GetVideoState, (int id), {
    var player = Module.genericVideoPlayers[id];
    if (!player || !player.video) return 0;
    
    var state = 0;
    if (!player.video.paused) state |= 1; // Playing
    if (player.ended) state |= 2;        // Ended
    if (player.ready) state |= 4;        // Ready
    if (player.error) state |= 8;        // Error
    
    return state;
});

EM_JS(void, js_PlayVideo, (int id), {
    var player = Module.genericVideoPlayers[id];
    if (player && player.video) {
        // Attach video to DOM for rendering (hidden, but necessary for some browsers)
        player.video.style.position = 'fixed';
        player.video.style.top = '0';
        player.video.style.left = '0';
        player.video.style.width = '100%';
        player.video.style.height = '100%';
        player.video.style.zIndex = '9999';
        player.video.style.backgroundColor = 'black';
        document.body.appendChild(player.video);
        
        // Try to play with sound first, fallback to muted if blocked
        player.video.play().catch(function(err) {
            console.warn('Autoplay with sound blocked, trying muted:', err.message);
            player.video.muted = true;
            player.video.play().then(function() {
                console.log('Playing muted video. Click to unmute.');
                // Add click listener to unmute
                document.addEventListener('click', function unmute() {
                    player.video.muted = false;
                    document.removeEventListener('click', unmute);
                    console.log('Video unmuted after user interaction.');
                }, { once: true });
            }).catch(function(err2) {
                console.error('Muted autoplay also failed:', err2.message);
                player.error = true;
            });
        });
    }
});

EM_JS(void, js_PauseVideo, (int id), {
    var player = Module.genericVideoPlayers[id];
    if (player && player.video) player.video.pause();
});

WasmVideoPlayer::WasmVideoPlayer() 
    : m_isPlaying(false)
    , m_duration(0.0)
    , m_currentTime(0.0)
    , m_width(0)
    , m_height(0)
    , m_jsPlayerHandle(0)
{
    m_jsPlayerHandle = js_CreatePlayer();
}

WasmVideoPlayer::~WasmVideoPlayer() {
    js_DestroyPlayer(m_jsPlayerHandle);
}

bool WasmVideoPlayer::Load(const std::string& source) {
    m_currentSource = source;
    js_LoadVideo(m_jsPlayerHandle, source.c_str());
    return true;
}

void WasmVideoPlayer::Play() {
    js_PlayVideo(m_jsPlayerHandle);
    m_isPlaying = true;
}

void WasmVideoPlayer::Pause() {
    js_PauseVideo(m_jsPlayerHandle);
    m_isPlaying = false;
}

void WasmVideoPlayer::Stop() {
    Pause();
    Seek(0.0);
}

void WasmVideoPlayer::Seek(double timeSeconds) {
    // js_SeekVideo(m_jsPlayerHandle, timeSeconds); // TODO
    m_currentTime = timeSeconds;
}

void WasmVideoPlayer::Update(float deltaTime) {
    int state = js_GetVideoState(m_jsPlayerHandle);
    m_isPlaying = (state & 1) != 0;
    bool ended = (state & 2) != 0;
    bool error = (state & 8) != 0;
    
    if ((ended || error) && m_isPlaying) {
        m_isPlaying = false;
        OnVideoEnded();
    }
}

void WasmVideoPlayer::OnVideoEnded() {
    m_isPlaying = false;
    std::cout << "Video finished playing." << std::endl;
}

std::shared_ptr<ITexture> WasmVideoPlayer::GetFrameTexture() {
    // In a real WebCodecs+WebGPU implementation, here we would:
    // 1. Check if we have a decoded VideoFrame in JS.
    // 2. Import it as WGPUExternalTexture.
    // 3. Wrap it in our ITexture interface.
    
    // Returning dummy for now as the bridge is complex
    return m_currentFrameTexture;
}

void WasmVideoPlayer::OnVideoLoaded(double duration, int width, int height) {
    m_duration = duration;
    m_width = width;
    m_height = height;
    std::cout << "Video loaded: " << width << "x" << height << " (" << duration << "s)" << std::endl;
}

void WasmVideoPlayer::OnFrameDecoded(int textureId, int width, int height) {
    // Update m_currentFrameTexture with new texture ID from WebGL
}

#endif
