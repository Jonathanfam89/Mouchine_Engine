/* Start Header ************************************************************************/
/*!
\file       Core.h
\author     Jonathan Fam Qian Wei [100% - 54 lines]
\par        Fam.q@digipen.edu
\date       October, 2, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include "System.h"
#include "audio.h"
#include <fstream>
#include <iomanip>
#include "GameObjectSystem.h"
#include "GraphicsSystem.h"
#include <GLFW/glfw3.h>
#include <thread>
#include <chrono>
#include <Physics.h>
#ifdef USE_IMGUI
#include "ImGuiSystem.h"
#include "LevelEditor.h"
#endif
#include "InputManager.h"
#include "transformSystem.h"
#include "CollisionSystem.h"
#include "PhysicsSystem.h"
#include "DebugSystem.h"
#include "ScriptingSystem.h"
#include "CameraSystem.h"
#include <map>
#include "MathLib.h"
#include "Serialization.h"
#include "logic.h"
#include "MenuLogic.h"
#include "AudioManager.h"

#include "GameManager.h"
#include "Message.h"
#include "IMessageHandler.h"

#include "TimerSystem.h"

// Inside CoreEngine class

// Forward declarations
class GameObjectSystem;
class GraphicsSystem;
#ifdef USE_IMGUI
class ImGuiSystem;
class LevelEditor;
#endif
class InputManager;
class CollisionSystem;
class PhysicsSystem;
class DebugSystem;
class TransformSystem;
class ScriptSystem;
class MenuLogicSystem;
class AudioManager;
class CoreEngine : public IMessageHandler
{
public:
    /**
     * @brief Constructs the CoreEngine and sets up initial variables.
     */
    CoreEngine();

    /**
     * @brief Destructs the CoreEngine and calls DestroySystems.
     */
    ~CoreEngine();

    /**
     * @brief The main loop that drives the game, including update and render cycles.
     */
    void GameLoop();

    /**
     * @brief Cleans up and destroys all active systems.
     */
    void DestroySystems();

    /**
     * @brief Initializes the CoreEngine and its systems.
     */
    void Init();

    /**
     * @brief Creates, initializes, and adds a new system to the engine.
     * @tparam T The type of the system to add (must derive from ISystem).
     * @tparam Args The types of the constructor arguments.
     * @param args The constructor arguments forwarded to the system's constructor.
     * @return A raw pointer to the newly created system.
     */
    template <typename T, typename... Args>
    T *AddSystem(Args &&...args)
    {
        auto system = std::make_unique<T>(std::forward<Args>(args)...);
        T *systemPtr = system.get();
        Systems.push_back(std::move(system));
        return systemPtr;
    }

    /**
     * @brief Gets a pointer to the GameObjectSystem.
     * @return A pointer to the GameObjectSystem.
     */
    GameObjectSystem *GetGameObjectSystem() const
    {
        return gameObjectSystem;
    }

    /**
     * @brief Gets a pointer to the GraphicsSystem.
     * @return A pointer to the GraphicsSystem.
     */
    GraphicsSystem *GetGraphicsSystem() const
    {
        return graphicsSystem;
    }

    /**
     * @brief Gets a pointer to the ImGuiSystem (for debugging/tools).
     * @return A pointer to the ImGuiSystem.
     */
#ifdef USE_IMGUI
    ImGuiSystem *GetImguiSystem() const
    {
        return imguiSystem;
    }
#endif

    /**
     * @brief Gets a pointer to the InputManager.
     * @return A pointer to the InputManager.
     */
    InputManager *GetInputManager() const
    {
        return inputManager;
    }

    /**
     * @brief Gets a pointer to the CollisionSystem.
     * @return A pointer to the CollisionSystem.
     */
    CollisionSystem *GetCollisionSystem() const
    {
        return collisionSystem;
    }

    /**
     * @brief Gets a pointer to the PhysicsSystem.
     * @return A pointer to the PhysicsSystem.
     */
    PhysicsSystem *GetPhysicsSystem() const
    {
        return physicsSystem;
    }

    /**
     * @brief Gets a pointer to the DebugSystem.
     * @return A pointer to the DebugSystem.
     */
    DebugSystem *GetDebugSystem() const
    {
        return debugSystem;
    }

    /**
     * @brief Gets a pointer to the TransformSystem.
     * @return A pointer to the TransformSystem.
     */
    TransformSystem *GetTransformSystem() const
    {

        return transformSystem;
    }

    /**
     * @brief Gets a pointer to the ScriptSystem.
     * @return A pointer to the ScriptSystem.
     */
    ScriptSystem *GetScriptSystem() const
    {
        return scriptSystem;
    }
    /**
     * @brief Gets a pointer to the CameraSystem.
     * @return A pointer to the CameraSystem.
     */
    CameraSystem *GetCameraSystem() const
    {

        return cameraSystem;
    }

    /**
     * @brief Gets a pointer to the LevelEditor.
     * @return A pointer to the LevelEditor.
     */
#ifdef USE_IMGUI
    LevelEditor *GetLevelEditor() const
    {
        return levelEditor;
    }
#endif
    /**
     * @brief Gets a pointer to the MenuLogicSystem.
     * @return A pointer to the MenuLogicSystem.
     */
    MenuLogicSystem *GetMenuLogic() const
    {
        return menuLogicSystem;
    }

    /**
     * @brief Gets a pointer to the AudioEngine.
     * @return A pointer to the AudioEngine.
     */
    AudioEngine *GetAudioEngine() const { return audioEngine; }
    /**
     * @brief Sets the pointer to the AudioEngine.
     * @param audio A pointer to the external AudioEngine instance.
     */
    void SetAudioEngine(AudioEngine *audio) { audioEngine = audio; }

    // audio manager stuff
    AudioManager *GetAudioManager() const { return audioManager; }

    TimerSystem *GetTimerSystem() const { return timerSystem; }

    /**
     * @brief Reads and applies configuration settings from a file.
     * @param configF The file path to the configuration file.
     */
    void ConfigFile(const char *configF);

    /**
     * @brief Sets the dimensions of the game window.
     * @param w The desired width of the window.
     * @param h The desired height of the window.
     */
    void setWindsize(int w, int h)
    {
        WindWidth = w;
        WindHeight = h;
    };

    /**
     * @brief Gets the current Frames Per Second (FPS).
     * @return The calculated FPS as a double.
     */
    static double GetFPS() { return fps; }

    /**
     * @brief Gets the time elapsed since the last frame (delta time).
     * @return The delta time as a double.
     */
    static double GetDeltaTime() { return delta_time; }

    /**
     * @brief Gets the fixed delta time used for physics updates.
     * @return The fixed delta time as a double.
     */
    static double GetFixedDeltaTime() { return fixed_dt; }

    /**
     * @brief Sets the fixed delta time (e.g., 1/60 for 60 updates per second).
     * @param dt The desired fixed delta time.
     */
    static void SetFixedDeltaTime(double dt) { fixed_dt = dt; }

    /**
     * @brief Requests a scene to be loaded on the next frame.
     * @param sceneName The name of the scene file to load.
     */
    void RequestSceneLoad(const std::string &sceneName, bool autoPlay = true)
    {
        m_pendingScene = sceneName;
        m_hasPendingLoad = true;
        m_autoPlayAfterLoad = autoPlay;
    }

    // Step-by-step physics debugging
    bool stepMode;
    bool stepOnce;
    double accumulator; // Accumulator for fixed timestep

    /**
     * @brief Gets a reference to the MessageBus for inter-system communication.
     * @return A reference to the MessageBus.
     */
    MessageBus &GetMessageBus() { return messageBus; }

    void SetGamePaused(bool paused);

    bool IsGamePaused() const { return m_gamePause; }

    bool LoadLevelDirectly(const std::string& filepath);

private:
    /**
     * @brief Initializes all engine systems in the correct order.
     */
    void InitSystems();
    std::vector<std::unique_ptr<ISystem>> Systems;

    GameObjectSystem *gameObjectSystem = nullptr;
    GraphicsSystem *graphicsSystem = nullptr;
#ifdef USE_IMGUI
    ImGuiSystem *imguiSystem = nullptr;
    LevelEditor *levelEditor = nullptr;
#endif
    InputManager *inputManager = nullptr;
    CollisionSystem *collisionSystem = nullptr;
    PhysicsSystem *physicsSystem = nullptr;
    DebugSystem *debugSystem = nullptr;
    AudioEngine *audioEngine = nullptr;
    TransformSystem *transformSystem = nullptr;
    ScriptSystem *scriptSystem = nullptr;
    CameraSystem *cameraSystem = nullptr;
    MenuLogicSystem *menuLogicSystem = nullptr;
    AudioManager *audioManager = nullptr;
    TimerSystem *timerSystem = nullptr;

    EventSystemRunner *eventSystemRunner = nullptr;

    unsigned LastTime;
    bool GameActive;
    static double fps;
    static double delta_time; // Variable delta time
    static double fixed_dt;   // Fixed delta time for physics
    int WindWidth;
    int WindHeight;
    MessageBus messageBus;
    std::string m_pendingScene = "";
    bool m_hasPendingLoad = false;
    bool m_autoPlayAfterLoad = false;
    bool m_gamePause = false;

    /**
     * @brief Updates the FPS and Delta Time counters.
     * @param fps_calc_interval The interval in seconds to recalculate FPS (defaults to 1.0).
     */
    void update_time(double fps_calc_interval = 1.0);
};

extern CoreEngine *CORE;