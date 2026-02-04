/* Start Header ************************************************************************/
/*!
\file       PauseMenuLogic.cpp
\author     Leck Xin Yee [100% - 309 lines]
\par        xinyee.leck@digipen.edu
\date       November, 28, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "PauseMenuLogic.h"
#include "core.h"
#include "LevelEditor.h"
#include <iostream>

/**
 * @brief Initializes all pause menu state variables.
 */
void PauseMenuLogicSystem::Initialize()
{
    isPauseMenuActive = false;
    hasCreatedUI = false;
    hoveredButtonName = "";
    pendingSceneChange = false;
}

/**
 * @brief Main update loop handling pause menu activation, UI movement, hover/click logic, and delayed scene actions.
 */
void PauseMenuLogicSystem::Update(float dt)
{
    // Update audio fade
    AudioManager* audioManager = CORE->GetAudioManager();
    if (audioManager)
    {
        audioManager->UpdateFade(dt);
    }

    // check if we're in play mode
#ifdef USE_IMGUI
    LevelEditor* editor = CORE ? CORE->GetLevelEditor() : nullptr;
    if (!editor || editor->GetEditorMode() != EditorMode::Play)
    {
        // not in play mode, hide pause menu if active
        if (isPauseMenuActive)
        {
            TogglePauseMenu();
        }
        return;
    }
#endif
    // update pause menu position to follow camera
    if (isPauseMenuActive || hasCreatedUI)
    {
        vec2 camPos = CameraManager::Instance().GetCameraPosition();
        auto& factory = GameObjectFactory::Instance();

        std::vector<std::string> uiElements = {
            "PauseOverlay",
            "PauseBoard",
            "PauseResumeButton",
            "PauseRestartButton",
            "PauseSettingsButton",
            "PauseMainMenuButton"
        };

        for (const auto& name : uiElements)
        {
            GameObject* obj = factory.FindGameObject(name);
            if (obj)
            {
                auto* transform = obj->GetComponent<TransformComponent>();
                if (transform)
                {
                    static std::map<std::string, vec2> originalPositions;
                    if (originalPositions.find(name) == originalPositions.end())
                    {
                        originalPositions[name] = vec2(transform->posX, transform->posY);
                    }

                    transform->posX = camPos.x + originalPositions[name].x;
                    transform->posY = camPos.y + originalPositions[name].y;
                }
            }
        }
    }

    // handle pending scene change
    if (pendingSceneChange)
    {
        pendingTimer -= dt;
        if (pendingTimer <= 0.0f)
        {
            if (hoveredButtonName == "PauseResumeButton")
            {
                std::cout << "[PauseMenu] Resume clicked" << std::endl;
                TogglePauseMenu(); // close menu and unpause
            }
            else if (hoveredButtonName == "PauseRestartButton")
            {
                std::cout << "[PauseMenu] Restart clicked" << std::endl;
                // close menu first
                if (isPauseMenuActive)
                {
                    SetPauseMenuVisible(false);
                    isPauseMenuActive = false;
                    CORE->SetGamePaused(false);
                }
                // restart the current level
                if (!currentScenePath.empty())
                {
                    CORE->RequestSceneLoad(currentScenePath, true);
                }
            }
            else if (hoveredButtonName == "PauseSettingsButton")
            {
                std::cout << "[PauseMenu] Settings clicked (not implemented yet)" << std::endl;
                // settings go here
            }
            else if (hoveredButtonName == "PauseMainMenuButton")
            {
                std::cout << "[PauseMenu] Main Menu clicked" << std::endl;

                // Start audio fade out before going to main menu
                AudioManager* audioManager = CORE->GetAudioManager();
                if (audioManager)
                {
                    audioManager->FadeOutCategory("music", 0.5f);
                    std::cout << "[PauseMenu] Started music fade out" << std::endl;
                }

                // close menu first
                if (isPauseMenuActive)
                {
                    SetPauseMenuVisible(false);
                    isPauseMenuActive = false;
                    CORE->SetGamePaused(false);
                }
                // back to main menu
                CORE->RequestSceneLoad("../Assets/Scenes/MainMenu.json", true);
            }

            pendingSceneChange = false;
            hoveredButtonName.clear();
        }
        return;
    }

    // only process menu interactions if pause menu is active
    if (!isPauseMenuActive)
    {
        return;
    }

    double mouseX = InputManager::GetMouseWorldX();
    double mouseY = InputManager::GetMouseWorldY();
    bool isClicked = InputManager::IsMouseLeftClicked();

    auto& factory = GameObjectFactory::Instance();
    std::string currentHover = "";

    if (IsMouseOver(factory.FindGameObject("PauseResumeButton"), mouseX, mouseY))
    {
        currentHover = "PauseResumeButton";
    }
    else if (IsMouseOver(factory.FindGameObject("PauseRestartButton"), mouseX, mouseY))
    {
        currentHover = "PauseRestartButton";
    }
    else if (IsMouseOver(factory.FindGameObject("PauseSettingsButton"), mouseX, mouseY))
    {
        currentHover = "PauseSettingsButton";
    }
    else if (IsMouseOver(factory.FindGameObject("PauseMainMenuButton"), mouseX, mouseY))
    {
        currentHover = "PauseMainMenuButton";
    }

    if (currentHover != hoveredButtonName)
    {
        if (!hoveredButtonName.empty())
        {
            SetButtonTexture(hoveredButtonName, false);
        }

        if (!currentHover.empty())
        {
            SetButtonTexture(currentHover, true);
            AudioManager* audioManager = CORE->GetAudioManager();
            if (audioManager)
            {
                audioManager->TriggerEvent("UI_Hover");
            }
        }

        hoveredButtonName = currentHover;
    }

    // only process clicks if we are actually hovering over a button
    if (isClicked && !hoveredButtonName.empty())
    {
        // play click sound
        AudioManager* audioManager = CORE->GetAudioManager();
        if (audioManager)
        {
            audioManager->TriggerEvent("UI_Click");
        }

        // start delay before executing action
        pendingSceneChange = true;
        pendingTimer = 0.20f; // 200ms delay
    }
}

/**
 * @brief Creates all UI elements for the pause menu.
 */
void PauseMenuLogicSystem::CreatePauseMenuUI()
{
    if (hasCreatedUI)
        return;

    auto& factory = GameObjectFactory::Instance();

    // create semi-transparent overlay background
    GameObject* overlay = factory.CreateGameObject("PauseOverlay");
    auto* overlayTransform = overlay->AddComponent<TransformComponent>();
    overlayTransform->posX = 0.0f;
    overlayTransform->posY = 0.0f;
    overlayTransform->scaleX = 5000.0f;
    overlayTransform->scaleY = 5000.0f;
    auto* overlayRender = overlay->AddComponent<RenderComponent>();

    // check if we're in main menu - use medium grey, otherwise use dark overlay
    bool isMainMenu = (currentScenePath.find("MainMenu") != std::string::npos);
    if (isMainMenu)
    {
        overlayRender->SetColor(vec4(0.5f, 0.5f, 0.5f, 1.0f)); // medium grey solid background
        overlayRender->SetLayer(500); // very high layer to cover main menu elements
    }
    else
    {
        overlayRender->SetColor(vec4(0.0f, 0.0f, 0.0f, 0.7f)); // dark semi-transparent for in-game
        overlayRender->SetLayer(100); // normal layer for in-game
    }

    // pause board background
    GameObject* board = factory.CreateGameObject("PauseBoard");
    auto* boardTransform = board->AddComponent<TransformComponent>();
    boardTransform->posX = 0.0f;
    boardTransform->posY = 0.0f;
    boardTransform->scaleX = 1920.0f;
    boardTransform->scaleY = 1080.0f;
    auto* boardRender = board->AddComponent<RenderComponent>();
    boardRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Pause_Menu_New.png"));
    boardRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    boardRender->SetLayer(501); // above overlay

    // paused button
    GameObject* pausedBtn = factory.CreateGameObject("PauseResumeButton");
    auto* pausedTransform = pausedBtn->AddComponent<TransformComponent>();
    pausedTransform->posX = 650.0f;
    pausedTransform->posY = -275.0f;
    pausedTransform->scaleX = 200.0f;
    pausedTransform->scaleY = 175.0f;
    auto* pausedRender = pausedBtn->AddComponent<RenderComponent>();
    pausedRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/paused_button.png"));
    pausedRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    pausedRender->SetLayer(502);

    // restart button
    GameObject* restartBtn = factory.CreateGameObject("PauseRestartButton");
    auto* restartTransform = restartBtn->AddComponent<TransformComponent>();
    restartTransform->posX = 0.0f;
    restartTransform->posY = -80.0f;
    restartTransform->scaleX = 420.0f;
    restartTransform->scaleY = 100.0f;
    auto* restartRender = restartBtn->AddComponent<RenderComponent>();
    restartRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets//Sprites/Icons/blankbtnL.png"));
    restartRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    restartRender->SetLayer(502);
    restartBtn->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", 38, "Restart", vec4(0.0f, 0.0f, 0.0f, 1.0f), false);
    //auto* restartText = restartBtn->AddComponent<TextComponent>(
    //    "../Assets/FrederickatheGreat-Regular.ttf", 32, "Restart", vec4(0.0f, 0.0f, 0.0f, 1.0f), false);

    // settings button
    GameObject* settingsBtn = factory.CreateGameObject("PauseSettingsButton");
    auto* settingsTransform = settingsBtn->AddComponent<TransformComponent>();
    settingsTransform->posX = 0.0f;
    settingsTransform->posY = 40.0f;
    settingsTransform->scaleX = 420.0f;
    settingsTransform->scaleY = 100.0f;
    auto* settingsRender = settingsBtn->AddComponent<RenderComponent>();
    settingsRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/blankbtnL.png"));
    settingsRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    settingsRender->SetLayer(502);
    settingsBtn->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", 38, "Settings", vec4(0.0f, 0.0f, 0.0f, 1.0f), false);
    //auto* settingsText = settingsBtn->AddComponent<TextComponent>(
    //    "../Assets/FrederickatheGreat-Regular.ttf", 32, "Settings", vec4(0.0f, 0.0f, 0.0f, 1.0f), false);

    // quit button
    GameObject* mainMenuBtn = factory.CreateGameObject("PauseMainMenuButton");
    auto* mainMenuTransform = mainMenuBtn->AddComponent<TransformComponent>();
    mainMenuTransform->posX = 0.0f;
    mainMenuTransform->posY = -200.0f;
    mainMenuTransform->scaleX = 420.0f;
    mainMenuTransform->scaleY = 100.0f;
    auto* mainMenuRender = mainMenuBtn->AddComponent<RenderComponent>();
    mainMenuRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/blankbtnL.png"));
    mainMenuRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    mainMenuRender->SetLayer(502);
    mainMenuBtn->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", 38, "Quit", vec4(0.0f, 0.0f, 0.0f, 1.0f), false);
    //auto* mainMenuText = mainMenuBtn->AddComponent<TextComponent>(
    //    "../Assets/FrederickatheGreat-Regular.ttf", 32, "Main Menu", vec4(0.0f, 0.0f, 0.0f, 1.0f), false);

    hasCreatedUI = true;

    // initially hide the UI
    SetPauseMenuVisible(false);
}

/**
 * @brief Destroys all pause menu UI elements.
 */
void PauseMenuLogicSystem::DestroyPauseMenuUI()
{
    if (!hasCreatedUI)
        return;

    auto& factory = GameObjectFactory::Instance();

    // xestroy all ui elements on pause menu
    std::vector<std::string> uiElements = {
        "PauseOverlay",
        "PauseBoard",
        "PauseResumeButton",
        "PauseRestartButton",
        "PauseSettingsButton",
        "PauseMainMenuButton"
    };

    for (const auto& name : uiElements)
    {
        GameObject* obj = factory.FindGameObject(name);
        if (obj)
        {
            factory.DestroyGameObject(obj->GetID());
        }
    }

    hasCreatedUI = false;
    std::cout << "[PauseMenu] UI destroyed" << std::endl;
}

/**
 * @brief Shows or hides pause menu UI elements.
 */
void PauseMenuLogicSystem::SetPauseMenuVisible(bool show)
{
    auto& factory = GameObjectFactory::Instance();

    std::vector<std::string> uiElements = {
        "PauseOverlay",
        "PauseBoard",
        "PauseResumeButton",
        "PauseRestartButton",
        "PauseSettingsButton",
        "PauseMainMenuButton"
    };

    for (const auto& name : uiElements)
    {
        GameObject* obj = factory.FindGameObject(name);
        if (obj)
        {
            obj->SetActive(show);
        }
    }

    // update overlay color and layer based on current scene when showing
    if (show)
    {
        GameObject* overlay = factory.FindGameObject("PauseOverlay");
        if (overlay)
        {
            auto* overlayRender = overlay->GetComponent<RenderComponent>();
            if (overlayRender)
            {
                // check if in main menu
                bool isMainMenu = (currentScenePath.find("MainMenu") != std::string::npos);
                std::cout << "[PauseMenu] Current scene: " << currentScenePath << ", isMainMenu: " << isMainMenu << std::endl;
                if (isMainMenu)
                {
                    overlayRender->SetColor(vec4(0.5f, 0.5f, 0.5f, 1.0f)); // medium grey solid background
                    overlayRender->SetLayer(500); // high layer to cover main menu elements
                }
                else
                {
                    overlayRender->SetColor(vec4(0.0f, 0.0f, 0.0f, 0.3f)); // semi-transparent for in-game
                    overlayRender->SetLayer(100); // normal layer
                }
            }
        }
    }
}

/**
 * @brief Toggles the pause menu open or closed and pauses/unpauses the game.
 */
void PauseMenuLogicSystem::TogglePauseMenu()
{
    // create ui on first toggle if not created
    if (!hasCreatedUI)
    {
        CreatePauseMenuUI();
    }

    isPauseMenuActive = !isPauseMenuActive;
    SetPauseMenuVisible(isPauseMenuActive);

    // pause/unpause the game
    CORE->SetGamePaused(isPauseMenuActive);
    //CORE->GameTimerSystem();

    std::cout << "[PauseMenu] Pause menu " << (isPauseMenuActive ? "opened" : "closed") << std::endl;
}

/**
 * @brief Checks whether the mouse overlaps a UI button area.
 */
bool PauseMenuLogicSystem::IsMouseOver(GameObject* button, double mouseX, double mouseY)
{
    if (!button) return false;
    if (!button->IsActive()) return false;

    auto* tx = button->GetComponent<TransformComponent>();
    if (!tx) return false;

    return IsAreaClicked(tx->posX, tx->posY, tx->scaleX, tx->scaleY, static_cast<float>(mouseX), static_cast<float>(mouseY));
}

/**
 * @brief Updates button texture based on hover state.
 */
void PauseMenuLogicSystem::SetButtonTexture(std::string buttonName, bool isHovered)
{
    auto& factory = GameObjectFactory::Instance();
    GameObject* btn = factory.FindGameObject(buttonName);
    if (!btn) return;

    auto* rc = btn->GetComponent<RenderComponent>();
    if (!rc) return;

    // swap between normal and hover button textures
    std::string texturePath;

    if (buttonName == "PauseResumeButton")
    {
        texturePath = isHovered ? "../Assets/Sprites/Icons/playing_button.png" : "../Assets/Sprites/Icons/paused_button.png";
    }
    else
    {
        texturePath = isHovered ? "../Assets/Sprites/Icons/hoveringbtnL.png" : "../Assets/Sprites/Icons/blankbtnL.png";
    }

    unsigned int newtex = GraphicsSystem::LoadTexture(texturePath.c_str());
    rc->SetTextureID(newtex);
}
