#include "PauseMenuLogic.h"
#include "core.h"
#include "LevelEditor.h"
#include <iostream>

void PauseMenuLogicSystem::Initialize()
{
    isPauseMenuActive = false;
    hasCreatedUI = false;
    hoveredButtonName = "";
    pendingSceneChange = false;
}

void PauseMenuLogicSystem::Update(float dt)
{
    // check if we're in play mode
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

    // update pause menu position to follow camera
    if (isPauseMenuActive || hasCreatedUI)
    {
        vec2 camPos = CameraManager::Instance().GetCameraPosition();
        auto& factory = GameObjectFactory::Instance();

        std::vector<std::string> uiElements = {
            "PauseOverlay",
            "PauseBoard",
            "PauseTitle",
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

void PauseMenuLogicSystem::CreatePauseMenuUI()
{
    if (hasCreatedUI)
        return;

    auto& factory = GameObjectFactory::Instance();

    // create semi-transparent overlay background
    GameObject* overlay = factory.CreateGameObject("PauseOverlay");
    auto* overlayTransform = overlay->AddComponent<TransformComponent>();
    overlayTransform->posX = 0.0f;
    overlayTransform->posY = 215.0f;
    overlayTransform->scaleX = 1980.0f;
    overlayTransform->scaleY = 942.0f;
    auto* overlayRender = overlay->AddComponent<RenderComponent>();
    overlayRender->SetColor(vec4(0.0f, 0.0f, 0.0f, 0.7f)); // dark semi-transparent
    overlayRender->SetLayer(100);

    // pause board background
    GameObject* board = factory.CreateGameObject("PauseBoard");
    auto* boardTransform = board->AddComponent<TransformComponent>();
    boardTransform->posX = 0.0f;
    boardTransform->posY = 0.0f;
    boardTransform->scaleX = 1500.0f;
    boardTransform->scaleY = 1000.0f;
    auto* boardRender = board->AddComponent<RenderComponent>();
    boardRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/pause_board.png"));
    boardRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    boardRender->SetLayer(101);

    // pause title text
    GameObject* title = factory.CreateGameObject("PauseTitle");
    auto* titleTransform = title->AddComponent<TransformComponent>();
    titleTransform->posX = 0.0f;
    titleTransform->posY = 110.0f;
    titleTransform->scaleX = 400.0f;
    titleTransform->scaleY = 0.0f;
    auto* titleText = title->AddComponent<TextComponent>("../Assets/FrederickatheGreat-Regular.ttf", 80, "PAUSED", vec4(0.0f, 0.0f, 0.0f, 1.0f), false);

    // resume button
    GameObject* resumeBtn = factory.CreateGameObject("PauseResumeButton");
    auto* resumeTransform = resumeBtn->AddComponent<TransformComponent>();
    resumeTransform->posX = 0.0f;
    resumeTransform->posY = 0.0f;
    resumeTransform->scaleX = 350.0f;
    resumeTransform->scaleY = 80.0f;
    auto* resumeRender = resumeBtn->AddComponent<RenderComponent>();
    resumeRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/blankbtnL.png"));
    resumeRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f)); // white tint
    resumeRender->SetLayer(102);
    auto* resumeText = resumeBtn->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", 32, "Resume", vec4(0.0f, 0.0f, 0.0f, 1.0f), false);

    // restart button
    GameObject* restartBtn = factory.CreateGameObject("PauseRestartButton");
    auto* restartTransform = restartBtn->AddComponent<TransformComponent>();
    restartTransform->posX = 0.0f;
    restartTransform->posY = -100.0f;
    restartTransform->scaleX = 350.0f;
    restartTransform->scaleY = 80.0f;
    auto* restartRender = restartBtn->AddComponent<RenderComponent>();
    restartRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/blankbtnL.png"));
    restartRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    restartRender->SetLayer(102);
    auto* restartText = restartBtn->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", 32, "Restart", vec4(0.0f, 0.0f, 0.0f, 1.0f), false);

    // settings button
    GameObject* settingsBtn = factory.CreateGameObject("PauseSettingsButton");
    auto* settingsTransform = settingsBtn->AddComponent<TransformComponent>();
    settingsTransform->posX = 0.0f;
    settingsTransform->posY = -200.0f;
    settingsTransform->scaleX = 350.0f;
    settingsTransform->scaleY = 80.0f;
    auto* settingsRender = settingsBtn->AddComponent<RenderComponent>();
    settingsRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/blankbtnL.png"));
    settingsRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    settingsRender->SetLayer(102);
    auto* settingsText = settingsBtn->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", 32, "Settings", vec4(0.0f, 0.0f, 0.0f, 1.0f), false);

    // main menu button
    GameObject* mainMenuBtn = factory.CreateGameObject("PauseMainMenuButton");
    auto* mainMenuTransform = mainMenuBtn->AddComponent<TransformComponent>();
    mainMenuTransform->posX = 0.0f;
    mainMenuTransform->posY = -300.0f;
    mainMenuTransform->scaleX = 350.0f;
    mainMenuTransform->scaleY = 80.0f;
    auto* mainMenuRender = mainMenuBtn->AddComponent<RenderComponent>();
    mainMenuRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/blankbtnL.png"));
    mainMenuRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    mainMenuRender->SetLayer(102);
    auto* mainMenuText = mainMenuBtn->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", 32, "Main Menu", vec4(0.0f, 0.0f, 0.0f, 1.0f), false);

    hasCreatedUI = true;

    // initially hide the UI
    SetPauseMenuVisible(false);
}

void PauseMenuLogicSystem::DestroyPauseMenuUI()
{
    if (!hasCreatedUI)
        return;

    auto& factory = GameObjectFactory::Instance();

    // xestroy all ui elements on pause menu
    std::vector<std::string> uiElements = {
        "PauseOverlay",
        "PauseBoard",
        "PauseTitle",
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

void PauseMenuLogicSystem::SetPauseMenuVisible(bool show)
{
    auto& factory = GameObjectFactory::Instance();

    std::vector<std::string> uiElements = {
        "PauseOverlay",
        "PauseBoard",
        "PauseTitle",
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
}

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

bool PauseMenuLogicSystem::IsMouseOver(GameObject* button, double mouseX, double mouseY)
{
    if (!button) return false;
    if (!button->IsActive()) return false;

    auto* tx = button->GetComponent<TransformComponent>();
    if (!tx) return false;

    return IsAreaClicked(tx->posX, tx->posY, tx->scaleX, tx->scaleY, mouseX, mouseY);
}

void PauseMenuLogicSystem::SetButtonTexture(std::string buttonName, bool isHovered)
{
    auto& factory = GameObjectFactory::Instance();
    GameObject* btn = factory.FindGameObject(buttonName);
    if (!btn) return;

    auto* rc = btn->GetComponent<RenderComponent>();
    if (!rc) return;

    // swap between normal and hover button textures
    std::string texturePath = isHovered ? "../Assets/hoveringbtnL.png" : "../Assets/blankbtnL.png";

    unsigned int newtex = GraphicsSystem::LoadTexture(texturePath.c_str());
    rc->SetTextureID(newtex);
}
