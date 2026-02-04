#include "ScriptComponentBase.h"
#include "../Core/core.h"
#include "ScriptingSystem.h"

ScriptComponentBase::ScriptComponentBase(GameObject* owner)
    : IComponent(owner)
    , scriptInstance(nullptr)
    , onStart(nullptr)
    , onUpdate(nullptr)
    , onDestroy(nullptr)
    , gcHandle(0)
    , isStarted(false)
{
}

ScriptComponentBase::~ScriptComponentBase()
{
    if (gcHandle != 0)
    {
        mono_gchandle_free(gcHandle);
        gcHandle = 0;
    }
}

void ScriptComponentBase::Initialize()
{
    if (CreateScriptInstance())
    {
        CallOnStart();
    }
}

void ScriptComponentBase::Update(float dt)
{
    if (!enabled || !scriptInstance) return;

    // Ensure OnStart is called before the first update
    if (!isStarted)
    {
        CallOnStart();
    }

    if (onUpdate)
    {
        void* args[1];
        args[0] = &dt;
        mono_runtime_invoke(onUpdate, scriptInstance, args, nullptr);
    }
}

void ScriptComponentBase::Destroy()
{
    if (scriptInstance && onDestroy)
    {
        mono_runtime_invoke(onDestroy, scriptInstance, nullptr, nullptr);
    }
}

bool ScriptComponentBase::CreateScriptInstance()
{
    if (!CORE || !CORE->GetScriptSystem())
    {
        return false;
    }

    // Use the ScriptSystem to create the instance and inject dependencies
    scriptInstance = CORE->GetScriptSystem()->CreateScriptInstance(GetScriptClassName(), owner);

    if (scriptInstance)
    {
        // Create a GC handle to prevent garbage collection
        gcHandle = mono_gchandle_new(scriptInstance, false);
        
        CacheMethods();
        return true;
    }

    return false;
}

void ScriptComponentBase::CacheMethods()
{
    if (!scriptInstance) return;

    MonoClass* instanceClass = mono_object_get_class(scriptInstance);
    
    onStart = mono_class_get_method_from_name(instanceClass, "OnStart", 0);
    onUpdate = mono_class_get_method_from_name(instanceClass, "OnUpdate", 1);
    onDestroy = mono_class_get_method_from_name(instanceClass, "OnDestroy", 0);
}

void ScriptComponentBase::CallOnStart()
{
    if (scriptInstance && onStart && !isStarted)
    {
        mono_runtime_invoke(onStart, scriptInstance, nullptr, nullptr);
        isStarted = true;
    }
}