#pragma once
#pragma once
#include "audio.h"
#include "IMessageHandler.h"
#include <string>
#include <map>
#include <vector>

struct SoundData {
    std::string name;
    std::string filepath;
    bool loop;
    float volume;
    float pitch;
    std::string category;
};

struct AudioEvent {
    std::string eventName;
    std::string soundName;
};

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
    AudioManager(AudioEngine* engine);
    ~AudioManager();

    bool LoadSound(const SoundData& sound);
    bool LoadAllSounds();
    void TriggerEvent(const std::string& eventName);
    void PlaySound(const std::string& soundName, const std::string& channelName = "");
    void SetSoundVolume(const std::string& soundName, float volume);
    void SetCategoryVolume(const std::string& category, float volume);
    void SetSoundPitch(const std::string& soundName, float pitch);
    void StopSound(const std::string& channelName);
    void StopAllSounds();
    void PauseAllSounds(bool pause); // Pause or resume all sounds
    void ToggleMute(); // Toggle mute for all audio
    void ReloadSound(const std::string& soundName);

    void ToggleMasterMute();
    bool IsMuted() const;

    void HandleMessage(const PlaySoundMessage& msg) override;
    void HandleMessage(const PauseSoundMessage& msg) override;

    void PrintConfig() const;

    // serialisation stuff
    const std::map<std::string, SoundData>& GetSoundDatabase() const { return soundDatabase; }
    std::map<std::string, SoundData>& GetSoundDatabase() { return soundDatabase; }
    const std::map<std::string, std::string>& GetEventMappings() const { return eventToSoundMap; }
    void AddSound(const SoundData& sound);
    void AddEventMapping(const std::string& eventName, const std::string& soundName);
    void RemoveSound(const std::string& soundName);

    void Clear();
};