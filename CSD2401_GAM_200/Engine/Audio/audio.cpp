/* Start Header ************************************************************************/
/*!
\file       audio.cpp
\author     Leck Xin Yee [100% - 191 lines]
\par        xinyee.leck@digipen.edu
\date       October, 2, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "audio.h"
#include <algorithm>

AudioEngine::AudioEngine() : sounds(), channels(), system(nullptr) {}
AudioEngine::~AudioEngine() { stopAudio(); }

/*  Core Functionalities  */
/**
 * @brief Initializes the FMOD audio system with 32 max concurrent sounds
 */
bool AudioEngine::initialiseAudio() {
	FMOD_RESULT result;

	result = FMOD::System_Create(&system);
	if (result != FMOD_OK) {
		checkError(result, "System_Create");
		return false;
	}

	// set 32 as max amt of sounds playing at once
	result = system->init(32, FMOD_INIT_NORMAL, nullptr);
	if (result != FMOD_OK) {
		checkError(result, "System init");
		return false;
	}

	std::cout << "Audio initialised sucessfully" << std::endl;
	return true;
}

/**
 * @brief Updates the FMOD audio system (must be called every frame)
 */
void AudioEngine::updateAudio() {
	// must be called every frame in game loop
	if (system) {
		system->update();
	}
}

/**
 * @brief Stops all audio, unloads all sounds, and shuts down the system
 */
void AudioEngine::stopAudio() {
	stopAllSound();
	unloadAllSound();

	if (system) {
		system->close();
		system->release();
		system = nullptr;
	}

	std::cout << "Audio stopped successfully" << std::endl;
}

/*   Sound loading  */
/**
 * @brief Registers a pre-existing FMOD sound with a name identifier
 */
bool AudioEngine::registerSound(const std::string& name, FMOD::Sound* sound) {
	if (!system) {
		std::cout << "Audio system not initialised" << std::endl;
		return false;
	}

	// check if sound already exists
	if (sounds.find(name) != sounds.end()) {
		std::cout << "Sound " << name << " already registered" << std::endl;
		return true;
	}

	sounds[name] = sound;
	std::cout << "Registered sound: " << name << std::endl;
	return true;
}

/**
 * @brief Loads a sound file from disk with optional looping
 */
bool AudioEngine::loadSound(const std::string& name, const std::string& filepath, bool loop) {
	if (!system) {
		std::cout << "It's not initialised" << std::endl;
		return false;
	}
	// check sound exist
	if (sounds.find(name) != sounds.end()) {
		std::cout << "Sound: " << name << " alr loaded" << std::endl;
		return true;
	}
	FMOD::Sound* sound = nullptr;
	FMOD_MODE mode = FMOD_DEFAULT;	// basic audio settings

	if (loop) {
		mode |= FMOD_LOOP_NORMAL;
	}
	else {
		mode |= FMOD_LOOP_OFF;
	}
	// load the audio file
	FMOD_RESULT result = system->createSound(filepath.c_str(), mode, nullptr, &sound);
	if (result != FMOD_OK) {
		checkError(result, "Loading sound " + filepath);
		return false;
	}
	sounds[name] = sound;
	std::cout << "Loaded sound from " << filepath << std::endl;
	return true;
}

/**
 * @brief Unloads a specific sound and removes it from the sound map
 */
void AudioEngine::unloadSound(const std::string& name) {
	auto it = sounds.find(name);
	if (it != sounds.end()) {
		it->second->release();
		sounds.erase(it);
		std::cout << "Unloaded sound: " << name << std::endl;
	}
}

/**
 * @brief Unloads all sounds and clears the sound and channel maps
 */
void AudioEngine::unloadAllSound() {
	for (auto& pair : sounds) {
		pair.second->release();
	}
	sounds.clear();
	channels.clear();
	std::cout << "All sound unloaded" << std::endl;
}

/*  Playback controls  */
/**
 * @brief Plays a loaded sound on a named or unnamed channel
 */
bool AudioEngine::playSound(const std::string& name, const std::string& channelName) {
	if (!system) {
		std::cout << "Audio system not initialised..." << std::endl;
		return false;
	}

	auto soundIt = sounds.find(name);
	if (soundIt == sounds.end()) {
		std::cout << "Sound cant be found: " << name << std::endl;
		return false;
	}

	FMOD::Channel* channel = nullptr;
	// Start paused to true to prevent initial volume click
	FMOD_RESULT result = system->playSound(soundIt->second, nullptr, true, &channel);
	if (result != FMOD_OK) {
		checkError(result, "Sound playing: " + name);
		return false;
	}

	if (!channelName.empty()) {
		channels[channelName] = channel;
	}

	// Set initial volume to 0 before unpausing to prevent click
	if (channel) {
		channel->setVolume(0.0f);
		channel->setPaused(false);  // Now unpause with volume at 0
	}

	std::cout << "Sound playing: " << name << "\n" << std::endl;
	return true;
}

/**
 * @brief Stops playback on a specific channel and removes it from the channel map
 */
void AudioEngine::stopSound(const std::string& channelName) {
	auto it = channels.find(channelName);
	if (it != channels.end() && it->second) {
		bool isPlaying = false;

		it->second->isPlaying(&isPlaying);
		if (isPlaying) {
			it->second->stop();
			std::cout << "Stopped sound on channel: " << channelName << std::endl;
		}
		channels.erase(it);
	}
}

/**
 * @brief Stops playback on all active channels and clears the channel map
 */
void AudioEngine::stopAllSound() {
	for (auto& pair : channels) {
		if (pair.second) {
			bool isPlaying = false;
			pair.second->isPlaying(&isPlaying);
			if (isPlaying) {
				pair.second->stop();
			}
		}
	}
	channels.clear();
	std::cout << "Stopped all sound" << std::endl;
}

/**
 * @brief Pauses or resumes playback on a specific channel
 */
void AudioEngine::pauseSound(const std::string& channelName, bool pause) {
	auto it = channels.find(channelName);
	if (it != channels.end() && it->second) {
		it->second->setPaused(pause);
		std::cout << (pause ? "paused" : "playing") << channelName << std::endl;
	}
}

/**
 * @brief Pauses or resumes all active channels
 */
void AudioEngine::pauseAllSound(bool pause) {
	for (auto& pair : channels) {
		if (pair.second) {
			pair.second->setPaused(pause);
		}
	}
	std::cout << (pause ? "Paused" : "Resumed") << " all sounds" << std::endl;
}

// Volume and effects controls
/**
 * @brief Sets the volume for a specific channel (clamped between 0.0 and 1.0)
 */
void AudioEngine::setVolume(const std::string& channelName, float volume) {
	auto it = channels.find(channelName);
	if (it != channels.end() && it->second) {
		volume = std::max(0.0f, std::min(1.0f, volume));
		it->second->setVolume(volume); // set volume for only that channel
	}
}

/**
 * @brief Sets the master volume for all audio (clamped between 0.0 and 1.0)
 */
void AudioEngine::masterVolume(float volume) {
	if (system) {
		FMOD::ChannelGroup* masterGroup = nullptr;
		system->getMasterChannelGroup(&masterGroup);
		if (masterGroup) {
			volume = std::max(0.0f, std::min(1.0f, volume));
			masterGroup->setVolume(volume); // set volume for all audio
			std::cout << "Set master volume to" << volume << std::endl;
		}
	}
}

/**
 * @brief Toggles mute for all audio (master channel group)
 */
void AudioEngine::masterMute() {
	if (system) {
		FMOD::ChannelGroup* masterGroup = nullptr;
		system->getMasterChannelGroup(&masterGroup);
		if (masterGroup) {
			bool muted = false;
			masterGroup->getMute(&muted);
			masterGroup->setMute(!muted);
			std::cout << "[AudioEngine] All audio " << (muted ? "unmuted" : "muted") << std::endl;
		}
	}
}

/**
 * @brief Checks if a specific channel is currently playing
 */
bool AudioEngine::isPlaying(const std::string& channelName) {
	auto it = channels.find(channelName);
	if (it != channels.end() && it->second) {
		bool isPlaying = false;
		it->second->isPlaying(&isPlaying);
		return isPlaying; // true if playing, false if stopped
	}
	return false;
}

/**
 * @brief Toggles the mute status for a specific channel
 */
void AudioEngine::muteSound(const std::string& channelName) {
	auto it = channels.find(channelName);
	if (it != channels.end() && it->second) {
		bool muted = false;
		it->second->getMute(&muted); // current mute state
		it->second->setMute(!muted); // if muted, unmute it,  if unmuted, mute it
		std::cout << (muted ? "unmuted" : "muted") << "channel" << channelName << std::endl;
	}
}

void AudioEngine::setPitch(const std::string& channelName, float pitch) 
{
	auto it = channels.find(channelName);
	if (it != channels.end() && it->second)
	{
		if (pitch < 0.0f)
			pitch = 0.0f; 
		it->second->setPitch(pitch);
	}
}

/*  Error Handling  */
/**
 * @brief Checks FMOD result and logs error message if operation failed
 */
void AudioEngine::checkError(FMOD_RESULT result, const std::string& operation) {
	if (result != FMOD_OK) {
		std::cout << "fmod error in " << operation << ": " << FMOD_ErrorString(result) << result << std::endl;
	}
}
