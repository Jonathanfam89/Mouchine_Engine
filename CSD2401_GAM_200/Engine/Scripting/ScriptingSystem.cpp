/* Start Header ************************************************************************/
/*!
\file       ScriptingSystem.cpp
\author     Jonathan Fam Qian Wei [100% - 825 lines]
\par        Fam.q@digipen.edu
\date       Novenber, 7, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "ScriptingSystem.h"
#include "gameObject.h"
#include "ScriptAPI.h"
#include <iostream>
#include <filesystem>

// Internal call functions - these will be called from C#

// === Debug ===
static void Internal_Log(MonoString *message)
{
    char *str = mono_string_to_utf8(message);
    std::cout << "[C# Script] " << str << std::endl;
    mono_free(str);
}

// === GameObject Creation ===
static void Internal_CreateShape(MonoString *name, int type, float x, float y,
                                 float width, float height,
                                 float r, float g, float b, float a,
                                 int layer, bool hasPhysics, bool isStatic, bool hasGravity)
{
    char *objName = mono_string_to_utf8(name);
    vec4 color = {r, g, b, a};
    API::CreateShape(objName, (RenderComponent::ShapeType)type, x, y, width, height,
                     color, (CollisionLayer)layer, hasPhysics, isStatic, hasGravity);
    mono_free(objName);
}

static void Internal_CreateImage(MonoString *name, int type, float x, float y,
                                 float width, float height, MonoString *texturePath)
{
    char *objName = mono_string_to_utf8(name);
    char *texPath = mono_string_to_utf8(texturePath);
    API::CreateImage(objName, (RenderComponent::ShapeType)type, x, y, width, height, texPath);
    mono_free(objName);
    mono_free(texPath);
}

static void Internal_CreateAnimatedImage(MonoString *name, float x, float y,
                                         float width, float height, MonoString *texturePath,
                                         int rows, int columns, int startRow, int startCol, int endRow, int endCol, float fps,
                                         int layer, bool hasPhysics, bool isStatic, bool hasGravity)
{
    char *objName = mono_string_to_utf8(name);
    char *texPath = mono_string_to_utf8(texturePath);
    API::CreateAnimatedImage(objName, x, y, width, height, texPath,
                             rows, columns, startRow, startCol, endRow, endCol, fps,
                             (CollisionLayer)layer, hasPhysics, isStatic, hasGravity);
    mono_free(objName);
    mono_free(texPath);
}

// === Transform ===
static void Internal_SetPosition(MonoString *objectName, float x, float y)
{
    char *name = mono_string_to_utf8(objectName);
    API::SetPos(name, x, y);
    mono_free(name);
}

static void Internal_SetRotation(MonoString *objectName, float rotation)
{
    char *name = mono_string_to_utf8(objectName);
    API::SetRot(name, rotation);
    mono_free(name);
}

static void Internal_AddRotation(MonoString *objectName, float rotation)
{
    char *name = mono_string_to_utf8(objectName);
    API::AddRot(name, rotation);
    mono_free(name);
}

static void Internal_SetScale_Single(MonoString *objectName, float scale)
{
    char *name = mono_string_to_utf8(objectName);
    API::SetScale(name, scale);
    mono_free(name);
}

static void Internal_SetScale_XY(MonoString* objectName, float x, float y)
{
    char* name = mono_string_to_utf8(objectName);
    API::SetScale(name, x, y);
    mono_free(name);
}

static float Internal_GetRotation(MonoString* objectName)
{
    char* name = mono_string_to_utf8(objectName);
    float result = API::GetRotation(name);
    mono_free(name);
    return result;
}

static void Internal_GetPosition(MonoString* objectName, float* x, float* y)
{
    char* name = mono_string_to_utf8(objectName);
    API::GetPosition(name, x, y);
    mono_free(name);
}

static void Internal_GetScale(MonoString* objectName, float* x, float* y)
{
    char* name = mono_string_to_utf8(objectName);
    API::GetScale(name, x, y);
    mono_free(name);
}

// === Graphics ===
static void Internal_SetShapeType(MonoString *objectName, int shapeType)
{
    char *name = mono_string_to_utf8(objectName);
    API::SetShapeType(name, (RenderComponent::ShapeType)shapeType);
    mono_free(name);
}

static void Internal_SetTexture(MonoString *objectName, MonoString *filename)
{
    char *name = mono_string_to_utf8(objectName);
    char *file = mono_string_to_utf8(filename);
    API::SetTexture(name, file);
    mono_free(name);
    mono_free(file);
}

static void Internal_SetColor(MonoString *objectName, float r, float g, float b, float a)
{
    char *name = mono_string_to_utf8(objectName);
    vec4 color = {r, g, b, a};
    API::setColor(name, color);
    mono_free(name);
}

static vec4 Internal_GetColor(MonoString* objectName)
{
    char* name = mono_string_to_utf8(objectName);
    vec4 result = API::GetColor(name);
    mono_free(name);
    return result;
}

static void Internal_SetAlpha(MonoString *objectName, float alpha)
{
    char *name = mono_string_to_utf8(objectName);
    API::SetAlpha(name, alpha);
    mono_free(name);
}

// === Physics ===
static void Internal_SetVelocity(MonoString *objectName, float vx, float vy)
{
    char *name = mono_string_to_utf8(objectName);
    API::SetVelocity(name, vx, vy);
    mono_free(name);
}

static void Internal_SetJumpValue(MonoString *objectName, float jumpForce)
{
    char *name = mono_string_to_utf8(objectName);
    API::SetJumpVal(name, jumpForce);
    mono_free(name);
}

static void Internal_SetGravity(MonoString *objectName, bool enabled)
{
    char *name = mono_string_to_utf8(objectName);
    API::SetGravBool(name, enabled);
    mono_free(name);
}


static vec2 Internal_GetVelocity(MonoString* objectName)
{
    char* name = mono_string_to_utf8(objectName);
    vec2 result = API::GetVelocity(name);
    mono_free(name);
    return result;
}

static void Internal_SetMass(MonoString* objectName, float mass)
{
    char* name = mono_string_to_utf8(objectName);
    API::SetMass(name, mass);
    mono_free(name);
}

static void Internal_SetFriction(MonoString* objectName, float friction)
{
    char* name = mono_string_to_utf8(objectName);
    API::SetFriction(name, friction);
    mono_free(name);
}

// === Collision ===
static void Internal_SetStatic(MonoString *objectName, bool isStatic)
{
    char *name = mono_string_to_utf8(objectName);
    API::SetStatic(name, isStatic);
    mono_free(name);
}

// === Collision ===
static void Internal_SetCollisionShape(MonoString *objectName, int shape)
{
    char *name = mono_string_to_utf8(objectName);
    API::SetColisionShape(name, (ColliderComponent::Type)shape);
    mono_free(name);
}

static void Internal_SetCollisionLayer(MonoString* objectName, int layer)
{
    char* name = mono_string_to_utf8(objectName);
    API::SetCollisionLayer(name, (CollisionLayer)layer);
}
static int Internal_GetCollisionLayer(MonoString* objectName)
{
    char* name = mono_string_to_utf8(objectName);
    int result = API::GetCollisionLayer(name);
    mono_free(name);
    return result;
}

static int Internal_GetCollisionShape(MonoString* objectName)
{
    char* name = mono_string_to_utf8(objectName);
    int result = API::GetCollisionShape(name);
    mono_free(name);
    return result;
}

// === Components ===
static bool Internal_AttachScript(MonoString* objectName, MonoString* scriptClassName)
{
    char* objName = mono_string_to_utf8(objectName);
    char* scriptName = mono_string_to_utf8(scriptClassName);
    bool result = API::AttachScript(objName, scriptName);
    mono_free(objName);
    mono_free(scriptName);
    return result;
}

static void Internal_SetComponentActive(MonoString* objectName, MonoString* componentName, bool active)
{
    char* objName = mono_string_to_utf8(objectName);
    char* compName = mono_string_to_utf8(componentName);
    API::SetComponentActive(objName, compName, active);
    mono_free(objName);
    mono_free(compName);
}

static void Internal_RemoveComponent(MonoString* objectName, MonoString* componentName)
{
    char* objName = mono_string_to_utf8(objectName);
    char* compName = mono_string_to_utf8(componentName);
    API::RemoveComponent(objName, compName);
    mono_free(objName);
    mono_free(compName);
}

static void Internal_AddTransformComponent(MonoString *objectName)
{
    char *name = mono_string_to_utf8(objectName);
    API::AddTransformComponent(name);
    mono_free(name);
}

static void Internal_AddRenderComponent(MonoString *objectName)
{
    char *name = mono_string_to_utf8(objectName);
    API::AddRenderComponent(name);
    mono_free(name);
}

static void Internal_AddCollisionComponent(MonoString *objectName)
{
    char *name = mono_string_to_utf8(objectName);
    API::AddCollisionComponent(name);
    mono_free(name);
}

static void Internal_AddPhysicsComponent(MonoString *objectName)
{
    char *name = mono_string_to_utf8(objectName);
    API::AddPhysicComponent(name);
    mono_free(name);
}

static void Internal_AddTextComponent(MonoString* objectName, MonoString* fontPath, unsigned int fontSize, MonoString* text, float r, float g, float b, float a, bool screenSpace)
{
    char* objName = mono_string_to_utf8(objectName);
    char* font = mono_string_to_utf8(fontPath);
    char* txt = mono_string_to_utf8(text);
    vec4 color = {r, g, b, a};
    API::AddTextComponent(objName, font, fontSize, txt, color, screenSpace);
    mono_free(objName);
    mono_free(font);
    mono_free(txt);
}

// === Text ===
static void Internal_SetText(MonoString* objectName, MonoString* text)
{
    char* name = mono_string_to_utf8(objectName);
    char* txt = mono_string_to_utf8(text);
    API::SetText(name, txt);
    mono_free(name);
    mono_free(txt);
}

static MonoString* Internal_GetText(MonoString* objectName)
{
    char* name = mono_string_to_utf8(objectName);
    std::string result = API::GetText(name);
    mono_free(name);
    return mono_string_new(mono_domain_get(), result.c_str());
}

static void Internal_SetTextColor(MonoString* objectName, float r, float g, float b, float a)
{
    char* name = mono_string_to_utf8(objectName);
    vec4 color = {r, g, b, a};
    API::SetTextColor(name, color);
    mono_free(name);
}

static vec4 Internal_GetTextColor(MonoString* objectName)
{
    char* name = mono_string_to_utf8(objectName);
    vec4 result = API::GetTextColor(name);
    mono_free(name);
    return result;
}

static void Internal_SetTextScale(MonoString* objectName, float scale)
{
    char* name = mono_string_to_utf8(objectName);
    API::SetTextScale(name, scale);
    mono_free(name);
}

static float Internal_GetTextScale(MonoString* objectName)
{
    char* name = mono_string_to_utf8(objectName);
    float result = API::GetTextScale(name);
    mono_free(name);
    return result;
}

static void Internal_SetTextScreenSpace(MonoString* objectName, bool screenSpace)
{
    char* name = mono_string_to_utf8(objectName);
    API::SetTextScreenSpace(name, screenSpace);
    mono_free(name);
}

static bool Internal_GetTextScreenSpace(MonoString* objectName)
{
    char* name = mono_string_to_utf8(objectName);
    bool result = API::GetTextScreenSpace(name);
    mono_free(name);
    return result;
}

static void Internal_AddSound(MonoString* name, MonoString* filepath, bool loop, float volume, float pitch,MonoString* category) {
    char* cName = mono_string_to_utf8(name);
    char* cPath = mono_string_to_utf8(filepath);
    char* cCat = mono_string_to_utf8(category);
    API::AddSound(cName, cPath, loop, volume, pitch, cCat);
    mono_free(cName);
    mono_free(cPath);
    mono_free(cCat);
}

static void Internal_PlaySound(MonoString* soundName, MonoString* channel) {
    char* cName = mono_string_to_utf8(soundName);
    char* cChannel = mono_string_to_utf8(channel);
    API::PlaySound(cName, cChannel);
    mono_free(cName);
    mono_free(cChannel);
}

static void Internal_StopSound(MonoString* soundName) {
    char* cName = mono_string_to_utf8(soundName);
    API::StopSound(cName);
    mono_free(cName);
}

static void Internal_StopAllSounds() {
    API::StopAllSound();
    
}

static void Internal_RemoveSound(MonoString* soundName) {
    char* cName = mono_string_to_utf8(soundName);
    API::RemoveSound(cName);
    mono_free(cName);

}

static void Internal_TriggerEvent(MonoString* eventName) {
    char* cName = mono_string_to_utf8(eventName);
    API::TriggerEvent(cName);
    mono_free(cName);
}

static void Internal_AddEventSound(MonoString* eventName, MonoString* soundName) {
    char* cEvent = mono_string_to_utf8(eventName);
    char* cSound = mono_string_to_utf8(soundName);
    API::AddEventSound(cEvent, cSound);
    mono_free(cEvent);
    mono_free(cSound);
}

static void Internal_SetVolume(MonoString* soundName, float volume) {
    char* cName = mono_string_to_utf8(soundName);
    API::SetVolume(cName, volume);
    mono_free(cName);
}

static void Internal_SetCategoryVolume(MonoString* catName, float volume) {
    char* cName = mono_string_to_utf8(catName);
    API::SetCategoryVolume(cName, volume);
    mono_free(cName);
}

// === Camera ===
static void Internal_SetCameraPosition(float x, float y) {
    vec2 pos = { x, y };
    API::setCameraPos(pos);
}

static void Internal_SetCameraZoom(float zoom) {
    API::setCameraZoom(zoom);
}

static vec2 Internal_GetCameraPosition()
{
    return API::GetCameraPosition();
}

static float Internal_GetCameraZoom()
{
    return API::GetCameraZoom();
}

static int Internal_GetCameraMode()
{
    return API::GetCameraMode();
}

static void Internal_SetCameraMode(int mode) {
    API::setCameraMode((CameraComponent::CameraMode)mode);
}

static bool Internal_Raycast(MonoString* selfName, float x, float y, float dx, float dy, float distance, MonoString** hitObjectName, float* hitDistance)
{
    return API::Raycast(x, y, dx, dy, distance, selfName, hitObjectName, hitDistance);
}

// === Collision Queries ===
static bool Internal_OnTriggerEnter(MonoString* obj1, MonoString* obj2) {
    char* n1 = mono_string_to_utf8(obj1);
    char* n2 = mono_string_to_utf8(obj2);
    bool result = API::OnTriggerEnter(n1, n2);
    mono_free(n1);
    mono_free(n2);
    return result;
}

static bool Internal_OnTriggerExit(MonoString* obj1, MonoString* obj2) {
    char* n1 = mono_string_to_utf8(obj1);
    char* n2 = mono_string_to_utf8(obj2);
    bool result = API::OnTriggerExit(n1, n2);
    mono_free(n1);
    mono_free(n2);
    return result;
}

static bool Internal_OnTriggerStay(MonoString* obj1, MonoString* obj2) {
    char* n1 = mono_string_to_utf8(obj1);
    char* n2 = mono_string_to_utf8(obj2);
    bool result = API::OnTriggerStay(n1, n2);
    mono_free(n1);
    mono_free(n2);
    return result;
}

static void Internal_PlayAnimation(MonoString* objectName, MonoString* animationName)
{
    char* objName = mono_string_to_utf8(objectName);
    char* animName = mono_string_to_utf8(animationName);
    API::PlayAnimation(objName, animName);
    mono_free(objName);
    mono_free(animName);
}

static void Internal_AddAnimationToQueue(MonoString* objectName, MonoString* animationName)
{
    char* objName = mono_string_to_utf8(objectName);
    char* animName = mono_string_to_utf8(animationName);
    API::AddAnimationToQueue(objName, animName);
    mono_free(objName);
    mono_free(animName);
}

static void Internal_RegisterAnimation(MonoString* objectName, MonoString* animationName, MonoString* texturePath, int rows, int cols, int startRow, int startCol, int endRow, int endCol, float fps)
{
    char* objName = mono_string_to_utf8(objectName);
    char* animName = mono_string_to_utf8(animationName);
    char* texPath = mono_string_to_utf8(texturePath);
    API::RegisterAnimation(objName, animName, texPath, rows, cols, startRow, startCol, endRow, endCol, fps);
    mono_free(objName);
    mono_free(animName);
    mono_free(texPath);
}

static void Internal_SetAnimation(MonoString* objectName, int startFrame, int endFrame, float fps)
{
    char* objName = mono_string_to_utf8(objectName);
    API::SetAnimation(objName, startFrame, endFrame, fps);
    mono_free(objName);
}

static MonoString* Internal_GetCurrentAnimationName(MonoString* objectName)
{
    char* objName = mono_string_to_utf8(objectName);
    std::string animName = API::GetCurrentAnimationName(objName);
    mono_free(objName);
    return mono_string_new(mono_domain_get(), animName.c_str());
}

// === Misc ===
static int Internal_RandomRange(int min, int max) {
    return API::RandomNumberGen(max, min); // Note: API args are (max, min)
}
static float Internal_GetDelta_Time() {
    return API::GetDelta_Time();
}
static float Internal_GetFixed_Delta_Time()
{
    return API::GetFixedDelta_time();
}
static float Internal_GetFramesPerSecond() {
    return API::GetFramePerSecond();
}


// === Input ===
static bool Internal_IsKeyPressed(int key)
{
    return API::IsKeyPressed(key);
}
static bool Internal_IsKeyReleased(int key)
{
    return API::IsKeyReleased(key);
}
static bool Internal_IsKeyHeld(int key)
{
    return API::IsKeyHeld(key);
}

static bool Internal_IsMouseButtonPressed(int button)
{
    return API::IsMouseButtonPressed(button);
}
static bool Internal_IsMouseButtonReleased(int button)
{
    return API::IsMouseButtonReleased(button);
}
static bool Internal_IsMouseButtonHeld(int button)
{
    return API::IsMouseButtonHeld(button);
}

static float Internal_GetMouseX()
{
    return API::GetMouseX();
}

static float Internal_GetMouseY()
{
    return API::GetMouseY();
}

static float Internal_GetMouseWorldX()
{
    return API::GetMouseWorldX();
}

static float Internal_GetMouseWorldY()
{
    return API::GetMouseWorldY();
}


ScriptSystem::~ScriptSystem()
{
    Shutdown();
}

void ScriptSystem::Initialize()
{
    std::cout << "\n--- Initializing Scripting System (Mono) ---" << std::endl;

    // Set Mono directories
    mono_set_dirs("mono/lib", "mono/etc");

    // Initialize Mono
    rootDomain = mono_jit_init("GameEngineRuntime");
    if (!rootDomain)
    {
        std::cerr << "Failed to initialize Mono JIT!" << std::endl;
        return;
    }

    // Create app domain for script reloading
    char domainName[] = "GameScriptDomain";
    appDomain = mono_domain_create_appdomain(domainName, nullptr);
    mono_domain_set(appDomain, true);

    // Register internal calls (C++ functions callable from C#)
    RegisterInternalCalls();

    // Load core assembly
    if (!LoadScriptAssembly("./Scripts/GameScripts.dll"))
    {
        std::cerr << "Warning: Could not load GameScripts.dll" << std::endl;
    }

    std::cout << "Scripting System initialized successfully" << std::endl;
}

void ScriptSystem::Shutdown()
{
    // Clean up all script instances
    for (auto &[gameObject, instances] : gameObjectScripts)
    {
        for (auto &instance : instances)
        {
            if (instance.gcHandle != 0)
            {
                mono_gchandle_free(instance.gcHandle);
            }
        }
    }
    gameObjectScripts.clear();
    scriptClasses.clear();

    // Shutdown Mono
    if (appDomain)
    {
        mono_domain_set(mono_get_root_domain(), false);
        mono_domain_unload(appDomain);
        appDomain = nullptr;
    }

    if (rootDomain)
    {
        mono_jit_cleanup(rootDomain);
        rootDomain = nullptr;
    }
}

void ScriptSystem::RegisterInternalCalls()
{
    // === Debug ===
    mono_add_internal_call("CSharpScript.Debug::Log", (void *)Internal_Log);

    // === GameObject Creation ===
    mono_add_internal_call("CSharpScript.GameObject::CreateShape", (void *)Internal_CreateShape);
    mono_add_internal_call("CSharpScript.GameObject::CreateImage", (void *)Internal_CreateImage);
    mono_add_internal_call("CSharpScript.GameObject::CreateAnimatedImage", (void *)Internal_CreateAnimatedImage);

    // === Transform ===
    mono_add_internal_call("CSharpScript.Transform::SetPosition", (void *)Internal_SetPosition);
    mono_add_internal_call("CSharpScript.Transform::SetRotation", (void *)Internal_SetRotation);
    mono_add_internal_call("CSharpScript.Transform::AddRotation", (void *)Internal_AddRotation);
    mono_add_internal_call("CSharpScript.Transform::SetScale_Single", (void *)Internal_SetScale_Single);
    mono_add_internal_call("CSharpScript.Transform::SetScale_XY", (void *)Internal_SetScale_XY);
    mono_add_internal_call("CSharpScript.Transform::GetPosition_Internal", (void*)Internal_GetPosition);
    mono_add_internal_call("CSharpScript.Transform::GetRotation", (void*)Internal_GetRotation);
    mono_add_internal_call("CSharpScript.Transform::GetScale_Internal", (void*)Internal_GetScale);

    // === Graphics ===
    mono_add_internal_call("CSharpScript.Graphics::SetShapeType", (void *)Internal_SetShapeType);
    mono_add_internal_call("CSharpScript.Graphics::SetTexture", (void *)Internal_SetTexture);
    mono_add_internal_call("CSharpScript.Graphics::SetColor", (void *)Internal_SetColor);
    mono_add_internal_call("CSharpScript.Graphics::SetAlpha", (void *)Internal_SetAlpha);
    mono_add_internal_call("CSharpScript.Graphics::GetColor", (void*)Internal_GetColor);

    // === Physics ===
    mono_add_internal_call("CSharpScript.Physics::SetVelocity", (void *)Internal_SetVelocity);
    mono_add_internal_call("CSharpScript.Physics::SetJumpValue", (void *)Internal_SetJumpValue);
    mono_add_internal_call("CSharpScript.Physics::SetGravity", (void *)Internal_SetGravity);
    mono_add_internal_call("CSharpScript.Physics::SetStatic", (void *)Internal_SetStatic);
    mono_add_internal_call("CSharpScript.Physics::GetVelocity", (void*)Internal_GetVelocity);
    mono_add_internal_call("CSharpScript.Physics::SetMass", (void*)Internal_SetMass);
    mono_add_internal_call("CSharpScript.Physics::SetFriction", (void*)Internal_SetFriction);

    // === Collision ===
    mono_add_internal_call("CSharpScript.Collision::SetCollisionShape", (void *)Internal_SetCollisionShape);
    mono_add_internal_call("CSharpScript.Collision::SetCollisionLayer", (void *)Internal_SetCollisionLayer);
    mono_add_internal_call("CSharpScript.Collision::GetCollisionLayer", (void*)Internal_GetCollisionLayer);
    mono_add_internal_call("CSharpScript.Collision::GetCollisionShape", (void*)Internal_GetCollisionShape);

    // === Scripting ===
    mono_add_internal_call("CSharpScript.Scripting::AttachScript", (void*)Internal_AttachScript);

    // === Components ===
    mono_add_internal_call("CSharpScript.Components::SetComponentActive", (void*)Internal_SetComponentActive);
    mono_add_internal_call("CSharpScript.Components::RemoveComponent", (void*)Internal_RemoveComponent);
    mono_add_internal_call("CSharpScript.Components::AddTransformComponent", (void *)Internal_AddTransformComponent);
    mono_add_internal_call("CSharpScript.Components::AddRenderComponent", (void *)Internal_AddRenderComponent);
    mono_add_internal_call("CSharpScript.Components::AddCollisionComponent", (void *)Internal_AddCollisionComponent);
    mono_add_internal_call("CSharpScript.Components::AddPhysicsComponent", (void *)Internal_AddPhysicsComponent);
    mono_add_internal_call("CSharpScript.Components::AddTextComponent", (void*)Internal_AddTextComponent);

    // === Text ===
    mono_add_internal_call("CSharpScript.Text::SetText", (void*)Internal_SetText);
    mono_add_internal_call("CSharpScript.Text::GetText", (void*)Internal_GetText);
    mono_add_internal_call("CSharpScript.Text::SetTextColor", (void*)Internal_SetTextColor);
    mono_add_internal_call("CSharpScript.Text::GetTextColor", (void*)Internal_GetTextColor);
    mono_add_internal_call("CSharpScript.Text::SetTextScale", (void*)Internal_SetTextScale);
    mono_add_internal_call("CSharpScript.Text::GetTextScale", (void*)Internal_GetTextScale);
    mono_add_internal_call("CSharpScript.Text::SetTextScreenSpace", (void*)Internal_SetTextScreenSpace);
    mono_add_internal_call("CSharpScript.Text::GetTextScreenSpace", (void*)Internal_GetTextScreenSpace);

    // === Audio ===
    mono_add_internal_call("CSharpScript.Audio::AddSound", (void*)Internal_AddSound);
    mono_add_internal_call("CSharpScript.Audio::PlaySound", (void*)Internal_PlaySound);
    mono_add_internal_call("CSharpScript.Audio::StopSound", (void*)Internal_StopSound);
    mono_add_internal_call("CSharpScript.Audio::StopAllSounds", (void*)Internal_StopAllSounds);
    mono_add_internal_call("CSharpScript.Audio::RemoveSound", (void*)Internal_RemoveSound);
    mono_add_internal_call("CSharpScript.Audio::TriggerEvent", (void*)Internal_TriggerEvent);
    mono_add_internal_call("CSharpScript.Audio::AddEventSound", (void*)Internal_AddEventSound);
    mono_add_internal_call("CSharpScript.Audio::SetVolume", (void*)Internal_SetVolume);
    mono_add_internal_call("CSharpScript.Audio::SetCategoryVolume", (void*)Internal_SetCategoryVolume);

    // === Animation ===
    mono_add_internal_call("CSharpScript.Animation::PlayAnimation", (void*)Internal_PlayAnimation);
    mono_add_internal_call("CSharpScript.Animation::AddToQueue", (void*)Internal_AddAnimationToQueue);
    mono_add_internal_call("CSharpScript.Animation::RegisterAnimation", (void*)Internal_RegisterAnimation);
    mono_add_internal_call("CSharpScript.Animation::SetAnimation", (void*)Internal_SetAnimation);
    mono_add_internal_call("CSharpScript.Animation::GetCurrentAnimationName", (void*)Internal_GetCurrentAnimationName);


    // === Camera ===
    mono_add_internal_call("CSharpScript.Camera::SetPosition", (void*)Internal_SetCameraPosition);
    mono_add_internal_call("CSharpScript.Camera::SetZoom", (void*)Internal_SetCameraZoom);
    mono_add_internal_call("CSharpScript.Camera::SetMode", (void*)Internal_SetCameraMode);
    mono_add_internal_call("CSharpScript.Camera::GetPosition", (void*)Internal_GetCameraPosition);
    mono_add_internal_call("CSharpScript.Camera::GetZoom", (void*)Internal_GetCameraZoom);
    mono_add_internal_call("CSharpScript.Camera::GetMode", (void*)Internal_GetCameraMode);

    // === Collision Queries ===
    mono_add_internal_call("CSharpScript.Collision::Raycast_Internal", (void*)Internal_Raycast);
    mono_add_internal_call("CSharpScript.Collision::OnTriggerEnter", (void*)Internal_OnTriggerEnter);
    mono_add_internal_call("CSharpScript.Collision::OnTriggerExit", (void*)Internal_OnTriggerExit);
    mono_add_internal_call("CSharpScript.Collision::OnTriggerStay", (void*)Internal_OnTriggerStay);

    // === Utils ===
    mono_add_internal_call("CSharpScript.Utils::RandomRange", (void*)Internal_RandomRange);
    mono_add_internal_call("CSharpScript.Utils::GetDelta_Time", (void*)Internal_GetDelta_Time);
    mono_add_internal_call("CSharpScript.Utils::GetFixed_Delta_Time", (void*)Internal_GetFixed_Delta_Time);
    mono_add_internal_call("CSharpScript.Utils::GetFramesPerSecond", (void*)Internal_GetFramesPerSecond);
	mono_add_internal_call("CSharpScript.Utils::MathfLerp", (void*)API::fLerp);
	mono_add_internal_call("CSharpScript.Utils::MathfInverseLerp", (void*)API::fInverseLerp);
	mono_add_internal_call("CSharpScript.Utils::MathfClamp", (void*)API::fClamp);
	mono_add_internal_call("CSharpScript.Utils::MathiClamp", (void*)API::iClamp);

    // === Input ===
    mono_add_internal_call("CSharpScript.Input::IsKeyPressed", (void*)Internal_IsKeyPressed);
    mono_add_internal_call("CSharpScript.Input::IsKeyReleased", (void*)Internal_IsKeyReleased);
    mono_add_internal_call("CSharpScript.Input::IsKeyHeld", (void*)Internal_IsKeyHeld);
    mono_add_internal_call("CSharpScript.Input::IsMouseButtonPressed", (void*)Internal_IsMouseButtonPressed);
    mono_add_internal_call("CSharpScript.Input::IsMouseButtonReleased", (void*)Internal_IsMouseButtonReleased);
    mono_add_internal_call("CSharpScript.Input::IsMouseButtonHeld", (void*)Internal_IsMouseButtonHeld);

    mono_add_internal_call("CSharpScript.Input::GetMouseX", (void*)Internal_GetMouseX);
    mono_add_internal_call("CSharpScript.Input::GetMouseY", (void*)Internal_GetMouseY);
    mono_add_internal_call("CSharpScript.Input::GetMouseWorldX", (void*)Internal_GetMouseWorldX);
    mono_add_internal_call("CSharpScript.Input::GetMouseWorldY", (void*)Internal_GetMouseWorldY);
}

MonoAssembly *ScriptSystem::LoadAssembly(const std::string &filepath)
{
    if (!std::filesystem::exists(filepath))
    {
        std::cerr << "Assembly file not found: " << filepath << std::endl;
        return nullptr;
    }

    MonoAssembly *assembly = mono_domain_assembly_open(appDomain, filepath.c_str());
    if (!assembly)
    {
        std::cerr << "Failed to load assembly: " << filepath << std::endl;
        return nullptr;
    }

    return assembly;
}

MonoClass *ScriptSystem::GetClassInAssembly(MonoAssembly *assembly, const char *namespaceName, const char *className)
{
    MonoImage *image = mono_assembly_get_image(assembly);
    MonoClass *monoClass = mono_class_from_name(image, namespaceName, className);

    if (!monoClass)
    {
        std::cerr << "Failed to find class: " << namespaceName << "." << className << std::endl;
    }

    return monoClass;
}

MonoObject *ScriptSystem::InstantiateClass(MonoClass *monoClass)
{
    MonoObject *instance = mono_object_new(appDomain, monoClass);

    if (!instance)
    {
        std::cerr << "Failed to instantiate class" << std::endl;
        return nullptr;
    }

    // Call constructor
    mono_runtime_object_init(instance);

    return instance;
}

bool ScriptSystem::LoadScriptAssembly(const std::string &filepath)
{
    coreAssembly = LoadAssembly(filepath);
    if (!coreAssembly)
    {
        return false;
    }

    coreImage = mono_assembly_get_image(coreAssembly);
    std::cout << "Loaded script assembly: " << filepath << std::endl;

    return true;
}

bool ScriptSystem::AttachScript(GameObject *gameObject, const std::string &scriptClassName)
{
    if (!gameObject || !coreAssembly)
    {
        return false;
    }

    // Check if class is already cached
    if (scriptClasses.find(scriptClassName) == scriptClasses.end())
    {
        // Load the class
        MonoClass *monoClass = GetClassInAssembly(coreAssembly, "GameScripts", scriptClassName.c_str());
        if (!monoClass)
        {
            return false;
        }

        // Cache the class and its methods
        ScriptClass scriptClass;
        scriptClass.monoClass = monoClass;
        scriptClass.constructor = mono_class_get_method_from_name(monoClass, ".ctor", 0);
        scriptClass.onStart = mono_class_get_method_from_name(monoClass, "OnStart", 0);
        scriptClass.onUpdate = mono_class_get_method_from_name(monoClass, "OnUpdate", 1);
        scriptClass.onDestroy = mono_class_get_method_from_name(monoClass, "OnDestroy", 0);

        scriptClasses[scriptClassName] = scriptClass;
    }

    // Create instance
    ScriptClass &scriptClass = scriptClasses[scriptClassName];
    MonoObject *instance = InstantiateClass(scriptClass.monoClass);

    if (!instance)
    {
        return false;
    }

    // Create script instance and add to GameObject
    ScriptInstance scriptInstance;
    scriptInstance.instance = instance;
    scriptInstance.scriptClass = &scriptClass;
    scriptInstance.gcHandle = mono_gchandle_new(instance, false);

    gameObjectScripts[gameObject].push_back(scriptInstance);

    std::cout << "Attached script " << scriptClassName << " to GameObject" << std::endl;

    return true;
}

void ScriptSystem::RemoveScript(GameObject *gameObject, const std::string &scriptClassName)
{
    auto it = gameObjectScripts.find(gameObject);
    if (it == gameObjectScripts.end())
    {
        return;
    }

    auto &instances = it->second;
    for (auto instIt = instances.begin(); instIt != instances.end(); ++instIt)
    {
        MonoClass *monoClass = mono_object_get_class(instIt->instance);
        const char *className = mono_class_get_name(monoClass);

        if (scriptClassName == className)
        {
            if (instIt->gcHandle != 0)
            {
                mono_gchandle_free(instIt->gcHandle);
            }
            instances.erase(instIt);
            break;
        }
    }
}

void ScriptSystem::CallOnStart(GameObject *gameObject)
{
    auto it = gameObjectScripts.find(gameObject);
    if (it == gameObjectScripts.end())
    {
        return;
    }

    for (auto &scriptInstance : it->second)
    {
        if (scriptInstance.scriptClass->onStart)
        {
            mono_runtime_invoke(scriptInstance.scriptClass->onStart, scriptInstance.instance, nullptr, nullptr);
        }
    }
}

void ScriptSystem::CallOnUpdate(GameObject *gameObject, float deltaTime)
{
    auto it = gameObjectScripts.find(gameObject);
    if (it == gameObjectScripts.end())
    {
        return;
    }

    void *args[1];
    args[0] = &deltaTime;

    for (auto &scriptInstance : it->second)
    {
        if (scriptInstance.scriptClass->onUpdate)
        {
            mono_runtime_invoke(scriptInstance.scriptClass->onUpdate, scriptInstance.instance, args, nullptr);
        }
    }
}

void ScriptSystem::CallOnDestroy(GameObject *gameObject)
{
    auto it = gameObjectScripts.find(gameObject);
    if (it == gameObjectScripts.end())
    {
        return;
    }

    for (auto &scriptInstance : it->second)
    {
        if (scriptInstance.scriptClass->onDestroy)
        {
            mono_runtime_invoke(scriptInstance.scriptClass->onDestroy, scriptInstance.instance, nullptr, nullptr);
        }
    }
}

void ScriptSystem::Update(float deltaTime)
{
    // Update all scripts attached to game objects
    for (auto &[gameObject, instances] : gameObjectScripts)
    {
        CallOnUpdate(gameObject, deltaTime);
    }
}

void ScriptSystem::ReloadScriptAssembly()
{
    std::cout << "Reloading script assembly..." << std::endl;

    for (auto &[gameObject, instances] : gameObjectScripts)
    {
        for (auto &instance : instances)
        {
            if (instance.gcHandle != 0)
            {
                mono_gchandle_free(instance.gcHandle);
            }
        }
    }
    // 2. Clear all cached data
    gameObjectScripts.clear();
    scriptClasses.clear();

    // 3. Unload the old domain
    mono_domain_set(rootDomain, false);
    mono_domain_unload(appDomain);

    // Create new domain
    char domainName[] = "GameScriptDomain";
    appDomain = mono_domain_create_appdomain(domainName, nullptr);
    mono_domain_set(appDomain, true);

    // Re-register all C++ functions for the new domain
    RegisterInternalCalls();

    // Reload the assembly
    LoadScriptAssembly("./Scripts/GameScripts.dll");

    std::cout << "Script assembly reloaded" << std::endl;
}