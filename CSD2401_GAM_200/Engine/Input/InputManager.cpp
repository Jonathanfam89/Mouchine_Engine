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
    // ===== HERO MOVEMENT CONSTANTS (Customizable) =====
    static float HERO_GRAVITY = -381.0f;
    static float HERO_MOVE_FORCE = 200.0f;
    static float HERO_JUMP_FORCE = 1000.0f;
    static float HERO_JUMP_DURATION = 0.4f;
    static float HERO_FRICTION_COEFFICIENT = 5.0f;
    static float HERO_KNOCKBACK_FORCE = 800.0f;         // Sustained push force
    static float HERO_KNOCKBACK_DURATION = 0.2f;
    static float HERO_KNOCKBACK_IMPULSE_X = 300.0f;     // Instant horizontal velocity change
    static float HERO_KNOCKBACK_IMPULSE_Y = 100.0f;     // Instant vertical velocity change

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

    // =====KINEMATIC SETUP=====
    static bool kinematicInitialized = false;
    if (!kinematicInitialized) {
        physics->SetKinematic(true);
        physics->SetGravityValue(HERO_GRAVITY);
        kinematicInitialized = true;
    }

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

        // Only apply movement if not blocked by wall
        bool blockedByWall = physics->isCollidingLeft &&
            physics->leftCollisionLayer == CollisionLayer::Ground;

        if (!blockedByWall) {
            // Remove friction when actively moving
            forcesManager.RemoveForcesByType(ForceType::Friction);
            // Remove existing movement force
            forcesManager.RemoveForcesByType(ForceType::Move);
            // Add movement force with left direction
            forcesManager.AddForce(ForceType::Move, HERO_MOVE_FORCE, -1.0f, -1.0f, 0.0f);
        }
    }
    else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        moveDirectionX = 1.0f;

        // Only apply movement if not blocked by wall
        bool blockedByWall = physics->isCollidingRight &&
            physics->rightCollisionLayer == CollisionLayer::Ground;

        if (!blockedByWall) {
            // Remove friction when actively moving
            forcesManager.RemoveForcesByType(ForceType::Friction);
            // Remove existing movement force
            forcesManager.RemoveForcesByType(ForceType::Move);
            // Add movement force with right direction
            forcesManager.AddForce(ForceType::Move, HERO_MOVE_FORCE, -1.0f, 1.0f, 0.0f);
        }
    }
    else if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
        CameraManager::Instance().AddCameraTrauma(1.f);
        CameraManager::Instance().FlashActiveCamera(0.3, vec4(1.f, 0.f, 0.f, 0.7f));
        //CORE->SetGamePaused(!CORE->IsGamePaused());
    }
    else
    {
        // No movement keys pressed - remove movement force
        forcesManager.RemoveForcesByType(ForceType::Move);

        // Add friction to slow down (only if not already present)
        if (!forcesManager.HasForceType(ForceType::Friction) && physics->IsGrounded())
        {
            forcesManager.AddForce(ForceType::Friction, HERO_FRICTION_COEFFICIENT, -1.0f, 0.0f, 0.0f, &physics->isGrounded);
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

    // Get current velocity for collision checks
    physics->GetVelocity(vx, vy);

    // ===== ENEMY COLLISION HANDLING ===== //
    // Check if colliding with enemies and apply knockback (impulse + force)
    if (physics->isCollidingLeft && physics->leftCollisionLayer == CollisionLayer::Enemy) {
        // Enemy on left, push player right

        // Apply instant impulse
        physics->velocity.x += HERO_KNOCKBACK_IMPULSE_X;
        physics->velocity.y += HERO_KNOCKBACK_IMPULSE_Y;

        // Apply knockback force
        forcesManager.ApplyKnockback(1.0f, 0.2f, HERO_KNOCKBACK_FORCE);

        // Can add other things besides knockback like damage
    }

    if (physics->isCollidingRight && physics->rightCollisionLayer == CollisionLayer::Enemy) {
        // Enemy on right, push player left

        // Apply instant impulse
        physics->velocity.x -= HERO_KNOCKBACK_IMPULSE_X;
        physics->velocity.y += HERO_KNOCKBACK_IMPULSE_Y;

        // Apply knockback force
        forcesManager.ApplyKnockback(-1.0f, 0.2f, HERO_KNOCKBACK_FORCE);
    }

    // ===== WALL/GROUND COLLISION HANDLING =====
    // Stop horizontal movement when hitting walls
    if (physics->isCollidingLeft &&
        physics->leftCollisionLayer == CollisionLayer::Ground) {
        if (physics->velocity.x < 0) {
            physics->velocity.x = 0.0f;
            forcesManager.RemoveForcesByType(ForceType::Move);
        }
    }

    if (physics->isCollidingRight &&
        physics->rightCollisionLayer == CollisionLayer::Ground) {
        if (physics->velocity.x > 0) {
            physics->velocity.x = 0.0f;
            forcesManager.RemoveForcesByType(ForceType::Move);
        }
    }

    // Stop vertical movement when hitting ceiling
    if (physics->isCollidingTop &&
        physics->topCollisionLayer == CollisionLayer::Ground) {
        if (physics->velocity.y > 0) {
            physics->velocity.y = 0.0f;
        }
    }

    // Stop downward movement when hitting ground
    if (physics->isCollidingBottom &&
        physics->bottomCollisionLayer == CollisionLayer::Ground) {
        if (physics->velocity.y < 0) {
            physics->velocity.y = 0.0f;
        }
    }

    // Wall slide effect (pretty cool ngl)
    bool touchingLeftWall = physics->isCollidingLeft &&
        physics->leftCollisionLayer == CollisionLayer::Ground;
    bool touchingRightWall = physics->isCollidingRight &&
        physics->rightCollisionLayer == CollisionLayer::Ground;

    if ((touchingLeftWall || touchingRightWall) &&
        !physics->IsGrounded() &&
        physics->velocity.y < -50.0f) {
        // Slow down fall when sliding on wall
        physics->velocity.y = -50.0f;
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
            // Apply jump
            physics->GetForcesManager().AddForce(ForceType::Jump, HERO_JUMP_FORCE, HERO_JUMP_DURATION);
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
            float dashDirection = 1.0f;  // Default right

            if (std::abs(moveDirectionX) > 0.01f) {
                // Use current movement direction
                dashDirection = moveDirectionX;
            }
            else if (vx < -0.1f) {
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
            }
        }
    }
    else if (!dashKeyDown) {
        dashKeyPressed = false;
    }

    // Check if dash just ended and restore gravity
    bool isDashing = physics->GetForcesManager().HasForceType(ForceType::Dash);
    if (wasDashing && !isDashing) {
        // Dash just ended, restore gravity
        physics->SetGravity(true);
        std::cout << "[Dash] Ended, gravity restored" << std::endl;
    }
    wasDashing = isDashing;

    // Restore air dash when grounded
    if (physics->IsGrounded() && hasUsedAirDash) {
        hasUsedAirDash = false;
        std::cout << "[Dash] Air dash restored (grounded)" << std::endl;
    }

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
