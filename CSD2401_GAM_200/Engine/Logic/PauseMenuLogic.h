/* Start Header ************************************************************************/
/*!
\file       PauseMenuLogic.h
\author     Leck Xin Yee [100% - 31 lines]
\par        xinyee.leck@digipen.edu
\date       November, 28, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include "System.h"
#include "factory.h"
#include "InputManager.h"
#include "transform.h"
#include "MathLib.h"
#include "GraphicsSystem.h"

class GraphicsSystem;

/**
 * @class PauseMenuLogicSystem
 * @brief Handles pause menu UI creation, visibility, interactions, and scene actions.
 */
class PauseMenuLogicSystem : public ISystem {
public:
    /**
     * @brief Default constructor for the pause menu system.
     */
    PauseMenuLogicSystem() = default;

    /**
     * @brief Default destructor.
     */
    ~PauseMenuLogicSystem() = default;

    /**
     * @brief Initializes the pause menu state.
     */
    void Initialize() override;

    /**
     * @brief Updates pause menu logic every frame, handling UI movement, hover/click logic, and pending scene transitions.
     * @param dt Delta time in seconds.
     */
    void Update(float dt) override;

    /**
     * @brief Returns the system name.
     * @return "PauseMenuLogicSystem"
     */
    std::string GetName() override { return "PauseMenuLogicSystem"; }


    /**
     * @brief Checks whether the pause menu is currently visible.
     * @return True if the pause menu is active.
     */
    bool IsPauseMenuActive() const { return isPauseMenuActive; }

    /**
     * @brief Sets the current scene path so Restart can reload the same scene.
     * @param scenePath File path of the scene.
     */
    void SetCurrentScene(const std::string& scenePath) { currentScenePath = scenePath; }

    /**
     * @brief Toggles the pause menu on/off and pauses/unpauses the game.
     */
    void TogglePauseMenu();

private:
    bool isPauseMenuActive = false;        // whether the pause menu is currently displayed
    std::string hoveredButtonName = "";    // name of the currently hovered button
    bool hasCreatedUI = false;             // whether UI elements have been created
    bool pendingSceneChange = false;       // whether a scene change is pending
    float pendingTimer = 0.0f;             // timer for scene change delay
    std::string currentScenePath = "";

    /**
     * @brief Creates all pause menu UI elements (overlay, buttons, title).
     */
    void CreatePauseMenuUI();

    /**
     * @brief Destroys all pause menu UI objects.
     */
    void DestroyPauseMenuUI();

    /**
     * @brief Sets all pause menu UI objects active or inactive.
     * @param show Whether to enable or disable UI visibility.
     */
    void SetPauseMenuVisible(bool show);

    /**
     * @brief Checks if the mouse cursor is within a button's area.
     * @param button The GameObject representing a UI button.
     * @param mouseX Mouse world X coordinate.
     * @param mouseY Mouse world Y coordinate.
     * @return True if the mouse is over the button.
     */
    bool IsMouseOver(GameObject* button, double mouseX, double mouseY);

    /**
     * @brief Changes a button's texture depending on hover state.
     * @param buttonName Name of the button object.
     * @param isHovered True for hover texture, false for normal.
     */
    void SetButtonTexture(std::string buttonName, bool isHovered);
};
