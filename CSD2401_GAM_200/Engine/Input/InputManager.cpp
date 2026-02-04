/* Start Header ************************************************************************/
/*!
\file       InputManager.cpp
\author     Lee Hwee Min [100% - 181 lines]
\par        l.hweemin@digipen.edu
\date       October, 2, 2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "inputmanager.h"
#include <iostream>
#include <unordered_map>
#include "Animator.hpp"
#include "core.h"
#include "Serialization.h"
#include "IMessageHandler.h"
#include "GameTimerSystem.h"
#include "PauseMenuLogic.h"
#include "LevelEditor.h"

bool InputManager::KeyStateImgui = true;
double InputManager::MouseWorldX = 0.0;
double InputManager::MouseWorldY = 0.0;
bool InputManager::MouseLeftClicked = false;
bool InputManager::ShowFPS = false;

/**
 * @brief Constructs an InputManager with specified parameters.
 * @param window Pointer to the GLFW window.
 * @param audio Optional pointer to the AudioEngine.
 */
InputManager::InputManager(GLFWwindow *window, AudioEngine *audio) : window(window), audioEngine(audio), width(0), height(0), mouseX(0.0), mouseY(0.0), scrollX(0.0), scrollY(0.0),cachedPlayerID(0)
{
    glfwSetWindowUserPointer(window, this);

    if (audioEngine)
    {
        std::cout << "InputManager: AudioEngine successfully linked" << std::endl;
    }
    else
    {
        std::cout << "InputManager: WARNING - AudioEngine is NULL!" << std::endl;
    }
	for (int i = 0; i < GLFW_KEY_LAST; ++i) {
		m_keyStates[i] = KEY_STATE_RELEASE;
	}
	for (int i = 0; i < GLFW_MOUSE_BUTTON_LAST; ++i) {
		m_mouseButtonStates[i] = KEY_STATE_RELEASE;
	}
}

/**
 * @brief Destructor for InputManager.
 */
InputManager::~InputManager() {}

/**
 * @brief Initializes the InputManager by fetching window size.
 */
void InputManager::Initialize()
{
    glfwGetWindowSize(window, &width, &height);
}
/**
 * @brief Gets the system name.
 * @return The string "Input System".
 */

std::string InputManager::GetName()
{
    return "InputSystem";
}

bool InputManager::IsKeyPressed(int key)
{
	return m_keyStates[key] == KEY_STATE_PRESS;
}

bool InputManager::IsKeyReleased(int key)
{
	return m_keyStates[key] == KEY_STATE_RELEASE;
}

bool InputManager::IsKeyHeld(int key)
{
	return m_keyStates[key] == KEY_STATE_HOLD;
}

bool InputManager::IsMouseButtonPressed(int button)
{
	return m_mouseButtonStates[button] == KEY_STATE_PRESS;
}

bool InputManager::IsMouseButtonReleased(int button)
{
	return m_mouseButtonStates[button] == KEY_STATE_RELEASE;
}

bool InputManager::IsMouseButtonHeld(int button)
{
	return m_mouseButtonStates[button] == KEY_STATE_HOLD;
}


/**
 * @brief Updates input handling, including key presses, mouse events,
 *        player movement, jumping, animations, and quitting logic.
 * @param dt Delta time since the last frame.
 */
void InputManager::Update(float dt)
{
	static bool firstUpdate = true;
	if (firstUpdate) {
		std::cout << "[InputManager] Update function is running!" << std::endl;
		firstUpdate = false;
	}

	for (auto& it : m_keyStates) {
		if (glfwGetKey(window, it.first) == GLFW_PRESS) {
			if (it.second == KEY_STATE_RELEASE) {
				it.second = KEY_STATE_PRESS;
			}
			else {
				it.second = KEY_STATE_HOLD;
			}
		}
		else {
			it.second = KEY_STATE_RELEASE;
		}
	}
	for (auto& it : m_mouseButtonStates) {
		if (glfwGetMouseButton(window, it.first) == GLFW_PRESS) {
			if (it.second == KEY_STATE_RELEASE) {
				it.second = KEY_STATE_PRESS;
			}
			else {
				it.second = KEY_STATE_HOLD;
			}
		}
		else {
			it.second = KEY_STATE_RELEASE;
		}
	}

    glfwGetWindowSize(window, &width, &height);
    static bool wasLeftMousePressed = false;
    bool isLeftMousePressed = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);

    MouseLeftClicked = (!wasLeftMousePressed && isLeftMousePressed);
    wasLeftMousePressed = isLeftMousePressed;

    // === UPDATE MOUSE POSITION FIRST - BEFORE ANY EARLY RETURNS ===
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    mouseX = xpos;
    mouseY = ypos;

    // Convert screen to world coordinates
    vec2 camPos = CameraManager::Instance().GetCameraPosition();
    MouseWorldX = static_cast<double>(xpos - width / 2.0) + camPos.x;
    MouseWorldY = static_cast<double>(height / 2.0 - ypos) + camPos.y; // Flip Y axis

    static bool keyPressed = false;
#ifdef USE_IMGUI
    LevelEditor *levelEditor = CORE->GetLevelEditor();
    if (levelEditor && levelEditor->GetEditorMode() != EditorMode::Play && levelEditor->GetEditorMode() != EditorMode::Stop)
    {
        static bool editorModeWarning = true;
        if (editorModeWarning) {
            std::cout << "[InputManager] Not in Play/Stop mode - input disabled" << std::endl;
            editorModeWarning = false;
        }
        // still allow escape key, but ignore player movement inputs
        double xposition, yposition;
        glfwGetCursorPos(window, &xposition, &yposition);
        mouseX = xposition;
        mouseY = yposition;
        return;
    }

    static bool playModeConfirmed = false;
    if (!playModeConfirmed) {
        std::cout << "[InputManager] In Play mode - input enabled" << std::endl;
        playModeConfirmed = true;
    }

    // ------------------- UNDO: Ctrl + Z -------------------//
    {
        LevelEditor* editor = CORE->GetLevelEditor();
        if (editor)
        {
            bool ctrl = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
                glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;

            bool zPressed = glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS;

            static bool undoConsumed = false;

            if (ctrl && zPressed)
            {
                if (!undoConsumed && !editor->undoStack.empty())
                {
                    editor->PerformUndo();
                    undoConsumed = true;       // Prevents rapid repeat
                }
            }
            else
            {
                undoConsumed = false;          // Reset when keys released
            }
        }
    }
#endif

    static std::unordered_map<int, bool> singleTriggerKeys = {
        {GLFW_KEY_SPACE, false},
        {GLFW_KEY_F1, false},
        {GLFW_KEY_F2, false},
        {GLFW_KEY_F3, false},
        {GLFW_KEY_F4, false},
        {GLFW_KEY_M, false}, // toggle pause (for now)
        {GLFW_KEY_ENTER, false},
        {GLFW_KEY_ESCAPE,false},
        {GLFW_KEY_P, false}
    };

    static bool altEnterPressed = false;
    bool altPressed = (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS) ||
        (glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS);
    bool enterPressed = (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) ||
        (glfwGetKey(window, GLFW_KEY_KP_ENTER) == GLFW_PRESS);

    if (altPressed && enterPressed) {
        if (!altEnterPressed) {
            altEnterPressed = true;
            // Toggle fullscreen
            if (CORE && CORE->GetGraphicsSystem()) {
                CORE->GetGraphicsSystem()->ToggleFullScreen();
            }
        }
    }
    else {
        altEnterPressed = false;
    }


    // handle M key for pausing and resuming background music
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
    {
        if (!singleTriggerKeys[GLFW_KEY_M])
        {
            AudioManager* audioManager = CORE->GetAudioManager();
            if (audioManager)
            {
                audioManager->ToggleMute();
            }
            singleTriggerKeys[GLFW_KEY_M] = true;
        }
    }
#ifdef USE_IMGUI
    // Check if in play mode for pause menu 
    LevelEditor* editor = CORE ? CORE->GetLevelEditor() : nullptr;
    bool isPlayMode = (editor && editor->GetEditorMode() == EditorMode::Play);
#else
    bool isPlayMode = true;
#endif

    // Handle ESC key
    if (isPlayMode)
    {
        // Check if win or lose screen is active - if so, disable ESC key
        WinLoseLogicSystem* winZone = CORE->GetWinLoseLogic();
        bool winScreenActive = winZone && winZone->IsWinScreenActive();
        bool loseScreenActive = winZone && winZone->IsLoseScreenActive();

        // In play mode: ESC toggles pause menu (unless win/lose screen is active)
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            if (!singleTriggerKeys[GLFW_KEY_ESCAPE] && !winScreenActive && !loseScreenActive) {
                PauseMenuLogicSystem* pauseMenu = CORE->GetPauseMenuLogic();
                if (pauseMenu)
                {
                    pauseMenu->TogglePauseMenu();
                }
                singleTriggerKeys[GLFW_KEY_ESCAPE] = true;
            }
        }
        else
        {
            singleTriggerKeys[GLFW_KEY_ESCAPE] = false;
        }
    }
    else
    {
        // Not in play mode: ESC quits the game
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            KeyStateQuit = true;
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }

    // Handle P key
    if (isPlayMode)
    {
        // In play mode: P toggles pause menu
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
        {
            if (!singleTriggerKeys[GLFW_KEY_P])
            {
                PauseMenuLogicSystem* pauseMenu = CORE->GetPauseMenuLogic();
                if (pauseMenu)
                {
                    pauseMenu->TogglePauseMenu();
                }
                singleTriggerKeys[GLFW_KEY_P] = true;
            }
        }
        else
        {
            singleTriggerKeys[GLFW_KEY_P] = false;
        }
    }
    else
    {
        // Not in play mode: P toggles ImGui
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
        {
            InputManager::KeyStateImgui = true;
        }
    }

    for (auto &kv : singleTriggerKeys)
    {
        int key = kv.first;
        bool &wasPressed = kv.second;

        if (glfwGetKey(window, key) == GLFW_PRESS)
        {
            if (!wasPressed)
            {
                if (key == GLFW_KEY_F1)
                {
                    CORE->GetGraphicsSystem()->toggleDebugMode();
                    std::cout << "Dubug mode toggled" << CORE->GetGraphicsSystem()->isDebugModeOn() << std::endl;
                }
                else if (key == GLFW_KEY_F2)
                {
                    CORE->stepMode = !CORE->stepMode;

                    if (!CORE->stepMode)
                    {
                        CORE->accumulator = 0.0;
                    }

                    std::cout << "\n[STEP MODE] " << (CORE->stepMode ? "ENABLED - Press F3 to advance frames" : "DISABLED - Normal play") << std::endl;
                }
                else if (key == GLFW_KEY_F3 && CORE->stepMode)
                {
                    CORE->stepOnce = true;
                    std::cout << "[STEP MODE] Advancing 1 frame..." << std::endl;
                }
                else if (key == GLFW_KEY_F4)
                {
                    ShowFPS = !ShowFPS;
                    std::cout << "FPS display " << (ShowFPS ? "ENABLED" : "DISABLED") << std::endl;
                }
                else if (key == GLFW_KEY_P) {

                }
            }
            wasPressed = true;
        }
        else
        {
            wasPressed = false;
        }
    }

    // Handles Holding F1 for How To Play in playmode
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_KEY_DOWN && isPlayMode)
    {
        //show how to play and pause game as long as key held
    }

    GameObject* player = nullptr;

    // Try to get cached player first
    if (cachedPlayerID != 0)
    {
        player = GameObjectFactory::Instance().GetGameObject(cachedPlayerID);
        if (!player)
        {
            // Cached player no longer exists (scene changed), clear cache
            std::cout << "[InputManager] Cached player no longer exists, clearing cache" << std::endl;
            cachedPlayerID = 0;
        }
    }

    // If no cached player, try to find Hero
    if (!player)
    {
        player = GameObjectFactory::Instance().FindGameObject("Hero");
        if (player)
        {
            cachedPlayerID = player->GetID();
            std::cout << "[InputManager] Found Hero with ID: " << cachedPlayerID << std::endl;
        }
        else
        {
            // Only print warning occasionally to avoid spam
            static int warningCounter = 0;
            if (warningCounter++ % 60 == 0)
            {
                std::cout << "[InputManager] WARNING: Hero not found in scene!" << std::endl;
            }
        }
    }

    if (!player)
    {
        return; // No player to control
    }


    auto* physics = player->GetComponent<PhysicsComponent>();
    if (!physics) return;

    TransformComponent* transform = player->GetComponent<TransformComponent>();
    if (!transform) return;

    // ===== PARTICLE TESTING SETUP
    
    //auto* dashTrail = player->GetComponent<ParticleEmitterComponent>();

    //if (!dashTrail) {
    //    // If it doesn't exist, add it and configure it once
    //    dashTrail = player->AddComponent<ParticleEmitterComponent>();
    //    if (dashTrail) {
    //        dashTrail->ConfigureDashTrail();
    //        dashTrail->SetTexture(0);
    //        std::cout << "Particle Emitter initialized successfully\n";
    //    }
    //    else {
    //        std::cerr << "particle not initialized properly\n";
    //    }
    //}

    /**
     * @brief Destroy 2500 Game Objects on screen, if ENTER key is pressed
     */
    if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
    {
        DemoM1_DestroyGameObjects();
    }
    else if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && isPlayMode) 
    {
        CameraManager::Instance().AddCameraTrauma(1.f);
        CameraManager::Instance().FlashActiveCamera(0.3f, vec4(1.f, 0.f, 0.f, 0.7f));
        //CORE->SetGamePaused(!CORE->IsGamePaused());
    }
}

    void InputManager::ResetState()
    {

        MouseLeftClicked = false;

        mouseX = 0.0;
        mouseY = 0.0;
        scrollX = 0.0;
        scrollY = 0.0;

        std::cout << "[InputManager] State reset (including all abilities)." << std::endl;
    }
