#include "../../Engine/SharedDefines.h"
#include "WebAudioAPI.h"
#include <emscripten.h>
#include <iostream>
#include <vector>
#include <map>
#include <string>

// Simple Web Audio API implementation using JavaScript
class WebAudioAPI {
private:
    bool isInitialized;
    float soundVolume;
    float musicVolume;
    bool soundEnabled;
    bool musicEnabled;

public:
    WebAudioAPI() : isInitialized(false), soundVolume(0.5f), musicVolume(0.5f), 
                    soundEnabled(true), musicEnabled(true) {
        // Initialize with safe defaults
        soundVolume = 1.0f;
        musicVolume = 1.0f;
    }

    bool Initialize() {
        if (isInitialized) {
            return true;
        }

        // Use JavaScript to initialize Web Audio API
        EM_ASM({
            try {
                // Create global audio context if it doesn't exist
                if (!window.audioContext) {
                    window.AudioContext = window.AudioContext || window.webkitAudioContext;
                    if (!window.AudioContext) {
                        console.error('Web Audio API is not supported in this browser');
                        return false;
                    }
                    
                    window.audioContext = new AudioContext();
                    
                    // Create and configure gain nodes
                    window.soundGainNode = window.audioContext.createGain();
                    window.musicGainNode = window.audioContext.createGain();
                    
                    // Set initial gain values
                    window.soundGainNode.gain.value = 1.0; // Full volume by default
                    window.musicGainNode.gain.value = 1.0; // Full volume by default
                    
                    // Connect gain nodes to audio context
                    window.soundGainNode.connect(window.audioContext.destination);
                    window.musicGainNode.connect(window.audioContext.destination);
                    
                    // Resume audio context on any user interaction
                    const resumeAudio = () => {
                        if (window.audioContext.state === 'suspended') {
                            window.audioContext.resume();
                        }
                        document.removeEventListener('click', resumeAudio);
                        document.removeEventListener('keydown', resumeAudio);
                        document.removeEventListener('touchstart', resumeAudio);
                    };
                    
                    document.addEventListener('click', resumeAudio);
                    document.addEventListener('keydown', resumeAudio);
                    document.addEventListener('touchstart', resumeAudio);
                    return true;
                }
            } catch (e) {
                console.error('Error initializing Web Audio API:', e);
                return false;
            }
        });
        
        isInitialized = true;
        // Set initial volumes
        SetSoundVolume(soundVolume);
        SetMusicVolume(musicVolume);
        
        return true;
    }

    bool LoadSound(const std::string& name, const char* data, size_t size) {
        if (!isInitialized) return false;
        
        // For now, just return true - we'll implement actual loading later
        return true;
    }

    bool PlaySound(const std::string& name, float volume = 1.0f) {
        if (!isInitialized || !soundEnabled) return false;
        return true;
    }

    bool PlayMusic(const std::string& name, bool looping = false) {
        if (!isInitialized || !musicEnabled) return false;
        return true;
    }

    void StopMusic() {
    }

    void PauseMusic() {
    }

    void ResumeMusic() {
    }

    void SetSoundVolume(float volume) {
        // Ensure volume is within valid range
        soundVolume = max(0.0f, min(1.0f, volume));
        if (!isInitialized) {
            return;
        }
        
        // Use EM_ASM to safely set the volume in JavaScript
        EM_ASM({
            try {
                if (window.soundGainNode) {
                    window.soundGainNode.gain.value = $0;
                }
            } catch (e) {
                console.error('Error setting sound volume:', e);
            }
        }, soundVolume);
    }

    void SetMusicVolume(float volume) {
        // Ensure volume is within valid range
        musicVolume = max(0.0f, min(1.0f, volume));
        if (!isInitialized) {
            return;
        }
        
        // Use EM_ASM to safely set the volume in JavaScript
        EM_ASM({
            try {
                if (window.musicGainNode) {
                    window.musicGainNode.gain.value = $0;
                }
            } catch (e) {
                console.error('Error setting music volume:', e);
            }
        }, musicVolume);
    }

    void SetSoundEnabled(bool enabled) {
        soundEnabled = enabled;
    }

    void SetMusicEnabled(bool enabled) {
        musicEnabled = enabled;
        if (!enabled) {
            StopMusic();
        }
    }

    void StopAllSounds() {
        StopMusic();
    }
};

// Global Web Audio API instance
static WebAudioAPI* g_webAudio = nullptr;

// C interface for the game
extern "C" {
    bool WebAudio_Initialize() {
        if (!g_webAudio) {
            g_webAudio = new WebAudioAPI();
        }
        return g_webAudio->Initialize();
    }

    bool WebAudio_LoadSound(const char* name, const char* data, size_t size) {
        if (!g_webAudio) return false;
        return g_webAudio->LoadSound(std::string(name), data, size);
    }

    bool WebAudio_PlaySound(const char* name, float volume) {
        if (!g_webAudio) return false;
        return g_webAudio->PlaySound(std::string(name), volume);
    }

    bool WebAudio_PlayMusic(const char* name, bool looping) {
        if (!g_webAudio) return false;
        return g_webAudio->PlayMusic(std::string(name), looping);
    }

    void WebAudio_StopMusic() {
        if (g_webAudio) g_webAudio->StopMusic();
    }

    void WebAudio_PauseMusic() {
        if (g_webAudio) g_webAudio->PauseMusic();
    }

    void WebAudio_ResumeMusic() {
        if (g_webAudio) g_webAudio->ResumeMusic();
    }

    void WebAudio_SetSoundVolume(float volume) {
        if (g_webAudio) g_webAudio->SetSoundVolume(volume);
    }

    void WebAudio_SetMusicVolume(float volume) {
        if (g_webAudio) g_webAudio->SetMusicVolume(volume);
    }

    void WebAudio_SetSoundEnabled(bool enabled) {
        if (g_webAudio) g_webAudio->SetSoundEnabled(enabled);
    }

    void WebAudio_SetMusicEnabled(bool enabled) {
        if (g_webAudio) g_webAudio->SetMusicEnabled(enabled);
    }

    void WebAudio_StopAllSounds() {
        if (g_webAudio) g_webAudio->StopAllSounds();
    }
}
