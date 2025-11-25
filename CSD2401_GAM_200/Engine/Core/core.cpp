/* Start Header ************************************************************************/
/*!
\file       Core.cpp
\author     Jonathan Fam Qian Wei [70% - 249 lines]
\co-author  Lee Hwee Min [30% - 45 lines]
\par        Fam.q@digipen.edu , l.hweemin@digipen.edu
\date       October, 2, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "core.h"
#ifdef USE_IMGUI
#include "LevelEditor.h"
#endif
#include "Message.h"
#include "IMessageHandler.h"

bool InputManager::KeyStateQuit = false;
CoreEngine *CORE;

double CoreEngine::fps = 0.0;
double CoreEngine::delta_time = 0.0;
double CoreEngine::fixed_dt = 1.0 / 60.0;
double g_totalLoopTime = 0.0;
std::map<std::string, double> g_systemTimings;
bool g_UseGameStates = false; // TRUE: GameState Mode, FALSE: Engine Mode

/* brief
   Constuctor

   */
CoreEngine::CoreEngine() : WindWidth(0), WindHeight(0), accumulator(0.0), stepMode(false), stepOnce(false)
{
    LastTime = 0;
    GameActive = true;
    CORE = this;
    std::cout << "[CoreEngine::CoreEngine] CORE set to " << this << std::endl;

    std::cout << "CoreEngine constructed" << std::endl;
}
/* brief
   deonstuctor
   */
CoreEngine::~CoreEngine()
{
    DestroySystems();
    std::cout << "CoreEngine destroyed" << std::endl;
}

/* brief
   function loads the config.txt file from the folder  directory and loads the
   window width and height to the graphics system

   */
void CoreEngine::ConfigFile(const char *configF)
{
    std::ifstream fp{configF};
    if (!fp)
    {
        std::cout << "Cannot find file" << std::endl;
        glfwTerminate();
    }

    int WindWid = 0, WindHei = 0;
    std::string temp;
    fp >> temp >> WindWid;
    fp >> temp >> WindHei;
    setWindsize(WindWid, WindHei);
}

/* brief
   This function init the systems and add the systems into the enigne

   */
void CoreEngine::InitSystems()
{
    CoreEngine::ConfigFile("../Logs/config.txt");
    std::cout << "Initializing default systems..." << std::endl;

    // 1. Graphics first (needed for window)
    graphicsSystem = AddSystem<GraphicsSystem>(WindWidth, WindHeight);

    // 2. ImGui (needs graphics window)
    //imguiSystem = AddSystem<ImGuiSystem>(graphicsSystem->getWindow());

    // 3. Input Manager - MUST BE EARLY to capture input before physics updates
    inputManager = AddSystem<InputManager>(graphicsSystem->getWindow(), audioEngine);
    debugSystem = AddSystem<DebugSystem>();

    // 4. GameObject System
    gameObjectSystem = AddSystem<GameObjectSystem>();

    // Transform System
    transformSystem = AddSystem<TransformSystem>();

    // 5. Collision System - detects collisions
    collisionSystem = AddSystem<CollisionSystem>();

    // 6. Physics System - processes collisions and applies physics
    physicsSystem = AddSystem<PhysicsSystem>();

    // 7. Camera System - Controls the cameras
    cameraSystem = AddSystem<CameraSystem>();

    // 8. Level Editor - for editing levels
    //levelEditor = AddSystem<LevelEditor>();

    // Connect CollisionSystem to PhysicsSystem
    collisionSystem->SetPhysicsSystem(physicsSystem);

    // 10. ScriptSystem - C# Script System
    scriptSystem = AddSystem<ScriptSystem>();

    menuLogicSystem = AddSystem<MenuLogicSystem>();

    timerSystem = AddSystem<TimerSystem>();
    eventSystemRunner = AddSystem<EventSystemRunner>();

    // Connect GraphicsSystem to LevelEditor for editor camera
    //graphicsSystem->SetLevelEditor(levelEditor);

    // Subscribe systems to specific message types
    messageBus.Subscribe<PlaySoundMessage>(audioEngine);
    messageBus.Subscribe<PauseSoundMessage>(audioEngine);
    messageBus.Subscribe<CollisionMessage>(physicsSystem);
    messageBus.Subscribe<CreateObjectMessage>(gameObjectSystem);

    std::cout << "Default systems initialized: GameObjectSystem, GraphicsSystem" << std::endl;
}
/* brief
   This function init the engine and add the systems into the enigne and as well as initialze the added systems.

   */

void CoreEngine::Init()
{
    InitSystems();
    if (audioEngine)
    {
        std::cout << "[CoreEngine] Creating AudioManager internally..." << std::endl;
        // We use 'new' here because we store it in a raw pointer member variable.
        // We MUST delete this in DestroySystems() to avoid leaks.
        audioManager = new AudioManager(audioEngine);
    }
    else
    {
        std::cerr << "[CoreEngine] WARNING: AudioEngine not set before Init! AudioManager will be null." << std::endl;
    }

    std::cout << "Initializing CoreEngine with " << Systems.size() << " systems" << std::endl;

    for (size_t i = 0; i < Systems.size(); ++i)
    {
        std::cout << "Initializing system: " << Systems[i]->GetName() << std::endl;
        Systems[i]->Initialize();
    }

    std::cout << "CoreEngine initialization complete" << std::endl;
}

/* brief
   This function starts the core engine loop to start the game engine process
   there is the a exeception error checker too to get error thrown too
   Uses fixed timestep for physics and variable timestep for rendering

   */
void CoreEngine::GameLoop()
{
    double fps_interval = 1.0;
    const double max_frame_time = 0.25; // Cap frame time to prevent spiral of death
    accumulator = 0.0;

    while (!glfwWindowShouldClose(graphicsSystem->getWindow()))
    {

        if (m_hasPendingLoad)
        {
            std::cout << "[CoreEngine] ===== PENDING SCENE LOAD DETECTED =====" << std::endl;
            std::cout << "[CoreEngine] Scene to load: " << m_pendingScene << std::endl;
            std::cout << "[CoreEngine] Auto-play flag: " << (m_autoPlayAfterLoad ? "TRUE" : "FALSE") << std::endl;

            LoadLevelDirectly(m_pendingScene);

            if (audioManager)
            {

                // Check what sounds are available and play the appropriate BGM
                const auto& sounds = audioManager->GetSoundDatabase();

                if (sounds.find("bgm") != sounds.end())
                {
                    // Game scene - play game BGM
                    audioManager->PlaySound("bgm", "bgm_channel");
                }
                else if (sounds.find("MainMenuBGM") != sounds.end())
                {
                    // Main menu scene - play menu BGM
                    audioManager->PlaySound("MainMenuBGM", "menu_bgm_channel");
                }
            }

            //LevelEditor *level = GetLevelEditor();
            //level->LoadLevel(m_pendingScene);
            //std::cout << "[CoreEngine] Scene loaded successfully" << std::endl;

            // ===== AUTO-ENTER PLAY MODE IF REQUESTED =====
            //if (m_autoPlayAfterLoad)
            //{
            //    std::cout << "[CoreEngine] Auto-play flag is TRUE - entering play mode\n";
            //    level->EnterPlayMode();
            //    std::cout << "[CoreEngine] Play mode entered\n";
            //}
            //else
            //{
            //    std::cout << "[CoreEngine] Auto-play flag is FALSE - NOT entering play mode\n";
            //}

            m_pendingScene = "";
            m_hasPendingLoad = false;
            m_autoPlayAfterLoad = false; // Reset flag

            if (inputManager)
                inputManager->ResetState();
            CameraManager::Instance().ResetState();
            accumulator = 0.0;
        }

        // Update variable delta_time and FPS
        update_time(fps_interval);

        // Cap delta_time to avoid spiral of death
        double frame_time = (delta_time > max_frame_time) ? max_frame_time : delta_time;
        accumulator += frame_time;

        g_systemTimings.clear();
        g_totalLoopTime = 0.0;

        if (inputManager)
            g_systemTimings[inputManager->GetName()] = 0.0;
        if (gameObjectSystem)
            g_systemTimings[gameObjectSystem->GetName()] = 0.0;
        if (transformSystem)
            g_systemTimings[transformSystem->GetName()] = 0.0;
        if (collisionSystem)
            g_systemTimings[collisionSystem->GetName()] = 0.0;
        if (physicsSystem)
            g_systemTimings[physicsSystem->GetName()] = 0.0;
        if (graphicsSystem)
            g_systemTimings[graphicsSystem->GetName()] = 0.0;
        if (debugSystem)
            g_systemTimings[debugSystem->GetName()] = 0.0;
        //if (levelEditor)
        //    g_systemTimings[levelEditor->GetName()] = 0.0;
        if (scriptSystem)
            g_systemTimings[scriptSystem->GetName()] = 0.0;
        //if (imguiSystem)
        //    g_systemTimings[imguiSystem->GetName()] = 0.0;
        if (menuLogicSystem)
            g_systemTimings[menuLogicSystem->GetName()] = 0.0;
        if (timerSystem)
            g_systemTimings[menuLogicSystem->GetName()] = 0.0;

        if (eventSystemRunner)
            g_systemTimings[eventSystemRunner->GetName()] = 0.0;

        // Start ImGui frame
        //if (imguiSystem)
        //{
        //    glBindFramebuffer(GL_FRAMEBUFFER, 0);
        //    glViewport(0, 0, graphicsSystem->GetWindowWidth(), graphicsSystem->GetWindowHeight());
        //    glClearColor(0.15f, 0.15f, 0.15f, 1.f);
        //    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //    ImGui_ImplOpenGL3_NewFrame();
        //    ImGui_ImplGlfw_NewFrame();
        //    ImGui::NewFrame();
        //    ImGuizmo::BeginFrame();
        //}

        // FIXED TIMESTEP LOOP - Run physics multiple times if needed
        while (accumulator >= fixed_dt)
        {
            // InputManager ALWAYS updates (even in step mode) to detect F2/F3 keys
            if (inputManager)
            {
                std::chrono::high_resolution_clock::time_point sysStart = std::chrono::high_resolution_clock::now();
                inputManager->Update(static_cast<float>(fixed_dt));
                std::chrono::high_resolution_clock::time_point sysEnd = std::chrono::high_resolution_clock::now();
                double ms = std::chrono::duration<double, std::milli>(sysEnd - sysStart).count();
                g_systemTimings[inputManager->GetName()] += ms;
                g_totalLoopTime += ms;
            }

            //bool isPlaying = (levelEditor && levelEditor->GetEditorMode() == EditorMode::Play);
            bool shouldRunPhysics = ((!stepMode) || stepOnce) && !m_gamePause;
            // Only run physics if NOT in step mode/paused, OR if stepping once
            if (shouldRunPhysics)
            {
                // Update physics-dependent systems with FIXED timestep
                if (gameObjectSystem)
                {
                    std::chrono::high_resolution_clock::time_point sysStart = std::chrono::high_resolution_clock::now();
                    gameObjectSystem->Update(static_cast<float>(fixed_dt));
                    std::chrono::high_resolution_clock::time_point sysEnd = std::chrono::high_resolution_clock::now();
                    double ms = std::chrono::duration<double, std::milli>(sysEnd - sysStart).count();
                    g_systemTimings[gameObjectSystem->GetName()] += ms;
                    g_totalLoopTime += ms;
                }

                if (transformSystem)
                {
                    std::chrono::high_resolution_clock::time_point sysStart = std::chrono::high_resolution_clock::now();
                    transformSystem->Update(static_cast<float>(fixed_dt));
                    std::chrono::high_resolution_clock::time_point sysEnd = std::chrono::high_resolution_clock::now();
                    double ms = std::chrono::duration<double, std::milli>(sysEnd - sysStart).count();
                    g_systemTimings[transformSystem->GetName()] += ms;
                    g_totalLoopTime += ms;
                }

                if (collisionSystem)
                {
                    std::chrono::high_resolution_clock::time_point sysStart = std::chrono::high_resolution_clock::now();
                    collisionSystem->Update(static_cast<float>(fixed_dt));
                    std::chrono::high_resolution_clock::time_point sysEnd = std::chrono::high_resolution_clock::now();
                    double ms = std::chrono::duration<double, std::milli>(sysEnd - sysStart).count();
                    g_systemTimings[collisionSystem->GetName()] += ms;
                    g_totalLoopTime += ms;
                }

                if (physicsSystem)
                {
                    std::chrono::high_resolution_clock::time_point sysStart = std::chrono::high_resolution_clock::now();
                    physicsSystem->Update(static_cast<float>(fixed_dt));
                    std::chrono::high_resolution_clock::time_point sysEnd = std::chrono::high_resolution_clock::now();
                    double ms = std::chrono::duration<double, std::milli>(sysEnd - sysStart).count();
                    g_systemTimings[physicsSystem->GetName()] += ms;
                    g_totalLoopTime += ms;
                }

                if (scriptSystem)
                {
                    std::chrono::high_resolution_clock::time_point sysStart = std::chrono::high_resolution_clock::now();
                    scriptSystem->Update(static_cast<float>(fixed_dt)); // Use fixed timestep for consistency
                    std::chrono::high_resolution_clock::time_point sysEnd = std::chrono::high_resolution_clock::now();
                    double ms = std::chrono::duration<double, std::milli>(sysEnd - sysStart).count();
                    g_systemTimings[scriptSystem->GetName()] = ms;
                    g_totalLoopTime += ms;
                }

                if (menuLogicSystem)
                {
                    std::chrono::high_resolution_clock::time_point sysStart = std::chrono::high_resolution_clock::now();
                    menuLogicSystem->Update(static_cast<float>(fixed_dt)); // Use fixed timestep for consistency
                    std::chrono::high_resolution_clock::time_point sysEnd = std::chrono::high_resolution_clock::now();
                    double ms = std::chrono::duration<double, std::milli>(sysEnd - sysStart).count();
                    g_systemTimings[menuLogicSystem->GetName()] = ms;
                    g_totalLoopTime += ms;
                }
                stepOnce = false; // Reset after processing one frame
            }

            if (eventSystemRunner)
            {
                std::chrono::high_resolution_clock::time_point sysStart = std::chrono::high_resolution_clock::now();
                eventSystemRunner->Update(static_cast<float>(fixed_dt)); // Use fixed timestep for consistency
                std::chrono::high_resolution_clock::time_point sysEnd = std::chrono::high_resolution_clock::now();
                double ms = std::chrono::duration<double, std::milli>(sysEnd - sysStart).count();
                g_systemTimings[eventSystemRunner->GetName()] = ms;
                g_totalLoopTime += ms;
            }

            // In step mode, only process ONE timestep then break
            if (stepMode)
                break;

            accumulator -= fixed_dt;
        }

        // VARIABLE TIMESTEP - Rendering and UI systems
        if (graphicsSystem)
        {
            std::chrono::high_resolution_clock::time_point sysStart = std::chrono::high_resolution_clock::now();
            graphicsSystem->Update(static_cast<float>(delta_time));
            std::chrono::high_resolution_clock::time_point sysEnd = std::chrono::high_resolution_clock::now();
            double ms = std::chrono::duration<double, std::milli>(sysEnd - sysStart).count();
            g_systemTimings[graphicsSystem->GetName()] = ms;
            g_totalLoopTime += ms;
        }

        if (debugSystem)
        {
            std::chrono::high_resolution_clock::time_point sysStart = std::chrono::high_resolution_clock::now();
            debugSystem->Update(static_cast<float>(delta_time));
            std::chrono::high_resolution_clock::time_point sysEnd = std::chrono::high_resolution_clock::now();
            double ms = std::chrono::duration<double, std::milli>(sysEnd - sysStart).count();
            g_systemTimings[debugSystem->GetName()] = ms;
            g_totalLoopTime += ms;
        }

        //if (levelEditor)
        //{
        //    std::chrono::high_resolution_clock::time_point sysStart = std::chrono::high_resolution_clock::now();
        //    levelEditor->Update(static_cast<float>(delta_time));
        //    std::chrono::high_resolution_clock::time_point sysEnd = std::chrono::high_resolution_clock::now();
        //    double ms = std::chrono::duration<double, std::milli>(sysEnd - sysStart).count();
        //    g_systemTimings[levelEditor->GetName()] = ms;
        //    g_totalLoopTime += ms;
        //}

        //if (levelEditor && levelEditor->HasPendingSceneReload())
        //{
        //    levelEditor->PerformSceneReload();
        //}

        if (timerSystem)
        {
            timerSystem->Update(static_cast<float>(delta_time));
        }

        //if (imguiSystem)
        //{
        //    std::chrono::high_resolution_clock::time_point sysStart = std::chrono::high_resolution_clock::now();
        //    imguiSystem->Update(static_cast<float>(delta_time));
        //    std::chrono::high_resolution_clock::time_point sysEnd = std::chrono::high_resolution_clock::now();
        //    double ms = std::chrono::duration<double, std::milli>(sysEnd - sysStart).count();
        //    g_systemTimings[imguiSystem->GetName()] = ms;
        //    g_totalLoopTime += ms;
        //}

        //// End ImGui frame and render
        //if (imguiSystem)
        //{
        //    ImGui::Render();
        //    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        //}

        if (g_UseGameStates)
        {
            GameManager::Instance().GameStateLoop(delta_time);
        }

        // Swap buffers and poll events
        glfwSwapBuffers(graphicsSystem->getWindow());
        glfwPollEvents();
    }
}

/* brief
   This function deletes the engines and stops the engine processes and clears all systems

   */
void CoreEngine::DestroySystems()
{
    std::cout << "Destroying " << Systems.size() << " systems" << std::endl;
    Systems.clear();
    if (audioManager)
    {
        std::cout << "[CoreEngine] Deleting AudioManager..." << std::endl;
        delete audioManager;
        audioManager = nullptr; // Good practice to nullify after delete
    }
}

void CoreEngine::update_time(double fps_calc_interval)
{
    static double prev_time = glfwGetTime();
    double curr_time = glfwGetTime();
    delta_time = curr_time - prev_time; // Now using static delta_time
    prev_time = curr_time;

    // fps calculations
    static double count = 0.0; // number of game loop iterations
    static double start_time = glfwGetTime();
    double elapsed_time = curr_time - start_time;

    ++count;

    if (elapsed_time > fps_calc_interval)
    {
        fps = count / elapsed_time; // Now using static fps
        count = 0.0;
        start_time = curr_time;
    }

    std::stringstream iostream;
    std::string tempfps, tempdt, tempfixeddt;
    std::string title_str;

    iostream << CoreEngine::fps;
    tempfps = iostream.str();

    iostream.str("");
    iostream.clear();

    iostream << CoreEngine::delta_time;
    tempdt = iostream.str();

    iostream.str("");
    iostream.clear();

    iostream << CoreEngine::fixed_dt;
    tempfixeddt = iostream.str();

    title_str = "Game Engine | FPS:" + tempfps + " | DT: " + tempdt + " | Fixed DT: " + tempfixeddt;

    GLFWwindow *Wind = CORE->GetGraphicsSystem()->getWindow();
    if (Wind)
    {
        glfwSetWindowTitle(Wind, title_str.c_str());
    }
}

void CoreEngine::SetGamePaused(bool paused)
{
    std::cout << (m_gamePause ? "Game is paused\n" : "Game is resuming\n");
    if (m_gamePause == paused)
    {
        return;
    }

    m_gamePause = paused;

    if (audioEngine)
    {
        audioEngine->stopAllSound();
    }
}


bool CoreEngine::LoadLevelDirectly(const std::string& filepath)
{
    try
    {
        std::cout << "LOADING LEVEL: " << filepath << "\n";

        // open & parse
        std::ifstream inFile(filepath);
        if (!inFile.is_open())
        {
            std::cerr << "Failed to open file for reading: " << filepath << std::endl;
            return false;
        }

        json root;
        inFile >> root;
        inFile.close();

        if (CORE)
        {
            TimerSystem* ts = CORE->GetTimerSystem();
            if (ts)
            {
                std::cout << "[LoadLevel] Clearing active timers...\n";
                ts->ClearAllTimers();
            }
            PhysicsSystem* ps = CORE->GetPhysicsSystem();
            if (ps)
            {
                ps->ClearCollisionQueue();
            }
        }


        // stop/unload audio (if any)
        auto& resourceMgr = ResourceManager::Instance();
        AudioEngine* audio = resourceMgr.GetAudioEngine();

        if (audio)
        {
            std::cout << "[LoadLevel] Stopping all sounds...\n";
            audio->stopAllSound();
            audio->updateAudio(); // ensure FMOD flushes
            std::cout << "[LoadLevel] All sounds stopped\n";
        }

        std::cout << "[LoadLevel] Unloading audio resources...\n";
        resourceMgr.UnloadAllAudio();

        // clear current level
        GameObjectFactory& factory = GameObjectFactory::Instance();
        factory.Clear();

        // also clear any active camera binding
        CameraManager::Instance().ClearActiveCamera();

        size_t lastSlash = filepath.find_last_of("/\\");
        size_t lastDot = filepath.find_last_of('.');
        std::string filenameOnly = filepath;

        if (lastSlash != std::string::npos)
            filenameOnly = filepath.substr(lastSlash + 1);

        if (lastDot != std::string::npos && lastDot > lastSlash)
            filenameOnly = filenameOnly.substr(0, lastDot - (lastSlash + 1));

        // load audio manager configuration
        if (root.contains("AudioManager") && CORE)
        {
            std::cout << "[LoadLevel] Found AudioManager section in JSON\n";
            AudioManager* audioManager = CORE->GetAudioManager();
            if (audioManager)
            {
                std::cout << "[LoadLevel] AudioManager pointer is valid\n";
                Serialization::DeserializeAudioManager(root, audioManager);
                std::cout << "[LoadLevel] AudioManager deserialization complete\n";
                audioManager->PrintConfig();
            }
            else
            {
                std::cerr << "[LoadLevel] ERROR: AudioManager pointer is NULL!\n";
            }
        }
        else
        {
            if (!root.contains("AudioManager"))
            {
                std::cerr << "[LoadLevel] ERROR: No AudioManager section found in JSON!\n";
            }
            if (!CORE)
            {
                std::cerr << "[LoadLevel] ERROR: CORE is NULL!\n";
            }
        }

        // load all game objects
        if (root.contains("GameObjects") && root["GameObjects"].is_array())
        {
            std::cout << "[LoadLevel] Loading game objects...\n";
            for (const auto& objData : root["GameObjects"])
            {
                Serialization::DeserializeGameObject(objData);
            }
        }
        Serialization::ResolveHierarchy();


        // check if MainCamera exists, create one if not
        GameObject* mainCam = factory.FindGameObject("MainCamera");
        if (!mainCam) {
            std::cout << "No MainCamera found in scene, creating one...\n";

            // create MainCamera
            mainCam = factory.CreateGameObject("MainCamera");
            mainCam->AddComponent<TransformComponent>();
            auto* camComp = mainCam->AddComponent<CameraComponent>();
            camComp->SetZoom(1.0f);
            camComp->SetMode(CameraComponent::CameraMode::FIXED);
            CameraManager::Instance().SetActiveCamera(mainCam);

            std::cout << "MainCamera created successfully\n";
        }
        else {
            // MainCamera exists, but check if it has a CameraComponent
            auto* camComp = mainCam->GetComponent<CameraComponent>();
            if (!camComp) {
                std::cout << "MainCamera exists but missing CameraComponent, adding one...\n";

                if (!mainCam->GetComponent<TransformComponent>()) {
                    mainCam->AddComponent<TransformComponent>();
                }
                camComp = mainCam->AddComponent<CameraComponent>();
                camComp->SetZoom(1.0f);
                camComp->SetMode(CameraComponent::CameraMode::FIXED);
                CameraManager::Instance().SetActiveCamera(mainCam);

                std::cout << "CameraComponent added successfully\n";
            }
            else {
                std::cout << "MainCamera with CameraComponent found in scene\n";
            }
        }

        std::cout << "Level loaded from: " << filepath << "\n";
        std::cout << "Loaded " << factory.GetAllGameObjects().size() << " game objects\n";

        return true;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error loading level: " << e.what() << std::endl;
        return false;
    }
}