#pragma once
#include "System.h"
#include "factory.h"
#include "Collidercomponent.h"
#include "transform.h"
#include "core.h"

class WinLoseLogicSystem : public ISystem {
public:
    WinLoseLogicSystem() = default;
    ~WinLoseLogicSystem() = default;

    void Initialize() override;
    void Update(float dt) override;

    std::string GetName() override { return "WinLoseLogicSystem"; }

    void SetCurrentScene(const std::string& scenePath);
    bool IsWinScreenActive() const { return hasCreatedWinScreen; }
    bool IsLoseScreenActive() const { return hasCreatedLoseScreen; }
    void TriggerLose();

private:
    // win state
    bool hasPlayerWon = false;
    float winDelayTimer = 0.0f;
    bool isWaitingForWin = false;
    bool hasCreatedWinScreen = false;

    // lose state
    bool hasCreatedLoseScreen = false;

    // shared state
    std::string hoveredButtonName = "";
    std::string currentScenePath = "";
    bool pendingSceneChange = false;
    float pendingSceneTimer = 0.0f;
    std::string pendingScenePath = "";
    bool hasCreatedTransitionOverlay = false;

    // win methods
    void CheckWinCondition();
    void TriggerWin();
    void HandleWinScreenButtons();
    void DestroyWinScreen();

    // lose methods
    void HandleLoseScreenButtons();
    void DestroyLoseScreen();

    // shared helper methods
    bool IsMouseOver(GameObject* button, double mouseX, double mouseY);
    void SetButtonTexture(std::string buttonName, bool isHovered);
};
