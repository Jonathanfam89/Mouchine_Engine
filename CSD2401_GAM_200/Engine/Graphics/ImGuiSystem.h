/* Start Header ************************************************************************/
/*!
\file       ImGuiSystem.h
\author     Guan Shao Jun [95%]
\par        shaojun.g@digipen.edu
\date       October 02 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include "System.h"
#include "factory.h"
#include "transform.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <map>
#include "InputManager.h"
#include "GraphicsSystem.h"
#include <filesystem>
#include "Serialization.h"
#include "logic.h"
#include "LevelEditor.h"
#include "ImGuizmo.h"
#include "TilemapEditor.h"  // NEW: Tilemap Editor

// FILE SYSTEM
namespace fs = std::filesystem;

const std::string AssetPayloadID{ "Asset_Filepath" };
const std::string PrefabPayloadID{ "Prefab_Filepath" };
const std::string HierarchyPayloadID{ "GameObject_Name" };
const std::string AudioPayloadID("Audio_Filepath");
const std::string ScenePayloadID{ "Scene_Name" };
const std::string SpritePayloadID{ "Sprite_Name" };

// ASSET MANAGER/ DIRECTORY NAVIGATOR VARIABLES
// Vars for ImGui::ImageButton
inline const char* icon_folder = "../Assets/Editor_Icons/Folder.png";
inline const char* icon_file = "../Assets/Editor_Icons/File_Generic.png";
inline const char* icon_plus = "../Assets/Editor_Icons/Plus.png";
inline const char* icon_prefab = "../Assets/Editor_Icons/Cube.png";
inline const char* icon_audio = "../Assets/Editor_Icons/File_Audio.png";
inline const char* icon_json = "../Assets/Editor_Icons/File_Json.png";
inline const char* icon_giz_select = "../Assets/Editor_Icons/Gizmo_Select.png";
inline const char* icon_giz_translate = "../Assets/Editor_Icons/Gizmo_Translate.png";
inline const char* icon_giz_scale = "../Assets/Editor_Icons/Gizmo_Scale.png";
inline const char* icon_giz_rotate = "../Assets/Editor_Icons/Gizmo_Rotate.png";
inline const char* icon_giz_resize = "../Assets/Editor_Icons/Gizmo_Resize.png";
inline const char* icon_active = "../Assets/Editor_Icons/Eye-Active.png";
inline const char* icon_inactive = "../Assets/Editor_Icons/Eye-Inactive.png";

extern GLuint icon_folder_ui;
extern GLuint icon_file_ui;
extern GLuint icon_plus_ui;
extern GLuint icon_prefab_ui;
extern GLuint icon_audio_ui;
extern GLuint icon_json_ui;
extern GLuint icon_giz_select_ui;
extern GLuint icon_giz_translate_ui;
extern GLuint icon_giz_scale_ui;
extern GLuint icon_giz_rotate_ui;
extern GLuint icon_giz_resize_ui;
extern GLuint icon_active_ui;
extern GLuint icon_inactive_ui;


// CONST VARIABLES
// AssetManager/ Hierarchy icon sizes
const ImVec2 icon_size(64, 64); 
const ImVec2 icon_size_large(48, 48);
const ImVec2 icon_size_medium(32, 32);
const ImVec2 icon_size_small(20, 20);
const ImVec2 icon_size_smallest(14, 14);

// Payload IDs (for different types of drag and drops)
static const fs::path assetPath = "../Assets";

using SystemTimingsMap = std::map<std::string, double>;

class GameObject;
class GameObjectFactory;
class Serialize_Writer;

/**
 * @class ImGuiSystem
 * @brief Handles ImGui initialization, updates, and rendering for the engine.
 */
class ImGuiSystem : public ISystem {
public:
    /**
     * @brief Construct a new ImGuiSystem with a GLFW window.
     * @param window Pointer to the GLFW window.
     */
    ImGuiSystem(GLFWwindow* window);

    /**
     * @brief Destroy the ImGuiSystem and cleanup ImGui resources.
     */
    ~ImGuiSystem();

    /**
     * @brief Initialize ImGui (create context, setup bindings).
     */
    void Initialize() override;

    /**
     * @brief Update and render ImGui elements each frame.
     * @param dt Delta time since last frame.
     */
    void Update(float) override;

    /**
     * @brief Return the system's name for identification.
     * @return std::string Name of the system.
     */
    std::string GetName() override;

    /**
     * @brief Render the hierarchy, also handles logic
     */
    void Hierarchy_Render();

    /**
     * @brief Specifically renders the parent and child part of the hierarchy
     * @param obj                   ptr to specified GO
     * @param depth                 how many layers (into the child->grandchildren etc) are we in (for indentation)
     * @param selectedGameObjectID  ID of selected GO
     * @param factory               Reference to factory
     */
    void Hierarchy_GameObjectTree_Renderer(GameObject* obj, int depth, EntityID& selectedGameObjectID, GameObjectFactory& factory);

    ///**
    // * @brief Add a new game object through the editor
    // * @param data GameObject data to instantiate
    // * @return Pointer to created GameObject, or nullptr if failed
    // */
    //GameObject* AddGameObject(const GameObjectData& data);

    /**
     * @brief Remove a game object from the level
     * @param id EntityID of the game object to remove
     */
    void RemoveGameObject(EntityID id);

    /**
     * @brief Get the currently selected game object
     * @return Pointer to selected GameObject, or nullptr if none selected
     */
    GameObject* GetSelectedGameObject() const;

    /**
     * @brief Set the currently selected game object
     * @param id EntityID of the game object to select
     */
    void SetSelectedGameObject(EntityID id);

    /**
     * @brief Deselects the selected game object. Has other logic things attached such as selection highlight etc.
     */
    void ResetSelectedGameObject();

    /**
     * @brief Render prefab spawn menu
     */
    void PrefabMenu_Render();

    /**
     * @brief Spawns specified prefab in menu ^
     */
    void PrefabMenu_Spawn();

    void PrefabMenu_Override();

    void PrefabMenu_Reinstantiate();

    /**
     * @brief Renders the property editor window. Also handles logic regarding which GO is selected and how to manipulate components
     */
    void RenderPropertyEditor();

    /**
     * @brief Function that handles logic of clicking on a GO in the viewport, to select. Also handles logic for drag and dropping to move the GOs around
     */
    void Viewport_ClickToSelect();

    void Viewport_ManageGizmos();

    void Viewport_RenderGizmoToolbar();

    void Viewport_HighlightGO();

    void RenderEventComponent(GameObject* obj);

private:
    /**
     * @brief Shutdown and cleanup ImGui resources.
     */
    void Shutdown();
    GameObjectFactory& factory;
    EntityID selectedGameObjectID;        // currently selected game object
    bool showPropertyEditor;              // show property editor panel
    const char* GetShapeTypeName(int type);         // get shape type name
    const char* GetCollisionLayerName(int layer);   // get collision layer name
    GLFWwindow* m_window; ///< Pointer to the GLFW window
};


class DirectoryNavigator
{
public:
    DirectoryNavigator();
    void RenderImGui(); 
private:
    std::filesystem::path currentDir;
};

class PropertiesSlot
{
public:
    PropertiesSlot() = delete;
    ~PropertiesSlot() { --_totalID; }
    PropertiesSlot(std::string const& displayText, std::string const& expectedPayload) : _displayText(displayText), _expectedPayload(expectedPayload), _id(++_totalID), _receivedData{} {}
    PropertiesSlot(PropertiesSlot const& rhs) : _displayText(rhs._displayText), _expectedPayload(rhs._expectedPayload), _id(++_totalID), _receivedData{} {}
    PropertiesSlot(PropertiesSlot&& rhs) noexcept : _displayText(std::move(rhs._displayText)), _expectedPayload(std::move(rhs._expectedPayload)), _id(++_totalID), _receivedData{} {}

    void RenderImGui();
    void SetDisplayText(std::string const& text) { _displayText = text; }

    unsigned int GetID() const { return _id; }
    std::string GetDisplayText() const { return _displayText; }
    std::string GetExpectedPayload() const { return _expectedPayload; }
private:
    unsigned int _id;
    std::string _displayText;
    std::string _expectedPayload;
    std::string _receivedData;
    inline static unsigned int _totalID = 0;
};

/**
 * @brief Gets the cursor pos with respect to the viewport's and world's coordinates. Externed for other files to access
 */
extern ImVec2 viewportCursorPos;
extern vec2 Prefab_Reinstantiate_Pos;

extern ImVec2 viewportSize;
extern ImVec2 viewportPos;

ImVec2 CursorToWorldPos(ImVec2 vpPos, ImVec2 vpSize);

extern bool isHoveringOverViewport;

// temporarily putting this here just for add component button (can just throw this into a struct ngl)
// 'group', name of component, icon texture, add component
enum ComponentType {
    Transform = 0,
    Render,
    Camera,
    Misc
};
extern std::vector < std::tuple<ComponentType, std::string, ImTextureID, std::function<void(GameObject*)>>>  AllComponents;

// instances of each spawned prefab, key: prefab name, value: vec of entityIDs
extern std::unordered_map<std::string, std::vector<EntityID>> prefabInstances;

// will be hard coded for now
void PopulateComponentList();

void ReloadPrefabsList();
/**
 * @brief Renders performance profiler
 */
void Profiler_Render();

/**
 * @brief Renders Game object creation menu
 */
void GOCreationMenu_Render();

/**
 * @brief Spawns specified GO in menu ^
 */
void GOCreationMenu_Spawn();

void StressTest_Spawn();

/**
  * @brief 1/2 functions for drag and drop operations within asset browser. This one is for sending. Sends a payload with data + leaves a trailing text for users to see what file is being moved.
  * @param PayloadID        ID that the Receiving end will check against, can be any string!
  * @param filePath         string containing filepath/ data to send over
  * @param displayText      string that shows near cursor while holding left click + dragging.
  */
template <typename DataType>
void SendDragAndDrop(std::string const& PayloadID, DataType const& dataToSend, std::string const& displayText);
void SendDragAndDrop(std::string const& PayloadID, std::string const& dataToSend, std::string const& displayText);


///**
// * @brief Wrapper to handle the receiving of drag and drop payloads
// * @param PayloadID       ID of payload
// * @param dataToReceive   data expecting to receive upon match with id
// */
void ReceiveDragAndDrop(std::string const& PayloadID, std::string& dataToReceive);

/**
 * @brief Renders the top menu tool bar
 */
void TopMenuBar_Render();

/**
 * @brief Saves current layout of imgui with specified file name
 */
void EditorLayout_SaveLayout(std::string const& fileName);

/**
 * @brief Loads current layout of imgui with specified file name
 */
void EditorLayout_LoadLayout(std::string const& fileName);

/**
 * @brief Refreshes vector of imgui layouts
 */
void EditorLayout_Refresh();

/**
 * @brief Renders pop up for saving current layout
 */
void EditorLayout_SaveNamePopUp();

/**
 * @brief Renders pop up for loading layouts
 */
void EditorLayout_LoadPopUp();

/**
 * @brief Wrapper to create hits when hovering over something
 * @param displayText     text to display when hovering
 */
void CreateHintText(std::string const& displayText);

/**
 * @brief Handles the management of the spawning 1000 Gos
 */
void M2_RenderStressTest();

void AudioAssetsPanel_Render();

void TileMap_Render(float dt);
