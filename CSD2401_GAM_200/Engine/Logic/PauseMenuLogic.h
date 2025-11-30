#pragma once
#include "System.h"
#include "factory.h"
#include "InputManager.h"
#include "transform.h"
#include "MathLib.h"
#include "GraphicsSystem.h"

class GraphicsSystem;

class PauseMenuLogicSystem : public ISystem {
public:
    PauseMenuLogicSystem() = default;
    ~PauseMenuLogicSystem() = default;

    void Initialize() override;
    void Update(float dt) override;

    std::string GetName() override { return "PauseMenuLogicSystem"; }

    bool IsPauseMenuActive() const { return isPauseMenuActive; }
    void SetCurrentScene(const std::string& scenePath) { currentScenePath = scenePath; }
    void TogglePauseMenu();

private:
    bool isPauseMenuActive = false;        // whether the pause menu is currently displayed
    std::string hoveredButtonName = "";    // name of the currently hovered button
    bool hasCreatedUI = false;             // whether UI elements have been created
    bool pendingSceneChange = false;       // whether a scene change is pending
    float pendingTimer = 0.0f;             // timer for scene change delay
    std::string currentScenePath = "";

    void CreatePauseMenuUI();
    void DestroyPauseMenuUI();
    void SetPauseMenuVisible(bool show);
    bool IsMouseOver(GameObject* button, double mouseX, double mouseY);
    void SetButtonTexture(std::string buttonName, bool isHovered);
};
