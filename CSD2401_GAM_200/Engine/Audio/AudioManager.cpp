#include "AudioManager.h"
#include "Message.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <iostream>

namespace
{
    bool IsSupportedAudioFile(const std::string& path)
    {
        namespace fs = std::filesystem;
        fs::path p(path);
        std::string ext = p.extension().string();

        std::transform(ext.begin(), ext.end(), ext.begin(),
            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

        return (ext == ".wav" || ext == ".mp3" || ext == ".ogg");
    }
}

AudioManager::AudioManager(AudioEngine* engine) : audioEngine(engine), m_isMuted(false)
{
    // default category volumes
    categoryVolumes["music"] = 1.0f;
    categoryVolumes["sfx"] = 1.0f;
    categoryVolumes["voice"] = 1.0f;
    categoryVolumes["ambient"] = 1.0f;
}

AudioManager::~AudioManager() 
{
    StopAllSounds();
}

bool AudioManager::IsMuted() const
{
    return m_isMuted;
}

bool AudioManager::LoadSound(const SoundData& sound) 
{
    if (!audioEngine) 
    {
        std::cerr << "AudioEngine not set in AudioManager" << std::endl;
        return false;
    }

    // reject unsupported extensions early
    if (!IsSupportedAudioFile(sound.filepath))
    {
        std::cerr << "[AudioManager] Unsupported audio file type for " << sound.filepath << " (expected .wav, .mp3, .ogg)" << std::endl;
        return false;
    }
        
    std::cout << "Loading (single): " << sound.name << " from " << sound.filepath << std::endl;

    bool success = audioEngine->loadSound(sound.name, sound.filepath, sound.loop);
    if (!success) 
    {
        std::cerr << "Failed to load sound: " << sound.name << std::endl;
    }
    return success;
}

bool AudioManager::LoadAllSounds() 
{
    if (!audioEngine) 
    {
        std::cerr << "AudioEngine not set in AudioManager" << std::endl;
        return false;
    }

    std::cout << "\n=== Loading Audio Files ===" << std::endl;

    bool allSuccessful = true;
    for (const auto& pair : soundDatabase) 
    {
        const SoundData& sound = pair.second;

        std::cout << "Loading: " << sound.name << " from " << sound.filepath << std::endl;

        bool success = audioEngine->loadSound(sound.name, sound.filepath, sound.loop);
        if (!success) 
        {
            std::cerr << "Failed to load sound: " << sound.name << std::endl;
            allSuccessful = false;
        }
    }

    std::cout << "Audio loading complete" << std::endl;
    return allSuccessful;
}

void AudioManager::TriggerEvent(const std::string& eventName) 
{
    auto it = eventToSoundMap.find(eventName);
    if (it != eventToSoundMap.end()) 
    {
        PlaySound(it->second);
    }
    else 
    {
        std::cout << "No sound mapped to event: " << eventName << std::endl;
    }
}

void AudioManager::PlaySound(const std::string& soundName, const std::string& channelName) 
{
    if (!audioEngine) 
    {
        std::cerr << "AudioEngine not available" << std::endl;
        return;
    }

    auto it = soundDatabase.find(soundName);
    if (it == soundDatabase.end()) 
    {
        std::cerr << "Sound not found in database: " << soundName << std::endl;
        return;
    }

    const SoundData& sound = it->second;

    // Play the sound
    std::string channel = channelName.empty() ? soundName + "_channel" : channelName;
    audioEngine->playSound(soundName, channel);

    // Apply volume (base volume * category volume)
    float finalVolume = sound.volume;
    auto categoryIt = categoryVolumes.find(sound.category);
    if (categoryIt != categoryVolumes.end()) 
    {
        finalVolume *= categoryIt->second;
    }

    // Enforce master mute
    if (m_isMuted) {
        finalVolume = 0.0f;
    }

    audioEngine->setVolume(channel, finalVolume);
    audioEngine->setPitch(channel, sound.pitch <= 0.0f ? 1.0f : sound.pitch);
}

void AudioManager::SetSoundVolume(const std::string& soundName, float volume)
{
    auto it = soundDatabase.find(soundName);
    if (it == soundDatabase.end()) return;

    it->second.volume = volume;

    // modify the currently playing channel
    std::string channel = soundName + "_channel";
    float finalVolume = volume;

    auto categoryIt = categoryVolumes.find(it->second.category);
    if (categoryIt != categoryVolumes.end())
    {
        finalVolume *= categoryIt->second;
    }

    audioEngine->setVolume(channel, finalVolume);
}

void AudioManager::SetCategoryVolume(const std::string& category, float volume) 
{
    float clampedVolume = std::max(0.0f, std::min(1.0f, volume));

    categoryVolumes[category] = clampedVolume;
    std::cout << "Set " << category << " volume to " << clampedVolume << std::endl;

    // If the user changes a volume while not muted, we should update our pre-mute reference.
    if (!m_isMuted) {
        m_preMuteCategoryVolumes = categoryVolumes;
    }

    if (!audioEngine)
    {
        return;
    }

    // Update volume for all sounds in this category
    for (const auto& pair : soundDatabase)
    {
        const SoundData& sound = pair.second;
        if (sound.category == category)
        {
            float finalVolume = sound.volume * categoryVolumes[category];
            std::string channelName = sound.name + "_channel";
            audioEngine->setVolume(channelName, finalVolume);
        }
    }
}

void AudioManager::ToggleMasterMute()
{
    m_isMuted = !m_isMuted;

    std::map<std::string, float> targetVolumes;

    if (m_isMuted)
    {
        // Muting: store current volumes and set target to 0
        m_preMuteCategoryVolumes = categoryVolumes;
        for (auto const& [category, vol] : categoryVolumes)
        {
            targetVolumes[category] = 0.0f;
        }
    }
    else
    {
        // Un-muting: target is the stored pre-mute volumes
        if (!m_preMuteCategoryVolumes.empty())
        {
            targetVolumes = m_preMuteCategoryVolumes;
        }
        else // Fallback
        {
            targetVolumes["music"] = 1.0f;
            targetVolumes["sfx"] = 1.0f;
            targetVolumes["voice"] = 1.0f;
            targetVolumes["ambient"] = 1.0f;
        }
    }

    // Now apply the target volumes
    for (auto const& [category, vol] : targetVolumes)
    {
        SetCategoryVolume(category, vol);
    }

    // --- Direct Patch ---
    // Force-update the BGM channels just in case the SetCategoryVolume loop missed them.
    if (audioEngine && categoryVolumes.count("music"))
    {
        float musicVol = categoryVolumes["music"];
        audioEngine->setVolume("bgm_channel", musicVol);
        audioEngine->setVolume("menu_bgm_channel", musicVol);
        std::cout << "Forcing BGM channel volumes to " << musicVol << std::endl;
    }
}

void AudioManager::SetSoundPitch(const std::string& soundName, float pitch)
{
    auto it = soundDatabase.find(soundName);
    if (it == soundDatabase.end())
        return;

    if (pitch < 0.25f) pitch = 0.25f;
    if (pitch > 3.0f)  pitch = 3.0f;

    it->second.pitch = pitch;

    if (!audioEngine)
        return;

    // update currently playing default channel for this sound
    std::string channel = soundName + "_channel";
    audioEngine->setPitch(channel, pitch);
}



void AudioManager::StopSound(const std::string& channelName) 
{
    if (audioEngine) 
    {
        audioEngine->stopSound(channelName);
    }
}

void AudioManager::StopAllSounds() 
{
    if (audioEngine) 
    {
        audioEngine->stopAllSound();
    }
}

void AudioManager::ReloadSound(const std::string& soundName)
{
    if (!audioEngine)
    {
        return;
    }
        
    auto it = soundDatabase.find(soundName);
    if (it == soundDatabase.end())
    {
        return;
    }
 
    const SoundData& sound = it->second;

    // stop the default channel for this sound if it�s playing
    std::string channelName = soundName + "_channel";
    audioEngine->stopSound(channelName);

    // unload the fmod sound so any old loop flag is cleared
    audioEngine->unloadSound(soundName);

    // reload with the updated loop flag from sound data
    audioEngine->loadSound(sound.name, sound.filepath, sound.loop);
}

void AudioManager::HandleMessage(const PlaySoundMessage& msg) 
{
    PlaySound(msg.soundName);
}

void AudioManager::HandleMessage(const PauseSoundMessage& msg) 
{
    if (audioEngine) 
    {
        audioEngine->pauseSound(msg.channelName, msg.pause);
    }
}

// TO BE REMOVED CAUSE JUST FOR DEBUGGING
void AudioManager::PrintConfig() const {
    std::cout << "\n=== Audio Manager Configuration ===" << std::endl;

    std::cout << "\nSounds (" << soundDatabase.size() << "):" << std::endl;
    for (const auto& pair : soundDatabase) 
    {
        const SoundData& sound = pair.second;
        std::cout << "  - " << sound.name << " [" << sound.category << "]" << " (loop: " << (sound.loop ? "yes" : "no") << ", vol: " << sound.volume << ")" << std::endl;
        std::cout << "    File: " << sound.filepath << std::endl;
    }

    std::cout << "\nEvent Mappings (" << eventToSoundMap.size() << "):" << std::endl;
    for (const auto& pair : eventToSoundMap) {
        std::cout << "  - " << pair.first << " -> " << pair.second << std::endl;
    }

    std::cout << "\nCategory Volumes:" << std::endl;
    for (const auto& pair : categoryVolumes) {
        std::cout << "  - " << pair.first << ": " << pair.second << std::endl;
    }
}

void AudioManager::AddSound(const SoundData& sound) 
{
    soundDatabase[sound.name] = sound;
}

void AudioManager::AddEventMapping(const std::string& eventName, const std::string& soundName) 
{
    eventToSoundMap[eventName] = soundName;
}

void AudioManager::RemoveSound(const std::string& soundName)
{
    // stop the default channel for this sound
    if (audioEngine)
    {
        audioEngine->stopSound(soundName + "_channel");
        audioEngine->unloadSound(soundName);
    }

    // remove from database
    soundDatabase.erase(soundName);

    // remove any event mappings that refer to this sound
    for (auto it = eventToSoundMap.begin(); it != eventToSoundMap.end(); )
    {
        if (it->second == soundName)
            it = eventToSoundMap.erase(it);
        else
            ++it;
    }

    std::cout << "[AudioManager] Removed sound: " << soundName << std::endl;
}

void AudioManager::Clear() 
{
    soundDatabase.clear();
    eventToSoundMap.clear();
}
