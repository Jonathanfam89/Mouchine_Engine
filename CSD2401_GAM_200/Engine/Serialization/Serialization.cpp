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

#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;
static const std::string PREFAB_DIR = PrefabFilePath;

// Stores hierarchy information until all objects are created
struct PendingHierarchy
{
	GameObject* go;
	std::string parentName;
	int siblingIndex;
	json childrenJson;
};
static std::vector<PendingHierarchy> pendingList;

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
	if (!go)
		return j;

	j["name"] = go->GetName();
	j["active"] = go->IsActive();

	// Parent
	if (go->GetParent()) {
		j["parent"] = go->GetParent()->GetName();

		// find index within parent's children
		const auto& siblings = go->GetParent()->GetAllChildren();
		int index = 0;
		for (int i = 0; i < (int)siblings.size(); ++i)
		{
			if (siblings[i] == go)
			{
				index = i;
				break;
			}
		}
		j["siblingIndex"] = index;
	}
	else {
		j["parent"] = nullptr;	

		// find index in root list
		const auto& roots = GameObjectFactory::Instance().GetAllGameObjects();
		int index = 0;
		for (int i = 0; i < (int)roots.size(); ++i)
		{
			if (roots[i].get() == go)
			{
				index = i;
				break;
			}
		}
		j["siblingIndex"] = index;
	}

	// Components
	ComponentSerialization::SerializeAllComponents(go, j);

	// Children (recursive)
	j["children"] = json::array();
	for (GameObject* child : go->GetAllChildren())
	{
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
void Serialization::DeserializeGameObject(const json &j)
{
	GameObjectFactory &factory = GameObjectFactory::Instance();
	std::string name = j.value("name", "Unnamed");

	std::cout << "[Deserialize] Creating: " << name << std::endl;
	GameObject *go = factory.CreateGameObject(name);
	std::cout << "[Deserialize] Created GO at " << go << " (ID " << go->GetID() << ")\n";

#ifdef USE_IMGUI
	prefabInstances[name].push_back(go->GetID());						// keeps tracks of the newly created prefabs
#endif
	go->SetActive(j.value("active", true));

	ComponentSerialization::DeserializeAllComponents(go, j);

	// Store parent + children for later
	PendingHierarchy info;
	info.go = go;

	info.parentName = (j.contains("parent") && j["parent"].is_string())
		? j["parent"].get<std::string>()
		: "";
	info.siblingIndex = j.value("siblingIndex", 0);
	info.childrenJson = j.value("children", json::array());

	pendingList.push_back(info);
}

void Serialization::ResolveHierarchy()
{
	GameObjectFactory& factory = GameObjectFactory::Instance();

	// Recursively create children
	std::vector<json> childrenBuffer;
	for (auto& entry : pendingList)
		for (auto& c : entry.childrenJson)
			childrenBuffer.push_back(c);

	for (auto& childJson : childrenBuffer)
		DeserializeGameObject(childJson);

	// Assign actual parents
	for (auto& entry : pendingList)
	{
		if (!entry.parentName.empty())
		{
			GameObject* parent = factory.FindByName(entry.parentName);
			if (parent)
				parent->AddChild(entry.go);
		}
	}

	pendingList.clear();
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
		for (const auto &obj : j["GameObjects"])
			Serialization::DeserializeGameObject(obj);

		ResolveHierarchy();
		std::cout << "[LoadPrefab] Prefab instantiated successfully.\n";
	}
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
	pendingList.clear();

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
