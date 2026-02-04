#include "ScriptComponent.h"
#include "core.h"
#include "ScriptingSystem.h"
#include "GameObject.h"
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

ScriptComponent::ScriptComponent(GameObject* owner)
	: IComponent(owner), NextScript_ID(0){}
void ScriptComponent::Initialize()
{
	
	auto scriptSys = CORE->GetScriptSystem();

	for (const auto& [id, scriptName] : ActiveScript) {
		// Check if already attached to avoid duplicates
		// (This is for scripts added during scene load)
		bool success = scriptSys->AttachScript(owner, scriptName);
		if (success) {
			std::cout << "Initialized script: " << scriptName << std::endl;
		}
		else {
			std::cerr << "ScriptComponent Failed to initialize: " << scriptName << std::endl;
		}
	}

}

void ScriptComponent::Update(float dt)
{
	(void)dt;

	for (auto& slot : _propertySlot)
	{
		std::string data = slot.GetReceivedData();
		if (!data.empty())
		{
			// Extract script name from path
			fs::path scriptPath(data);
			std::string scriptName = scriptPath.stem().string();

			AddScript(scriptName);
			slot.ClearReceivedData();
		}
	}
}

void ScriptComponent::Destroy()
{
}

std::unique_ptr<IComponent> ScriptComponent::Clone(GameObject* NewClone) const
{
	auto newComp = std::make_unique<ScriptComponent>(NewClone);

	
	for (const auto& [id, scriptName] : ActiveScript)
	{
		newComp->AddScript(scriptName); 
	}

	return newComp;
}
void ScriptComponent::AddScript(const std::string& scriptName) {
	// Check if script already exists
	for (const auto& [id, existingName] : ActiveScript)
	{
		if (existingName == scriptName) {
			std::cout << "ScriptComponent already has " << scriptName << std::endl;
			return; // Exit early if duplicate
		}
	}

	// Add to our internal map
	ScriptID newID = NextScript_ID++;
	ActiveScript[newID] = scriptName;
	std::cout << "ScriptComponent Added " << scriptName << " with ID: " << newID << std::endl;

	// IMMEDIATELY attach to ScriptSystem if owner exists
	if (owner)
	{
		auto scriptSys = CORE->GetScriptSystem();
		if (scriptSys)
		{
			bool success = scriptSys->AttachScript(owner, scriptName);
			if (success)
			{
				std::cout << "Successfully attached " << scriptName << " to " << owner->GetName() << std::endl;

				// Call OnStart immediately after attachment
				scriptSys->CallOnStart(owner);
			}
			else
			{
				std::cerr << "Failed to attach " << scriptName << " to ScriptSystem" << std::endl;
			}
		}
		else
		{
			std::cerr << "ScriptSystem not available!" << std::endl;
		}
	}
}
void ScriptComponent::RemoveScript(const std::string& scriptName)
{
	ScriptID id = nameToID(scriptName);
	if (id != -1) // Assuming -1 or MAX_UINT is invalid
	{
		RemoveScript(id);
	}
}

void ScriptComponent::RemoveScript(ScriptID id)
{
	auto it = ActiveScript.find(id);
	if (it != ActiveScript.end())
	{
		std::cout << "ScriptComponent Removing " << it->second << " (ID: " << id << ")\n";
		ActiveScript.erase(it);
	}
}

std::string ScriptComponent::idToName(ScriptID id)
{
	auto it = ActiveScript.find(id);
	if (it != ActiveScript.end())
	{
		return it->second;
	}
	return ""; 
}

ScriptID ScriptComponent::nameToID(const std::string& name)
{
	// Linear search since map is keyed by ID
	for (auto& [id, scriptName] : ActiveScript)
	{
		if (scriptName == name)
		{
			return id;
		}
	}
	return (ScriptID) - 1;
}
