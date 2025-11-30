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
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        mouseX = xpos;
        mouseY = ypos;
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
    // ===== HERO MOVEMENT CONSTANTS (Customizable) =====
    static float HERO_GRAVITY = -381.0f;
    static float HERO_MOVE_FORCE = 200.0f;
    static float HERO_JUMP_FORCE = 1000.0f;
    static float HERO_JUMP_DURATION = 0.4f;
    static float HERO_FRICTION_COEFFICIENT = 5.0f;
    static float HERO_KNOCKBACK_FORCE = 800.0f;
    static float HERO_KNOCKBACK_DURATION = 0.2f;
    static float HERO_KNOCKBACK_IMPULSE_X = 300.0f;
    static float HERO_KNOCKBACK_IMPULSE_Y = 100.0f;

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

    // =====KINEMATIC SETUP=====
    static bool kinematicInitialized = false;
    if (!kinematicInitialized) {
        physics->SetKinematic(true);
        physics->SetGravityValue(HERO_GRAVITY);
        kinematicInitialized = true;
    }

    float vx, vy;
    physics->GetVelocity(vx, vy);

    physics->SetFrictionCoefficient(HERO_FRICTION_COEFFICIENT);

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
        static bool aDebugPrinted = false;
        if (!aDebugPrinted) {
            std::cout << "[InputManager] A key pressed - moving left" << std::endl;
            aDebugPrinted = true;
        }

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
        static bool dDebugPrinted = false;
        if (!dDebugPrinted) {
            std::cout << "[InputManager] D key pressed - moving right" << std::endl;
            dDebugPrinted = true;
        }

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
        // No movement keys pressed remove movement force
        forcesManager.RemoveForcesByType(ForceType::Move);

        // Add friction to slow down (only if not already present)
        if (!forcesManager.HasForceType(ForceType::Friction) && physics->IsGrounded())
        {
            forcesManager.AddForce(ForceType::Friction, -1.0f, -1.0f, 0.0f, 0.0f, &physics->isGrounded);
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
    // Check if colliding with enemies and apply knockback
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

    // ===== WALL SLIDE SYSTEM =====
    bool touchingLeftWall = physics->isCollidingLeft &&
        physics->leftCollisionLayer == CollisionLayer::Ground;
    bool touchingRightWall = physics->isCollidingRight &&
        physics->rightCollisionLayer == CollisionLayer::Ground;

    // Check if player is pushing towards wall
    bool isPushingTowardsWall = (touchingRightWall && moveDirectionX > 0) ||
                                 (touchingLeftWall && moveDirectionX < 0);

    // Wall slide activates when: touching wall, pushing towards it, in air, and falling
    if ((touchingLeftWall || touchingRightWall) &&
        isPushingTowardsWall &&
        !physics->IsGrounded() &&
        physics->velocity.y < 0) {

        if (!isWallSliding) {
            isWallSliding = true;
            std::cout << "[Wall Slide] Started!" << std::endl;
        }

        // Slow down fall when sliding on wall
        static float WALL_SLIDE_SPEED = -50.0f;
        if (physics->velocity.y < WALL_SLIDE_SPEED) {
            physics->velocity.y = WALL_SLIDE_SPEED;
        }
    }
    else {
        if (isWallSliding) {
            isWallSliding = false;
            std::cout << "[Wall Slide] Stopped!" << std::endl;
        }
    }

    //if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    //{
    //    //KeyStateQuit = true;
    //    //glfwSetWindowShouldClose(window, GLFW_TRUE);
    //    CORE->RequestSceneLoad("../Assets/Scenes/MainMenu.json", true);
    //}
    /**
     * @brief Toggle on and off imgui
     */
    //if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
    //{
    //    InputManager::KeyStateImgui = true;
    //}
    // ===== JUMP & DOUBLE JUMP SYSTEM =====
    // Reset jump count when grounded
    if (physics->IsGrounded() && numberOfJumpsUsed > 0) {
        numberOfJumpsUsed = 0;
    }

    static bool jumpKeyPressed = false;
    bool jumpKeyDown = (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);

    if (jumpKeyDown && !jumpKeyPressed)
    {
        jumpKeyPressed = true;
        is_jumping = true;

        // Can jump if: grounded OR in air but haven't used all jumps
        bool canJump = physics->IsGrounded() || numberOfJumpsUsed < maxJumps;

        if (canJump && !isGroundPounding && !isRolling)
        {
            is_jumping = false;

            // Apply jump force
            physics->GetForcesManager().AddForce(ForceType::Jump, HERO_JUMP_FORCE, HERO_JUMP_DURATION);
            physics->GetVelocity(vx, vy); // Get updated VY after jump

            // Increment jump count
            numberOfJumpsUsed++;

            // Decrease timer on jump
            auto* timerSystem = CORE->GetGameTimerSystem();
            if (timerSystem)
            {
                timerSystem->DecreaseTime(1.0f); // Decrease by 1 second per jump
            }

            // Play jump sound effect
            AudioManager* audioManager = CORE->GetAudioManager();
            if (audioManager)
            {
                audioManager->TriggerEvent("PlayerJump");
                std::cout << "[InputManager] SPACE pressed to sending PlaySoundMessage\n";
                std::cout << "[InputManager] CORE pointer = " << CORE << std::endl;
                audioManager->PlaySound("jump_sfx", "jump_channel");
            }

            // Log jump type
            if (numberOfJumpsUsed == 1) {
                std::cout << "[Jump] First jump" << std::endl;
            }
            else if (numberOfJumpsUsed == 2) {
                std::cout << "[Jump] Double jump!" << std::endl;
            }
        }
    }
    else if (!jumpKeyDown) {
        jumpKeyPressed = false;
    }

    // ===== DASH SYSTEM =====
    static bool hasUsedAirDash = false;
    static bool dashKeyPressed = false;
    static bool wasDashing = false;

    // Dash input (J key)
    bool dashKeyDown = (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS);

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

            // Decrease timer on dash
            auto* timerSystem = CORE->GetGameTimerSystem();
            if (timerSystem)
            {
                timerSystem->DecreaseTime(1.0f); // Decrease by 1 second per dash
            }

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

    // ===== WALL KICK SYSTEM =====
    static float WALL_KICK_SPEED = 400.0f;
    static float WALL_KICK_VERTICAL = 300.0f;
    static float WALL_KICK_DURATION = 0.15f;
    static float WALL_KICK_COOLDOWN = 0.5f;

    // Update cooldown timer
    if (wallKickCooldownTimer > 0) {
        wallKickCooldownTimer -= dt;
    }

    // Update wall kick timer
    if (isWallKicking && wallKickTimer > 0) {
        wallKickTimer -= dt;
        if (wallKickTimer <= 0) {
            isWallKicking = false;
            std::cout << "[Wall Kick] Ended" << std::endl;
        }
    }

    // Check if touching a wall
    bool canWallKick = (touchingLeftWall || touchingRightWall) && !physics->IsGrounded();

    // Wall Kick input (L key)
    bool wallKickKeyDown = (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS);

    if (wallKickKeyDown && !wallKickKeyPressed) {
        wallKickKeyPressed = true;

        // Can wall kick if touching wall, not grounded, and cooldown ready
        if (canWallKick && wallKickCooldownTimer <= 0 && !isWallKicking) {
            isWallKicking = true;
            wallKickTimer = WALL_KICK_DURATION;
            wallKickCooldownTimer = WALL_KICK_COOLDOWN;

            // Determine kick direction (opposite of wall)
            float kickDirectionX = touchingRightWall ? -1.0f : 1.0f;

            // Apply wall kick velocity
            physics->SetVelocity(kickDirectionX * WALL_KICK_SPEED, WALL_KICK_VERTICAL);

            // Decrease timer on wall kick
            auto* timerSystem = CORE->GetGameTimerSystem();
            if (timerSystem) {
                timerSystem->DecreaseTime(1.0f);
            }

            std::cout << "[Wall Kick] Initiated! Direction: " << (kickDirectionX > 0 ? "Right" : "Left") << std::endl;
        }
    }
    else if (!wallKickKeyDown) {
        wallKickKeyPressed = false;
    }

    // ===== GROUND POUND SYSTEM =====
    static float GROUND_POUND_PAUSE_TIME = 0.2f;
    static float GROUND_POUND_FALL_SPEED = 800.0f;

    // Update ground pound pause timer
    if (isGroundPounding && groundPoundPauseTimer > 0) {
        groundPoundPauseTimer -= dt;
        // Keep player stationary during pause
        physics->SetVelocity(0.0f, 0.0f);
    }

    // Ground Pound input (S key when in air)
    bool groundPoundKeyDown = (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS);

    if (groundPoundKeyDown && !groundPoundKeyPressed) {
        groundPoundKeyPressed = true;

        // Can ground pound if in air and not already ground pounding
        if (!physics->IsGrounded() && !isGroundPounding) {
            isGroundPounding = true;
            groundPoundPauseTimer = GROUND_POUND_PAUSE_TIME;

            // Cancel other abilities
            isGliding = false;

            // Decrease timer on ground pound
            auto* timerSystem = CORE->GetGameTimerSystem();
            if (timerSystem) {
                timerSystem->DecreaseTime(1.0f);
            }

            std::cout << "[Ground Pound] Initiated!" << std::endl;
        }
    }
    else if (!groundPoundKeyDown) {
        groundPoundKeyPressed = false;
    }

    // Execute ground pound (after pause)
    if (isGroundPounding && groundPoundPauseTimer <= 0) {
        if (!physics->IsGrounded()) {
            // Apply downward velocity
            physics->SetVelocity(0.0f, -GROUND_POUND_FALL_SPEED);
        }
        else {
            // Hit ground - convert to roll
            isGroundPounding = false;
            isRolling = true;
            rollTimer = 0.6f; // Extended roll from ground pound
            std::cout << "[Ground Pound] Landed! Converting to roll" << std::endl;
        }
    }

    // ===== ROLL SYSTEM =====
    static float ROLL_SPEED = 350.0f;
    static float ROLL_DURATION = 0.4f;

    // Update roll timer
    if (isRolling && rollTimer > 0) {
        rollTimer -= dt;

        // Apply roll velocity (maintain direction)
        float rollDirection = isFacingRight ? 1.0f : -1.0f;
        physics->SetVelocity(rollDirection * ROLL_SPEED, physics->velocity.y);

        // Stop rolling when hitting a wall
        if ((touchingLeftWall && rollDirection < 0) || (touchingRightWall && rollDirection > 0)) {
            isRolling = false;
            rollTimer = 0.0f;
            std::cout << "[Roll] Stopped by wall" << std::endl;
        }

        if (rollTimer <= 0) {
            isRolling = false;
            std::cout << "[Roll] Ended" << std::endl;
        }
    }

    // Roll input (K key when grounded)
    bool rollKeyDown = (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS);

    if (rollKeyDown && !rollKeyPressed) {
        rollKeyPressed = true;

        // Can roll if grounded and not already rolling
        if (physics->IsGrounded() && !isRolling) {
            isRolling = true;
            rollTimer = ROLL_DURATION;

            // Decrease timer on roll
            auto* timerSystem = CORE->GetGameTimerSystem();
            if (timerSystem) {
                timerSystem->DecreaseTime(1.0f);
            }

            std::cout << "[Roll] Initiated!" << std::endl;
        }
    }
    else if (!rollKeyDown) {
        rollKeyPressed = false;
    }

    // ===== GLIDE SYSTEM =====
    static float GLIDE_FALL_SPEED = -80.0f; // Target fall speed while gliding (adjust as needed)
    static float GLIDE_COOLDOWN = 2.0f;

    // Update glide cooldown timer
    if (glideCooldownTimer > 0) {
        glideCooldownTimer -= dt;
    }

    // Glide input (W key held when in air)
    bool glideKeyHeld = (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS);

    if (glideKeyHeld && !physics->IsGrounded() && glideCooldownTimer <= 0)
    {
        if (!isGliding) {
            isGliding = true;
            std::cout << "[Glide] Started!" << std::endl;
        }

        // Apply glide effect - clamp fall speed to glide speed
        if (physics->velocity.y < GLIDE_FALL_SPEED) {
            // Gradually slow down to glide speed
            physics->velocity.y += 500.0f * dt; // Adjust deceleration rate as needed
            if (physics->velocity.y > GLIDE_FALL_SPEED) {
                physics->velocity.y = GLIDE_FALL_SPEED;
            }
        }
        else if (physics->velocity.y > GLIDE_FALL_SPEED) {
            // If falling slower than glide speed, set to glide speed
            physics->velocity.y = GLIDE_FALL_SPEED;
        }

        // Decrease timer periodically while gliding
        static float glideTimerAccumulator = 0.0f;
        glideTimerAccumulator += dt;
        if (glideTimerAccumulator >= 0.5f) {
            auto* timerSystem = CORE->GetGameTimerSystem();
            if (timerSystem) {
                timerSystem->DecreaseTime(0.5f);
            }
            glideTimerAccumulator = 0.0f;
        }
    }
    else {
        // Stop gliding
        if (isGliding) {
            isGliding = false;
            glideCooldownTimer = GLIDE_COOLDOWN;
            std::cout << "[Glide] Stopped! Cooldown: " << GLIDE_COOLDOWN << "s" << std::endl;
        }
    }

    // Cancel glide when grounded or dashing
    if (isGliding && (physics->IsGrounded() || isDashing)) {
        isGliding = false;
        glideCooldownTimer = GLIDE_COOLDOWN;
        std::cout << "[Glide] Cancelled!" << std::endl;
    }
    auto* anim = player->GetComponent<Spritesheet>();
        if (transform && anim)
        {
            float absWidth = std::abs(transform->scaleX);
            transform->SetScale(absWidth, transform->scaleY);

        // Update facing direction based on movement
        if (moveDirectionX > 0.0f) isFacingRight = true;
        if (moveDirectionX < 0.0f) isFacingRight = false;

        // Determine Animation Name based on State AND Direction
        // Priority: GroundPound > Dash > Roll > Glide > Jump (in air) > Run > Idle
        std::string requestedState = "";

        if (isGroundPounding)
        {
            requestedState = isFacingRight ? "GroundPound_R" : "GroundPound_L";
        }
        else if (isDashing)
        {
            requestedState = isFacingRight ? "Dash_R" : "Dash_L";
        }
        else if (isRolling)
        {
            requestedState = isFacingRight ? "Roll_R" : "Roll_L";
        }
        else if (isGliding)
        {
            requestedState = isFacingRight ? "Glide_R" : "Glide_L";
        }
        else if (!physics->IsGrounded())  // Player is in the air
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

        // Play the specific Left/Right animation
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

        // Reset all ability states
        isWallKicking = false;
        wallKickTimer = 0.0f;
        wallKickCooldownTimer = 0.0f;
        wallKickKeyPressed = false;

        isGroundPounding = false;
        groundPoundPauseTimer = 0.0f;
        groundPoundKeyPressed = false;

        isRolling = false;
        rollTimer = 0.0f;
        rollKeyPressed = false;

        isGliding = false;
        glideCooldownTimer = 0.0f;
        glideKeyPressed = false;

        isWallSliding = false;

        numberOfJumpsUsed = 0;

        isFacingRight = true;

        std::cout << "[InputManager] State reset (including all abilities)." << std::endl;
    }
