/* Start Header ************************************************************************/
/*!
\file       WinLoseLogic.h
\author     Leck Xin Yee [100% - 38 lines]
\par        xinyee.leck@digipen.edu
\date       November, 29, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include "System.h"
#include "factory.h"
#include "Collidercomponent.h"
#include "transform.h"
#include "core.h"

/**
 * @class WinLoseLogicSystem
 * @brief Handles win and lose state transitions, UI screens, and scene loading logic.
 */
class WinLoseLogicSystem : public ISystem {
public:
    /**
     * @brief Constructs the WinLoseLogicSystem.
     */
    WinLoseLogicSystem() = default;

    /**
     * @brief Destructs the WinLoseLogicSystem.
     */
    ~WinLoseLogicSystem() = default;

    /**
     * @brief Initializes all win/lose state variables.
     */
    void Initialize() override;

    /**
     * @brief Updates win/lose logic every frame.
     * @param dt Delta time in seconds.
     */
    void Update(float dt) override;

    /**
     * @brief Gets the name of this system.
     * @return The string "WinLoseLogicSystem".
     */
    std::string GetName() override { return "WinLoseLogicSystem"; }

    /**
     * @brief Sets the current scene filepath for restart operations.
     * @param scenePath Path to the active scene.
     */
    void SetCurrentScene(const std::string& scenePath);
    //void SetCurrentScene(const std::string& scenePath) { currentScenePath = scenePath; }

    /**
     * @brief Checks if the win screen UI is currently active.
     * @return True if win screen is enabled.
     */
    bool IsWinScreenActive() const { return hasCreatedWinScreen; }

    /**
     * @brief Checks if the lose screen UI is currently active.
     * @return True if lose screen is enabled.
     */
    bool IsLoseScreenActive() const { return hasCreatedLoseScreen; }

    /**
     * @brief Triggers the lose sequence and displays the lose UI.
     */
    void TriggerLose();

private:
    // ---------------- WIN STATE ----------------
    bool hasPlayerWon = false;
    float winDelayTimer = 0.0f;
    bool isWaitingForWin = false;
    bool hasCreatedWinScreen = false;

    // ---------------- LOSE STATE ----------------
    bool hasCreatedLoseScreen = false;

    // ---------------- SHARED STATE ----------------
    std::string hoveredButtonName = "";
    std::string currentScenePath = "";
    bool pendingSceneChange = false;
    float pendingSceneTimer = 0.0f;
    std::string pendingScenePath = "";
    bool hasCreatedTransitionOverlay = false;

    /**
     * @brief Checks if the player has entered the win zone.
     */
    void CheckWinCondition();

    /**
     * @brief Creates win screen UI objects and pauses the game.
     */
    void TriggerWin();

    /**
     * @brief Handles hover/click logic for win screen buttons.
     */
    void HandleWinScreenButtons();

    /**
     * @brief Destroys win screen UI objects and resets state.
     */
    void DestroyWinScreen();

    /**
     * @brief Handles hover/click logic for lose screen buttons.
     */
    void HandleLoseScreenButtons();

    /**
     * @brief Destroys lose screen UI objects and resets state.
     */
    void DestroyLoseScreen();

    /**
     * @brief Checks if the mouse is over a UI button.
     * @param button Pointer to the GameObject representing the button.
     * @param mouseX Mouse world X position.
     * @param mouseY Mouse world Y position.
     * @return True if mouse overlaps button region.
     */
    bool IsMouseOver(GameObject* button, double mouseX, double mouseY);

    /**
     * @brief Updates the texture of a UI button based on hover state.
     * @param buttonName Name of the button object.
     * @param isHovered Whether the button is currently hovered.
     */
    void SetButtonTexture(std::string buttonName, bool isHovered);
};
