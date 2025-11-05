#include "ColliderComponent.h"
#include "LevelEditor.h"
#include "physics.h"
#include "ResourceManager.h"
#include "Serialization.h"

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
      showPropertyEditor(true),
      showSaveDialog(false),
      showLoadDialog(false),
      currentFilename(""),
      editorMode(EditorMode::Stop),
      hasUnsavedChanges(false),
      hasValidSaveState(false),
      cameraX(0.0f),
      cameraY(0.0f),
      cameraZoom(1.0f),
      isPanning(false),
      lastMouseX(0.0f),
      lastMouseY(0.0f),
      panSpeed(1.0f),
      zoomSpeed(0.1f), // zoom to be implemented
      minZoom(0.1f),
      maxZoom(5.0f)
{
    strcpy(saveFilepath, "");
    strcpy(loadFilepath, "../Assets/Scenes//Prefab_TestScene.json");
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
}

/**
 * @brief Update - Render all editor GUI panels
 */
void LevelEditor::Update(float /*dt*/)
{
    if (!InputManager::KeyStateImgui)
        return;

    // render menu bar
    RenderMenuBar();

    // render hierarchy panel
    RenderHierarchy();

    // render property editor
    if (showPropertyEditor && selectedGameObjectID != 0)
    {
        return;
    }

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
    UpdateCameraControls();
    RenderPlayControls();

    // show editor UI only when not in live Play
    if (editorMode != EditorMode::Play)
    {
        RenderMenuBar();
        RenderHierarchy();

        // only show camera controls in edit mode
        if (editorMode == EditorMode::Stop)
        {
            RenderCameraControls();
        }

        if (showPropertyEditor && selectedGameObjectID != 0)
        {
            RenderPropertyEditor();
        }
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
                if (LoadLevelJSON(loadFilepath))
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

void LevelEditor::RenderPlayControls()
{
    ImGui::Begin("Editor Controls");

    const bool isRunning = (editorMode == EditorMode::Play || editorMode == EditorMode::Pause);
    const char *mainLabel = isRunning ? "Stop" : "Play";
    if (ImGui::Button(mainLabel))
    {
        TogglePlayStop();
    }

    // pause / resume � always visible
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
    ImGui::TextDisabled("Mode: %s",
                        editorMode == EditorMode::Play ? "Play" : editorMode == EditorMode::Pause ? "Pause"
                                                                                                  : "Stop");

    if (!isRunning)
    {
        ImGui::TextDisabled("Editing scene (camera active).");
    }
    else
    {
        ImGui::TextDisabled("Game running � use Pause/Stop.");
    }
    ImGui::End();
}

// play state
void LevelEditor::EnterPlayMode()
{
    if (editorMode == EditorMode::Play)
        return;
    if (!hasValidSaveState)
    {
        SaveSceneState(); // save the current scene as a JSON snapshot
        hasValidSaveState = true;
        std::cout << "[LevelEditor] Saved original scene state.\n";
    }
    else
    {
        std::cout << "[LevelEditor] Using existing saved state (not overwriting).\n";
    }
    editorMode = EditorMode::Play;
    std::cout << "[LevelEditor] Entered Play Mode.\n";
}

// stop state
void LevelEditor::StopPlayMode()
{
    if (editorMode == EditorMode::Stop)
        return;

    RestoreSceneState(); // restore saved scene
    hasValidSaveState = false;
    editorMode = EditorMode::Stop; // static camera
    selectedGameObjectID = 0;      // clear selection
    std::cout << "[LevelEditor] Returned to Edit Mode.\n";
}

// pause state
void LevelEditor::TogglePause()
{
    if (editorMode == EditorMode::Play)
    {
        editorMode = EditorMode::Pause;
        std::cout << "[LevelEditor] Game paused." << std::endl;
    }
    else if (editorMode == EditorMode::Pause)
    {
        editorMode = EditorMode::Play;
        std::cout << "[LevelEditor] Game resumed." << std::endl;
    }
}

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
    }
}

// save current scene state
void LevelEditor::SaveSceneState()
{
    savedSceneState.gameObjects.clear();

    // serialise all current GameObjects
    for (auto &goPtr : this->factory.GetAllGameObjects())
     {
        GameObject *go = goPtr.get();
        savedSceneState.gameObjects.push_back(SerialiseGameObject(go));
    }

    // store the current entity counter
    savedEntityIDCounter = this->factory.GetNextEntityID();

    std::cout << "[LevelEditor] Scene state saved with "
              << savedSceneState.gameObjects.size() << " objects.\n";
}

// restore scene state from saved data
void LevelEditor::RestoreSceneState()
{
    if (hasValidSaveState)
    {
        std::cout << "[LevelEditor] Warning: No valid save state to restore!\n";
        return;
    }

    std::vector<GameObjectData> sceneDataCopy = savedSceneState.gameObjects;
    EntityID savedIDCopy = savedEntityIDCounter;
    std::cout << "[LevelEditor] Restoring " << sceneDataCopy.size() << " objects...\n";

    this->factory.DestroyAllGameObjects();

    for (auto &data : sceneDataCopy)
    {
        DeserialiseGameObject(data);
    }

    this->factory.SetNextEntityID(savedIDCopy);

    std::cout << "[LevelEditor] Scene state restored." << this->factory.GetAllGameObjects().size() << " objects.\n";
}

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

        if (SaveLevelJSON(filepath))
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
                std::string fullPath = "../Assets/Scenes/" + file;
                LoadLevelJSON(fullPath);
            }
            ImGui::PopID();
            sceneIndex++;
        }
    }

    ImGui::End();
}

void LevelEditor::RenderHierarchy()
{
    ImGui::SetNextWindowPos(ImVec2(10, 370), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(250, 600), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("Hierarchy##LevelEditor", nullptr, ImGuiWindowFlags_NoCollapse))
    {
        ImGui::End();
        return;
    }

    // display current file name
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 1.0f, 1.0f));
    ImGui::Text("Scene: %s", currentFilename.c_str());
    ImGui::PopStyleColor();
    ImGui::Separator();

    // display object count
    auto &allObjects = factory.GetAllGameObjects();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
    ImGui::Text("Game Objects: %zu", allObjects.size());
    ImGui::PopStyleColor();
    ImGui::Separator();

    // list all game objects
    for (const auto &objPtr : allObjects)
    {
        if (!objPtr->IsActive())
            continue;

        GameObject *obj = objPtr.get();
        EntityID id = obj->GetID();

        // highlight if selected
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        if (selectedGameObjectID == id)
        {
            flags |= ImGuiTreeNodeFlags_Selected;
        }

        bool nodeOpen = ImGui::TreeNodeEx((void *)(intptr_t)id, flags, "%s (ID: %u)",
                                          obj->GetName().c_str(), id);

        // select on click
        if (ImGui::IsItemClicked())
        {
            SetSelectedGameObject(id);
        }

        // context menu
        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem("Select"))
            {
                SetSelectedGameObject(id);
            }
            if (ImGui::MenuItem("Save as Prefab"))
            {
                Serialization::SavePrefab(*obj);
            }
            if (ImGui::MenuItem("Clone"))
            {
                factory.CloneGameObject(id);
            }
            if (ImGui::MenuItem("Delete"))
            {
                RemoveGameObject(id);
                if (selectedGameObjectID == id)
                    selectedGameObjectID = 0;
            }
            ImGui::EndPopup();
        }
    }
    ImGui::End();
}

void LevelEditor::RenderPropertyEditor()
{
    GameObject *obj = GetSelectedGameObject();
    if (!obj)
    {
        showPropertyEditor = false;
        return;
    }

    ImGui::SetNextWindowPos(ImVec2(1400, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(500, 900), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("Properties##LevelEditor", &showPropertyEditor, ImGuiWindowFlags_NoCollapse))
    {
        ImGui::End();
        return;
    }

    ImGui::Text("GameObject: %s", obj->GetName().c_str());
    ImGui::Text("ID: %u", obj->GetID());
    ImGui::Separator();

    bool isActive = obj->IsActive();
    if (ImGui::Checkbox("Active", &isActive))
    {
        obj->SetActive(isActive);
    }

    ImGui::Spacing();

    // transform
    if (auto *transform = obj->GetComponent<TransformComponent>())
    {
        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
        {
            float pos[2] = {0.0f, 0.0f};
            transform->GetPosition(pos[0], pos[1]);
            if (ImGui::DragFloat2("Position", pos, 0.5f))
            {
                transform->SetPosition(pos[0], pos[1]);
            }

            float scale[2] = {1.0f, 1.0f};
            transform->GetScale(scale[0], scale[1]);
            if (ImGui::DragFloat2("Scale", scale, 0.5f))
            {
                transform->SetScale(scale[0], scale[1]);
            }

            float rotation = transform->GetRotation();
            if (ImGui::DragFloat("Rotation", &rotation, 0.5f))
            {
                transform->SetRotation(rotation);
            }
        }
    }

    // render component
    if (auto *render = obj->GetComponent<RenderComponent>())
    {
        if (ImGui::CollapsingHeader("Render", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Text("Shape Type: %s", GetShapeTypeName((int)render->GetShapeType()));

            if (render->GetTextureID() == 0)
            {
                vec4 color = render->GetColor();
                float colorArray[4] = {color.x, color.y, color.z, color.w};
                if (ImGui::ColorEdit4("Color", colorArray))
                {
                    render->SetColor(vec4(colorArray[0], colorArray[1], colorArray[2], colorArray[3]));
                }
            }
            else
            {
                ImGui::Text("Texture ID: %u", render->GetTextureID());
            }
        }
    }

    // physics component
    if (auto *physics = obj->GetComponent<PhysicsComponent>())
    {
        if (ImGui::CollapsingHeader("Physics"))
        {
            float vel[2] = {0.0f, 0.0f};
            physics->GetVelocity(vel[0], vel[1]);
            if (ImGui::DragFloat2("Velocity", vel, 0.1f))
            {
                physics->SetVelocity(vel[0], vel[1]);
            }

            bool isStatic = physics->IsStatic();
            if (ImGui::Checkbox("Static", &isStatic))
            {
                physics->SetStatic(isStatic);
            }

            bool hasGravity = physics->useGravity;
            if (ImGui::Checkbox("Gravity", &hasGravity))
            {
                physics->SetGravity(hasGravity);
            }

            ImGui::Text("Mass: %.2f", physics->GetMass());
            ImGui::Text("Grounded: %s", physics->IsGrounded() ? "Yes" : "No");
        }
    }

    // collider component
    if (auto *collider = obj->GetComponent<ColliderComponent>())
    {
        if (ImGui::CollapsingHeader("Collider"))
        {
            ImGui::Text("Type: %s", collider->GetType() == ColliderComponent::Type::Circle ? "Circle" : "Square");

            if (collider->GetType() == ColliderComponent::Type::Circle)
            {
                Circle circle = collider->GetCircle();
                ImGui::Text("Radius: %.2f", circle.radius);
            }
            else
            {
                AABB aabb = collider->GetAABB();
                float width = aabb.max.x - aabb.min.x;
                float height = aabb.max.y - aabb.min.y;
                ImGui::Text("Width: %.2f", width);
                ImGui::Text("Height: %.2f", height);
            }

            ImGui::Text("Layer: %s", GetCollisionLayerName((int)collider->GetLayer()));
        }
    }
    ImGui::End();
}

bool LevelEditor::SaveLevelJSON(const std::string &filepath)
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

        // serialise all active game objects
        auto &allObjects = factory.GetAllGameObjects();
        for (const auto &objPtr : allObjects)
        {
            if (objPtr && objPtr->IsActive())
            {
                json objData = Serialization::SerializeGameObject(objPtr.get());
                root["GameObjects"].push_back(objData);
            }
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

bool LevelEditor::LoadLevelJSON(const std::string& filepath)
{
    try
    {
        std::cout << "\n========================================" << std::endl;
        std::cout << "LOADING LEVEL: " << filepath << std::endl;
        std::cout << "========================================\n" << std::endl;

        std::ifstream inFile(filepath);
        if (!inFile.is_open())
        {
            std::cerr << "Failed to open file for reading: " << filepath << std::endl;
            return false;
        }

        json root;
        inFile >> root;
        inFile.close();

        // === Stop and cleanup audio ===
        auto& resourceMgr = ResourceManager::Instance();
        AudioEngine* audio = resourceMgr.GetAudioEngine();

        if (audio)
        {
            std::cout << "[LoadLevel] Stopping all sounds..." << std::endl;
            audio->stopAllSound();
            audio->updateAudio();
            std::cout << "[LoadLevel] All sounds stopped" << std::endl;
        }

        // Unload all audio resources
        std::cout << "[LoadLevel] Unloading audio resources..." << std::endl;
        resourceMgr.UnloadAllAudio();

        // ===  Clear current level ===
        factory.Clear();
        selectedGameObjectID = 0;

        // ===  Extract filename ===
        size_t lastSlash = filepath.find_last_of("/\\");
        size_t lastDot = filepath.find_last_of(".");
        std::string filenameOnly = filepath;

        if (lastSlash != std::string::npos)
            filenameOnly = filepath.substr(lastSlash + 1);

        if (lastDot != std::string::npos && lastDot > lastSlash)
            filenameOnly = filenameOnly.substr(0, lastDot - (lastSlash + 1));

        currentFilename = filenameOnly;

        // === Load Scene Settings ===
        if (root.contains("SceneSettings"))
        {
            std::cout << "[LoadLevel] Loading scene settings..." << std::endl;
            const auto& settings = root["SceneSettings"];

            if (settings.contains("backgroundMusic") && audio)
            {
                const auto& bgm = settings["backgroundMusic"];
                std::string name = bgm.value("name", "bgm");
                std::string audioPath = bgm.value("path", "");
                bool looping = bgm.value("looping", true);
                float volume = bgm.value("volume", 0.8f);
                std::string channel = bgm.value("channel", "bgm");

                if (!audioPath.empty())
                {
                    std::cout << "[LoadLevel] Loading BGM: " << name << " from " << audioPath << std::endl;

                    // Load the new audio
                    if (resourceMgr.LoadAudio(name, audioPath, looping))
                    {
                        std::cout << "[LoadLevel] Playing BGM on channel: " << channel << std::endl;
                        audio->playSound(name, channel);
                        audio->setVolume(channel, volume);
                        std::cout << "[LoadLevel] BGM started successfully" << std::endl;
                    }
                    else
                    {
                        std::cerr << "[LoadLevel] Failed to load BGM: " << audioPath << std::endl;
                    }
                }
            }
        }
        else
        {
            std::cout << "[LoadLevel] No SceneSettings found in JSON" << std::endl;
        }

        
        if (root.contains("GameObjects") && root["GameObjects"].is_array())
        {
            std::cout << "[LoadLevel] Loading game objects..." << std::endl;
            for (const auto& objData : root["GameObjects"])
            {
                Serialization::DeserializeGameObject(objData);
            }
        }

        std::cout << "Level loaded from: " << filepath << std::endl;
        std::cout << "Loaded " << factory.GetAllGameObjects().size() << " game objects" << std::endl;
        std::cout << "========================================\n" << std::endl;

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
 * @brief Get all scene files from the prefabs directory
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

    std::cout << "New level created" << std::endl;
}

/**
 * @brief Save level to file
 */
bool LevelEditor::SaveLevel(const std::string &filepath)
{
    return SaveLevelJSON(filepath); // redirect to JSON version
}

/**
 * @brief Load level from file
 */
bool LevelEditor::LoadLevel(const std::string &filepath)
{
    // check if it's a JSON file
    if (filepath.ends_with(".json"))
    {
        return LoadLevelJSON(filepath);
    }

    LevelData data;
    if (LoadLevelFromFile(filepath, data))
    {
        // convert to current level
        currentLevel = data;

        // clear and recreate game objects
        factory.Clear();
        selectedGameObjectID = 0;

        for (const auto &objData : data.gameObjects)
        {
            DeserialiseGameObject(objData);
        }

        return true;
    }

    return false;
}

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
 * @brief Get the selected game object
 */
GameObject *LevelEditor::GetSelectedGameObject() const
{
    return factory.GetGameObject(selectedGameObjectID);
}

/**
 * @brief Set the selected game object
 */
void LevelEditor::SetSelectedGameObject(EntityID id)
{
    selectedGameObjectID = id;
}

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
        data.velocityX = physics->velocityX;
        data.velocityY = physics->velocityY;
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

    // add animation / spritesheet component
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
    // create base game object with same id so selection etc. still works
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
            // We'll try to get or load the texture through ResourceManager
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

    // --- Animation / Spritesheet ---
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

/**
 * @brief Load level data from file
 */
bool LevelEditor::LoadLevelFromFile(const std::string &filepath, LevelData &outData)
{
    std::ifstream file(filepath);
    if (!file.is_open())
    {
        std::cerr << "Failed to open file for reading: " << filepath << std::endl;
        return false;
    }

    std::string line;
    outData = LevelData(); // reset

    // read level
    while (std::getline(file, line))
    {
        if (line == "LEVEL_BEGIN")
        {
            continue;
        }
        else if (line == "GAMEOBJECT_BEGIN")
        {
            break; // start reading game objects
        }
    }

    // read game objects
    GameObjectData currentObj;
    bool readingObject = true;

    while (readingObject && std::getline(file, line))
    {
        if (line == "GAMEOBJECT_BEGIN")
        {
            currentObj = GameObjectData(); // reset for new object
        }
        else if (line.find("Name: ") == 0)
        {
            currentObj.name = line.substr(6);
        }
        else if (line.find("Active: ") == 0)
        {
            currentObj.isActive = (line.substr(8) == "1");
        }
        else if (line.find("Transform: ") == 0)
        {
            std::istringstream iss(line.substr(11));
            iss >> currentObj.posX >> currentObj.posY >> currentObj.scaleX >> currentObj.scaleY >> currentObj.rotation;
            currentObj.hasTransform = true;
        }
        else if (line.find("Render: ") == 0)
        {
            std::istringstream iss(line.substr(8));
            int hasTexInt;
            iss >> currentObj.shapeType >> hasTexInt >> currentObj.colorR >> currentObj.colorG >> currentObj.colorB >> currentObj.colorA;
            currentObj.hasRender = true;
            currentObj.hasTexture = (hasTexInt == 1);
        }
        else if (line.find("Texture: ") == 0)
        {
            currentObj.texturePath = line.substr(9);
        }
        else if (line.find("Physics: ") == 0)
        {
            std::istringstream iss(line.substr(9));
            int staticInt, gravityInt;
            iss >> currentObj.mass >> currentObj.velocityX >> currentObj.velocityY >> staticInt >> gravityInt;
            currentObj.hasPhysics = true;
            currentObj.isStatic = (staticInt == 1);
            currentObj.hasGravity = (gravityInt == 1);
        }
        else if (line.find("Collider: ") == 0)
        {
            std::istringstream iss(line.substr(10));
            iss >> currentObj.colliderType >> currentObj.colliderWidth >> currentObj.colliderHeight >> currentObj.collisionLayer;
            currentObj.hasCollider = true;
        }
        else if (line == "GAMEOBJECT_END")
        {
            outData.gameObjects.push_back(currentObj);
            currentObj = GameObjectData();
        }
        else if (line == "LEVEL_END")
        {
            readingObject = false;
            break;
        }
    }

    file.close();
    return true;
}

/**
 * @brief Get shape type name from enum value
 */
const char *LevelEditor::GetShapeTypeName(int type)
{
    switch (type)
    {
    case 0:
        return "Square";
    case 1:
        return "Circle";
    case 2:
        return "Line";
    case 3:
        return "Point";
    default:
        return "Unknown";
    }
}

/**
 * @brief Get collision layer name from enum value
 */
const char *LevelEditor::GetCollisionLayerName(int layer)
{
    switch (layer)
    {
    case 0:
        return "Player";
    case 1:
        return "Ground";
    case 2:
        return "Wall";
    case 3:
        return "Enemy";
    case 4:
        return "Platform";
    case 5:
        return "Default";
    default:
        return "Unknown";
    }
}

// camera control
/**
 * @brief Render camera control panel
 */
void LevelEditor::RenderCameraControls()
{
    ImGui::SetNextWindowPos(ImVec2(10, 400), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("Camera Controls##LevelEditor", nullptr, ImGuiWindowFlags_NoCollapse))
    {
        ImGui::End();
        return;
    }

    ImGui::Text("Level Editor Camera");
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "For navigation & editing");
    ImGui::Separator();

    // display current camera info
    ImGui::Text("Position: (%.1f, %.1f)", cameraX, cameraY);
    ImGui::Separator();

    // camera position sliders
    if (ImGui::DragFloat("Camera X", &cameraX, 1.0f, -10000.0f, 10000.0f))
    {
    }
    if (ImGui::DragFloat("Camera Y", &cameraY, 1.0f, -10000.0f, 10000.0f))
    {
    }
    ImGui::Separator();

    // camera settings
    ImGui::DragFloat("Pan Speed", &panSpeed, 0.1f, 0.1f, 10.0f);
    ImGui::Separator();

    // reset button
    if (ImGui::Button("Reset Camera", ImVec2(-1, 0)))
    {
        ResetCamera();
    }

    ImGui::Separator();
    ImGui::TextWrapped("Controls:\n"
                       "- Middle Mouse: Pan camera\n"
                       "- WASD: Pan camera (alternative)");

    ImGui::Separator();
    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.3f, 1.0f), "Only active in Editor Mode");
    ImGui::End();
}

/**
 * @brief Update camera controls (handle input for panning and zooming in the future)
 */
void LevelEditor::UpdateCameraControls()
{
    ImGuiIO &io = ImGui::GetIO();

    // camera moves when not hovering over imgui windows and if its in editor mode
    if (editorMode != EditorMode::Stop || io.WantCaptureMouse)
    {
        isPanning = false;
        return;
    }

    // middle mouse button
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Middle))
    {
        isPanning = true;
        lastMouseX = io.MousePos.x;
        lastMouseY = io.MousePos.y;
    }
    else if (ImGui::IsMouseReleased(ImGuiMouseButton_Middle))
    {
        isPanning = false;
    }

    if (isPanning && ImGui::IsMouseDown(ImGuiMouseButton_Middle))
    {
        float deltaX = io.MousePos.x - lastMouseX;
        float deltaY = io.MousePos.y - lastMouseY;

        // pan camera
        cameraX -= (deltaX * panSpeed) / cameraZoom;
        cameraY += (deltaY * panSpeed) / cameraZoom;

        lastMouseX = io.MousePos.x;
        lastMouseY = io.MousePos.y;
    }

    // keyboard panning
    if (!io.WantCaptureKeyboard)
    {
        float keyPanSpeed = panSpeed * 5.0f / cameraZoom;

        if (ImGui::IsKeyDown(ImGuiKey_DownArrow))
        {
            cameraY -= keyPanSpeed;
        }
        if (ImGui::IsKeyDown(ImGuiKey_UpArrow))
        {
            cameraY += keyPanSpeed;
        }
        if (ImGui::IsKeyDown(ImGuiKey_RightArrow))
        {
            cameraX += keyPanSpeed;
        }
        if (ImGui::IsKeyDown(ImGuiKey_LeftArrow))
        {
            cameraX -= keyPanSpeed;
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
 * @brief Apply camera transformations to the rendering system
 */
void LevelEditor::ApplyCameraTransform() {}

/**
 * @brief Get the editor camera's view-projection matrix
 */
mat4 LevelEditor::GetEditorViewProjection(float windowWidth, float windowHeight) const
{
    // same as your graphics system
    mat4 projection = mat4::ortho(0.0f, windowWidth, 0.0f, windowHeight);

    // create view matrix with editor camera position
    vec3 editorCamPos(cameraX - windowWidth / 2.0f, cameraY - windowHeight / 2.0f, 0.0f);
    mat4 view = mat4::translate(-editorCamPos);

    // apply zoom by scaling the projection
    mat4 zoomMatrix = mat4::translate(vec3(windowWidth / 2.0f, windowHeight / 2.0f, 0.0f));
    zoomMatrix = zoomMatrix * mat4::scale(vec3(cameraZoom, cameraZoom, 1.0f));
    zoomMatrix = zoomMatrix * mat4::translate(vec3(-windowWidth / 2.0f, -windowHeight / 2.0f, 0.0f));

    // projection * zoom * view
    return zoomMatrix * projection * view;
}
