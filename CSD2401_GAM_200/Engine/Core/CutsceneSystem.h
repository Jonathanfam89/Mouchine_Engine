/* Start Header ************************************************************************/
/*!
\file       CutsceneSystem.h
\author     Lee Hwee Min [100% - ??? lines]
\par        l.hweemin@digipen.edu
\date       Janauary, 31, 2026

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include "System.h"
#include <GL/glew.h>
#include <string>
#include <set>

// Forward declaration of the PLM struct
struct plm_t;

/**
 * @class CutsceneSystem
 * @brief Manages the playback of video and image-based cutscenes.
 */
class CutsceneSystem : public ISystem {
public:
	/**
	 * @brief Constructor - Initializes playback state and buffer pointers.
	 */
	CutsceneSystem()
		: m_isPlaying(false),
		m_videoTextureID(0),
		m_vao(0),
		m_vbo(0),
		m_plm(nullptr),
		m_timeAccumulator(0.0),

		m_rgbBuffer(nullptr),    // ADD THIS: Fixes crash on destruction/init
		m_bufferWidth(0),        // ADD THIS
		m_bufferHeight(0)        // ADD THIS
	{}

	/**
	 * @brief Destructor - Ensures all video decoders, buffers, and textures are freed.
	 */
	~CutsceneSystem() override {
		StopCutscene();
		if (m_videoTextureID) glDeleteTextures(1, &m_videoTextureID);
		if (m_vao) glDeleteVertexArrays(1, &m_vao);
		if (m_vbo) glDeleteBuffers(1, &m_vbo);
		if (m_rgbBuffer) delete[] m_rgbBuffer; // Clean up the buffer
	}


	void Initialize() override;

	/** @brief Updates the video decoder and advances playback by the delta time. */
	void Update(float dt) override;

	/**
	 * @brief Returns the system's name for debugging/identification.
	 * @return A string: "Cutscene System".
	 */
		std::string GetName() override;

	/**
	 * @brief Plays an MPEG-1 video file.
	 * @param videoPath Path to the .mpeg file.
	 * @param audioName Optional name of the audio file to play via AudioManager.
	 */
	void PlayVideo(const std::string& videoPath, const std::string& audioName = "");

	/**
	 * @brief Stops current playback and closes video file handles.
	 */
	void StopCutscene();

	/** @return True if a video is currently being decoded and displayed. */
	bool IsPlaying() const { return m_isPlaying; }

	/**
	 * @brief Copies raw frame data into the OpenGL texture.
	 * @param frame_ptr Pointer to the decoded RGB/RGBA frame data.
	 */
	void UpdateVideoTexture(void* frame_ptr);

	/**
	 * @brief Returns true if this specific file has finished playing before
	 * @param videoPath Path to the .mpeg file.
	 */
	bool HasFinished(const std::string& videoPath) const {
		return m_finishedVideos.find(videoPath) != m_finishedVideos.end();
	}

private:
	bool m_isPlaying;

	std::string m_currentVideoPath;         // The file currently playing
	std::set<std::string> m_finishedVideos; // List of all completed videos

	// Audio syncing
	std::string m_currentAudio;

	// OpenGL handles
	unsigned int m_videoTextureID;  // Texture ID used for rendering the video frame
	unsigned int m_vao;             // Vertex Array Object for the video quad
	unsigned int m_vbo;             // Vertex Buffer Object for the video quad

	unsigned char* m_rgbBuffer;     // Intermediate CPU buffer for pixel data conversion
	int m_bufferWidth;              // Width of the current video file
	int m_bufferHeight;             // Height of the current video file

	// Video decoder state
	plm_t* m_plm;				// Pointer for the MPEG decoder
	double m_timeAccumulator;	// To track decoding time



	void RenderVideo();
};