/* Start Header ************************************************************************/
/*!
\file       Serialization.cpp
\author     Lee Hwee Min [100% - ?? lines]
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

	ComponentSerialization::SerializeAllComponents(go, j);
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

	go->SetActive(j.value("active", true));

	ComponentSerialization::DeserializeAllComponents(go, j);
}

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

		std::cout << "[LoadPrefab] Prefab instantiated successfully.\n";
	}
}

void Serialization::SaveScene(const std::string &scenePath)
{
	std::string path = SceneFilePath + scenePath + ".json";
	json j;
	j["GameObjects"] = json::array();

	// Get all GameObjects currently managed by factory
	const auto &objects = GameObjectFactory::Instance().GetAllGameObjects();

	for (const auto &obj : objects)
	{
		if (obj && obj->IsActive())
		{
			// Serialize and add to JSON array
			j["GameObjects"].push_back(Serialization::SerializeGameObject(obj.get()));
		}
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

void Serialization::LoadScene(const std::string& scenePath)
{

	auto& resourceMgr = ResourceManager::Instance();
	AudioEngine* audio = resourceMgr.GetAudioEngine();

	if (audio)
	{
		// Stop ALL sounds
		audio->stopAllSound();
		std::cout << "Stopped all exisiting sound" << std::endl;
	}

	// Unload all audio resources
	resourceMgr.UnloadAllAudio();
	std::cout << "unloaded all sound" << std::endl;

	// Destroy all GameObjects
	GameObjectFactory::Instance().DestroyAllGameObjects();

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

	if (j.contains("SceneSettings"))
	{
		const auto& settings = j["SceneSettings"];
		if (settings.contains("backgroundMusic"))
		{
			const auto& bgm = settings["backgroundMusic"];
			std::string name = bgm.value("name", "bgm");
			std::string audioPath = bgm.value("path", "");
			bool looping = bgm.value("looping", true);
			float volume = bgm.value("volume", 0.8f);
			std::string channel = bgm.value("channel", "bgm");

			if (!audioPath.empty() && audio)
			{
				resourceMgr.LoadAudio(name, audioPath, looping);
				audio->playSound(name, channel);
				audio->setVolume(channel, volume);
				std::cout << "[LoadScene] Loaded and playing BGM: " << name << std::endl;
			}
		}
	}

	// Load new GameObjects
	if (j.contains("GameObjects"))
	{
		for (const auto& obj : j["GameObjects"])
			Serialization::DeserializeGameObject(obj);

		std::cout << "[LoadScene] Scene GameObjects loaded successfully." << std::endl;
	}
}
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

void Serialization::RenamePrefab(std::string const &oldName, std::string const &newName)
{
	if (!std::filesystem::exists(PrefabFilePath + oldName + ".json"))
	{
		std::cout << "File Path " + PrefabFilePath + oldName + ".json\" cannot be found!\n";
		return;
	}

	if (!std::filesystem::exists(PrefabFilePath + newName + ".json"))
	{
		std::filesystem::rename(PrefabFilePath + oldName + ".json", PrefabFilePath + newName + ".json");
	}
	else
	{
		std::filesystem::rename(PrefabFilePath + oldName + ".json", PrefabFilePath + newName + "(1).json");
	}

	std::cout << "Prefab: " << oldName << " changed to " << newName << std::endl;
}

// void LoadPrefab(const std::string& path)
// {
// 	if (path == currentPrefabPath) {
// 		GameObjectFactory::Instance().Clear();
// 	}
// 	else if (!currentPrefabPath.empty()) {
// 		GameObjectFactory::Instance().Clear();
// 	}
// 	std::ifstream in(path);
// 	if (!in.is_open()) {
// 		std::cerr << "[LoadPrefab] Cannot open " << path << "\n";
// 		return;
// 	}

// 	json j;
// 	try { in >> j; }
// 	catch (const std::exception& e) {
// 		std::cerr << "[LoadPrefab] Parse error: " << e.what() << "\n";
// 		return;
// 	}

// 	if (j.contains("GameObjects")) {
// 		for (const auto& obj : j["GameObjects"])
// 			DeserializeGameObject(obj);
// 	}
// 	else {
// 		DeserializeGameObject(j);
// 	}

// 	previousPrefabPath = currentPrefabPath;
// 	currentPrefabPath = path;
// 	std::cout << "[LoadPrefab] Loaded prefab: " << path << "\n";
// }

// namespace PrefabTools
// {
// 	/**
//    * @brief Takes existing GO, and turns it into a prefab!!! As of right now, we are storing it in Logs! Will move to its own folder soon!
//    * @param GO Reference to Game Object to read data of
//    *      (Will require tweaking to read component data! But as of rn, basic data works!)
//    */
// 	void CreatePrefab(GameObject& GO)
// 	{
// 		std::string filePath{ PrefabFilePath + GO.GetName() + ".txt" }; // Create a textfile in Logs folder with GOs name
// 		std::ofstream ofs(filePath);                   // New ofs

// 		if (!ofs.good()) // If stream bad, stop process immediately!
// 		{
// 			std::cerr << "File Stream: " << filePath << " is not good!\n";
// 			return;
// 		}

// 		auto* transform = GO.GetComponent<TransformComponent>();
// 		auto* render = GO.GetComponent<RenderComponent>();
// 		vec4 color = render->GetColor();

// 		// format: Name, Shape Type, PosX, PosY, ScaleX, ScaleY, Colour RGBA
// 		ofs << GO.GetName() << " " << render->GetShapeTypeName() << " " << transform->posX << " " << transform->posY << " " << transform->scaleX << " " << transform->scaleY << " " << color.x << " " << color.y << " " << color.z << " " << color.w << std::endl;

// 		std::cout << "Prefab created successfully!";
// 		ofs.clear();
// 		ofs.close();
// 	}/**
//    * @brief Takes file path to read prefab data! Actual loading of prefab data is somewhere else, will shift into here soon for cleanliness!
//    * @param destination c string for destination of where to read prefab data from
//    */
// 	std::string ReadPrefabData(const char* destination)
// 	{
// 		std::ifstream ifs(destination); // Create ifstream of destination, check if its valid (existing prefab)
// 		if (!ifs.good())
// 		{
// 			std::cerr << "File Stream: " << destination << " is not good!\n";
// 			ifs.clear();
// 			ifs.close();
// 			return ""; // Return EMPTY. No data has been read (can change to "missing data" or something)
// 		}

// 		std::string data{};
// 		std::stringstream ss;

// 		ss << ifs.rdbuf(); // Read data obtained and store into string
// 		data = ss.str();

// 		ifs.clear();
// 		ifs.close();
// 		return data; // Return data obtained (white space separated)
// 	}

// 	std::vector<std::string> LoadPrefabList()
// 	{
// 		std::vector<std::string> temp;
// 		for (auto const& prefab : std::filesystem::directory_iterator(PrefabFilePath))
// 		{
// 			if (prefab.path().extension() == ".txt")
// 			{
// 				std::string prefabName = prefab.path().stem().string();
// 				temp.push_back(prefabName);
// 			}
// 		}
// 		temp.shrink_to_fit();
// 		return temp;
// 	}

// 	void RemovePrefab(std::string const& prefabName)
// 	{
// 		for (auto const& prefab : std::filesystem::directory_iterator(PrefabFilePath))
// 		{
// 			if (prefab.path().stem().string() == prefabName)
// 			{
// 				std::filesystem::remove(prefab);
// 			}
// 			else
// 			{
// 				std::cout << "Error! No matching prefab found!\n";
// 			}
// 		}
// 	}

// 	void RenamePrefab(std::string const& oldName, std::string const& newName)
// 	{
// 		if (!std::filesystem::exists(PrefabFilePath + oldName + ".txt"))
// 		{
// 			std::cout << "File Path " + PrefabFilePath + oldName + ".txt\" cannot be found!\n";
// 			return;
// 		}

// 		if (!std::filesystem::exists(PrefabFilePath + newName + ".txt"))
// 		{
// 			std::filesystem::rename(PrefabFilePath + oldName + ".txt", PrefabFilePath + newName + ".txt");
// 		}
// 		else
// 		{
// 			std::filesystem::rename(PrefabFilePath + oldName + ".txt", PrefabFilePath + newName + "(1).txt");
// 		}

// 		std::cout << "Prefab: " << oldName << " changed to " << newName << std::endl;
// 	}
//     std::cout << "[Serialization] Loaded prefab: " << path << "\n";
// }