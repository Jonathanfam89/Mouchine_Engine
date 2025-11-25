/* Start Header ************************************************************************/
/*!
\file       LevelEditor.h
\author     Leck Xin Yee [100% - 150 lines]
\par        xinyee.leck@digipen.edu
\date       November, 7, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#ifdef USE_IMGUI
#pragma once

#include "Animator.hpp"
#include "factory.h"
#include "GameObject.h"
#include "RenderComponent.h"
#include "System.h"
#include "transform.h"
#include "PrefabManager.h"

#include <imgui.h>
#include <memory>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

class GameObjectFactory;
using json = nlohmann::ordered_json;

/**
 * @enum EditorMode
 * @brief Represents the current state of the level editor
 */
enum class EditorMode
{
    Play,   // game running, objects active
    Pause,  // game paused, objects frozen
    Stop    // stopped, back to editing
};

/**
 * @struct GameObjectData
 * @brief Serialisable data structure for saving/loading game object properties
 */ 
struct GameObjectData
{
    std::string name;
    EntityID originalID;
    bool isActive;

    // transform data
    bool hasTransform;
    float posX, posY;
    float scaleX, scaleY;
    float rotation;

    // render data
    bool hasRender;
    int shapeType;
    bool hasTexture;
    std::string texturePath;
    float colorR, colorG, colorB, colorA;

    // physics data
    bool hasPhysics;
    float mass;
    float velocityX, velocityY;
    bool isStatic;
    bool hasGravity;

    // collider data
    bool hasCollider;
    int colliderType;
    float colliderWidth;
    float colliderHeight;
    int collisionLayer;

    // animation data
    bool hasAnimation;
    std::string spritesheetPath;
    int numRows, numCols;
    int startFrame, endFrame;
    float fps;
    int currentRow;
    int currentCol;
    float currentFrameTime;

    GameObjectData() :
        originalID(0), isActive(true),
        hasTransform(false), posX(0), posY(0), scaleX(1), scaleY(1), rotation(0),
        hasRender(false), shapeType(0), hasTexture(false), colorR(1), colorG(1), colorB(1), colorA(1),
        hasPhysics(false), mass(1), velocityX(0), velocityY(0), isStatic(false), hasGravity(false),
        hasCollider(false), colliderType(0), colliderWidth(0), colliderHeight(0), collisionLayer(0),
        hasAnimation(false), numRows(1), numCols(1), startFrame(0), endFrame(0), fps(1),
        currentRow(0), currentCol(0), currentFrameTime(0.0f)
    {}
};

/**
 * @struct LevelData
 * @brief Container for an entire level's game objects and metadata
 */
struct LevelData
{
    std::vector<GameObjectData> gameObjects;
};

struct EditorUndoCommand {
    enum class ActionType { ObjectTransform, ObjectCreated, ObjectDeleted };

    ActionType action = ActionType::ObjectTransform;
    EntityID entityID = 0;

    json beforeJson;
    json afterJson;

    std::string parentName;
    std::vector<std::string> childNames;
};

/**
 * @class LevelEditor
 * @brief System for editing, saving, and loading game levels through ImGui interface
 */
class LevelEditor : public ISystem
{
public:
    /**
     * @brief Construct a new level editor
     */
    LevelEditor();

    /**
     * @brief Destroy the level editor
     */
    ~LevelEditor();

    /**
     * @brief Initialise the level editor system
     */
    void Initialize() override;

    /**
     * @brief Update the level editor GUI (call every frame)
     * @param dt Delta time
     */
    void Update(float dt) override;

    /**
     * @brief Get the system name
     * @return System name string
     */
    std::string GetName() override { return "LevelEditor"; }

    /*  Json  */
    /**
     * @brief Saves the current level as a JSON file
     * @param filepath File path to save to
     * @return True if the save operation succeeded
     */
    bool SaveLevel(const std::string& filepath);

    /**
     * @brief Loads a level from a JSON file
     * @param filepath File path to load from
     * @return True if the load operation succeeded
     */
    bool LoadLevel(const std::string& filepath);

    /**
     * @brief Retrieves all available scene files from the Scenes directory
     * @return Vector of filenames
     */
    std::vector<std::string> GetAllSceneFiles();

    /*  Level Management  */
    /**
     * @brief Create a new empty level
     */
    void NewLevel();

    /**
     * @brief Get current level data
     * @return Reference to current level data
     */
    LevelData& GetCurrentLevel() { return currentLevel; }

    // Game object management
    /**
     * @brief Add a new game object through the editor
     * @param data GameObject data to instantiate
     * @return Pointer to created GameObject, or nullptr if failed
     */
    GameObject* AddGameObject(const GameObjectData& data);

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

    // Play/Stop functionality
    /**
     * @brief Get the current editor mode
     * @return Current EditorMode
     */
    EditorMode GetEditorMode() const { return editorMode; }

    /**
     * @brief Enter play mode (save state and start playing)
     */
    void EnterPlayMode();

    /**
     * @brief Stop play mode (restore saved state)
     */
    void StopPlayMode();

    /**
     * @brief Pause/unpause the game
     */
    void TogglePause();

    /**
     * @brief Play and stop the game
     */
    void TogglePlayStop();

    /*  Camera control functions  */
    /**
     * @brief Updates camera control UI and logic
     * @param renderUI Whether to render the camera control panel
     */
    void UpdateCameraControls(bool renderUI);

    /**
     * @brief Resets the editor camera position and zoom
     */
    void ResetCamera();

    /**
     * @brief Computes and retrieves the editor camera's view-projection matrix
     * @param windowWidth The viewport width
     * @param windowHeight The viewport height
     * @return The orthographic view-projection matrix
     */
    mat4 GetEditorViewProjection(float windowWidth, float windowHeight) const;

    /**
     * @return Current camera X position
     */
    float GetCameraX() const { return cameraX; }

    /**
     * @return Current camera Y position
     */
    float GetCameraY() const { return cameraY; }

    /**
     * @return Current camera zoom
     */
    float GetCameraZoom() const { return cameraZoom; }

    /**
     * @brief Set now position of camera.
     */
    void SetCameraPos(vec2 const& pos) { cameraX = pos.x; cameraY = pos.y; }

    /**
     * @brief Set now zoom value of camera.
     */
    void SetCameraZoom(float zoomValue) { cameraZoom = zoomValue; }

    /**
     * @brief Get the smoothing factor for zooming.
     */
    float GetCameraSmoothing() const { return zoomSmoothing; }

    /**
     * @brief (0.0f - 1.0f) Set the smoothing factor for zooming.
     */
    void SetCameraSmoothing(float smoothValue) { zoomSmoothing = smoothValue; }

    /**
     * @return Name of the currently loaded scene
     */
    const std::string& GetCurrentFilename() const { return currentFilename; }

    /**
     * @return True if a scene reload is pending
     */
    bool HasPendingSceneReload() const { return pendingSceneReload; }

    /**
     * @brief Executes a pending scene reload
     */
    void PerformSceneReload();

    /**
     * @brief Plays the loaded background music if available
     */
    //void PlayLoadedBGM();

    std::string GetCurrentBGMChannel() const { return currentBGMChannel; }

    // Helper functions
    /**
     * @brief Serialise a GameObject to GameObjectData structure
     * @param obj GameObject to serialise
     * @return GameObjectData structure with object's data
     */
    GameObjectData SerialiseGameObject(GameObject* obj);

    /**
     * @brief Deserialise GameObjectData and create GameObject instance
     * @param data GameObject data to deserialise
     * @return Pointer to created GameObject
     */
    GameObject* DeserialiseGameObject(const GameObjectData& data);

    void RecordUndo(const EditorUndoCommand& cmd);
    void PerformUndo();

    std::vector<EditorUndoCommand> undoStack;
    std::vector<EditorUndoCommand> redoStack;

private:
    // GUI rendering functions
    /**
     * @brief Render the main menu bar (File menu with New/Save/Load)
     */
    void RenderMenuBar();

    /**
     * @brief Render the play controls toolbar
     */
    void RenderPlayControls();

    // Play mode functions
    /**
     * @brief Save the current scene state for later restoration
     */
    void SaveSceneState();

    /**
     * @brief Restore the scene state from saved data
     */
    void RestoreSceneState();

    //// Helper functions
    ///**
    // * @brief Serialise a GameObject to GameObjectData structure
    // * @param obj GameObject to serialise
    // * @return GameObjectData structure with object's data
    // */
    //GameObjectData SerialiseGameObject(GameObject* obj);

    ///**
    // * @brief Deserialise GameObjectData and create GameObject instance
    // * @param data GameObject data to deserialise
    // * @return Pointer to created GameObject
    // */
    //GameObject* DeserialiseGameObject(const GameObjectData& data);

private:
    GameObjectFactory& factory;           // reference to game object factory
    LevelData currentLevel;               // current level being edited
    EntityID selectedGameObjectID;        // currently selected game object
    std::string currentFilename;          // show current filename

    // file dialog states
    bool showSaveDialog;
    bool showLoadDialog;
    char saveFilepath[256];
    char loadFilepath[256];

    std::vector<std::string> recentLevels;
    static constexpr int MAX_RECENT_LEVELS = 5;

    // Play / stop state
    EditorMode editorMode;                // current editor mode
    LevelData savedSceneState;            // saved state for when entering play mode
    bool hasUnsavedChanges;               // track if there are unsaved changes
    EntityID savedEntityIDCounter;
    std::vector<GameObjectData> savedSceneData;
    bool hasValidSaveState;               // track if we have a valid saved state to restore
    bool pendingSceneReload = false;
    std::string pendingScenePath;         // path to reload

    // Editor camera state
    float cameraX;                        // camera position X
    float cameraY;                        // camera position Y
    float cameraZoom;                     // camera zoom level
    bool isPanning;                       // is currently panning with mouse
    float lastMouseX;                     // last mouse X position for panning
    float lastMouseY;                     // last mouse Y position for panning
    float panSpeed;                       // camera pan sensitivity
    float zoomSpeed;                      // camera zoom sensitivity
    float minZoom;                        // minimum zoom level
    float maxZoom;                        // maximum zoom level
    float zoomSmoothing;                  // (0-1) controls how much added smoothing there is 

    std::string currentBGMName = "";
    std::string currentBGMChannel = "bgm_channel";
    float currentBGMVolume = 0.5f;
    static const int undoCapacity = 20;  // Maximum (action) history size
};
#endif