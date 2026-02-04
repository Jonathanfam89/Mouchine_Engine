/* Start Header ************************************************************************/
/*!
\file       AudioManager.cpp
\author     Leck Xin Yee [100% - 55 lines]
\par        xinyee.leck@digipen.edu
\date       November, 29, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#pragma once
#include "audio.h"
#include "IMessageHandler.h"
#include <string>
#include <map>
#include <vector>

/**
 * @struct SoundData
 * @brief Stores all properties required to load and play a sound.
 */
struct SoundData {
    std::string name;
    std::string filepath;
    bool loop;
    float volume;
    float pitch;
    std::string category;
};

/**
 * @struct AudioEvent
 * @brief Maps an event name to the sound that should be triggered.
 */
struct AudioEvent {
    std::string eventName;
    std::string soundName;
};


/**
 * @class AudioManager
 * @brief High-level manager for sound loading, playback, categories, events, and mute logic.
 */
class AudioManager : public IMessageHandler {
private:
    AudioEngine* audioEngine;

    // store loaded sound data
    std::map<std::string, SoundData> soundDatabase;

    // event to sound mappings
    std::map<std::string, std::string> eventToSoundMap;

    // category volumes (music, sfx, voice, etc.)
    std::map<std::string, float> categoryVolumes;

    // Mute state
    bool m_isMuted;
    std::map<std::string, float> m_preMuteCategoryVolumes;

public:
    /**
     * @brief Constructs the AudioManager with an existing AudioEngine.
     * @param engine Pointer to AudioEngine used for low-level playback.
     */
    AudioManager(AudioEngine* engine);

    /**
     * @brief Destroys the AudioManager and stops all sounds.
     */
    ~AudioManager();

    /**
     * @brief Loads a single sound based on metadata.
     * @param sound Structure containing sound properties.
     * @return True if loading succeeded.
     */
    bool LoadSound(const SoundData& sound);

    /**
     * @brief Loads all sounds stored in the sound database.
     * @return True if all sounds loaded successfully.
     */
    bool LoadAllSounds();

    /**
     * @brief Triggers an event, playing its mapped sound.
     * @param eventName Name of the event.
     */
    void TriggerEvent(const std::string& eventName);

    /**
     * @brief Plays a sound by name.
     * @param soundName Identifier of the sound.
     * @param channelName Optional channel override.
     */
    void PlaySound(const std::string& soundName, const std::string& channelName = "");

    /**
     * @brief Sets the volume of a specific sound.
     * @param soundName Name of the sound.
     * @param volume New volume (0.0�1.0).
     */
    void SetSoundVolume(const std::string& soundName, float volume);

    /**
     * @brief Sets the volume of a sound category.
     * @param category Category name (music, sfx, etc.)
     * @param volume New volume (0.0�1.0).
     */
    void SetCategoryVolume(const std::string& category, float volume);

    /**
     * @brief Sets pitch for a specific sound.
     * @param soundName Name of the sound.
     * @param pitch Pitch multiplier.
     */
    void SetSoundPitch(const std::string& soundName, float pitch);

    /**
     * @brief Stops playback on a specific channel.
     * @param channelName Name of the playback channel.
     */
    void StopSound(const std::string& channelName);

    /**
     * @brief Stops all active sound channels.
     */
    void StopAllSounds();

    /**
     * @brief Pauses or resumes all sounds.
     * @param pause True to pause, false to resume.
     */
    void PauseAllSounds(bool pause);

    /**
     * @brief Toggles muting on the FMOD master channel group.
     */
    void ToggleMute();

    /**
     * @brief Toggles muting on the FMOD master channel group.
     */
    void ReloadSound(const std::string& soundName);

    /**
     * @brief Toggles the AudioManager's master mute state.
     */
    void ToggleMasterMute();

    /**
     * @brief Checks whether the audio manager is currently muted.
     * @return True if muted.
     */
    bool IsMuted() const;

    /**
     * @brief Handles PlaySoundMessage from the message bus.
     * @param msg Message containing sound name.
     */

    void HandleMessage(const PlaySoundMessage& msg) override;

    /**
     * @brief Handles PauseSoundMessage from the message bus.
     * @param msg Message containing channel and pause flag.
     */
    void HandleMessage(const PauseSoundMessage& msg) override;

    /**
     * @brief Prints all sound configuration (debug only).
     */
    void PrintConfig() const;

    // serialisation stuff
    /**
     * @brief Gets the internal sound database (modifiable).
     */
    const std::map<std::string, SoundData>& GetSoundDatabase() const { return soundDatabase; }

    /**
     * @brief Gets the event-to-sound mapping table.
     */
    std::map<std::string, SoundData>& GetSoundDatabase() { return soundDatabase; }

    /**
     * @brief Gets the event-to-sound mapping table.
     */
    const std::map<std::string, std::string>& GetEventMappings() const { return eventToSoundMap; }

    /**
     * @brief Registers a new sound entry.
     * @param sound Sound metadata.
     */
    void AddSound(const SoundData& sound);

    /**
     * @brief Assigns a sound to a specific event.
     * @param eventName Name of event.
     * @param soundName Name of sound.
     */
    void AddEventMapping(const std::string& eventName, const std::string& soundName);

    /**
     * @brief Removes a sound and any associated events.
     * @param soundName Name of the sound.
     */
    void RemoveSound(const std::string& soundName);

    /**
     * @brief Clears all sound data and event mappings.
     */
    void Clear();

    /**
     * @brief Starts fading out a specific category of audio.
     * @param category The category to fade out (e.g., "music", "sfx").
     * @param duration The duration of the fade in seconds.
     */
    void FadeOutCategory(const std::string& category, float duration);

    /**
     * @brief Starts fading in a specific category of audio.
     * @param category The category to fade in (e.g., "music", "sfx").
     * @param duration The duration of the fade in seconds.
     * @param targetVolume The target volume to fade to (default 1.0).
     */
    void FadeInCategory(const std::string& category, float duration, float targetVolume = 1.0f);

    /**
     * @brief Starts fading out all audio categories.
     * @param duration The duration of the fade in seconds.
     */
    void FadeOutAll(float duration);

    /**
     * @brief Updates the fade state (should be called every frame).
     * @param dt Delta time in seconds.
     */
    void UpdateFade(float dt);

    /**
     * @brief Resets all fade states and restores default category volumes.
     */
    void ResetFadeStates();

    /**
     * @brief Fade state tracking structure (public for UI access).
     */
    struct FadeState {
        bool isFading = false;
        bool isFadeIn = false;  // true for fade in, false for fade out
        float startVolume = 1.0f;
        float targetVolume = 1.0f;
        float currentTime = 0.0f;
        float duration = 1.0f;
    };

    /**
     * @brief Gets the fade states for all categories (for UI display).
     */
    const std::map<std::string, FadeState>& GetFadeStates() const { return categoryFadeStates; }

    /**
     * @brief Checks if any category is currently fading.
     * @return True if any fade is in progress.
     */
    bool IsAnyFadeActive() const;

private:
    std::map<std::string, FadeState> categoryFadeStates;
};