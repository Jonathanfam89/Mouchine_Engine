/* Start Header ************************************************************************/
/*!
\file       Serialization.h
\author     Muhammad Ashfer [100% - ?? lines]
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
#include <nlohmann/json.hpp>
#include "GameObject.h"
#include "RenderComponent.h"
#include "factory.h"
#include "ComponentSerialization.h"
#include "ResourceManager.h" 
#include "audio.h"           
using json = nlohmann::json;
const std::string PrefabFilePath { "../Assets/Prefabs/" };
const std::string SceneFilePath{ "../Assets/Scenes/" };

namespace Serialization
{
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

    void SavePrefab(GameObject& go);
    void LoadPrefab(const std::string& path);

    void SaveScene(const std::string& existingPath);
    void LoadScene(const std::string& path);

    void RemovePrefab(std::string const& prefabName);
    void RenamePrefab(std::string const& oldName, std::string const& newName);
}
