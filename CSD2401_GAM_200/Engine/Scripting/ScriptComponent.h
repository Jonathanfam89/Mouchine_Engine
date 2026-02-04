#pragma once

#include "Component.h"
#include "core.h"
#include "ScriptingSystem.h"
#include <string>

using ScriptID = std::uint32_t;
class ScriptComponent : public IComponent {

	public:
	ScriptComponent(GameObject* owner);
	~ScriptComponent() override = default;

	void Initialize() override;
	void Update(float deltaTime) override;
	void Destroy() override;

	std::unique_ptr<IComponent> Clone(GameObject* NewClone) const override;
	std::string GetName() const override { return "ScriptComponent"; }
	
	void AddScript(const std::string& Script_Name);

	void RemoveScript(ScriptID);
	void RemoveScript(const std::string& Script_Name);

	std::string idToName(ScriptID);
	ScriptID nameToID(const std::string &script_Name);

	const std::unordered_map<ScriptID, std::string>& GetScripts() const { return ActiveScript; }

private:
	std::string script_Name;
	ScriptID script_ID = 0;
	ScriptID NextScript_ID;
	std::unordered_map<ScriptID,std::string> ActiveScript;
};