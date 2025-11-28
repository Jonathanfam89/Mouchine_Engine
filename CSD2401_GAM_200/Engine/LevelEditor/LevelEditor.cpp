/* Start Header ************************************************************************/
/*!
\file       LevelEditor.cpp
\author     Leck Xin Yee [100% - 1062 lines]
\par        xinyee.leck@digipen.edu
\date       November, 7, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#ifdef USE_IMGUI
#include "core.h"
#include "ColliderComponent.h"
#include "LevelEditor.h"
#include "physics.h"
#include "ResourceManager.h"
#include "Serialization.h"
#include "audio.h"
#include "AudioManager.h"
#include "TimerSystem.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace fs = std::filesystem;

/**
 * @brief Constructor - initialise level editor
 */
LevelEditor::LevelEditor()
    : factory(GameObjectFactory::Instance()),
    selectedGameObjectID(0),
    savedEntityIDCounter(0),
    showSaveDialog(false),
    showLoadDialog(false),
    currentFilename(""),
    editorMode(EditorMode::Play),
    hasUnsavedChanges(false),
    hasValidSaveState(false),
    cameraX(0.0f), cameraY(0.0f), cameraZoom(1.0f),
    isPanning(false), lastMouseX(0.0f), lastMouseY(0.0f), panSpeed(1.0f),
    zoomSpeed(0.1f),
    minZoom(0.1f),
    maxZoom(5.0f),
    zoomSmoothing(0.2f)
{
    strcpy(saveFilepath, "");
    strcpy(loadFilepath, "../Assets/Scenes/Prefab_TestScene.json");
}

/**
 * @brief Destructor
 */
LevelEditor::~LevelEditor()
{
}

/**
 * @brief Initialise the level editor system
 */
void LevelEditor::Initialize()
{
    std::cout << "Level editor initialised" << std::endl;

    currentFilename = "start level";
    editorMode = EditorMode::Stop;
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable docking
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

    //icon_plus_ui = GraphicsSystem::LoadTexture(icon_plus);
}

/**
 * @brief Update - Render all editor GUI panels
 */
void LevelEditor::Update(float /*dt*/)
{
    if (!InputManager::KeyStateImgui)
        return;


    // Full window docking
    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
                                    ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                                    ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
                                    ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("Editor Dock", nullptr, window_flags);
    ImGui::PopStyleVar(3);

    ImGuiID dockspace_id = ImGui::GetID("DockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
    ImGui::End();

    // camera update with conditional rendering
    bool shouldRenderCameraUI = (editorMode == EditorMode::Stop || editorMode == EditorMode::Pause);
    UpdateCameraControls(shouldRenderCameraUI);

    RenderPlayControls();

    // show editor UI only when not in live Play
    if (editorMode != EditorMode::Play) {
        RenderMenuBar();
    }

    if (showLoadDialog)
    {
        ImGui::OpenPopup("Load Level##LevelEditor");
        if (ImGui::BeginPopupModal("Load Level##LevelEditor", &showLoadDialog, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Load level from file:");
            ImGui::InputText("Filepath", loadFilepath, sizeof(loadFilepath));

            if (ImGui::Button("Load", ImVec2(120, 0)))
            {
                if (LoadLevel(loadFilepath))
                {
                    std::cout << "Level loaded successfully from: " << loadFilepath << std::endl;
                    showLoadDialog = false;
                }
                else
                {
                    std::cerr << "Failed to load level!" << std::endl;
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0)))
            {
                showLoadDialog = false;
            }

            ImGui::EndPopup();
        }
    }
}

/**
 * @brief Render play/pause/stop control panel
 */
void LevelEditor::RenderPlayControls()
{
    //ImGui::SetNextWindowSize(ImVec2(250, 200), ImGuiCond_Once);
    ImGui::Begin("Editor Controls", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x / 2.f);
    ImGui::BeginGroup();
    

    const bool isRunning = (editorMode == EditorMode::Play || editorMode == EditorMode::Pause);
    const char *mainLabel = isRunning ? "Stop" : "Play";
    if (ImGui::Button(mainLabel))
    {
        TogglePlayStop();
    }

    // pause / resume
    ImGui::SameLine();
    const bool canPause = isRunning;
    const char *pauseLabel = (editorMode == EditorMode::Pause) ? "Resume" : "Pause";
    ImGui::BeginDisabled(!canPause);
    if (ImGui::Button(pauseLabel))
    {
        TogglePause();
    }
    ImGui::EndDisabled();

    // status text
    ImGui::SameLine();
    ImGui::TextDisabled(" [Mode: %s",
                        editorMode == EditorMode::Play ? "Play" : editorMode == EditorMode::Pause ? "Pause]" : "Stop]");

    if (!isRunning)
    {
        ImGui::TextDisabled("Editing scene (camera active).");
    }
    else
    {
        ImGui::TextDisabled("Game running, use Pause/Stop.");
    }

    ImGui::EndGroup();
    ImGui::End();
}

/**
 * @brief Enter play mode - Save current state and start game
 */
void LevelEditor::EnterPlayMode()
{
    if (editorMode == EditorMode::Play)
        return;

    if (!hasValidSaveState)
    {
        SaveSceneState();
        hasValidSaveState = true;
        std::cout << "[LevelEditor] Saved original scene state.\n";
    }
    else
    {
        std::cout << "[LevelEditor] Using existing saved state (not overwriting).\n";
    }

    // setting up game camera to follow hero
    GameObject* cameraObj = factory.FindGameObject("MainCamera");
    if (cameraObj)
    {
        auto* camComponent = cameraObj->GetComponent<CameraComponent>();
        if (camComponent)
        {
            // set camera to follow the hero
            GameObject* hero = factory.FindGameObject("Hero");
            if (hero)
            {
                auto* heroTransform = hero->GetComponent<TransformComponent>();
                if (heroTransform)
                {
                    float hx, hy;
                    heroTransform->GetPosition(hx, hy);
                    camComponent->SetPosition(hx, hy);
                    std::cout << "[LevelEditor] Camera position set to: (" << hx << ", " << hy << ")\n";
                }

                // now set up follow mode
                camComponent->SetMode(CameraComponent::CameraMode::FOLLOW_TARGET);
                camComponent->SetFollowTarget(hero);
                camComponent->SetSmoothing(true, 5.0f);
                std::cout << "[LevelEditor] Camera set to follow hero.\n";
            }

            // set this as the active camera
            CameraManager::Instance().SetActiveCamera(cameraObj);
            std::cout << "[LevelEditor] Active camera set.\n";
        }
    }

    editorMode = EditorMode::Play;

    // start bgm when play is click
    AudioManager* audioManager = CORE ? CORE->GetAudioManager() : nullptr;
    if (audioManager)
    {
        std::cout << "[LevelEditor] Play button clicked - starting BGM\n";

        // Check what sounds are available and play the appropriate BGM
        const auto& sounds = audioManager->GetSoundDatabase();

        if (sounds.find("bgm") != sounds.end())
        {
            // Game scene - play game BGM
            std::cout << "[LevelEditor] Playing game BGM\n";
            audioManager->PlaySound("bgm", "bgm_channel");
        }
        else if (sounds.find("MainMenuBGM") != sounds.end())
        {
            // Main menu scene - play menu BGM
            std::cout << "[LevelEditor] Playing main menu BGM\n";
            audioManager->PlaySound("MainMenuBGM", "menu_bgm_channel");
        }
        else
        {
            std::cout << "[LevelEditor] No BGM found in AudioManager\n";
        }
    }

    std::cout << "[LevelEditor] Entered Play Mode.\n";
}

/**
 * @brief Stop play mode - Restore saved scene state
 */
void LevelEditor::StopPlayMode()
{
    if (editorMode == EditorMode::Stop)
        return;

    // audio manager to stop souund
    AudioManager* audioManager = CORE ? CORE->GetAudioManager() : nullptr;
    if (audioManager)
    {
        std::cout << "[LevelEditor] Stopping background music through AudioManager\n";
        audioManager->StopAllSounds();
    }

    if (CORE)
    {
        TimerSystem* ts = CORE->GetTimerSystem();
        if (ts)
        {
            std::cout << "[LevelEditor] Clearing all active timers.\n";
            ts->ClearAllTimers();
        }
    }

    RestoreSceneState();           // restore saved scene
    hasValidSaveState = false;
    editorMode = EditorMode::Stop; // static camera
    selectedGameObjectID = 0;      // clear selection
    //CameraManager::Instance().ClearActiveCamera();                                // COMMENTED OUT 18TH NOV BC OF BUG W PLAYING -> STOPPING -> PLAYING
    std::cout << "[LevelEditor] Returned to Edit Mode.\n";
}

/**
 * @brief Toggle pause state (Play and Pause)
 */
void LevelEditor::TogglePause()
{
    AudioManager* audioManager = CORE ? CORE->GetAudioManager() : nullptr;

    if (editorMode == EditorMode::Play)
    {
        editorMode = EditorMode::Pause;

        if (audioManager)
        {
            std::cout << "[LevelEditor] Pausing music" << std::endl;
            // stop the sound, to implement proper pause in audio manager
            audioManager->StopSound("bgm_channel");
        }
        std::cout << "[LevelEditor] Game paused." << std::endl;
    }
    else if (editorMode == EditorMode::Pause)
    {
        editorMode = EditorMode::Play;

        if (audioManager)
        {
            std::cout << "[LevelEditor] Resuming music" << std::endl;
            audioManager->PlaySound("bgm", "bgm_channel");
        }

        if (CORE)
        {
            CORE->GetGraphicsSystem()->SetEditorMode(false);
        }
        std::cout << "[LevelEditor] Game resumed." << std::endl;
    }
}

/**
 * @brief Toggle between play and stop
 */
void LevelEditor::TogglePlayStop()
{
    if (editorMode == EditorMode::Play || editorMode == EditorMode::Pause)
    {
        // currently playing, stop & restore
        StopPlayMode();
    }
    else
    {
        // currently stopped, save & enter play
        EnterPlayMode();
        CORE->GetGraphicsSystem()->SetEditorMode(false);
    }
}

/*  Scene State Management  */
/**
 * @brief Save current scene state for later restoration
 */
void LevelEditor::SaveSceneState()
{
    savedSceneState.gameObjects.clear();

    // serialise all current game objects
    for (auto &goPtr : this->factory.GetAllGameObjects())
     {
        GameObject *go = goPtr.get();
        GameObjectData data = SerialiseGameObject(go);
        savedSceneState.gameObjects.push_back(SerialiseGameObject(go));

        // debug check if Hero position is being saved
        if (go->GetName() == "Hero" && data.hasTransform)
        {
            std::cout << "[DEBUG] Saving Hero - Position: (" << data.posX << ", " << data.posY << ")\n";
        }
    }

    // store the current entity counter
    savedEntityIDCounter = this->factory.GetNextEntityID();
    hasValidSaveState = true;

    std::cout << "[LevelEditor] Restored camera position: (" << cameraX << ", " << cameraY << ")\n";
    std::cout << "[LevelEditor] Scene state saved with " << savedSceneState.gameObjects.size() << " objects.\n";
}

/**
 * @brief Restore scene state from saved data
 */
void LevelEditor::RestoreSceneState()
{
    if (!hasValidSaveState)
    {
        std::cout << "[LevelEditor] Warning: No valid save state to restore!\n";
        return;
    }

    std::cout << "[LevelEditor] Restoring scene state    (transform & physics only)...\n";


    std::cout << "[LevelEditor] Saved objects: " << savedSceneState.gameObjects.size()
        << ", Current objects: " << this->factory.GetAllGameObjects().size() << "\n";

    // create a map of saved data by original ID for quick lookup
    std::unordered_map<EntityID, const GameObjectData*> savedDataMap;
    //for (auto &data : sceneDataCopy)
    for (const auto& data : savedSceneState.gameObjects)
    {
        savedDataMap[data.originalID] = &data;
    }

    // restore each object's state without destroying/recreating
    for (const auto& objPtr : this->factory.GetAllGameObjects())
    {
        GameObject* obj = objPtr.get();
        EntityID objID = obj->GetID();

        // find saved data for this object
        auto it = savedDataMap.find(objID);
        if (it == savedDataMap.end())
        {
            std::cout << "[DEBUG] No saved data for object: " << obj->GetName()
                << " (ID: " << objID << "), skipping\n";
            continue;
        }

        const GameObjectData* savedData = it->second;

        // restore transform
        if (savedData->hasTransform)
        {
            if (auto* transform = obj->GetComponent<TransformComponent>())
            {
                transform->SetPosition(savedData->posX, savedData->posY);
                transform->SetScale(savedData->scaleX, savedData->scaleY);
                transform->SetRotation(savedData->rotation);
            }
        }

        // restore physics (reset velocities to saved state)
        if (savedData->hasPhysics)
        {
            if (auto* physics = obj->GetComponent<PhysicsComponent>())
            {
                physics->SetVelocity(savedData->velocityX, savedData->velocityY);
                physics->SetStatic(savedData->isStatic);
                physics->SetGravity(savedData->hasGravity);
                physics->SetGrounded(false); // reset grounded state
            }
        }

        // restore active state
        obj->SetActive(savedData->isActive);
    }

    GameObject* hero = factory.FindGameObject("Hero");
    if (hero != nullptr)
    {
        if (auto* transform = hero->GetComponent<TransformComponent>())
        {
            float heroX, heroY;
            transform->GetPosition(heroX, heroY);

            // debug check restored position
            std::cout << "[DEBUG] Hero restored to: (" << heroX << ", " << heroY << ")\n";

            cameraX = heroX;
            cameraY = heroY;
            std::cout << "[LevelEditor] Camera reset to hero position: (" << cameraX << ", " << cameraY << ")\n";
        }
    }

    std::cout << "[LevelEditor] Scene restored. Camera: (" << cameraX << ", " << cameraY << ")\n";
}

/*  GUI rendering  */
/**
 * @brief Render the editor menu windows
 */
void LevelEditor::RenderMenuBar()
{
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(250, 350), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("Editor Menu##LevelEditor", nullptr, ImGuiWindowFlags_NoCollapse))
    {
        ImGui::End();
        return;
    }

    // new lvl
    if (ImGui::Button("New Level", ImVec2(200, 0)))
    {
        NewLevel();
    }

    ImGui::Separator();

    // save lvl
    ImGui::Text("Save Level:");
    ImGui::InputText("Filename", saveFilepath, sizeof(saveFilepath));
    ImGui::TextDisabled("Will save to: ../Assets/Scenes/%s.json", saveFilepath);

    if (ImGui::Button("Save", ImVec2(200, 0)))
    {
        std::string filepath = saveFilepath;

        if (filepath.find("../Assets/Scenes/") != 0)
        {
            filepath = "../Assets/Scenes/" + filepath;
        }

        if (filepath.length() < 5 || filepath.substr(filepath.length() - 5) != ".json")
        {
            filepath += ".json";
        }

        if (SaveLevel(filepath))
        {
            std::cout << "Level saved successfully to: " << filepath << std::endl;
        }
        else
        {
            std::cerr << "Failed to save level!" << std::endl;
        }
    }

    ImGui::Separator();

    ImGui::Text("Load Scene:");
    std::vector<std::string> sceneFiles = GetAllSceneFiles();
    if (sceneFiles.empty())
    {
        ImGui::TextDisabled("(No scene files found)");
    }
    else
    {
        int sceneIndex = 0;
        for (const auto &file : sceneFiles)
        {
            ImGui::PushID(("scene_" + std::to_string(sceneIndex)).c_str());
            if (ImGui::Button(file.c_str(), ImVec2(200, 0)))
            {
                CORE->GetImguiSystem()->ResetSelectedGameObject();
                pendingSceneReload = true;
                pendingScenePath = "../Assets/Scenes/" + file;
                std::cout << "[LevelEditor] Scheduled scene reload: " << pendingScenePath << "\n";
            }
            ImGui::PopID();
            sceneIndex++;
        }
    }

    ImGui::End();
}

/*  File I/O in JSON  */
/**
 * @brief Save level to JSON file
 */
bool LevelEditor::SaveLevel(const std::string &filepath)
{
    try
    {
        // filename from path without scenes or .jsn
        size_t lastSlash = filepath.find_last_of("/\\");
        size_t lastDot = filepath.find_last_of(".");
        std::string filenameOnly = filepath;

        if (lastSlash != std::string::npos)
            filenameOnly = filepath.substr(lastSlash + 1);

        if (lastDot != std::string::npos && lastDot > lastSlash)
            filenameOnly = filenameOnly.substr(0, lastDot - (lastSlash + 1));

        currentFilename = filenameOnly;

        json root;
        root["GameObjects"] = json::array();

        //// serialise all active game objects
        //auto &allObjects = factory.GetAllGameObjects();
        //for (const auto &objPtr : allObjects)
        //{
        //    if (objPtr && objPtr->IsActive())
        //    {
        //        json objData = Serialization::SerializeGameObject(objPtr.get());
        //        root["GameObjects"].push_back(objData);
        //    }
        //}
        // Collect only ROOT objects
        std::vector<GameObject*> roots;
        auto& allObjects = factory.GetAllGameObjects();

        for (const auto& objPtr : allObjects)
        {
            if (!objPtr || !objPtr->IsActive())
                continue;

            GameObject* obj = objPtr.get();
            if (obj->GetParent() == nullptr)   // <-- ROOT OBJECT ONLY
                roots.push_back(obj);
        }

        // Recursively serialize ONLY the root trees 
        for (GameObject* rootObj : roots)
        {
            json objData = Serialization::SerializeGameObject(rootObj);
            root["GameObjects"].push_back(objData);
        }

        if (CORE)
        {
            AudioManager* audioMgr = CORE->GetAudioManager();
            if (audioMgr)
            {
                root["AudioManager"] = Serialization::SerializeAudioManager(audioMgr);
            }
            else
            {
                std::cerr << "[SaveLevel] WARNING: AudioManager is NULL" << std::endl;
            }
        }
        else
        {
            std::cerr << "[SaveLevel] WARNING: CORE is NULL" << std::endl;
        }
        
        // write to file
        std::ofstream outFile(filepath);
        if (!outFile.is_open())
        {
            std::cerr << "Failed to open file for writing: " << filepath << std::endl;
            return false;
        }

        outFile << root.dump(2);
        outFile.close();

        std::cout << "Level saved to: " << filepath << std::endl;
        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error saving level: " << e.what() << std::endl;
        return false;
    }
}

/**
 * @brief Load level from JSON file
 */
bool LevelEditor::LoadLevel(const std::string& filepath)
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
        factory.Clear();
        selectedGameObjectID = 0;
        ClearUndoStack();

        // reset editor mode to stop when loading a new scene
        editorMode = EditorMode::Stop;
        hasValidSaveState = false; 

        // also clear any active camera binding
        CameraManager::Instance().ClearActiveCamera();

        size_t lastSlash = filepath.find_last_of("/\\");
        size_t lastDot = filepath.find_last_of('.');
        std::string filenameOnly = filepath;

        if (lastSlash != std::string::npos)
            filenameOnly = filepath.substr(lastSlash + 1);

        if (lastDot != std::string::npos && lastDot > lastSlash)
            filenameOnly = filenameOnly.substr(0, lastDot - (lastSlash + 1));

        currentFilename = filenameOnly;

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
            std::cout << "[LevelEditor] No MainCamera found in scene, creating one...\n";

            // create MainCamera
            mainCam = factory.CreateGameObject("MainCamera");
            mainCam->AddComponent<TransformComponent>();
            auto* camComp = mainCam->AddComponent<CameraComponent>();
            camComp->SetZoom(1.0f);
            camComp->SetMode(CameraComponent::CameraMode::FIXED);
            CameraManager::Instance().SetActiveCamera(mainCam);

            std::cout << "[LevelEditor] MainCamera created successfully\n";
        }
        else {
            // MainCamera exists, but check if it has a CameraComponent
            auto* camComp = mainCam->GetComponent<CameraComponent>();
            if (!camComp) {
                std::cout << "[LevelEditor] MainCamera exists but missing CameraComponent, adding one...\n";

                if (!mainCam->GetComponent<TransformComponent>()) {
                    mainCam->AddComponent<TransformComponent>();
                }
                camComp = mainCam->AddComponent<CameraComponent>();
                camComp->SetZoom(1.0f);
                camComp->SetMode(CameraComponent::CameraMode::FIXED);
                CameraManager::Instance().SetActiveCamera(mainCam);

                std::cout << "[LevelEditor] CameraComponent added successfully\n";
            }
            else {
                std::cout << "[LevelEditor] MainCamera with CameraComponent found in scene\n";
            }
        }

        std::cout << "Level loaded from: " << filepath << "\n";
        std::cout << "Loaded " << factory.GetAllGameObjects().size() << " game objects\n";

        // any fresh load invalidates the previous save-state snapshot
        hasValidSaveState = false;
        return true;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error loading level: " << e.what() << std::endl;
        return false;
    }
}

/**
 * @brief Get all scene files from the Assets/Scenes directory
 */
std::vector<std::string> LevelEditor::GetAllSceneFiles()
{
    std::vector<std::string> files;

    try
    {
        if (fs::exists(SceneFilePath) && fs::is_directory(SceneFilePath))
        {
            for (const auto &entry : fs::directory_iterator(SceneFilePath))
            {
                if (entry.is_regular_file())
                {
                    std::string filename = entry.path().filename().string();
                    if (filename.ends_with(".json"))
                    {
                        files.push_back(filename);
                    }
                }
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error reading scene directory: " << e.what() << std::endl;
    }

    return files;
}

/*  Level Management  */
/**
 * @brief Create a new empty level
 */
void LevelEditor::NewLevel()
{
    currentLevel = LevelData();
    currentFilename = "Untitled";

    factory.Clear();
    selectedGameObjectID = 0;
    hasValidSaveState = false;
    CameraManager::Instance().ClearActiveCamera();

    // stop and unload all sounds from the underlying audi engine
    AudioEngine* audioEngine = ResourceManager::Instance().GetAudioEngine();
    if (audioEngine)
    {
        audioEngine->stopAllSound();
        audioEngine->unloadAllSound();
    }

    // clear audio resources from resource manager
    ResourceManager::Instance().UnloadAllAudio();

    // clear the editor audio manager database (all added sounds)
    if (CORE)
    {
        if (AudioManager* audioMgr = CORE->GetAudioManager())
        {
            audioMgr->Clear();
        }
    }

    { // Just to make sure gizmo work on new level, not sure needed or not
        GameObject* mainCam = factory.CreateGameObject("MainCamera");

        // Add Transform
        auto* t = mainCam->AddComponent<TransformComponent>();
        t->SetPosition(0.f, 0.f);
        t->SetScale(1.f, 1.f);

        // Add CameraComponent
        auto* camComp = mainCam->AddComponent<CameraComponent>();
        camComp->SetZoom(1.0f);
        camComp->SetMode(CameraComponent::CameraMode::FIXED);

        // Register as active camera
        CameraManager::Instance().SetActiveCamera(mainCam);

        std::cout << "[NewLevel] MainCamera created and set as active.\n";
    }

    ClearUndoStack();
    std::cout << "New level created" << std::endl;
}

/*  Game object management  */
/**
 * @brief Add a game object to the level
 */
GameObject *LevelEditor::AddGameObject(const GameObjectData &data)
{
    return DeserialiseGameObject(data);
}

/**
 * @brief Remove a game object from the level
 */
void LevelEditor::RemoveGameObject(EntityID id)
{
    factory.DestroyGameObject(id);
}

/**
 * @brief Get the currently selected game object
 */
GameObject *LevelEditor::GetSelectedGameObject() const
{
    return factory.GetGameObject(selectedGameObjectID);
}

/**
 * @brief Set the currently selected game object
 */
void LevelEditor::SetSelectedGameObject(EntityID id)
{
    selectedGameObjectID = id;
}

/*  Serialisation */
/**
 * @brief Serialise a game object to data structure
 */
GameObjectData LevelEditor::SerialiseGameObject(GameObject *obj)
{
    GameObjectData data;

    if (!obj)
    {
        return data;
    }

    data.name = obj->GetName();
    data.originalID = obj->GetID();
    data.isActive = obj->IsActive();

    // serialise transform
    if (auto *transform = obj->GetComponent<TransformComponent>())
    {
        data.hasTransform = true;
        data.posX = transform->posX;
        data.posY = transform->posY;
        data.scaleX = transform->scaleX;
        data.scaleY = transform->scaleY;
        data.rotation = transform->GetRotation();
    }

    // serialise render
    if (auto *render = obj->GetComponent<RenderComponent>())
    {
        data.hasRender = true;
        data.shapeType = (int)render->GetShapeType();

        std::string texPath = render->GetTexturePath();
        if (!texPath.empty())
        {
            data.hasTexture = true;
            data.texturePath = texPath;
        }
        else
        {
            // fallback: infer from texture ID
            if (render->GetTextureID() != 0)
            {
                data.hasTexture = true;
                data.texturePath = texPath;
            }
            else
            {
                data.hasTexture = false;
                data.texturePath = "";
            }
        }

        vec4 color = render->GetColor();
        data.colorR = color.x;
        data.colorG = color.y;
        data.colorB = color.z;
        data.colorA = color.w;
    }

    // serialise physics
    if (auto *physics = obj->GetComponent<PhysicsComponent>())
    {
        data.hasPhysics = true;
        data.mass = physics->GetMass();
        data.velocityX = physics->velocity.x;
        data.velocityY = physics->velocity.y;
        data.isStatic = physics->IsStatic();
        data.hasGravity = physics->useGravity;
    }

    // serialise collider
    if (auto *collider = obj->GetComponent<ColliderComponent>())
    {
        data.hasCollider = true;
        data.colliderType = (int)collider->GetType();

        if (auto *transform = obj->GetComponent<TransformComponent>())
        {
            data.colliderWidth = transform->scaleX;
            data.colliderHeight = transform->scaleY;
        }

        data.collisionLayer = (int)collider->GetLayer();
    }

    // serialise animation / spritesheet
    if (auto *spritesheet = obj->GetComponent<Spritesheet>())
    {
        data.hasAnimation = true;
        data.numRows = spritesheet->GetRows();
        data.numCols = spritesheet->GetCols();
        data.startFrame = spritesheet->GetCurrentStartFrame();
        data.endFrame = spritesheet->GetCurrentEndFrame();
        data.fps = (spritesheet->GetTimePerFrame() > 0.0f) ? (1.0f / spritesheet->GetTimePerFrame()) : 1.0f;

        const char *spritePath = spritesheet->GetSpriteFile();
        if (spritePath != nullptr)
        {
            data.spritesheetPath = spritePath;
        }

        if (auto *render = obj->GetComponent<RenderComponent>())
        {
            std::string rcPath = render->GetTexturePath();
            if (!rcPath.empty())
            {
                data.spritesheetPath = rcPath;
            }
        }

        if (!data.spritesheetPath.empty())
        {
            data.hasTexture = true;
            data.texturePath = data.spritesheetPath;
        }

        std::cout << "[LevelEditor] Serialized animation for "
                  << data.name << ": " << data.spritesheetPath
                  << " (" << data.numRows << "x" << data.numCols << ")"
                  << std::endl;
    }
    else
    {
        data.hasAnimation = false;
    }

    return data;
}

/**
 * @brief Deserialise game object data and create instance
 */
GameObject *LevelEditor::DeserialiseGameObject(const GameObjectData &data)
{
    // create base game object with same id
    GameObject *obj = factory.CreateGameObjectWithID(data.originalID, data.name);
    if (!obj)
    {
        return nullptr;
    }

    bool willHaveAnimation = data.hasAnimation && !data.spritesheetPath.empty();

    // add transform component
    if (data.hasTransform)
    {
        auto *transform = obj->AddComponent<TransformComponent>(data.posX, data.posY);
        transform->SetScale(data.scaleX, data.scaleY);
        transform->SetRotation(data.rotation);
    }

    // add render component
    if (data.hasRender)
    {
        RenderComponent::ShapeType shape =
            (RenderComponent::ShapeType)data.shapeType;

        // animated
        if (willHaveAnimation)
        {
            std::cout << "[LevelEditor] Creating RenderComponent for animated object: "
                      << data.name << std::endl;

            auto *render = obj->AddComponent<RenderComponent>(
                shape,
                vec4(data.colorR, data.colorG, data.colorB, data.colorA));

            // store path so future serialisation can recover it
            render->SetTexturePath(data.spritesheetPath);
        }
        // static texture
        else if (data.hasTexture && !data.texturePath.empty())
        {
            // try to get or load the texture through resource manager
            auto &rm = ResourceManager::Instance();
            GLuint texID = 0;

            TextureResource *texRes = rm.GetTexture(data.name);
            if (texRes && texRes->IsLoaded())
            {
                texID = texRes->GetTextureID();
            }
            else
            {
                // not loaded yet, try to load now using saved path
                if (rm.LoadTexture(data.name, data.texturePath))
                {
                    texRes = rm.GetTexture(data.name);
                    if (texRes && texRes->IsLoaded())
                    {
                        texID = texRes->GetTextureID();
                    }
                }
                else
                {
                    texID = GraphicsSystem::LoadTexture(data.texturePath.c_str());
                }
            }

            auto *render = obj->AddComponent<RenderComponent>(texID);
            render->SetTexturePath(data.texturePath);
            render->SetColor(vec4(data.colorR, data.colorG, data.colorB, data.colorA));

            glBindTexture(GL_TEXTURE_2D, texID);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        // square, circle etc
        else
        {
            vec4 color(data.colorR, data.colorG, data.colorB, data.colorA);
            obj->AddComponent<RenderComponent>(shape, color);
        }
    }

    // add physics component
    if (data.hasPhysics)
    {
        auto *physics = obj->AddComponent<PhysicsComponent>(data.mass, data.isStatic);
        physics->SetVelocity(data.velocityX, data.velocityY);

        if (data.hasGravity)
        {
            physics->SetGravity(true);
        }
    }

    // add collider component
    if (data.hasCollider)
    {
        ColliderComponent::Type colliderType = (ColliderComponent::Type)data.colliderType;
        CollisionLayer layer = (CollisionLayer)data.collisionLayer;
        obj->AddComponent<ColliderComponent>(
            colliderType,
            data.colliderWidth,
            data.colliderHeight,
            layer);
    }

    // add animation / spritesheet component
    if (willHaveAnimation)
    {
        std::cout << "[LevelEditor] Restoring animation for " << data.name << std::endl;

        auto *spritesheet = obj->AddComponent<Spritesheet>(
            data.spritesheetPath.c_str(),
            data.numRows,
            data.numCols);

        if (spritesheet)
        {
            spritesheet->SetAnimation(
                data.startFrame,
                data.endFrame,
                data.fps);

            // now sync the render component with the spritesheet texture
            auto *render = obj->GetComponent<RenderComponent>();
            if (render)
            {
                GLuint texID = spritesheet->GetTextureID();
                render->SetTextureID(texID);
                render->SetTexturePath(data.spritesheetPath);

                // animated sprites should usually render with white tint
                render->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));

                glBindTexture(GL_TEXTURE_2D, texID);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glBindTexture(GL_TEXTURE_2D, 0);
            }
        }
    }

    obj->SetActive(data.isActive);

    return obj;
}

 /*  Camera control  */
/**
 * @brief Render camera control panel in ImGui
 */
void LevelEditor::UpdateCameraControls(bool renderUI)
{
    // RENDER UI PANEL (if requested)
    if (renderUI) {
        ImGui::SetNextWindowPos(ImVec2(10, 400), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_FirstUseEver);

        if (ImGui::Begin("Camera Controls##LevelEditor", nullptr, ImGuiWindowFlags_NoCollapse)) {
            ImGui::Text("Level Editor Camera");
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "For navigation & editing");
            ImGui::Separator();

            // display current camera info
            ImGui::Text("Position: (%.1f, %.1f)", cameraX, cameraY);
            ImGui::Text("Zoom: %.2fx", cameraZoom);
            ImGui::Separator();

            // camera position sliders
            ImGui::DragFloat("Camera X", &cameraX, 1.0f, -10000.0f, 10000.0f);
            ImGui::DragFloat("Camera Y", &cameraY, 1.0f, -10000.0f, 10000.0f);

            if (ImGui::SliderFloat("Zoom", &cameraZoom, minZoom, maxZoom, "%.2f")) {
                // clamp zoom to min/max
                if (cameraZoom < minZoom) cameraZoom = minZoom;
                if (cameraZoom > maxZoom) cameraZoom = maxZoom;
            }
            ImGui::Separator();

            // Hardcoded values because they really shouldn't be anything else aside from 0 and 1 (well 0.99f) :)
            if (ImGui::SliderFloat("Smoothing", &zoomSmoothing, 0.f, 0.99f, "%.2f")) {
                // clamp zoom to min/max
                if (zoomSmoothing < 0.f) zoomSmoothing = 0.f;
                if (zoomSmoothing > 0.99f) zoomSmoothing = 0.99f;
            }
            ImGui::Separator();

            // camera settings
            ImGui::DragFloat("Pan Speed", &panSpeed, 0.1f, 0.1f, 10.0f);
            ImGui::Separator();

            // reset button
            if (ImGui::Button("Reset Camera", ImVec2(-1, 0))) {
                ResetCamera();
            }

            ImGui::Separator();
            ImGui::TextWrapped("Controls:\n"
                "- Middle Mouse: Pan camera\n"
                "- Arrow Keys: Pan camera (alternative)\n"
                "- Mouse Wheel: Zoom");

            ImGui::Separator();
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.3f, 1.0f), "Only active in Edit and Pause Mode");
        }
        ImGui::End();
    }

    // update camera input
    ImGuiIO& io = ImGui::GetIO();

    // in play mode camera follows hero
    if (editorMode == EditorMode::Play) {
        isPanning = false;
        return;
    }

    // middle mouse button panning
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Middle)) {
        isPanning = true;
        lastMouseX = io.MousePos.x;
        lastMouseY = io.MousePos.y;
    }
    else if (ImGui::IsMouseReleased(ImGuiMouseButton_Middle)) {
        isPanning = false;
    }

    if (isPanning && ImGui::IsMouseDown(ImGuiMouseButton_Middle)) {
        float deltaX = io.MousePos.x - lastMouseX;
        float deltaY = io.MousePos.y - lastMouseY;

        // Pan camera
        cameraX -= (deltaX * panSpeed) / cameraZoom;
        cameraY += (deltaY * panSpeed) / cameraZoom;

        lastMouseX = io.MousePos.x;
        lastMouseY = io.MousePos.y;
    }

    // keyboard panning
    bool keyboardOverImGuiWindow = io.WantCaptureKeyboard && ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow);

    if (!keyboardOverImGuiWindow) {
        float keyPanSpeed = panSpeed * 5.0f / cameraZoom;

        if (ImGui::IsKeyDown(ImGuiKey_DownArrow)) {
            cameraY -= keyPanSpeed;
        }
        if (ImGui::IsKeyDown(ImGuiKey_UpArrow)) {
            cameraY += keyPanSpeed;
        }
        if (ImGui::IsKeyDown(ImGuiKey_RightArrow)) {
            cameraX += keyPanSpeed;
        }
        if (ImGui::IsKeyDown(ImGuiKey_LeftArrow)) {
            cameraX -= keyPanSpeed;
        }
    }

    if (isHoveringOverViewport)
    {
        // mouse wheel zoom
        if (io.MouseWheel != 0.0f) {
            //world pos under before zoom
            ImVec2 worldBefore = CursorToWorldPos(viewportPos, viewportSize);
            
            // apply zoom 
            float oldZoom = cameraZoom;
            cameraZoom += io.MouseWheel * zoomSpeed;                            // increase zooming
            cameraZoom = cameraZoom + (oldZoom - cameraZoom) * zoomSmoothing;   // lerping algorithm to help with smoothness

            if (cameraZoom < minZoom) cameraZoom = minZoom;         // clamp
            if (cameraZoom > maxZoom) cameraZoom = maxZoom;

            //world pos under after zoom
            ImVec2 worldAfter = CursorToWorldPos(viewportPos, viewportSize);

            // set diff as camera pos
            cameraX += worldBefore.x - worldAfter.x;
            cameraY += worldBefore.y - worldAfter.y;
        }
    }
}

/**
 * @brief Reset camera to default position and zoom
 */
void LevelEditor::ResetCamera()
{
    cameraX = 0.0f;
    cameraY = 0.0f;
    cameraZoom = 1.0f;
    isPanning = false;
    std::cout << "[LevelEditor] Camera reset to default." << std::endl;
}

/**
 * @brief Get the editor camera's view-projection matrix
 */
mat4 LevelEditor::GetEditorViewProjection(float windowWidth, float windowHeight) const
{
    // In play pause mode, use the game camera
    if (editorMode == EditorMode::Play)
    {
        if (CameraManager::Instance().HasActiveCamera())
        {
            return CameraManager::Instance().GetViewProjectMatrix(windowWidth, windowHeight);
        }
    }

    // in edit mode use editor camera
    float halfWidth = (windowWidth / 2.0f) / cameraZoom;
    float halfHeight = (windowHeight / 2.0f) / cameraZoom;

    float left = cameraX - halfWidth;
    float right = cameraX + halfWidth;
    float bottom = cameraY - halfHeight;
    float top = cameraY + halfHeight;

    mat4 projection = mat4::ortho(left, right, bottom, top);

    return projection;
}

/*  Deferred scene loading  */
/**
 * @brief Perform deferred scene reload
 */
void LevelEditor::PerformSceneReload()
{
    if (!pendingSceneReload)
        return;

    std::cout << "[LevelEditor] Performing deferred scene reload: "
        << pendingScenePath << "\n";

    // clear and reload safely at start of frame
    factory.Clear();
    selectedGameObjectID = 0;
    ClearUndoStack();

    if (LoadLevel(pendingScenePath))
    {
        std::string filename = fs::path(currentFilename).filename().string();

        currentFilename = filename;
        std::cout << "[LevelEditor] Scene reloaded successfully.\n";
    }
    else
    {
        std::cerr << "[LevelEditor] Failed to reload scene: "
            << pendingScenePath << "\n";
    }

    hasValidSaveState = false;
    pendingSceneReload = false;
    pendingScenePath.clear();
}

void LevelEditor::RecordUndo(const EditorUndoCommand& undocmd) {
    
    // Check if undo exceed 20
    if (undoStack.size() >= undoCapacity) {
        undoStack.erase(undoStack.begin()); // Remove oldest
    }

    undoStack.push_back(undocmd);

    // New edit after undo means redo should be cleared
    //redoStack.clear();
    
}

void LevelEditor::PerformUndo() {
    if (undoStack.empty()) return;

    EditorUndoCommand undocmd = undoStack.back();
    undoStack.pop_back();

    switch (undocmd.action) {
        case EditorUndoCommand::ActionType::ObjectTransform: {
            GameObject* obj = factory.GetGameObject(undocmd.entityID);
            if (!obj) break;

            auto* transform = obj->GetComponent<TransformComponent>();
            if (!transform) break;

            json t = undocmd.beforeJson;

            // Restore ONLY transform fields
            if (t.contains("pos"))
                transform->SetPosition(t["pos"][0], t["pos"][1]);

            if (t.contains("scale"))
                transform->SetScale(t["scale"][0], t["scale"][1]);

            if (t.contains("rotation"))
                transform->SetRotation(t["rotation"]);

            break;
        }

        case EditorUndoCommand::ActionType::ObjectCreated: {
            GameObject* obj = factory.GetGameObject(undocmd.entityID);
            
            if (obj)
            {
                if (CORE && CORE->GetImguiSystem()) // Deselect from Editor
                {
                    CORE->GetImguiSystem()->ResetSelectedGameObject();
                }

                factory.DestroyAllRelatedGameObjects(undocmd.entityID);
            }
            break;
        }
        case EditorUndoCommand::ActionType::ObjectDeleted: {
            Serialization::DeserializeGameObject(undocmd.beforeJson);
            Serialization::ResolveHierarchy(); // Reconnect hierarchy via parentID & children

            GameObject* restored = factory.GetGameObject(undocmd.entityID);
            GameObject* parent = factory.GetGameObject(undocmd.parentID);

             //Reattach restored object to parent
            if (restored && parent) {
                parent->AddChild(restored);
            }

            if (parent && !undocmd.siblingOrderIDs.empty()) // Restore original sibling order
            {
                parent->GameObject::ReorderChildren(undocmd.siblingOrderIDs);
            }
            else {                                         // Restore root order
                if (!undocmd.rootOrderIDs.empty()) {
                    factory.ReorderRootObjects(undocmd.rootOrderIDs);
                }
            }
         
            break;
        }

    }

    //redoStack.push_back(undocmd);
}

void LevelEditor::ClearUndoStack() {
    undoStack.clear();
}

/*  Audio Management */
/**
 * @brief Play loaded background music
 */
//void LevelEditor::PlayLoadedBGM()
//{
//    AudioEngine* audioEngine = ResourceManager::Instance().GetAudioEngine();
//
//    if (audioEngine && !currentBGMName.empty())
//    {
//        std::cout << "[LevelEditor::PlayLoadedBGM] Starting BGM: " << currentBGMName
//            << " on channel: " << currentBGMChannel << std::endl;
//
//        // stop any existing sound on this channel first
//        audioEngine->stopSound(currentBGMChannel);
//
//        // play the BGM
//        audioEngine->playSound(currentBGMName, currentBGMChannel);
//        audioEngine->setVolume(currentBGMChannel, currentBGMVolume);
//
//        std::cout << "[LevelEditor::PlayLoadedBGM] BGM playing successfully.\n";
//    }
//    else
//    {
//        if (!audioEngine)
//        {
//            std::cout << "[LevelEditor::PlayLoadedBGM] No audio engine available.\n";
//        }
//        else if (currentBGMName.empty())
//        {
//            std::cout << "[LevelEditor::PlayLoadedBGM] No BGM loaded for this scene.\n";
//        }
//    }
//}
#endif