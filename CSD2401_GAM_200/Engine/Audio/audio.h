/* Start Header ************************************************************************/
/*!
\file       audio.h
\author     Leck Xin Yee [80% - 52 lines]  
			Lee Hwee Min [20% - 8 lines]
\par        xinyee.leck@digipen.edu
			l.hweemin@digipen.edu
\date       October, 2, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#ifndef AUDIO_H
#define AUDIO_H

#include "fmod.hpp"
#include "fmod_studio.hpp"
#include "fmod_errors.h"
#include "System.h"

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <list>

#include "Message.h"
#include "IMessageHandler.h"

/**
 * @class AudioEngine
 * @brief Manages audio playback using FMOD sound system
 */
class AudioEngine : public IMessageHandler {
private:
	FMOD::System* system;
	std::map<std::string, FMOD::Sound*> sounds;
	std::map<std::string, FMOD::Channel*> channels;

public:
	/**
	 * @brief Constructs an AudioEngine
	 */
	AudioEngine();

	/**
	 * @brief Destructs the AudioEngine and cleans up resources
	 */
	~AudioEngine();

	AudioEngine(const AudioEngine&) = delete;
	AudioEngine& operator=(const AudioEngine&) = delete;

	/*  Core Functionalities  */
	/**
	 * @brief Initializes the FMOD audio system
	 * @return True if initialization was successful, false otherwise
	 */
	bool initialiseAudio();

	/**
	 * @brief Updates the audio system (should be called every frame)
	 */
	void updateAudio();

	/**
	 * @brief Stops all audio and shuts down the audio system
	 */
	void stopAudio();

	/*   Sound loading  */
	/**
	 * @brief Registers a pre-existing FMOD sound with a name
	 * @param name Identifier for the sound
	 * @param sound Pointer to FMOD::Sound object
	 * @return True if registration was successful, false otherwise
	 */
	bool registerSound(const std::string& name, FMOD::Sound* sound);

	/**
	 * @brief Loads a sound file from disk
	 * @param name Identifier for the sound
	 * @param filepath Path to the audio file
	 * @param loop Whether the sound should loop continuously
	 * @return True if loading was successful, false otherwise
	 */
	bool loadSound(const std::string& name, const std::string& filepath, bool loop = false);

	/**
	 * @brief Unloads a specific sound from memory
	 * @param name Identifier of the sound to unload
	 */
	void unloadSound(const std::string& name);

	/**
	 * @brief Unloads all sounds from memory
	 */
	void unloadAllSound();

	/*  Playback controls  */
	/**
	 * @brief Plays a loaded sound on a channel
	 * @param name Identifier of the sound to play
	 * @param channelName Optional name to assign to the playback channel
	 * @return True if playback started successfully, false otherwise
	 */
	bool playSound(const std::string& name, const std::string& channelName = "");

	/**
	 * @brief Stops playback on a specific channel
	 * @param channelName Name of the channel to stop
	 */
	void stopSound(const std::string& channelName);

	/**
	 * @brief Stops playback on all channels
	 */
	void stopAllSound();

	/**
	 * @brief Pauses or resumes playback on a channel
	 * @param channelName Name of the channel to pause/resume
	 * @param pause True to pause, false to resume
	 */
	void pauseSound(const std::string& channelName, bool pause);

	/**
	 * @brief Pauses or resumes all active channels
	 * @param pause True to pause all, false to resume all
	 */
	void pauseAllSound(bool pause);

	/*  Volume and effects controls  */
	/**
	 * @brief Sets the volume for a specific channel
	 * @param channelName Name of the channel
	 * @param volume Volume level (0.0 to 1.0)
	 */
	void setVolume(const std::string& channelName, float volume);

	/**
	 * @brief Sets the master volume for all audio
	 * @param volume Volume level (0.0 to 1.0)
	 */
	void masterVolume(float volume);

	/**
	 * @brief Toggles mute for all audio (master channel group)
	 */
	void masterMute();

	/**
	 * @brief Checks if a channel is currently playing
	 * @param channelName Name of the channel to check
	 * @return True if the channel is playing, false otherwise
	 */
	bool isPlaying(const std::string& channelName);

	/**
	 * @brief Toggles mute status for a channel
	 * @param channelName Name of the channel to mute/unmute
	 */
	void muteSound(const std::string& channelName);

	void setPitch(const std::string& channelName, float pitch);

	/*  Error Handling  */
	/**
	 * @brief Checks and logs FMOD errors
	 * @param result FMOD result code to check
	 * @param operation Description of the operation being performed
	 */
	void checkError(FMOD_RESULT result, const std::string& operation);

	/**
	 * @brief Gets the FMOD system pointer
	 * @return Pointer to the FMOD::System object
	 */
	FMOD::System* GetFMODSystem() const { return system; }

	/**
	 * @brief Plays a sound when a PlaySoundMessage is received.
	 * @param msg Contains the sound name to play.
	 */
	void HandleMessage(const PlaySoundMessage& msg) override {
		std::cout << "[AudioEngine] Playing sound: " << msg.soundName << std::endl;
		playSound(msg.soundName);
	}

	/**
	 * @brief Pauses or resumes a sound when a PauseSoundMessage is received.
	 * @param msg Contains the channel name and pause state.
	 */
	void HandleMessage(const PauseSoundMessage& msg) override {
		std::cout << "[AudioEngine] Pausing sound: " << msg.channelName << std::endl;
		pauseSound(msg.channelName, msg.pause);
	}
};

#endif // AUDIO_H
