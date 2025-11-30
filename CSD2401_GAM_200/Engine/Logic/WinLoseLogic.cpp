#include "WinLoseLogic.h"
#include "LevelEditor.h"
#include "CameraManager.h"
#include "InputManager.h"
#include "AudioManager.h"
#include "GraphicsSystem.h"
#include "MathLib.h"
#include <iostream>

void WinLoseLogicSystem::Initialize()
{
    hasPlayerWon = false;
    winDelayTimer = 0.0f;
    isWaitingForWin = false;
    hasCreatedWinScreen = false;
    hasCreatedLoseScreen = false;
    pendingSceneChange = false;
    pendingSceneTimer = 0.0f;
    pendingScenePath = "";
}

void WinLoseLogicSystem::SetCurrentScene(const std::string& scenePath)
{
    currentScenePath = scenePath;
}

void WinLoseLogicSystem::Update(float dt)
{
    static int totalFrames = 0;
    static bool firstUpdate = true;

    if (firstUpdate)
    {
        firstUpdate = false;
    }

    totalFrames++;

    if (pendingSceneChange)
    {
        pendingSceneTimer -= dt;
        if (pendingSceneTimer <= 0.0f)
        {
            pendingSceneChange = false;
            // unpause right before loading to avoid showing game world
            CORE->SetGamePaused(false);
            CORE->RequestSceneLoad(pendingScenePath, true);
            pendingScenePath = "";
        }
        return;
    }

    // check if in play mode
#ifdef USE_IMGUI
    LevelEditor* editor = CORE ? CORE->GetLevelEditor() : nullptr;
    if (!editor)
    {
        if (totalFrames % 300 == 0)
            std::cout << "[WinZone] No editor found!" << std::endl;
        return;
    }

    EditorMode mode = editor->GetEditorMode();
    if (mode != EditorMode::Play)
    {
        if (totalFrames % 300 == 0)
            std::cout << "[WinZone] Not in Play mode. Current mode: " << (int)mode << std::endl;
        return;
    }
#endif

    if (isWaitingForWin)
    {
        winDelayTimer -= dt;
        if (winDelayTimer <= 0.0f)
        {
            TriggerWin();
            isWaitingForWin = false;
        }
        return;
    }

    // update win screen position to follow camera
    if (hasCreatedWinScreen)
    {
        vec2 camPos = CameraManager::Instance().GetCameraPosition();
        auto& factory = GameObjectFactory::Instance();

        // update all win screen elements to follow camera
        std::vector<std::pair<std::string, vec2>> uiElements = {
            {"WinScreen", vec2(0.0f, 0.0f)},
            {"WinReplayLogo", vec2(550.0f, 50.0f)},
            {"WinYesButton", vec2(550.0f, -100.0f)},
            {"WinNoButton", vec2(550.0f, -220.0f)}
        };

        for (const auto& element : uiElements)
        {
            GameObject* obj = factory.FindGameObject(element.first);
            if (obj)
            {
                auto* transform = obj->GetComponent<TransformComponent>();
                if (transform)
                {
                    transform->posX = camPos.x + element.second.x;
                    transform->posY = camPos.y + element.second.y;
                }
            }
        }

        HandleWinScreenButtons();
        return;
    }

    // update lose screen position to follow camera
    if (hasCreatedLoseScreen)
    {
        vec2 camPos = CameraManager::Instance().GetCameraPosition();
        auto& factory = GameObjectFactory::Instance();

        // update all lose screen elements to follow camera
        std::vector<std::pair<std::string, vec2>> uiElements = {
            {"LoseScreen", vec2(0.0f, 0.0f)},
            {"LoseReplayLogo", vec2(550.0f, 50.0f)},
            {"LoseYesButton", vec2(550.0f, -100.0f)},
            {"LoseNoButton", vec2(550.0f, -220.0f)}
        };

        for (const auto& element : uiElements)
        {
            GameObject* obj = factory.FindGameObject(element.first);
            if (obj)
            {
                auto* transform = obj->GetComponent<TransformComponent>();
                if (transform)
                {
                    transform->posX = camPos.x + element.second.x;
                    transform->posY = camPos.y + element.second.y;
                }
            }
        }

        HandleLoseScreenButtons();
        return;
    }

    if (hasPlayerWon)
    {
        return;
    }

    CheckWinCondition();
}

void WinLoseLogicSystem::CheckWinCondition()
{
    auto& factory = GameObjectFactory::Instance();

    // find the win zone
    GameObject* winZone = factory.FindGameObject("WinZone");
    if (!winZone)
    {
        static bool warnedOnce = false;
        if (!warnedOnce)
        {
            warnedOnce = true;
        }
        return;
    }

    if (!winZone->IsActive())
    {
        return;
    }

    // find player
    GameObject* player = factory.FindGameObject("Hero");
    if (!player)
    {
        static bool warnedOnce = false;
        if (!warnedOnce)
        {
            std::cout << "[WinZone] WARNING: Hero object not found in scene!" << std::endl;
            warnedOnce = true;
        }
        return;
    }

    if (!player->IsActive())
    {
        return;
    }

    auto* winZoneTransform = winZone->GetComponent<TransformComponent>();
    auto* playerTransform = player->GetComponent<TransformComponent>();

    if (!winZoneTransform || !playerTransform)
    {
        return;
    }

    // simple AABB collision check
    float winX, winY, playerX, playerY;
    winZoneTransform->GetPosition(winX, winY);
    playerTransform->GetPosition(playerX, playerY);

    float winHalfWidth = winZoneTransform->scaleX / 2.0f;
    float winHalfHeight = winZoneTransform->scaleY / 2.0f;
    float playerHalfWidth = playerTransform->scaleX / 2.0f;
    float playerHalfHeight = playerTransform->scaleY / 2.0f;

    // check AABB collision
    bool colliding = (std::abs(playerX - winX) < (playerHalfWidth + winHalfWidth)) &&
        (std::abs(playerY - winY) < (playerHalfHeight + winHalfHeight));

    if (colliding)
    {
        std::cout << "[WinZone] Player reached the win zone!" << std::endl;
        hasPlayerWon = true;
        isWaitingForWin = true;
        winDelayTimer = 0.5f; // 0.5 second delay before transitioning
    }
}

// ============ WIN ============
void WinLoseLogicSystem::TriggerWin()
{
    // pause the game immediately to stop timer and gameplay
    CORE->SetGamePaused(true);

    // Play victory sound
    AudioManager* audioManager = CORE->GetAudioManager();
    if (audioManager)
    {
        audioManager->PlaySound("victory", "victory_channel");
    }

    //std::cout << "you win!!!" << std::endl;
    auto& factory = GameObjectFactory::Instance();
    vec2 camPos = CameraManager::Instance().GetCameraPosition();

    // win screen background
    GameObject* board = factory.CreateGameObject("WinScreen");
    auto* boardTransform = board->AddComponent<TransformComponent>();
    boardTransform->posX = camPos.x + 0.0f;
    boardTransform->posY = camPos.y + 0.0f;
    boardTransform->scaleX = 1920.0f;
    boardTransform->scaleY = 1080.0f;
    auto* boardRender = board->AddComponent<RenderComponent>();
    boardRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Game_win_screen.png"));
    boardRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    boardRender->SetLayer(101);

    // replay logo
    GameObject* replayLogo = factory.CreateGameObject("WinReplayLogo");
    auto* replayTransform = replayLogo->AddComponent<TransformComponent>();
    replayTransform->posX = camPos.x + 550.0f;
    replayTransform->posY = camPos.y + 50.0f;
    replayTransform->scaleX = 500.0f;
    replayTransform->scaleY = 150.0f;
    auto* replayRender = replayLogo->AddComponent<RenderComponent>();
    replayRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/Replay.png"));
    replayRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    replayRender->SetLayer(102);

    // yes button which restarts level
    GameObject* yesBtn = factory.CreateGameObject("WinYesButton");
    auto* yesTransform = yesBtn->AddComponent<TransformComponent>();
    yesTransform->posX = camPos.x + 550.0f;
    yesTransform->posY = camPos.y + -100.0f;  // below replay logo
    yesTransform->scaleX = 400.0f;
    yesTransform->scaleY = 100.0f;
    auto* yesRender = yesBtn->AddComponent<RenderComponent>();
    yesRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/Yes_dark.png"));
    yesRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    yesRender->SetLayer(102);

    // no button which goes bk to main menu
    GameObject* noBtn = factory.CreateGameObject("WinNoButton");
    auto* noTransform = noBtn->AddComponent<TransformComponent>();
    noTransform->posX = camPos.x + 550.0f;
    noTransform->posY = camPos.y + -220.0f;
    noTransform->scaleX = 400.0f;
    noTransform->scaleY = 100.0f;
    auto* noRender = noBtn->AddComponent<RenderComponent>();
    noRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/No_dark.png"));
    noRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    noRender->SetLayer(102);

    hasCreatedWinScreen = true;
}

void WinLoseLogicSystem::HandleWinScreenButtons()
{
    double mouseX = InputManager::GetMouseWorldX();
    double mouseY = InputManager::GetMouseWorldY();
    bool isClicked = InputManager::IsMouseLeftClicked();

    auto& factory = GameObjectFactory::Instance();
    std::string currentHover = "";

    if (IsMouseOver(factory.FindGameObject("WinYesButton"), mouseX, mouseY))
    {
        currentHover = "WinYesButton";
    }
    else if (IsMouseOver(factory.FindGameObject("WinNoButton"), mouseX, mouseY))
    {
        currentHover = "WinNoButton";
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

    if (isClicked && !hoveredButtonName.empty())
    {
        AudioManager* audioManager = CORE->GetAudioManager();
        if (audioManager)
        {
            audioManager->TriggerEvent("UI_Click");
        }

        if (hoveredButtonName == "WinYesButton")
        {
            if (!currentScenePath.empty())
            {
                std::string sceneToLoad = currentScenePath;
                DestroyWinScreen();
                CORE->SetGamePaused(false);
                CORE->RequestSceneLoad(sceneToLoad, true);
                return;
            }
        }
        else if (hoveredButtonName == "WinNoButton")
        {
            DestroyWinScreen();
            CORE->SetGamePaused(false);
            CORE->RequestSceneLoad("../Assets/Scenes/MainMenu.json", true);
            return;
        }
    }
}

void WinLoseLogicSystem::DestroyWinScreen()
{
    if (!hasCreatedWinScreen)
        return;

    auto& factory = GameObjectFactory::Instance();

    std::vector<std::string> uiElements = {
        "WinScreen",
        "WinReplayLogo",
        "WinYesButton",
        "WinNoButton"
    };

    for (const auto& name : uiElements)
    {
        GameObject* obj = factory.FindGameObject(name);
        if (obj)
        {
            factory.DestroyGameObject(obj->GetID());
        }
    }

    // reset win state
    hasCreatedWinScreen = false;
    hasPlayerWon = false;
    isWaitingForWin = false;
    hoveredButtonName.clear();

    std::cout << "[WinZone] Win screen destroyed and state reset" << std::endl;
}

bool WinLoseLogicSystem::IsMouseOver(GameObject* button, double mouseX, double mouseY)
{
    if (!button) return false;
    if (!button->IsActive()) return false;

    auto* tx = button->GetComponent<TransformComponent>();
    if (!tx) return false;

    return IsAreaClicked(tx->posX, tx->posY, tx->scaleX, tx->scaleY, mouseX, mouseY);
}

void WinLoseLogicSystem::SetButtonTexture(std::string buttonName, bool isHovered)
{
    auto& factory = GameObjectFactory::Instance();
    GameObject* btn = factory.FindGameObject(buttonName);
    if (!btn) return;

    auto* rc = btn->GetComponent<RenderComponent>();
    if (!rc) return;

    std::string texturePath;
    if (buttonName == "WinYesButton" || buttonName == "LoseYesButton")
    {
        texturePath = isHovered ? "../Assets/Sprites/Icons/Yes_light.png" : "../Assets/Sprites/Icons/Yes_dark.png";
    }
    else if (buttonName == "WinNoButton" || buttonName == "LoseNoButton")
    {
        texturePath = isHovered ? "../Assets/Sprites/Icons/No_light.png" : "../Assets/Sprites/Icons/No_dark.png";
    }
    else
    {
        return;
    }

    unsigned int newtex = GraphicsSystem::LoadTexture(texturePath.c_str());
    rc->SetTextureID(newtex);
}

// ============ LOSE ============

void WinLoseLogicSystem::TriggerLose()
{
    // pause the game immediately to stop timer and gameplay
    CORE->SetGamePaused(true);

    // Play game over sound
    AudioManager* audioManager = CORE->GetAudioManager();
    if (audioManager)
    {
        audioManager->PlaySound("gameover", "gameover_channel");
    }

    std::cout << "[WinLoseLogic] YOU LOSE!" << std::endl;

    auto& factory = GameObjectFactory::Instance();
    vec2 camPos = CameraManager::Instance().GetCameraPosition();

    // lose screen background
    GameObject* board = factory.CreateGameObject("LoseScreen");
    auto* boardTransform = board->AddComponent<TransformComponent>();
    boardTransform->posX = camPos.x + 0.0f;
    boardTransform->posY = camPos.y + 0.0f;
    boardTransform->scaleX = 1920.0f;
    boardTransform->scaleY = 1080.0f;
    auto* boardRender = board->AddComponent<RenderComponent>();
    boardRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Game_over_screen.png"));
    boardRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    boardRender->SetLayer(101);

    // replay logo
    GameObject* replayLogo = factory.CreateGameObject("LoseReplayLogo");
    auto* replayTransform = replayLogo->AddComponent<TransformComponent>();
    replayTransform->posX = camPos.x + 550.0f;
    replayTransform->posY = camPos.y + 50.0f;
    replayTransform->scaleX = 500.0f;
    replayTransform->scaleY = 150.0f;
    auto* replayRender = replayLogo->AddComponent<RenderComponent>();
    replayRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/Replay.png"));
    replayRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    replayRender->SetLayer(102);

    // yes button which restarts level
    GameObject* yesBtn = factory.CreateGameObject("LoseYesButton");
    auto* yesTransform = yesBtn->AddComponent<TransformComponent>();
    yesTransform->posX = camPos.x + 550.0f;
    yesTransform->posY = camPos.y + -100.0f;
    yesTransform->scaleX = 400.0f;
    yesTransform->scaleY = 100.0f;
    auto* yesRender = yesBtn->AddComponent<RenderComponent>();
    yesRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/Yes_dark.png"));
    yesRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    yesRender->SetLayer(102);

    // no button which goes back to main menu
    GameObject* noBtn = factory.CreateGameObject("LoseNoButton");
    auto* noTransform = noBtn->AddComponent<TransformComponent>();
    noTransform->posX = camPos.x + 550.0f;
    noTransform->posY = camPos.y + -220.0f;
    noTransform->scaleX = 400.0f;
    noTransform->scaleY = 100.0f;
    auto* noRender = noBtn->AddComponent<RenderComponent>();
    noRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/No_dark.png"));
    noRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    noRender->SetLayer(102);

    hasCreatedLoseScreen = true;
}

void WinLoseLogicSystem::HandleLoseScreenButtons()
{
    double mouseX = InputManager::GetMouseWorldX();
    double mouseY = InputManager::GetMouseWorldY();
    bool isClicked = InputManager::IsMouseLeftClicked();

    auto& factory = GameObjectFactory::Instance();
    std::string currentHover = "";

    if (IsMouseOver(factory.FindGameObject("LoseYesButton"), mouseX, mouseY))
    {
        currentHover = "LoseYesButton";
    }
    else if (IsMouseOver(factory.FindGameObject("LoseNoButton"), mouseX, mouseY))
    {
        currentHover = "LoseNoButton";
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

    if (isClicked && !hoveredButtonName.empty())
    {
        AudioManager* audioManager = CORE->GetAudioManager();
        if (audioManager)
        {
            audioManager->TriggerEvent("UI_Click");
        }

        if (hoveredButtonName == "LoseYesButton")
        {
            std::cout << "[WinLoseLogic] Yes clicked - Restarting level" << std::endl;

            if (!currentScenePath.empty())
            {
                std::string sceneToLoad = currentScenePath;
                DestroyLoseScreen();
                CORE->SetGamePaused(false);
                CORE->RequestSceneLoad(sceneToLoad, true);
                return;
            }
        }
        else if (hoveredButtonName == "LoseNoButton")
        {
            std::cout << "[WinLoseLogic] No clicked - Going to main menu" << std::endl;

            DestroyLoseScreen();
            CORE->SetGamePaused(false);
            CORE->RequestSceneLoad("../Assets/Scenes/MainMenu.json", true);
            return;
        }
    }
}

void WinLoseLogicSystem::DestroyLoseScreen()
{
    if (!hasCreatedLoseScreen)
        return;

    auto& factory = GameObjectFactory::Instance();

    std::vector<std::string> uiElements = {
        "LoseScreen",
        "LoseReplayLogo",
        "LoseYesButton",
        "LoseNoButton"
    };

    for (const auto& name : uiElements)
    {
        GameObject* obj = factory.FindGameObject(name);
        if (obj)
        {
            factory.DestroyGameObject(obj->GetID());
        }
    }

    hasCreatedLoseScreen = false;
    hoveredButtonName.clear();

    std::cout << "[WinLoseLogic] Lose screen destroyed and state reset" << std::endl;
}
