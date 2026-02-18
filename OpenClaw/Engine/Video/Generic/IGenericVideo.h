#pragma once

#include <string>
#include <memory>
#include "../../Graphics/Generic/IGenericRenderer.h" // For ITexture

/**
 * @brief Interface for video playback functionality
 */
class IGenericVideo {
public:
    virtual ~IGenericVideo() = default;

    /**
     * @brief Load a video from a URL or file path
     * @param source URL or file path to video
     * @return true if loading started successfully
     */
    virtual bool Load(const std::string& source) = 0;

    /**
     * @brief Play the loaded video
     */
    virtual void Play() = 0;

    /**
     * @brief Pause video playback
     */
    virtual void Pause() = 0;

    /**
     * @brief Stop playback and rewind to beginning
     */
    virtual void Stop() = 0;

    /**
     * @brief Seek to specific time
     * @param timeSeconds Time in seconds
     */
    virtual void Seek(double timeSeconds) = 0;

    /**
     * @brief Update video state (decode frames)
     * @param deltaTime Time since last update
     */
    virtual void Update(float deltaTime) = 0;

    /**
     * @brief Get the current video frame as a texture
     * @return Shared pointer to texture containing current frame, or nullptr if not ready
     */
    virtual std::shared_ptr<ITexture> GetFrameTexture() = 0;

    /**
     * @brief Check if video is currently playing
     */
    virtual bool IsPlaying() const = 0;

    /**
     * @brief Get duration of video in seconds
     */
    virtual double GetDuration() const = 0;

    /**
     * @brief Get current playback time in seconds
     */
    virtual double GetCurrentTime() const = 0;

    /**
     * @brief Get video width
     */
    virtual int GetWidth() const = 0;

    /**
     * @brief Get video height
     */
    virtual int GetHeight() const = 0;
};
