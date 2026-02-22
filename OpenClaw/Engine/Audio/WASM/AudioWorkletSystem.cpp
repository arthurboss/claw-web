#include "AudioWorkletSystem.h"
#include <iostream>
#include <algorithm>
#include <cstdio>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#endif

AudioWorkletSystem::AudioWorkletSystem()
    : m_initialized(false)
    , m_soundEnabled(true)
    , m_musicEnabled(true)
    , m_soundVolume(1.0f)
    , m_musicVolume(1.0f)
    , m_musicPlaying(false)
    , m_musicLooping(false) {
}

AudioWorkletSystem::~AudioWorkletSystem() {
    Shutdown();
}

bool AudioWorkletSystem::Initialize() {
    if (m_initialized) {
        return true;
    }

#ifdef __EMSCRIPTEN__
    // Initialize SDL2_mixer for audio decoding
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "Failed to initialize SDL2_mixer: " << Mix_GetError() << std::endl;
        return false;
    }
    
    // Initialize Web Audio API directly (simpler approach)
    if (!InitializeAudioWorklet()) {
        std::cerr << "Failed to initialize Web Audio API" << std::endl;
        Mix_CloseAudio();
        return false;
    }
    
    // Skip AudioWorklet for now, use direct Web Audio API
    m_initialized = true;
    return true;
#endif

    m_initialized = true;
    return true;
}

void AudioWorkletSystem::Shutdown() {
    if (!m_initialized) {
        return;
    }

    StopAllSounds();
    StopMusic();
    
    m_soundBuffers.clear();
    m_musicBuffers.clear();
    m_initialized = false;
    
#ifdef __EMSCRIPTEN__
    Mix_CloseAudio();
#endif
}

bool AudioWorkletSystem::LoadSound(const std::string& name, const char* data, size_t size) {
    if (!m_initialized || !data || size == 0) {
        return false;
    }

    // Store sound data
    m_soundBuffers[name] = std::vector<char>(data, data + size);

#ifdef __EMSCRIPTEN__
    // Try to load actual WAV files for menu sounds
    return EM_ASM_INT({
        try {
            const name = UTF8ToString($0);
            
            // Map menu sound paths to our organized structure
            let wavFileName = 'sounds/menu/CLICK.WAV'; // default
            
            // Check if this is a menu selection sound
            if (name.includes('SELECT.WAV') || name.includes('SELECT_MENU_ITEM')) {
                wavFileName = 'sounds/menu/SELECT.WAV';
            } else if (name.includes('CLICK.WAV') || name.includes('CHANGE_MENU_ITEM')) {
                wavFileName = 'sounds/menu/CLICK.WAV';
            } else if (name.includes('MENUBED.WAV') || name.includes('MENUMUSIC')) {
                wavFileName = 'sounds/menu/MENUBED.WAV';

            }

            // Use fetch to load the WAV file
            fetch(wavFileName)
                .then(function(response) {
                    if (!response.ok) {
                        throw new Error('Failed to load WAV file: ' + response.status + ' ' + response.statusText);
                    }
                    return response.arrayBuffer();
                })
                .then(function(arrayBuffer) {
                    const audioContext = window.audioContext;
                    return audioContext.decodeAudioData(arrayBuffer);
                })
                .then(function(audioBuffer) {
                    window.soundBuffers = window.soundBuffers || new Map();
                    window.soundBuffers.set(name, audioBuffer);
                })
                .catch(function(error) {
                    console.error('Error loading WAV file:', error);
                    // Fallback to oscillator if WAV loading fails
                    window.soundBuffers = window.soundBuffers || new Map();
                    window.soundBuffers.set(name, {
                        type: 'oscillator',
                        frequency: 800,
                        duration: 0.5
                    });
                });
            
            return true;
        } catch (e) {
            console.error('Error loading sound:', e);
            return false;
        }
    }, name.c_str());
#endif

    return true;
}

bool AudioWorkletSystem::PlaySound(const std::string& name, float volume) {
    if (!m_initialized || !m_soundEnabled || m_soundBuffers.find(name) == m_soundBuffers.end()) {
        return false;
    }

#ifdef __EMSCRIPTEN__
    // Play sound using Web Audio API
    return EM_ASM_INT({
        try {
            const name = UTF8ToString($0);
            const volume = $1;
            
            const soundBuffers = window.soundBuffers;
            if (!soundBuffers || !soundBuffers.has(name)) {
                return false;
            }
            
            const soundData = soundBuffers.get(name);
            const audioContext = window.audioContext;
            
            if (soundData.type === 'oscillator') {
                // Play oscillator-based test sound
                const oscillator = audioContext.createOscillator();
                const gainNode = audioContext.createGain();
                
                oscillator.frequency.setValueAtTime(soundData.frequency, audioContext.currentTime);
                oscillator.type = 'sine';
                
                gainNode.gain.setValueAtTime(volume * window.soundVolume, audioContext.currentTime);
                gainNode.gain.exponentialRampToValueAtTime(0.01, audioContext.currentTime + soundData.duration);
                
                oscillator.connect(gainNode);
                gainNode.connect(audioContext.destination);
                oscillator.start();
                oscillator.stop(audioContext.currentTime + soundData.duration);
                return true;
            } else {
                // Play buffer-based sound (original code)
                const source = audioContext.createBufferSource();
                const gainNode = audioContext.createGain();
                
                source.buffer = soundData;
                source.loop = false;
                gainNode.gain.value = volume * window.soundVolume;
                
                source.connect(gainNode);
                gainNode.connect(audioContext.destination);
                source.start();
                return true;
            }
        } catch (e) {
            console.error('Error playing sound:', e);
            return false;
        }
    }, name.c_str(), volume * m_soundVolume);
#endif

    return true;
}

void AudioWorkletSystem::StopSound(const std::string& name) {
#ifdef __EMSCRIPTEN__
    EM_ASM({
        if (window.activeSources && window.activeSources.has(UTF8ToString($0))) {
            const name = UTF8ToString($0);
            try {
                window.activeSources.get(name).stop();
                window.activeSources.delete(name);
            } catch(e) { }
        }
    
        if (window.audioWorkletNode) {
            window.audioWorkletNode.port.postMessage({
                type: 'stopSound',
                name: UTF8ToString($0)
            });
        }
    }, name.c_str());
#endif
}

void AudioWorkletSystem::StopAllSounds() {
#ifdef __EMSCRIPTEN__
    EM_ASM({
        // Stop all active sources
        if (window.activeSources) {
            window.activeSources.forEach((source, path) => {
                try {
                    source.stop();
                } catch(e) { }
            });
            window.activeSources.clear();
        }
        
        if (window.audioWorkletNode) {
            window.audioWorkletNode.port.postMessage({
                type: 'stopAllSounds'
            });
        }
    });
#endif
}

bool AudioWorkletSystem::LoadMusic(const std::string& name, const char* data, size_t size) {
    if (!m_initialized || !data || size == 0) {
        return false;
    }

    // Store music data
    m_musicBuffers[name] = std::vector<char>(data, data + size);
    
#ifdef __EMSCRIPTEN__
    // Send music data to AudioWorklet
    EM_ASM({
        if (window.audioWorkletNode) {
            window.audioWorkletNode.port.postMessage({
                type: 'loadMusic',
                name: UTF8ToString($0),
                data: new Uint8Array(HEAPU8.buffer, $1, $2)
            });
        }
    }, name.c_str(), data, size);
#endif

    return true;
}

bool AudioWorkletSystem::PlayMusic(const std::string& name, bool looping) {
    if (!m_initialized || !m_musicEnabled || m_musicBuffers.find(name) == m_musicBuffers.end()) {
        return false;
    }

    m_currentMusic = name;
    m_musicLooping = looping;
    m_musicPlaying = true;

#ifdef __EMSCRIPTEN__
    // Send play command to AudioWorklet
    EM_ASM({
        if (window.audioWorkletNode) {
            window.audioWorkletNode.port.postMessage({
                type: 'playMusic',
                name: UTF8ToString($0),
                looping: $1,
                volume: $2
            });
        }
    }, name.c_str(), looping, m_musicVolume);
#endif

    return true;
}

void AudioWorkletSystem::StopMusic() {
    if (!m_musicPlaying) {
        return;
    }

    m_musicPlaying = false;
    m_currentMusic.clear();

#ifdef __EMSCRIPTEN__
    EM_ASM({
        // Stop the currently playing music source
        if (window.musicSource) {
            window.musicSource.stop();
            window.musicSource = null;
        }
        if (window.musicGainNode) {
            window.musicGainNode = null;
        }
        
        if (window.audioWorkletNode) {
            window.audioWorkletNode.port.postMessage({
                type: 'stopMusic'
            });
        }
    });
#endif
}

void AudioWorkletSystem::PauseMusic() {
    if (!m_musicPlaying) {
        return;
    }

#ifdef __EMSCRIPTEN__
    EM_ASM({
        if (window.audioWorkletNode) {
            window.audioWorkletNode.port.postMessage({
                type: 'pauseMusic'
            });
        }
    });
#endif
}

void AudioWorkletSystem::ResumeMusic() {
    if (!m_musicPlaying) {
        return;
    }

#ifdef __EMSCRIPTEN__
    EM_ASM({
        if (window.audioWorkletNode) {
            window.audioWorkletNode.port.postMessage({
                type: 'resumeMusic'
            });
        }
    });
#endif
}

bool AudioWorkletSystem::PlaySoundWithPath(const std::string& originalPath, const char* data, size_t size, float volume, int loops) {
    if (!m_initialized || !m_soundEnabled || !data || size == 0) {
        return false;
    }

    // Store sound data
    m_soundBuffers[originalPath] = std::vector<char>(data, data + size);

#ifdef __EMSCRIPTEN__
    // Use provided data parameter for all sounds (from CLAW.REZ)
    return EM_ASM_INT({
        try {
            const dataPtr = $0;
            const dataSize = $1;
            const volume = $2;
            const loops = $3;
            const originalPath = UTF8ToString($4);

            // Create a copy of the audio data from WASM memory
            const audioData = new Uint8Array(dataSize);
            audioData.set(HEAPU8.subarray(dataPtr, dataPtr + dataSize));
            const arrayBuffer = audioData.buffer;

            // Validate WAV file format
            const header = String.fromCharCode.apply(null, audioData.slice(0, 4));
            if (header !== 'RIFF') {
                console.error('[Audio] Invalid WAV header:', header);
                return false;
            }

            const audioContext = window.audioContext;
            audioContext.decodeAudioData(arrayBuffer.slice(0))
                .then(function(audioBuffer) {
                    console.log('[Audio] ✓ Loaded from CLAW.REZ:', originalPath,
                        '(' + (dataSize / 1024).toFixed(1) + 'KB)');

                    window.soundBuffers = window.soundBuffers || new Map();
                    window.soundBuffers.set(originalPath, audioBuffer);

                    // Play the sound immediately
                    const source = audioContext.createBufferSource();
                    const gainNode = audioContext.createGain();

                    source.buffer = audioBuffer;
                    source.loop = (loops === -1);

                    // Use music volume for MENUBED.WAV, sound volume for everything else
                    let volumeMultiplier = window.soundVolume;
                    let isMusic = false;
                    if (originalPath.includes('MENUBED.WAV') || originalPath.includes('MENUMUSIC')) {
                        volumeMultiplier = window.musicVolume;
                        isMusic = true;
                    }

                    let finalVolume = volume * volumeMultiplier;
                    if (isMusic && !window.musicEnabled) {
                        finalVolume = 0;
                    }

                    finalVolume = Math.max(0.0, Math.min(1.0, finalVolume));
                    gainNode.gain.value = finalVolume;

                    if (isMusic) {
                        window.musicGainNode = gainNode;
                        window.musicSource = source;
                        window.musicVolume = volumeMultiplier;
                    }

                    window.activeSources = window.activeSources || new Map();
                    const isLooping = (loops === -1);

                    if (isLooping || isMusic) {
                        if (window.activeSources.has(originalPath)) {
                            try {
                                window.activeSources.get(originalPath).stop();
                            } catch(e) {}
                        }
                        window.activeSources.set(originalPath, source);
                    }

                    source.onended = function() {
                        if (window.activeSources && window.activeSources.get(originalPath) === source) {
                             window.activeSources.delete(originalPath);
                        }
                    };

                    source.connect(gainNode);
                    gainNode.connect(audioContext.destination);
                    source.start();
                })
                .catch(function(error) {
                    console.error('[Audio] Failed to decode:', originalPath, error);
                });

            return true;
        } catch (e) {
            console.error('[Audio] Exception:', e);
            return false;
        }
    }, (const char*)data, size, volume * m_soundVolume, loops, originalPath.c_str());
#endif
    
    return true;
}

void AudioWorkletSystem::SetSoundVolume(float volume) {
    m_soundVolume = std::max(0.0f, std::min(1.0f, volume));
    
#ifdef __EMSCRIPTEN__
    EM_ASM({
        window.soundVolume = $0;
        if (window.audioWorkletNode) {
            window.audioWorkletNode.port.postMessage({
                type: 'setSoundVolume',
                volume: $0
            });
        }
    }, m_soundVolume);
#endif
}

void AudioWorkletSystem::SetMusicEnabled(bool enabled) {
    m_musicEnabled = enabled;
    
#ifdef __EMSCRIPTEN__
    EM_ASM({
        window.musicEnabled = $0;
        
        if (!window.musicEnabled) {
            // Mute music if disabled (but don't stop it)
            if (window.musicGainNode) {
                window.musicGainNode.gain.value = 0;
            }
        } else {
            // Resume music volume if enabled and we have a gain node
            if (window.musicGainNode) {
                window.musicGainNode.gain.value = window.musicVolume;
            }
        }
        
        if (window.audioWorkletNode) {
            window.audioWorkletNode.port.postMessage({
                type: 'setMusicEnabled',
                enabled: $0
            });
        }
    }, enabled);
#endif
}

void AudioWorkletSystem::SetMusicVolume(float volume) {
    // Volume is already clamped to 0.0-1.0 range by Audio::SetMusicVolume
    m_musicVolume = volume;
    
#ifdef __EMSCRIPTEN__
    EM_ASM({
        window.musicVolume = $0;
        
        // Update currently playing music gain if it exists and music is enabled
        if (window.musicGainNode && window.musicEnabled) {
            window.musicGainNode.gain.value = $0;
        }
        
        if (window.audioWorkletNode) {
            window.audioWorkletNode.port.postMessage({
                type: 'setMusicVolume',
                volume: $0
            });
        }
    }, m_musicVolume);
#endif
}

#ifdef __EMSCRIPTEN__
bool AudioWorkletSystem::InitializeAudioWorklet() {
    return EM_ASM_INT({
        try {
            // Create AudioContext
            window.AudioContext = window.AudioContext || window.webkitAudioContext;
            if (!window.AudioContext) {
                console.error('Web Audio API not supported');
                return false;
            }
            
            window.audioContext = new AudioContext();
            
            // Initialize global volume variables
            window.soundVolume = 1.0;
            window.musicVolume = 1.0;
            window.musicEnabled = true;
            window.soundEnabled = true;
            
            // Resume audio context on user interaction
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
        } catch (e) {
            console.error('Error initializing AudioContext:', e);
            return false;
        }
    });
}

bool AudioWorkletSystem::LoadAudioWorkletScript() {
    return EM_ASM_INT({
        try {
            // Check if AudioWorklet is supported
            if (!window.audioContext || !window.audioContext.audioWorklet) {
                console.error('AudioWorklet not supported in this browser');
                return false;
            }
            
            // Create AudioWorklet script
            const audioWorkletScript = 
                'class GameAudioProcessor extends AudioWorkletProcessor {' +
                '    constructor() {' +
                '        super();' +
                '        this.soundVolume = 1.0;' +
                '        this.musicVolume = 1.0;' +
                '        this.port.onmessage = (event) => { this.handleMessage(event.data); };' +
                '    }' +
                '    handleMessage(data) {' +
                '        switch (data.type) {' +
                '            case "setSoundVolume": this.soundVolume = data.volume; break;' +
                '            case "setMusicVolume": this.musicVolume = data.volume; break;' +
                '        }' +
                '    }' +
                '    process(inputs, outputs, parameters) {' +
                '        // Simple audio processing - just pass through with volume control' +
                '        const output = outputs[0];' +
                '        if (output && output.length > 0) {' +
                '            for (let channel = 0; channel < output.length; channel++) {' +
                '                const outputChannel = output[channel];' +
                '                if (outputChannel) {' +
                '                    for (let i = 0; i < outputChannel.length; i++) {' +
                '                        outputChannel[i] = 0; // Clear output' +
                '                    }' +
                '                }' +
                '            }' +
                '        }' +
                '        return true;' +
                '    }' +
                '}' +
                'registerProcessor("game-audio-processor", GameAudioProcessor);';
            
            // Create blob and load AudioWorklet
            const blob = new Blob([audioWorkletScript], { type: 'application/javascript' });
            const url = URL.createObjectURL(blob);
            
            // Load AudioWorklet synchronously to avoid timing issues
            return new Promise((resolve, reject) => {
                window.audioContext.audioWorklet.addModule(url)
                    .then(() => {
                        try {
                            window.audioWorkletNode = new AudioWorkletNode(window.audioContext, 'game-audio-processor');
                            window.audioWorkletNode.connect(window.audioContext.destination);
                            resolve(true);
                        } catch (e) {
                            console.error('Error creating AudioWorkletNode:', e);
                            reject(e);
                        }
                    })
                    .catch(error => {
                        console.error('Error loading AudioWorklet module:', error);
                        reject(error);
                    });
            }).then(() => true).catch(() => false);
        } catch (e) {
            console.error('Error setting up AudioWorklet:', e);
            return false;
        }
    });
}

void AudioWorkletSystem::SendMessageToAudioWorklet(const std::string& message) {
    EM_ASM({
        if (window.audioWorkletNode) {
            window.audioWorkletNode.port.postMessage(JSON.parse(UTF8ToString($0)));
        }
    }, message.c_str());
}
#else
bool AudioWorkletSystem::InitializeAudioWorklet() {
    return false;
}

bool AudioWorkletSystem::LoadAudioWorkletScript() {
    return false;
}

void AudioWorkletSystem::SendMessageToAudioWorklet(const std::string& message) {
    // No-op for non-WASM builds
}
#endif
