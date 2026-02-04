/* Start Header ************************************************************************/
/*!
\file       CutsceneSystem.cpp
\author     Lee Hwee Min [100% - ??? lines]
\par        l.hweemin@digipen.edu
\date       Janauary, 31, 2026

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/


#include "CutsceneSystem.h"
#include "core.h"
#include "GraphicsSystem.h"
#include "AudioManager.h"
#include "InputManager.h"
#include <iostream>

// Disable warnings from pl_mpeg.h
#pragma warning(push)
#pragma warning(disable : 4267) // Disable size_t to int/long conversion
#pragma warning(disable : 4244) // int to uint8_t conversion
#pragma warning(disable : 4305) // double to float truncation

#define PL_MPEG_IMPLEMENTATION
extern "C" {
    #include "pl_mpeg.h"
}
#pragma warning(pop)

/**
 * @brief Initializes OpenGL resources required for video rendering.
 * @Creates a VAO and VBO for a full-screen quad.
 */
void CutsceneSystem::Initialize() {
	std::cout << "CutsceneSystem Initialized." << std::endl;

	float quadVertices[] = {
		// positions        // uv
		-1.0f,  1.0f, 0.0f,  0.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,  0.0f, 1.0f,
		 1.0f, -1.0f, 0.0f,  1.0f, 1.0f,

		-1.0f,  1.0f, 0.0f,  0.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,  1.0f, 1.0f,
		 1.0f,  1.0f, 0.0f,  1.0f, 0.0f
	};

	glGenVertexArrays(1, &m_vao);
	glGenBuffers(1, &m_vbo);

	glBindVertexArray(m_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

	// Position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	// UV attribute
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	// Create Texture that will hold video frames
	glGenTextures(1, &m_videoTextureID);
	glBindTexture(GL_TEXTURE_2D, m_videoTextureID);

	// Video textures Clamp to Edge to avoid weird borders
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Linear filtering -> low-res videos look smoother
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

/**
 * @brief Main update loop for video playback.
 * @param dt Delta time since last frame in seconds.
 */
void CutsceneSystem::Update(float dt) {
	if (!m_isPlaying || !m_plm) return;

	double fps = plm_get_framerate(m_plm);
	if (fps <= 5.0 || fps > 120.0) fps = 10.0; // force setting
	double frameTime = 1.0 / fps;

	m_timeAccumulator += static_cast<double>(dt);

	// Sync with the clock
	while (m_timeAccumulator >= frameTime) {
		plm_frame_t* frame = plm_decode_video(m_plm);

		if (frame) {
			UpdateVideoTexture(frame);
			m_timeAccumulator -= frameTime; // one frame's time
		}
		else {
			StopCutscene(); // End of video file
			return;
		}
		// throttle (avoid sprinting through frames)
		if (m_timeAccumulator > frameTime) m_timeAccumulator = frameTime * 0.5;
	}

	RenderVideo();

	// Check if Skip (Spacebar)
	if (CORE->GetInputManager()->IsKeyPressed(GLFW_KEY_SPACE) || CORE->GetInputManager()->IsMouseLeftClicked()) {
		StopCutscene();
	}
}

/**
 * @brief Loads and begins playback of a video file.
 * @param videoPath Relative path to the .mpeg file.
 */
void CutsceneSystem::PlayVideo(const std::string& videoPath, const std::string& audioName) {
	m_plm = plm_create_with_filename(videoPath.c_str());
	if (!m_plm) return;

	m_currentVideoPath = videoPath; // Store the name
	m_isPlaying = true;
	m_timeAccumulator = 0.0;

	plm_set_audio_enabled(m_plm, 0); // Disable internal audio decoding (using AudioManager instead)
	CORE->SetGamePaused(true);		 // Pause game loop

	if (!audioName.empty()) {		 // Play Audio if provided
		m_currentAudio = audioName;
		CORE->GetAudioManager()->PlaySound(audioName);
	}
}

/**
 * @brief Stops video playback and releases resources.
 */
void CutsceneSystem::StopCutscene() {
	if (!m_isPlaying) return;

	if (!m_currentVideoPath.empty()) {
		m_finishedVideos.insert(m_currentVideoPath);
	}

	m_isPlaying = false;

	// Cleanup Decoder
	if (m_plm) {
		plm_destroy(m_plm);
		m_plm = nullptr;
	}

	// Stop Audio if provided
	if (!m_currentAudio.empty()) {
		CORE->GetAudioManager()->StopSound(m_currentAudio + "_channel");
		m_currentAudio = "";
	}

	m_timeAccumulator = 0.0;

	// Resume Game
	//m_currentVideoPath = ""; // Reset
	CORE->SetGamePaused(false);
}

/**
 * @brief Updates the OpenGL texture with the latest decoded YUV frame.
 * @param frame_ptr Pointer to the decoded plm_frame_t.
 */
void CutsceneSystem::UpdateVideoTexture(void* frame_ptr) {
	if (!frame_ptr) return;
	plm_frame_t* frame = static_cast<plm_frame_t*>(frame_ptr);

	// Reallocate RGB buffer if video dimensions change
	if (!m_rgbBuffer || m_bufferWidth != frame->width || m_bufferHeight != frame->height) {
		m_bufferWidth = frame->width;
		m_bufferHeight = frame->height;
		delete[] m_rgbBuffer;
		m_rgbBuffer = new unsigned char[m_bufferWidth * m_bufferHeight * 3];

		// Allocate the texture memory
		glBindTexture(GL_TEXTURE_2D, m_videoTextureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_bufferWidth, m_bufferHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	}

	// Convert YUV to RGB and upload to GPU
	plm_frame_to_rgb(frame, m_rgbBuffer, m_bufferWidth * 3);

	// Update the existing texture data
	glBindTexture(GL_TEXTURE_2D, m_videoTextureID);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_bufferWidth, m_bufferHeight, GL_RGB, GL_UNSIGNED_BYTE, m_rgbBuffer);
}

/**
 * @brief Renders the video texture to a full-screen quad.
 */
void CutsceneSystem::RenderVideo() {
	if (!m_isPlaying) return;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, CORE->GetGraphicsSystem()->GetWindowWidth(), CORE->GetGraphicsSystem()->GetWindowHeight());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GLSLShader* videoShader = CORE->GetGraphicsSystem()->GetDefaultShader();
	if (videoShader) {
		videoShader->Use();
		videoShader->SetUniform("uMVP", glm::mat4(1.0f));
		videoShader->SetUniform("uUseBatching", 0);
		videoShader->SetUniform("uHasTexture", 1);
		videoShader->SetUniform("uIsCircle", 0);
		videoShader->SetUniform("uColor", glm::vec4(1.0f));
		videoShader->SetUniform("uTiling", glm::vec2(1.0f));
		videoShader->SetUniform("uTexOffset", glm::vec2(0.0f));

		// Bind Texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_videoTextureID);

		// Draw
		glBindVertexArray(m_vao);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
	}
}

/**
 * @brief Returns the name of the system.
 * @return A string representing the system's name.
 */
std::string CutsceneSystem::GetName()
{
	return "Cutscene System";
}