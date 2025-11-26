/* Start Header ************************************************************************/
/*!
\file       Serialization.cpp
\author     Lee Hwee Min [100% - ? lines]
\par        l.hweemin@digipen.edu
\date       November, 3, 2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "serialization.h"
#include "ComponentSerialization.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>

using namespace Serialization;
namespace fs = std::filesystem;

static const std::string PREFAB_DIR = PrefabFilePath;
static GameObject* s_CapturedRoot = nullptr;

// Global temporary loading maps (cleared after load)
std::unordered_map<EntityID, GameObject*> Serialization::idToObject{};
std::vector<PendingHierarchyInfo> Serialization::pendingHierarchy{};
std::unordered_map<EntityID, EntityID> Serialization::g_OldToNewID;

std::string Serialization::GetFullPath(const std::string &filename)
{
	return PREFAB_DIR + filename + ".json";
}

/**
 * @brief Returns all prefab JSON filenames found in the prefab directory.
 */
std::vector<std::string> Serialization::GetAllPrefabs()
{
	std::vector<std::string> temp;
	for (auto const &prefab : std::filesystem::directory_iterator(PrefabFilePath))
	{
		if (prefab.path().extension() == ".json")
		{
			std::string prefabName = prefab.path().stem().string();
			temp.push_back(prefabName);
		}
	}
	temp.shrink_to_fit();
	return temp;
}

/**
 * @brief Converts a GameObject and its components into a JSON object.
 *
 * Collects GameObject properties such as name and active state,
 * then delegates the serialization of all attached components to
 * ComponentSerialization.
 *
 * @param go Pointer to the GameObject to serialize.
 * @return JSON object representing the serialized GameObject.
 */
json Serialization::SerializeGameObject(GameObject *go)
{
	json j;
	if (!go) return j;

	j["name"] = go->GetName();
	j["localID"] = go->GetID();
	j["parentID"] = go->GetParent() ? go->GetParent()->GetID() : -1; // -1 if root

	j["active"] = go->IsActive();

	// Serialize components
	ComponentSerialization::SerializeAllComponents(go, j);

	// Serialize children in order
	j["children"] = json::array();
	for (GameObject* child : go->GetAllChildren()) {
		j["children"].push_back(SerializeGameObject(child));
	}

	return j;
}

/**
 * @brief Creates a GameObject from JSON data and registers it with the factory.
 *
 * Reads GameObject attributes such as name and active state, and reconstructs
 * all serialized components through ComponentSerialization.
 *
 * @param j JSON object describing the GameObject.
 */
void Serialization::DeserializeGameObject(const json& j)
{
	std::string name = j.value("name", "Unnamed");

	EntityID oldID = j.value("localID", -1);
	EntityID oldParentID = j.value("parentID", -1);

	// Check if ID already exists
	GameObjectFactory& factory = GameObjectFactory::Instance();
	//bool idExists = (oldID != -1 && factory.GetGameObject(oldID) != nullptr);

	EntityID newID = oldID;
	// Get smallest available ID map to GameObjects 
	newID = factory.GetSmallestAvailableID();   // assign new unique ID
	g_OldToNewID[oldID] = newID;         // store mapping

	// Create GameObject
	GameObject* go = factory.CreateGameObjectWithID(newID, name);
	idToObject[newID] = go;

	// Return GameObject for LoadPrefabWithReturn
	if (s_CapturedRoot == nullptr)
		s_CapturedRoot = go;


#ifdef USE_IMGUI
	prefabInstances[name].push_back(go->GetID());						// keeps tracks of the newly created prefabs
#endif
	go->SetActive(j.value("active", true));

	// Store parent-child link using ORIGINAL old parent ID
	pendingHierarchy.push_back({ oldID, oldParentID });

	// Deserialize components
	ComponentSerialization::DeserializeAllComponents(go, j);

	// Load children recursively
	if (j.contains("children"))
	{
		for (const auto& childJSON : j["children"])
			DeserializeGameObject(childJSON);
	}
}

void Serialization::ResolveHierarchy()
{
	for (auto& info : pendingHierarchy)
	{
		EntityID oldChildID = info.localID;
		EntityID oldParentID = info.parentID;

		EntityID newChildID = g_OldToNewID[oldChildID];
		EntityID newParentID = (oldParentID == -1 ? -1 : g_OldToNewID[oldParentID]);

		GameObject* child = idToObject[newChildID];

		if (newParentID != -1)
		{
			GameObject* parent = idToObject[newParentID];
			if (child && parent) {
				parent->AddChild(child);
			}
		}
	}

	idToObject.clear();
	pendingHierarchy.clear();
	g_OldToNewID.clear();
}


/**
 * @brief Saves a single GameObject as a prefab JSON file.
 * @param go Reference to the GameObject to save.
 */
void Serialization::SavePrefab(GameObject &go)
{
	// Make sure the prefab directory exists
	if (!fs::exists(PrefabFilePath))
		fs::create_directories(PrefabFilePath);

	// Construct full prefab path
	std::string path = PrefabFilePath + go.GetName() + ".json";

	// JSON object
	json j;
	j["GameObjects"] = json::array();

	// Serialize only this single GameObject
	j["GameObjects"].push_back(Serialization::SerializeGameObject(&go));

	// Write to disk
	std::ofstream out(path, std::ios::trunc);
	if (out.is_open())
	{
		out << j.dump(4);
		out.close();
		std::cout << "[SavePrefab] Prefab saved successfully: " << path << "\n";
	}
	else
	{
		std::cerr << "[SavePrefab] Failed to write prefab: " << path << "\n";
	}
}

/**
 * @brief Loads and instantiates a prefab from disk.
 * @param prefabName Name of the prefab file (without extension).
 */
void Serialization::LoadPrefab(const std::string &prefabName)
{
	// Construct full prefab file path
	std::string path = PrefabFilePath + prefabName + ".json";
	std::cout << "[LoadPrefab] Loading prefab from: " << path << "\n";

	// Verify the prefab exists
	if (!fs::exists(path))
	{
		std::cerr << "[LoadPrefab] Prefab not found: " << path << "\n";
		return;
	}

	// Read file
	std::ifstream in(path);
	if (!in.is_open())
	{
		std::cerr << "[LoadPrefab] Cannot open prefab: " << path << "\n";
		return;
	}

	json j;
	try
	{
		in >> j;
	}
	catch (const std::exception &e)
	{
		std::cerr << "[LoadPrefab] JSON Parse error: " << e.what() << "\n";
		return;
	}

	if (j.contains("GameObjects"))
	{
		if (Prefab_DragAndDropped)
		{
			// RESET the DND transform state for THIS prefab load
			DND_firstObject = true;
			DND_rootJsonX = 0.0f;
			DND_rootJsonY = 0.0f;
		}

		for (const auto& obj : j["GameObjects"])
			Serialization::DeserializeGameObject(obj);

		ResolveHierarchy();

	}
}

GameObject* Serialization::LoadPrefabWithReturn(const std::string& filename)
{
	s_CapturedRoot = nullptr;  // reset
	LoadPrefab(filename);      // existing void LoadPrefab function

	return s_CapturedRoot;     // filled by deserializer
}

/**
 * @brief Saves all active GameObjects in the scene to a JSON file.
 * @param scenePath Name of the scene file (without extension).
 */
void Serialization::SaveScene(const std::string &scenePath)
{
	GameObjectFactory& factory = GameObjectFactory::Instance();
	std::string path = SceneFilePath + scenePath + ".json";
	json j;
	j["GameObjects"] = json::array();

	// Get all GameObjects currently managed by factory
	const auto &objects = GameObjectFactory::Instance().GetAllGameObjects();

	for (const auto &obj : objects)
	{
		if (obj && obj->IsActive())
		{
			if (obj->GetParent() == nullptr)  // Only save top-level objects
			{
				// Serialize and add to JSON array
				j["GameObjects"].push_back(Serialization::SerializeGameObject(obj.get()));
			}
		}
	}

	// serialise audio manager
	if (CORE)
	{
		AudioManager* audioMgr = CORE->GetAudioManager();
		if (audioMgr)
		{
			j["AudioManager"] = SerializeAudioManager(audioMgr);
			std::cout << "[SaveScene] AudioManager configuration saved" << std::endl;
		}
		else
		{
			std::cerr << "[SaveScene] WARNING: AudioManager is NULL, no audio config saved" << std::endl;
		}
	}
	else
	{
		std::cerr << "[SaveScene] WARNING: CORE is NULL, cannot save AudioManager" << std::endl;
	}

	// Write scene JSON to disk
	std::ofstream out(path, std::ios::trunc);
	if (out.is_open())
	{
		out << j.dump(4);
		out.close();
		std::cout << "[SaveScene] Scene saved successfully :" << path << "\n";
	}

	else
	{
		std::cerr << "[SaveScene] Failed to open file for writing (saving): " << path << "\n";
	}
}

/**
 * @brief Loads a full scene from a JSON file, clearing current objects and audio.
 * @param scenePath Name of the scene file (without extension).
 */
void Serialization::LoadScene(const std::string& scenePath)
{
	GameObjectFactory& factory = GameObjectFactory::Instance();
	factory.Clear();

	std::string path = SceneFilePath + scenePath + ".json";
	if (!fs::exists(path))
	{
		std::cerr << "[LoadScene] Scene file not found: " << path << "\n";
		return;
	}

	std::ifstream in(path);
	if (!in.is_open())
	{
		std::cerr << "[LoadScene] Cannot open scene file: " << path << "\n";
		return;
	}

	json j;
	try
	{
		in >> j;
	}
	catch (const std::exception& e)
	{
		std::cerr << "[LoadScene] Parse error: " << e.what() << "\n";
		return;
	}

	// Clear all existing GameObjects before loading new scene
	std::cout << "[LoadScene] Clearing previous scene GameObjects...\n";
	GameObjectFactory::Instance().Clear();

	// Stop all currently playing audio and unload audio resources
	std::cout << "[LoadScene] Stopping all audio and clearing audio resources...\n";
	AudioEngine* audioEngine = ResourceManager::Instance().GetAudioEngine();
	if (audioEngine)
	{
		audioEngine->stopAllSound();
		audioEngine->unloadAllSound();
	}

	// Clear all audio resources from ResourceManager
	ResourceManager::Instance().UnloadAllAudio();

	if (j.contains("GameObjects"))
	{
		for (const auto& obj : j["GameObjects"])
			Serialization::DeserializeGameObject(obj);

		// load audio manager configuration
		if (CORE && j.contains("AudioManager")) 
		{
			AudioManager* audioMgr = CORE->GetAudioManager();
			if (audioMgr) 
			{
				DeserializeAudioManager(j, audioMgr);
				std::cout << "[LoadScene] AudioManager configuration loaded" << std::endl;
			}
		}
		std::cout << "[LoadScene] Scene loaded successfully: " << path << "\n";
	}
	ResolveHierarchy();
}

/**
 * @brief Deletes a prefab file by name.
 * @param prefabName Name of the prefab file (without extension).
 */
void Serialization::RemovePrefab(std::string const &prefabName)
{
	for (auto const &prefab : std::filesystem::directory_iterator(PrefabFilePath))
	{
		if (prefab.path().stem().string() == prefabName)
		{
			std::filesystem::remove(prefab);
		}
		else
		{
			std::cout << "Error! No matching prefab found!\n";
		}
	}
}

/**
 * @brief Renames an existing prefab file on disk.
 * @param oldName Current prefab file name (without extension).
 * @param newName New prefab file name (without extension).
 */
void Serialization::RenamePrefab(std::string const &oldName, std::string const &newName)
{
	std::string path = PrefabFilePath + oldName + ".json";
	std::string newPath = PrefabFilePath + newName + ".json";

	if (!std::filesystem::exists(path))
	{
		std::cout << "File Path: \"" << path << "\" cannot be found!\n";
		return;
	}

	if (std::filesystem::exists(newPath))
	{
		newPath = PrefabFilePath + newName + "(1).json";
	}

	std::filesystem::rename(path, newPath);

	std::ifstream in(newPath);
	json j;
	try
	{
		in >> j;
	}
	catch (const std::exception& e)
	{
		std::cerr << "[LoadPrefab] JSON Parse error: " << e.what() << "\n";
		return;
	}
	if (j.contains("GameObjects") && j["GameObjects"].is_array() && !j["GameObjects"].empty())
	{
		j["GameObjects"][0]["name"] = newName;
	}
	
	std::ofstream out(newPath);
	out << j.dump(4);

	std::cout << "Prefab: " << oldName << " changed to " << newName << std::endl;
}

/**
* @brief Use full filepath name! Simply checks if file stream is valid & readable.
*		 Nvm just found out that ifstream automatically closes fstreams but if u ever want to use this its here i guess
*/
const bool Serialization::IsStreamGood(std::string const& fullFilePath)
{
	// literally just checks if file is readable and closes filestreams properly.
	std::ifstream ifs(fullFilePath);
	if (ifs.good())
	{
		std::cout << "filepath: " << fullFilePath << " is good.\n";
		ifs.close();
		return true;
	}
	else
	{
		ifs.close();
		if (std::filesystem::exists(fullFilePath))
		{
			std::cout << "WARNING - unable to open: " << fullFilePath << " . Check file type.\n";
		}
		else
		{
			std::cout << "WARNING - filepath: " << fullFilePath << " DOES NOT EXIST! Check filepath name.\n";
		}
		return false;
	}
}

json Serialization::SerializeAudioManager(AudioManager* audioManager) 
{
	if (!audioManager) 
	{
		return json();
	}

	json j;
	j["sounds"] = json::array();
	j["events"] = json::array();

	// serialise sound
	for (const auto& pair : audioManager->GetSoundDatabase())
	{
		const SoundData& sound = pair.second;

		//volume
		float v = sound.volume;
		v = std::clamp(v, 0.0f, 1.0f);

		// pitch
		float p = sound.pitch;
		p = std::clamp(p, 0.5f, 2.0f);

		// format as strings with 2 decimal places
		std::ostringstream volStream;
		volStream << std::fixed << std::setprecision(2) << v;
		std::string volStr = volStream.str();

		std::ostringstream pitchStream;
		pitchStream << std::fixed << std::setprecision(2) << p;
		std::string pitchStr = pitchStream.str();

		json soundJson = 
		{ 
			{"name", sound.name},
			{"filepath", sound.filepath},
			{"loop", sound.loop},
			{"volume", v},
			{"pitch", p},
			{"category", sound.category}
		};
		j["sounds"].push_back(soundJson);
	}

	// serialise events
	for (const auto& pair : audioManager->GetEventMappings())
	{
		json eventJson = 
		{
			{"event", pair.first},
			{"sound", pair.second}
		};
		j["events"].push_back(eventJson);
	}
	return j;
}

void Serialization::DeserializeAudioManager(const json& j, AudioManager* audioManager)
{
	if (!audioManager || !j.contains("AudioManager"))
	{
		return;
	}

	const auto& audioJson = j["AudioManager"];

	audioManager->Clear();

	// deserialise sounds
	if (audioJson.contains("sounds")) 
	{
		for (const auto& soundJson : audioJson["sounds"]) 
		{
			SoundData sound;
			sound.name = soundJson.value("name", "");
			sound.filepath = soundJson.value("filepath", "");
			sound.loop = soundJson.value("loop", false);
			sound.volume = soundJson.value("volume", 1.0f);
			sound.pitch = soundJson.value("pitch", 1.0f);
			sound.category = soundJson.value("category", "sfx");

			audioManager->AddSound(sound);
		}
	}

	// deserialise events
	if (audioJson.contains("events")) 
	{
		for (const auto& eventJson : audioJson["events"]) 
		{
			std::string eventName = eventJson.value("event", "");
			std::string soundName = eventJson.value("sound", "");

			if (!eventName.empty() && !soundName.empty()) 
			{
				audioManager->AddEventMapping(eventName, soundName);
			}
		}
	}
	audioManager->LoadAllSounds();
}
