/* Start Header ************************************************************************/
/*!
\file       Serialization.h
\author     Muhammad Ashfer [100% - 43 lines]
\par        m.binafzalkhan@digipen.edu
\date       October, 3, 2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <nlohmann/json.hpp>

#include "GameObject.h"
#include "RenderComponent.h"
#include "factory.h"
#include "gameObject.h"
#include "CameraComponent.h"
#include "ComponentSerialization.h"
#include "AudioManager.h"

using json = nlohmann::ordered_json;

const std::string PrefabFilePath { "../Assets/Prefabs/" };
const std::string SceneFilePath{ "../Assets/Scenes/" };
const std::string LayoutsFilePath{ "../Logs/EditorLayout/" };

namespace Serialization
{
    /**
     * @brief Temporary structure used during loading.
     *
     * Stores the relationship between a child object and its parent
     * BEFORE the actual parenting happens. Parenting is resolved
     * AFTER all GameObjects are fully created.
     */
    struct PendingHierarchyInfo {
        EntityID localID;   // The ID of this object
        EntityID parentID;  // The ID of its parent (-1 if root)
    };

    extern std::unordered_map<EntityID, GameObject*> idToObject;    // Maps EntityID to GameObject*
    extern std::vector<PendingHierarchyInfo> pendingHierarchy;      // Stores parent-child relationships during loading
    extern std::unordered_map<EntityID, EntityID> g_OldToNewID;

    std::vector<std::string> GetAllPrefabs();
    std::string GetFullPath(const std::string& filename);

    /**
     * @brief Serializes a GameObject and all its components into JSON format.
     * @param go Pointer to GameObject.
     * @return JSON object representing the serialized GameObject.
     */
    json SerializeGameObject(GameObject* go);

    /**
     * @brief Creates a GameObject from JSON data and adds it to the factory.
     * @param j JSON object describing the GameObject.
     */
    void DeserializeGameObject(const json& j);

    /**
    * @brief Saves a single GameObject as a prefab file.
    */
    void SavePrefab(GameObject& go);

    /**
    * @brief Loads a prefab from file and instantiates it into the scene.
    */
    void LoadPrefab(const std::string& path);

    GameObject* LoadPrefabWithReturn(const std::string& filename);

    /** 
    * @brief Saves all GameObjects in the current scene to a JSON file. 
    */
    void SaveScene(const std::string& existingPath);

    /** 
    * @brief Loads a complete scene from a JSON file. 
    */
    void LoadScene(const std::string& path);

    /** 
    * @brief Deletes a prefab file by name.
    */
    void RemovePrefab(std::string const& prefabName);

    /**
    * @brief Renames an existing prefab file.
    */
    void RenamePrefab(std::string const& oldName, std::string const& newName);

    /**
    * @brief Use full filepath name! Simply checks if file stream is valid & readable.
    */
    const bool IsStreamGood(std::string const& fullFilePath);

    /**
    * @brief Extern bool for checking if a something is currently being dragged and dropped. For DND from asset browser to viewport.
    */
    // audio serialisation
    json SerializeAudioManager(AudioManager* audioManager);
    void DeserializeAudioManager(const json& j, AudioManager* audioManager);

    extern bool Prefab_DragAndDropped;
    extern bool Prefab_Reinstantiate;

    /**
     * @brief After all GameObjects are created, attach them to their parents.
     */
    void ResolveHierarchy();
}
