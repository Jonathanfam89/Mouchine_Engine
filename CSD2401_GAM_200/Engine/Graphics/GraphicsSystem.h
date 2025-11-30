/* Start Header ************************************************************************/
/*!
\file       GraphicsSystem.h
\author     Guan Shao Jun [100%]
\par        shaojun.g@digipen.edu
\date       November 07 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
// Standard Library and Third-Party Headers
#include "stb_image.h" // Image loading library
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <map>

// Engine Core and Component Headers
#include "System.h"
#include "gameObject.h"
#include "glslshader.h"
#include "factory.h"
#include "transform.h"
#include "DebugSystem.h"
#include "RenderComponent.h"
#include "TextComponent.h"
#include "CameraManager.h"
#include "MathLib.h"
#include "GameObjectSystem.h"
#include "Message.h"

/**
 * @class GraphicsSystem
 * @brief Manages the rendering pipeline, GLFW window, OpenGL context, and
 * is responsible for efficient 2D batch rendering and debug visualization.
 *
 * Implements the ISystem interface for engine loop integration.
 */
class GraphicsSystem : public ISystem
{
public:
	/**
	 * @brief Default constructor. Initializes GLFW and creates a default 800x600 window.
	 */
	GraphicsSystem();

	/**
	 * @brief Constructor with specified window dimensions.
	 * @param W Initial window width.
	 * @param H Initial window height.
	 */
	GraphicsSystem(GLint W, GLint H);

	/**
	 * @brief Destructor. Cleans up OpenGL resources (FBOs, Buffers) and terminates GLFW.
	 */
	~GraphicsSystem();

	/**
	 * @brief Initializes OpenGL state, GLEW, shaders (default, debug, text), and rendering buffers (VAOs/VBOs).
	 */
	void Initialize() override;

	/**
	 * @brief The main rendering loop function. Binds framebuffer, clears screen, accumulates
	 * GameObjects into batches, draws batches, and renders debug/text elements.
	 * @param deltaTime Time elapsed since the last frame (not directly used for rendering logic).
	 */
	void Update(float) override;

	/**
	 * @brief Returns the system's name for debugging/identification.
	 * @return A string: "Graphics System".
	 */
	std::string GetName() override;

	/**
	 * @brief Loads a 2D texture from file using stb_image.
	 * @param filepath Path to an image file (e.g., PNG, JPG).
	 * @return GLuint handle to the OpenGL texture ID, or 0 on failure.
	 */
	static GLuint LoadTexture(const char* filepath);

	/**
	 * @brief Returns the GLFW window pointer.
	 * @return Pointer to the underlying GLFWwindow object.
	 */
	GLFWwindow* getWindow() { return window; }

	/**
	 * @brief Draws a debug line segment between two 2D points in world space.
	 * @param start World coordinate for line start.
	 * @param end World coordinate for line end.
	 * @param color RGBA color of the line.
	 * @param viewProjection The combined view-projection matrix (MVP).
	 */
	void DrawLine(const vec2& start, const vec2& end,
		const vec4& color, const mat4& viewProjection);

	/**
	 * @brief Draws a single debug point in world space.
	 * @param pos Position in 2D world space.
	 * @param color RGBA color of the point.
	 * @param viewProjection The combined view-projection matrix (MVP).
	 */
	void DrawPoint(const vec2& pos,
		const vec4& color, const mat4& viewProjection);

	/**
	 * @brief Draws the outline of an axis-aligned rectangle.
	 * @param x Center X position.
	 * @param y Center Y position.
	 * @param w Width of rectangle.
	 * @param h Height of rectangle.
	 * @param color RGBA color of outline.
	 * @param viewProjection The combined view-projection matrix (MVP).
	 */
	void DrawRectOutline(float x, float y, float w, float h,
		const vec4& color, const mat4& viewProjection);

	/**
	 * @brief Draws a circle outline approximated by line segments.
	 * @param cx Center X position.
	 * @param cy Center Y position.
	 * @param r Radius of the circle.
	 * @param segments Number of line segments (higher = smoother).
	 * @param color RGBA color.
	 * @param viewProjection The combined view-projection matrix (MVP).
	 */
	void DrawCircleOutline(float cx, float cy, float r, int segments,
		const vec4& color, const mat4& viewProjection);

	/**
	 * @brief Checks if debug drawing mode is currently enabled.
	 * @return true if debug drawing is enabled, false otherwise.
	 */
	bool isDebugModeOn() const { return debugMode; }

	/**
	 * @brief Toggles the debug drawing mode (on/off).
	 */
	void toggleDebugMode() { debugMode = !debugMode; }

	/**
	 * @brief Returns the current view-projection matrix being used for rendering.
	 * @return Constant reference to the mat4 view-projection matrix.
	 */
	const mat4& getVp() const { return vp; }

	/**
	 * @brief Sets a pointer to the LevelEditor instance for camera control.
	 * @param editor Pointer to the LevelEditor object.
	 */
#ifdef USE_IMGUI
	void SetLevelEditor(class LevelEditor* editor) { levelEditor = editor; }
#endif
	/**
	 * @brief Executes all draw calls for the accumulated render batches.
	 * Uploads vertex and index data and issues GL_TRIANGLES draw calls.
	 */
	void DrawBatches();

	void SetEditorSelection(float x, float y, float sx, float sy) {
		currentSelection = { x,y,sx,sy,true };
	}

	void SetEditorSelection_ColliderOnly(float x, float y, float col_sx, float col_sy) {
		currentSelection_ColliderOnly = { x,y,col_sx,col_sy,true };
	}

	void ClearEditorSelection() {
		currentSelection.exist = false;
		currentSelection_ColliderOnly.exist = false;
	}

	/**
	 * @brief Renders a string of text using a font atlas.
	 * @param characters Map containing glyph data (texture ID, metrics).
	 * @param text The string content to render.
	 * @param x Starting X position.
	 * @param y Starting Y position.
	 * @param scale Scaling factor for text size.
	 * @param color RGBA color of the text.
	 * @param screenSpace If true, uses orthographic screen-space projection; otherwise, uses world-space camera VP.
	 */
	void RenderText(const std::map<char, Character>& characters, const std::string& text, float x, float y, float scale, const vec4& color, bool screenSpace = true);

	/**
	 * @brief Manages the off-screen framebuffer for ImGui rendering.
	 * Resizes the framebuffer if the provided viewport dimensions change.
	 * @param viewportSizeX Reference to the desired viewport width.
	 * @param viewportSizeY Reference to the desired viewport height.
	 */
	void RenderGameViewportImGui(float& viewportSizeX, float& viewportSizeY);

	/**
	 * @brief Checks if the system is currently rendering to the off-screen framebuffer (Editor Mode).
	 * @return true if rendering to FBO, false if rendering directly to the window.
	 */
	bool IsEditorMode() const { return editorMode; };

	/**
	 * @brief Enables or disables Editor Mode (rendering to the FBO).
	 * @param enable True to enable FBO rendering, false to disable.
	 */
	void SetEditorMode(bool enable) { editorMode = enable; };

	/**
	 * @brief Returns the OpenGL texture ID of the FBO's color attachment.
	 * Used by ImGui to display the game viewport.
	 * @return GLuint texture ID.
	 */
	GLuint GetFrameBufferTexture() const { return frameBufferTexture; };

	/**
	 * @brief Gets the current GLFW window width.
	 * @return The width in pixels.
	 */
	int GetWindowWidth() const { return Wwidth; };

	/**
	 * @brief Gets the current GLFW window height.
	 * @return The height in pixels.
	 */
	int GetWindowHeight() const { return Wheight; };

	void DrawFlashOverlay(const vec4& color);

	/**
	 * @brief Handles messages relevant to the Graphics System, specifically CreateObjectMessage
	 * to ensure new objects receive a default RenderComponent.
	 * @param msg The incoming CreateObjectMessage.
	 */
	void HandleMessage(const CreateObjectMessage& msg);

	void ToggleFullScreen();
	bool IsFullscreen() const { return isFullscreen; }

	void LoadClickCursors(const char* normalPath, const char* clickPath, int normalHotX = 0, int normalHotY = 0, int clickHotX = 0, int clickHotY = 0);
	void LoadDefaultCursor(const char* cursorPath, int hotX = 0, int hotY = 0);
	GLFWcursor* GetNormalCursor() const { return normalCursor; }
	GLFWcursor* GetClickCursor() const { return clickCursor; }

private:
	// --- Window and State ---
	GLFWwindow* window;						// The handle to the GLFW window.
	GLint Wwidth, Wheight;					// Current dimensions of the window's client area (in screen coordinates).
	int windowedPosX, windowedPosY;
	int windowedWidth, windowedHeight;
	bool isFullscreen = true;
	mat4 vp;								// Current view-projection matrix (camera transform).
	bool debugMode = false;					// Flag to control rendering of debug shapes (lines, outlines).
	bool editorMode;						// Flag to indicate if rendering should target the FBO (true) or the screen (false).
#ifdef USE_IMGUI
	class LevelEditor* levelEditor = nullptr; // Pointer to the level editor for camera manipulation when in editor mode.
#endif
	GLFWcursor* normalCursor = nullptr;
	GLFWcursor* clickCursor = nullptr;

	// --- Shaders ---
	GLSLShader shader;		// Default shader for batched quads/circles.
	GLSLShader textShader;	// Shader dedicated to rendering font glyphs.
	GLSLShader debugShader; // Shader for drawing non-batched items like lines and points.

	// --- Single Quad Rendering (Legacy/Test) Buffers ---
	GLuint vao = 0, vbo = 0, tex = 0;		// VAO/VBO for a simple quad/rectangle draw (used in older pipelines).

	// --- Batch Rendering Structures and Buffers ---
	/**
	 * @brief Structure for a single vertex in the batch buffer. Contains local position
	 * and world transformation data to be processed in the vertex shader.
	 */
	struct BatchVertex
	{
		vec3 pos;		// Local position relative to the object's center (-0.5 to 0.5).
		vec2 tex;		// Texture coordinates (UV).
		vec4 color;		// Color tint (RGBA).
		vec2 uvScale;	// UV tiling/scaling factor (for spritesheets).
		vec2 uvOffset;	// UV offset (for spritesheets/animation frame selection).
		vec3 transform;	// World Translation (x, y) and Rotation (z).
		vec2 scale;		// World Scale (x, y).
	};

	/**
	 * @brief Represents a single group of GameObjects that can be drawn in one GL draw call.
	 * Grouped by Texture ID, Shape Type (Square/Circle), and Layer.
	 */
	struct RenderBatch
	{
		GLuint textureID;				// The OpenGL texture ID bound for the entire batch.
		bool isCircle;					// Flag indicating if the geometry should be rendered as a circle (uses specialized fragment shader logic).
		std::vector<BatchVertex> vertices; // Vertex data for all quads in this batch.
		std::vector<GLuint> indices;	// Index data (EBO) for all quads in this batch.
		int layer;						// Rendering layer (for depth sorting, typically based on Z-position).

		/**
		 * @brief Constructor for RenderBatch.
		 */
		RenderBatch(GLuint texID, bool circle, int l) : textureID(texID), isCircle(circle), layer(l) {}
	};

	// For drawing outline of individual GOs
	struct EditorSelection {
		float x, y, sx, sy;
		bool exist = false;
	};

	EditorSelection currentSelection;
	EditorSelection currentSelection_ColliderOnly;

	std::vector<RenderBatch> batches;		// Collection of all accumulated batches for the current frame.
	GLuint batchVAO, batchVBO, batchEBO;	// OpenGL buffers for the batch rendering system.

	// --- Text Rendering Buffers ---
	GLuint textVAO, textVBO;				// VAO/VBO dedicated to rendering dynamically generated text quads.

	//Flash Overlay
	GLuint flashVAO = 0;
	GLuint flashVBO = 0;

	// Batch size constants (must match memory allocation in Initialize)
	const size_t MAX_BATCH_VERTICES = 12000; // Max vertices the batch VBO can hold (e.g., 3000 quads * 4 vertices).
	const size_t MAX_BATCH_INDICES = 18000;	 // Max indices the batch EBO can hold (e.g., 3000 quads * 6 indices).

	// --- Frame Buffer Object (FBO) for Editor Viewport ---
	GLuint frameBuffer, frameBufferTexture, depthRenderBuffer; // FBO, Color Attachment, and Depth/Stencil Attachment.
	int frameBufferWidth, frameBufferHeight;					// Dimensions of the FBO attachments.

	/**
	 * @brief Creates and configures the Frame Buffer Object (FBO) and its attachments (texture and renderbuffer).
	 * @param width The width for the attachments.
	 * @param height The height for the attachments.
	 */
	void CreateFrameBuffer(int width, int height);

	/**
	 * @brief Deletes all OpenGL resources associated with the FBO.
	 */
	void DeleteFrameBuffer();

	/**
	 * @brief Deletes and recreates the FBO and attachments with new dimensions.
	 * @param width The new width.
	 * @param height The new height.
	 */
	void ResizeFrameBuffer(int width, int height);
};