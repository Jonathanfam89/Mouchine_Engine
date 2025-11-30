/* Start Header ************************************************************************/
/*!
\file       MenuLogic.h
\author     Jonathan Fam Qian Wei [100% - 21 lines]
\par        Fam.q@digipen.edu
\date       Novenber, 7, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include "System.h"
#include "factory.h"
#include "Serialization.h"
#include "InputManager.h" //
#include "transform.h"
#include "MathLib.h"
#include "GraphicsSystem.h"
#include "LevelEditor.h"

class LevelEditor;
class GraphicsSystem;
/**
 * @brief System responsible for handling menu logic and button interactions.
 */
class MenuLogicSystem : public ISystem {
public:
    /**
     * @brief Default constructor.
     */
    MenuLogicSystem() = default;
    
    /**
     * @brief Default destructor.
     */
    ~MenuLogicSystem() = default;

    /**
     * @brief The main update loop for menu logic, handles button hover states and interactions.
     * @param dt The time elapsed since the last frame.
     */
    void Update(float dt) override;

    /**
     * @brief Gets the name of this system.
     * @return The system name as a string.
     */
    std::string GetName() override { return "MenuLogicSystem"; }

    /**
     * @brief Initializes the menu logic system.
     */
    void Initialize() override {}
 
    /**
     * @brief Resets the menu state to the default (Home screen).
     */
    void Reset();

private:
    std::string hoveredButtonName = "";   ///< The name of the currently hovered button
    bool pendingSceneChange = false;
    float pendingTimer = 0.0f;
    
    /**
     * @brief Checks if the mouse cursor is over a button GameObject.
     * @param button The button GameObject to check.
     * @param mouseX The current mouse X coordinate.
     * @param mouseY The current mouse Y coordinate.
     * @return True if the mouse is over the button, false otherwise.
     */
    bool IsMouseOver(GameObject* button, double mouseX, double mouseY);
    
    /**
     * @brief Sets the texture of a button based on its hover state.
     * @param buttonName The name of the button to update.
     * @param isHovered Whether the button is currently being hovered.
     */
    void SetButtonTexture(std::string buttonName, bool isHovered);
};