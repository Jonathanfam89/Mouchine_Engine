#pragma once
#include "Component.h"
#include "GameObject.h"
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/object.h>
#include <mono/metadata/mono-config.h>
#include <mono/metadata/debug-helpers.h>
#include <string>
#include <memory>

/**
 * @brief Base class for all C# script components.
 * This acts as a bridge between C++ Components and C# MonoBehaviour-like scripts.
 *
 * Each derived class represents a specific C# script type and handles:
 * - Mono instance creation
 * - Lifecycle method invocation (OnStart, OnUpdate, OnDestroy)
 * - Memory management via GC handles
 *
 * Usage:
 *   class MyScriptComponent : public ScriptComponentBase
 *   {
 *       std::string GetScriptClassName() const override { return "MyScript"; }
 *       // ... implement required methods
 *   };
 */
class ScriptComponentBase : public IComponent
{
protected:
    // Mono runtime objects
    MonoObject* scriptInstance;     // The C# script instance
    MonoMethod* onStart;            // Cached OnStart method
    MonoMethod* onUpdate;           // Cached OnUpdate method
    MonoMethod* onDestroy;          // Cached OnDestroy method
    uint32_t gcHandle;              // GC handle to prevent collection

    // State tracking
    bool isStarted;                 // Has OnStart been called?

public:
    /**
     * @brief Constructor
     * @param owner The GameObject this component is attached to
     */
    ScriptComponentBase(GameObject* owner);

    /**
     * @brief Destructor - cleans up Mono resources
     */
    virtual ~ScriptComponentBase();

    /**
     * @brief Returns the C# script class name (e.g., "SpinningSquare")
     * Must be implemented by derived classes.
     */
    virtual std::string GetScriptClassName() const = 0;

    /**
     * @brief Initializes the script component.
     * Creates the Mono instance, caches methods, injects OwnerName, and calls OnStart.
     */
    void Initialize() override;

    /**
     * @brief Updates the script by calling its OnUpdate method.
     * @param dt Delta time in seconds
     */
    void Update(float dt) override;

    /**
     * @brief Destroys the script by calling its OnDestroy method.
     */
    void Destroy() override;

    /**
     * @brief Gets the Mono instance (for advanced use cases)
     */
    MonoObject* GetScriptInstance() const { return scriptInstance; }

    /**
     * @brief Checks if the script has been started
     */
    bool IsStarted() const { return isStarted; }

protected:
    /**
     * @brief Creates the Mono script instance and injects OwnerName.
     * @return True if successful, false otherwise
     */
    bool CreateScriptInstance();

    /**
     * @brief Caches the script's lifecycle methods (OnStart, OnUpdate, OnDestroy)
     */
    void CacheMethods();

    /**
     * @brief Calls OnStart if it hasn't been called yet
     */
    void CallOnStart();
};