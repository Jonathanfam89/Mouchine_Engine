/* Start Header ************************************************************************/
/*!
\file       ScriptingSystem.h
\author     Jonathan Fam Qian Wei [100% - 49 lines]
\par        Fam.q@digipen.edu
\date       Novenber, 7, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/


#pragma once

#include "System.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <variant>
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/mono-config.h>

class GameObject;

// Forward declaration
struct ScriptInstance;

enum class ScriptFieldType {
    None,
    Float,
    Int,
    Bool,
    String,
    Vec2,
    Vec3,
    Vec4
};

struct ScriptField {
    std::string name;
    ScriptFieldType type;
    std::variant<float, int, bool, std::string> value;
};
/**
 * @brief Represents a C# script class loaded from a Mono assembly.
 */
struct ScriptClass {
    MonoClass* monoClass = nullptr;
    MonoMethod* constructor = nullptr;
    MonoMethod* onStart = nullptr;
    MonoMethod* onUpdate = nullptr;
    MonoMethod* onDestroy = nullptr;
};

/**
 * @brief Represents an instance of a script attached to a GameObject.
 */
struct ScriptInstance {
    MonoObject* instance = nullptr;
    ScriptClass* scriptClass = nullptr;
    uint32_t gcHandle = 0;
};

/**
 * @brief System responsible for managing C# scripting integration using Mono.
 */
class ScriptSystem : public ISystem {
private:
    MonoDomain* rootDomain = nullptr;
    MonoDomain* appDomain = nullptr;
    MonoAssembly* coreAssembly = nullptr;
    MonoImage* coreImage = nullptr;

    // Cache for loaded script classes
    std::unordered_map<std::string, ScriptClass> scriptClasses;

    // Map GameObject to their script instances
    std::unordered_map<GameObject*, std::vector<ScriptInstance>> gameObjectScripts;

    /**
     * @brief Loads a Mono assembly from a file path.
     * @param filepath The path to the assembly DLL file.
     * @return A pointer to the loaded MonoAssembly, or nullptr on failure.
     */
    MonoAssembly* LoadAssembly(const std::string& filepath);
    
    /**
     * @brief Retrieves a class from a Mono assembly by namespace and class name.
     * @param assembly The Mono assembly to search in.
     * @param namespaceName The namespace of the class.
     * @param className The name of the class.
     * @return A pointer to the MonoClass, or nullptr if not found.
     */
    MonoClass* GetClassInAssembly(MonoAssembly* assembly, const char* namespaceName, const char* className);
    
    /**
     * @brief Creates an instance of a Mono class.
     * @param monoClass The Mono class to instantiate.
     * @return A pointer to the created MonoObject, or nullptr on failure.
     */
    MonoObject* InstantiateClass(MonoClass* monoClass);
    
    /**
     * @brief Registers internal C++ function calls that can be invoked from C# scripts.
     */
    void RegisterInternalCalls();

public:
    /**
     * @brief Destructor that cleans up Mono resources.
     */
    ~ScriptSystem() override;

    /**
     * @brief Initializes the scripting system and Mono runtime.
     */
    void Initialize() override;
    
    /**
     * @brief Shuts down the scripting system and cleans up Mono domains.
     */
    void Shutdown();

    /**
     * @brief Gets the name of this system.
     * @return The system name as a string.
     */
    std::string GetName() override {
        return "ScriptingSystem";
    }

    /**
     * @brief Updates all script instances, calling OnUpdate methods.
     * @param deltaTime The time elapsed since the last frame.
     */
    void Update(float deltaTime) override;

    /**
     * @brief Loads a C# script assembly from a file.
     * @param filepath The path to the assembly DLL file.
     * @return True if the assembly was loaded successfully, false otherwise.
     */
    bool LoadScriptAssembly(const std::string& filepath);

    /**
     * @brief Attaches a script to a GameObject by class name.
     * @param gameObject The GameObject to attach the script to.
     * @param scriptClassName The fully qualified name of the script class (namespace.classname).
     * @return True if the script was attached successfully, false otherwise.
     */
    bool AttachScript(GameObject* gameObject, const std::string& scriptClassName);

    /**
     * @brief Removes a script from a GameObject.
     * @param gameObject The GameObject to remove the script from.
     * @param scriptClassName The name of the script class to remove.
     */
    void RemoveScript(GameObject* gameObject, const std::string& scriptClassName);

    /**
     * @brief Calls the OnStart method for all scripts attached to a GameObject.
     * @param gameObject The GameObject whose scripts should have OnStart called.
     */
    void CallOnStart(GameObject* gameObject);
    
    /**
     * @brief Calls the OnUpdate method for all scripts attached to a GameObject.
     * @param gameObject The GameObject whose scripts should have OnUpdate called.
     * @param deltaTime The time elapsed since the last frame.
     */
    void CallOnUpdate(GameObject* gameObject, float deltaTime);
    
    /**
     * @brief Calls the OnDestroy method for all scripts attached to a GameObject.
     * @param gameObject The GameObject whose scripts should have OnDestroy called.
     */
    void CallOnDestroy(GameObject* gameObject);

    /**
     * @brief Reloads the script assembly, enabling hot reload functionality.
     */
    void ReloadScriptAssembly();

    /**
    * @brief Creates a C# script instance and injects the owner name
    * @param scriptClassName The name of the C# class (e.g., "SpinningSquare")
    * @param owner The GameObject that owns this script
    * @return MonoObject* pointer to the created instance, or nullptr on failure
    */
    MonoObject* CreateScriptInstance(const std::string& scriptClassName, GameObject* owner);

    std::vector<ScriptField> GetPublicFields(ScriptInstance& instance);
    
    void SetPublicField(ScriptInstance& instance, const std::string& fieldName, const float& value);
    void SetPublicField(ScriptInstance& instance, const std::string& fieldName, const int& value);
    void SetPublicField(ScriptInstance& instance, const std::string& fieldName, const bool& value);
    void SetPublicField(ScriptInstance& instance, const std::string& fieldName, const std::string& value);

    /**
     * @brief Gets the core assembly for direct class access
     */
    MonoAssembly* GetCoreAssembly() const { return coreAssembly; }

    /**
     * @brief Gets the core image for direct class access
     */
    MonoImage* GetCoreImage() const { return coreImage; }
    
    /**
    * @brief Returns the script instances for a given GameObject
    */
    std::vector<ScriptInstance>* GetScriptInstances(GameObject* gameObject) {
        if (gameObjectScripts.find(gameObject) != gameObjectScripts.end()) {
            return &gameObjectScripts[gameObject];
        }
        return nullptr;
    }
};