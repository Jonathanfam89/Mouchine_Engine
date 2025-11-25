/* Start Header ************************************************************************/
/*!
\file       InputManager.h
\author     Lee Hwee Min [100% - 41 lines]
\par        l.hweemin@digipen.edu
\date       October, 2, 2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include "System.h"
#include "factory.h"
#include "transform.h"
#include "physics.h"
#include "GraphicsSystem.h"
#include "core.h"
#include "audio.h"

/**
 * @class InputManager
 * @brief Handles keyboard, mouse, and input-related events for the game engine.
 *
 * The InputManager processes player inputs, manages key states,
 * handles mouse positions, and communicates with other subsystems
 * such as audio and graphics.
 */

class InputManager : public ISystem
{
public:
    /**
     * @brief Constructs an InputManager.
     * @param window Pointer to the GLFW window.
     * @param audio Optional pointer to the AudioEngine for sound control.
     */
    InputManager(GLFWwindow *window, AudioEngine *audio = nullptr);

    /**
     * @brief Destructor for InputManager.
     */
    ~InputManager();

    /**
     * @brief Initializes the input system (fetches window size, etc.).
     */
    void Initialize();

    /**
     * @brief Updates input handling each frame.
     * @param dt Delta time since last frame.
     */
    void Update(float dt) override;

    /**
     * @brief Returns the name of this system.
     * @return "Input System"
     */
    std::string GetName() override;

    // ------------------- Getters -------------------

    /**
     * @brief Gets the current X position of the mouse.
     * @return Mouse X position in screen coordinates.
     */
    double GetMouseX() const { return mouseX; }

    /**
     * @brief Gets the current Y position of the mouse.
     * @return Mouse Y position in screen coordinates.
     */
    double GetMouseY() const { return mouseY; }

    /**
     * @brief Gets the scroll X offset.
     * @return Horizontal scroll offset.
     */
    double GetScrollX() const { return scrollX; }

    /**
     * @brief Gets the scroll Y offset.
     * @return Vertical scroll offset.
     */
    double GetScrollY() const { return scrollY; }

    /**
     * @brief Gets if the left mouse was clicked (pressed AND released).
     * @return True if a click happened this frame, false otherwise.
     */
    static bool IsMouseLeftClicked() { return MouseLeftClicked; }

    /**
     * @brief Gets the mouse's X position in world coordinates.
     * @return World X position.
     */
    static double GetMouseWorldX() { return MouseWorldX; }

    /**
     * @brief Gets the mouse's Y position in world coordinates.
     * @return World Y position.
     */
    static double GetMouseWorldY() { return MouseWorldY; }


    // Static quit flag triggered when Escape is pressed.
    static bool KeyStateQuit;
    
    // Static Imgui flag triggered when Escape is pressed.
    static bool KeyStateImgui;

    // Will store the mouse position in world coordinates (0,0 at center)
    static double MouseWorldX;
    static double MouseWorldY;

    // Stores if the mouse was clicked *this frame*
    static bool MouseLeftClicked;

    void ResetState();

    static bool ShowFPS;
private:
    // Window info
    GLFWwindow *window;
    int width, height;
    AudioEngine *audioEngine;

    // Mouse position / scroll
    double mouseX, mouseY;
    double scrollX, scrollY;

    // Movement and jump
    double movementspeed = 300.f;
    bool isJumping = false;
    float jumpStartY = 0.0f;
    float jumpMaxHeight = 5.0f; // max height relative to start
    float jumpSpeed = 10.0f;    // units per second

    // Footstep audio timing
    float footstepTimer = 0.0f;
    float footstepInterval = 0.4f; // time between footstep sounds
    EntityID cachedPlayerID = 0;
};

#endif