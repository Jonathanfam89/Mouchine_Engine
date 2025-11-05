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

bool InputManager::KeyStateImgui = true;
double InputManager::MouseWorldX = 0.0;
double InputManager::MouseWorldY = 0.0;
bool InputManager::MouseLeftClicked = false;

/**
 * @brief Constructs an InputManager with specified parameters.
 * @param window Pointer to the GLFW window.
 * @param audio Optional pointer to the AudioEngine.
 */
InputManager::InputManager(GLFWwindow *window, AudioEngine *audio) : window(window), audioEngine(audio), width(0), height(0), mouseX(0.0), mouseY(0.0), scrollX(0.0), scrollY(0.0)
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

/**
 * @brief Updates input handling, including key presses, mouse events,
 *        player movement, jumping, animations, and quitting logic.
 * @param dt Delta time since the last frame.
 */
void InputManager::Update(float dt)
{
    static bool wasLeftMousePressed = false;
    bool isLeftMousePressed = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
    MouseLeftClicked = (!wasLeftMousePressed && isLeftMousePressed);
    wasLeftMousePressed = isLeftMousePressed; 


    static bool keyPressed = false;
    LevelEditor* levelEditor = CORE->GetLevelEditor();
    if (levelEditor && levelEditor->GetEditorMode() != EditorMode::Play && levelEditor->GetEditorMode() != EditorMode::Stop)
    {
        // still allow escape key, but ignore player movement inputs
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        mouseX = xpos;
        mouseY = ypos;
        return;
    }

    static std::unordered_map<int, bool> singleTriggerKeys = {
        {GLFW_KEY_SPACE, false},
        {GLFW_KEY_F1, false},
        {GLFW_KEY_F2, false},
        {GLFW_KEY_F3, false},
        {GLFW_KEY_F4, false},
        {GLFW_KEY_M, false}, // toggle pause (for now)
    };

    // handle M key for pausing and resuming background music
    static bool bgmPaused = false;

    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
    {
        if (!singleTriggerKeys[GLFW_KEY_M])
        {
            if (audioEngine)
            {
                bgmPaused = !bgmPaused;
                audioEngine->pauseSound("bgm_channel", bgmPaused);
                std::cout << "bgm paused" << std::endl;
            }
            singleTriggerKeys[GLFW_KEY_M] = true;
        }
    }

    for (auto& kv : singleTriggerKeys)
    {
        int key = kv.first;
        bool& wasPressed = kv.second;

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

                    if (!CORE->stepMode) {
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
                    CORE->GetGraphicsSystem()->SetEditorMode();
                    std::cout << "Editor Mode Toggled" << CORE->GetGraphicsSystem()->IsEditorMode() << std::endl;
                }
            }
            wasPressed = true;
        }
        else
        {
            wasPressed = false;
        }
    }

        auto* player = GameObjectFactory::Instance().FindGameObject("Hero");
        if (!player)
        {
            return;
        }

        auto* physics = player->GetComponent<PhysicsComponent>();
        if (!physics)
        {
            return;
        }

        TransformComponent* transform = player->GetComponent<TransformComponent>();
        if (!transform)
        {
            return;
        }

        //// Ensure player has drag force for smooth stopping (add once)
        //static bool dragInitialized = false;
        //if (!dragInitialized && physics)
        //{
        //    auto& forcesManager = physics->GetForcesManager();
        //    forcesManager.AddDrag();
        //    dragInitialized = true;
        //}

        float vx, vy;
        physics->GetVelocity(vx, vy);

        /**
         * @brief Destroy 2500 Game Objects on screen, if ENTER key is pressed
         */
        if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
        {
            DemoM1_DestroyGameObjects();
        }

        //---------------NEW SYSTEM USING PHYSICS FOR MOVEMENT-------------------//
        // Horizontal movement using forces
        /*float moveForce = 800.0f;*/ // Force magnitude for movement
        float moveDirectionX = 0.0f;  // Track intended direction (1=Right, -1=Left)
        bool is_jumping = false;

        auto& forcesManager = physics->GetForcesManager();

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        {
            moveDirectionX = -1.0f;

            // Add left movement force using helper
            forcesManager.AddForce(ForceType::MoveLeft, physics->GetMass());
            // Remove right movement if present
            forcesManager.RemoveForcesByType(ForceType::MoveRight);
        }
        else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        {
            moveDirectionX = 1.0f;

            // Add right movement force using helper
            forcesManager.AddForce(ForceType::MoveRight, physics->GetMass());
            // Remove left movement if present
            forcesManager.RemoveForcesByType(ForceType::MoveLeft);
        }
        else
        {
            // No movement keys pressed - remove all movement forces
            forcesManager.RemoveForcesByType(ForceType::MoveLeft);
            forcesManager.RemoveForcesByType(ForceType::MoveRight);
        }

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            KeyStateQuit = true;
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        /**
         * @brief Toggle on and off imgui
         */
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
        {
            InputManager::KeyStateImgui = true;
        }
        // Jump
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        {
            // Only jump if grounded
            is_jumping = true;
            if (physics->IsGrounded())
            {
                is_jumping = false;
                physics->GetForcesManager().AddForce(ForceType::Jump, physics->GetMass());
                physics->GetVelocity(vx, vy); // Get updated VY after jump
                // Play jump sound effect
                if (audioEngine)
                {
                    audioEngine->playSound("jump_sfx", "jump_channel");
                }
                /*if (audioEngine) {
                    Message msg;
                    msg.type = "PlaySound";
                    msg.sender = this;
                    msg.data = (void*)"jump_sfx";
                    CORE->GetMessageBus().Broadcast(msg);
                }*/
            }
        }

        auto* anim = player->GetComponent<Spritesheet>();

        if (transform && anim)
        {

            if (std::abs(moveDirectionX) > 0.0f)
            {
                float baseWidth = std::abs(transform->scaleX);
                float newScaleX = (moveDirectionX < 0) ? -baseWidth : baseWidth;
                transform->SetScale(newScaleX, transform->scaleY);
            }

            const char* IDLE_SS = "./Assets/Mouch_Idle.png";
            int _ROWS = 1;
            int _COLS = 2;
            const int FRAME_START = 0;
            const int FRAME_END = 1;
            const float IDLE_FPS = 2.0f;
            const float RUN_FPS = 7.0f;
            const char* RUN_SS = "./Assets/Mouch_Running.png";
            const char* JUMP_SS = "./Assets/Mouch_jump.png";
            // --------------------------------------------------------

            float newFPS;
            const char* newSpritesheet;

            if (std::abs(vx) > 0.0f) // Player is running/moving
            {
                newFPS = RUN_FPS;
                newSpritesheet = RUN_SS;
            }
            else if (is_jumping)
            {
                _COLS = 1;
                newFPS = 0;
                newSpritesheet = JUMP_SS;
            }
            else // Player is stopped (vx is 0)
            {
                newFPS = IDLE_FPS;
                newSpritesheet = IDLE_SS;
            }

            float desiredTimePerFrame = 1.0f / newFPS;

            if (std::abs(anim->GetTimePerFrame() - desiredTimePerFrame) > 0.0001f)
            {

                anim->LoadNewSpritesheet(newSpritesheet, _ROWS, _COLS);

                anim->SetAnimation(FRAME_START, FRAME_END, newFPS);
            }
        }

        // Note: Don't call SetVelocity anymore - forces handle movement
        // The Integrate() method will update velocity based on forces

        // Mouse
        //if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        //{
        //    std::cout << "Left Mouse Press, X:" << mouseX << " Y:" << mouseY << std::endl;
        //}
        //if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
        //{
        //    std::cout << "Right Mouse Press" << mouseX << " Y:" << mouseY << std::endl;
        //}

        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        mouseX = xpos;
        mouseY = ypos;

        MouseWorldX = mouseX - (width / 2.0);
        MouseWorldY = (height / 2.0) - mouseY;
    }