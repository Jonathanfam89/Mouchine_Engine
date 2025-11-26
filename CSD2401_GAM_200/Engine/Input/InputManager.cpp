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
        // still allow escape key, but ignore player movement inputs
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        mouseX = xpos;
        mouseY = ypos;
        return;
    }
#endif

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


    static std::unordered_map<int, bool> singleTriggerKeys = {
        {GLFW_KEY_SPACE, false},
        {GLFW_KEY_F1, false},
        {GLFW_KEY_F2, false},
        {GLFW_KEY_F3, false},
        {GLFW_KEY_F4, false},
        {GLFW_KEY_M, false}, // toggle pause (for now)
        {GLFW_KEY_ENTER, false}, 
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
    static bool bgmPaused = false;

    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
    {
        if (!singleTriggerKeys[GLFW_KEY_M])
        {
            AudioManager* audioManager = CORE->GetAudioManager();
            if (audioManager)
            {
                bgmPaused = !bgmPaused;
                audioManager->StopSound("bgm_channel");
                std::cout << "BGM " << (bgmPaused ? "paused" : "resumed") << std::endl;
            }
            singleTriggerKeys[GLFW_KEY_M] = true;
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
                    CORE->GetGraphicsSystem()->SetEditorMode(!CORE->GetGraphicsSystem()->IsEditorMode());
                    std::cout << "Editor Mode Toggled" << CORE->GetGraphicsSystem()->IsEditorMode() << std::endl;
                }
                else if (key == GLFW_KEY_P) {
                    ShowFPS = !ShowFPS;
                    std::cout << "FPS display " << (ShowFPS ? "ENABLED" : "DISABLED") << std::endl;
                }
            }
            wasPressed = true;
        }
        else
        {
            wasPressed = false;
        }
    }
    GameObject* player = nullptr;


    if (cachedPlayerID != 0)
    {
        player = GameObjectFactory::Instance().GetGameObject(cachedPlayerID);
    }

    if (!player)
    {
        player = GameObjectFactory::Instance().FindGameObject("Hero");
        if (player)
        {
            cachedPlayerID = player->GetID();
        }
    }

    if (!player)
    {
        return;
    }


    auto* physics = player->GetComponent<PhysicsComponent>();
    if (!physics) return;

    TransformComponent* transform = player->GetComponent<TransformComponent>();
    if (!transform) return;


  

    //// Ensure player has drag force for smooth stopping (add once)
    // static bool dragInitialized = false;
    // if (!dragInitialized && physics)
    //{
    //     auto& forcesManager = physics->GetForcesManager();
    //     forcesManager.AddDrag();
    //     dragInitialized = true;
    // }

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

    auto &forcesManager = physics->GetForcesManager();

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        moveDirectionX = -1.0f;

        // Remove friction when actively moving
        forcesManager.RemoveForcesByType(ForceType::Friction);
        // Add left movement force using helper
        forcesManager.AddForce(ForceType::MoveLeft);
        // Remove right movement if present
        forcesManager.RemoveForcesByType(ForceType::MoveRight);
    }
    else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        moveDirectionX = 1.0f;

        // Remove friction when actively moving
        forcesManager.RemoveForcesByType(ForceType::Friction);
        // Add right movement force using helper
        forcesManager.AddForce(ForceType::MoveRight);
        // Remove left movement if present
        forcesManager.RemoveForcesByType(ForceType::MoveLeft);
    }
    else if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
        CameraManager::Instance().AddCameraTrauma(1.f);
        CameraManager::Instance().FlashActiveCamera(0.3, vec4(1.f, 0.f, 0.f, 0.7f));
        //CORE->SetGamePaused(!CORE->IsGamePaused());
    }
    else
    {
        // No movement keys pressed - remove all movement forces
        forcesManager.RemoveForcesByType(ForceType::MoveLeft);
        forcesManager.RemoveForcesByType(ForceType::MoveRight);

        // Add friction to slow down (only if not already present)
        if (!forcesManager.HasForceType(ForceType::Friction) && physics->IsGrounded())
        {
            forcesManager.AddFriction(&physics->isGrounded);
        }

        // Reset footstep timer when not moving
        footstepTimer = 0.0f;
    }

    // Handle footstep sounds
    // Play footsteps if: moving horizontally, grounded, and not jumping
    bool isMoving = std::abs(vx) > 0.1f; // Small threshold to avoid playing when barely moving
    if (isMoving && physics->IsGrounded() && audioEngine)
    {
        footstepTimer += dt;

        // Play footstep sound at intervals
        if (footstepTimer >= footstepInterval)
        {
            AudioManager* audioManager = CORE->GetAudioManager();
            if (audioManager)
            {
                audioManager->TriggerEvent("PlayerLand");
                //audioManager->PlaySound("footstep_sfx");
            }
            footstepTimer = 0.0f; // reset timer
        }
    }
    else
    {
        // Reset timer when not moving or in air
        footstepTimer = 0.0f;
    }

    // After forces are applied, clamp velocity
    if (std::abs(vx) > 200.f)
    {
        float clampedVx = (vx > 0) ? 200.f : -200.f;
        physics->SetVelocity(clampedVx, vy);
    }

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        KeyStateQuit = true;
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    /**
     * @brief Toggle on and off imgui
     */
    //if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
    //{
    //    InputManager::KeyStateImgui = true;
    //}
    // Jump
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        // Only jump if grounded
        is_jumping = true;
        if (physics->IsGrounded())
        {
            is_jumping = false;
            physics->GetForcesManager().AddForce(ForceType::Jump);
            physics->GetVelocity(vx, vy); // Get updated VY after jump
            // Play jump sound effect
            AudioManager* audioManager = CORE->GetAudioManager();
            if (audioManager)
            {
                audioManager->TriggerEvent("PlayerJump");
                std::cout << "[InputManager] SPACE pressed to sending PlaySoundMessage\n";
                std::cout << "[InputManager] CORE pointer = " << CORE << std::endl;
                //PlaySoundMessage msg("jump_sfx");
                //CORE->GetMessageBus().Publish(msg);
                 audioEngine->playSound("jump_sfx", "jump_channel");
            }
        }
    }

    // ===== DASH SYSTEM =====
    static bool hasUsedAirDash = false;
    static bool dashKeyPressed = false;
    static bool wasDashing = false;

    // Dash input (LEFT SHIFT key)
    bool dashKeyDown = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS);

    if (dashKeyDown && !dashKeyPressed)
    {
        dashKeyPressed = true;

        // Check conditions
        bool isDashing = physics->GetForcesManager().HasForceType(ForceType::Dash);
        bool isGrounded = physics->IsGrounded();

        // Can dash if: not already dashing AND (grounded OR has air dash available)
        if (!isDashing && (isGrounded || !hasUsedAirDash))
        {
            // Determine dash direction using moveDirectionX if available, otherwise use velocityX
            float dashDirection = 1.0f;  // Default right

            if (std::abs(moveDirectionX) > 0.01f) {
                // Use current movement direction
                dashDirection = moveDirectionX;
            }
            else if (vx < -0.1f) {
                // Fallback to velocity direction if not pressing movement keys
                dashDirection = -1.0f;
            }
            else if (vx > 0.1f) {
                dashDirection = 1.0f;
            }
            // If both are zero, dash right by default

            // Add dash force
            physics->ApplyDashImpulse(dashDirection);

            // Disable gravity during dash
            physics->SetGravity(false);

            // Mark air dash as used if in air
            if (!isGrounded) {
                hasUsedAirDash = true;
                std::cout << "[Dash] Air dash used!" << std::endl;
            }

            std::cout << "[Dash] Started! Direction: " << (dashDirection > 0 ? "Right" : "Left") << std::endl;
        }
        else if (isDashing) {
            std::cout << "[Dash] Already dashing!" << std::endl;
        }
        else if (!isGrounded && hasUsedAirDash) {
            std::cout << "[Dash] Air dash already used!" << std::endl;
        }
    }
    else if (!dashKeyDown) {
        dashKeyPressed = false;
    }

    // Check if dash just ended and restore gravity
    bool isDashing = physics->GetForcesManager().HasForceType(ForceType::Dash);
    if (wasDashing && !isDashing) {
        // Dash just ended - restore gravity
        physics->SetGravity(true);
        std::cout << "[Dash] Ended, gravity restored" << std::endl;
    }
    wasDashing = isDashing;

    // Restore air dash when grounded
    if (physics->IsGrounded() && hasUsedAirDash) {
        hasUsedAirDash = false;
        std::cout << "[Dash] Air dash restored (grounded)" << std::endl;
    }
    // ===== END DASH SYSTEM =====

    auto* anim = player->GetComponent<Spritesheet>();
        if (transform && anim)
        {
            float absWidth = std::abs(transform->scaleX);
            transform->SetScale(absWidth, transform->scaleY);

      
            static bool isFacingRight = true;

            if (moveDirectionX > 0.0f) isFacingRight = true;
            if (moveDirectionX < 0.0f) isFacingRight = false;

            // 3. Determine Animation Name based on State AND Direction
            std::string requestedState = "";

            if (is_jumping)
            {
                requestedState = isFacingRight ? "Jump_R" : "Jump_L";
            }
            else if (std::abs(vx) > 1.0f)
            {
                requestedState = isFacingRight ? "Run_R" : "Run_L";
            }
            else
            {
                // If stopped, use the last known direction
                requestedState = isFacingRight ? "Idle_R" : "Idle_L";
            }

            // 4. Play the specific Left/Right animation
            anim->PlayAnimation(requestedState);
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

    }

    void InputManager::ResetState()
    {
 
        MouseLeftClicked = false;

        mouseX = 0.0;
        mouseY = 0.0;
        scrollX = 0.0;
        scrollY = 0.0;

        isJumping = false;
        jumpStartY = 0.0f;
        footstepTimer = 0.0f;
        cachedPlayerID = 0;
        std::cout << "[InputManager] State reset." << std::endl;
    }

