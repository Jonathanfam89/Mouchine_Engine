#pragma once
#include <iostream>
#include <cmath>
#include <string>
#include "core.h"
#include "factory.h"
#include "GameObjectSystem.h"
#include "GraphicsSystem.h"
#include "ImGuiSystem.h"
#include "DebugSystem.h"
#include "Transform.h"
#include "Collision.h"
#include "Collidercomponent.h"
#include "Physics.h"
#include "RenderComponent.h"
#include "Animator.hpp"
#include "ResourceManager.h"
#include "audio.h"
#include "InputManager.h"
#include "gameObject.h"

enum Shape {
    SQUARE,
    CIRCLE,
    POINT,
    LINE
};

namespace API {
#pragma region Engine
    void Init_Engine(CoreEngine& engine, AudioEngine& audio);
    void StartEngineLoop(CoreEngine& engine);
#pragma endregion

#pragma region Creations
    void CreateShape(const std::string& name,
        RenderComponent::ShapeType type,
        float x, float y,
        float width, float height,
        vec4 color,
        CollisionLayer layer = CollisionLayer::Default,
        bool hasPhysics = false,
        bool isStatic = false,
        bool hasGravity = false);

    void CreateImage(const std::string& name,
        RenderComponent::ShapeType type,
        float x, float y,
        float width, float height,
        const std::string& texturePath);

    void CreateAnimatedImage(const std::string& name,
        float x, float y,
        float width, float height,
        const std::string& texturePath,
        int rows, int columns,
        int startFrame, int endFrame,
        float fps,
        CollisionLayer layer = CollisionLayer::Default,
        bool hasPhysics = false,
        bool isStatic = false,
        bool hasGravity = false);

#pragma endregion

#pragma region Graphics
    void SetShapeType(const std::string& objectName, RenderComponent::ShapeType);
    void SetTexture(const std::string& objectName, const char* filename);
    void setColor(const std::string& objectName, vec4 color);
    void SetAlpha(const std::string& objectName,float);
#pragma endregion

#pragma region Physics
    void GetVelocity(const std::string&);
    void SetVelocity(const std::string&, float, float);

    
    void SetStatic(const std::string&,bool);
    void SetJumpVal(const std::string&, float);
    void SetGravBool(const std::string&,bool); //set whether it have a Gravity

#pragma endregion

#pragma region Transform
    void SetPos(const std::string&, float, float);

    void SetScale(const std::string&, float);
    void SetScale(const std::string&, float, float);

    void SetRot(const std::string&, float);
    void AddRot(const std::string&, float);

    void GetRot(const std::string&);
    void GetPos(const std::string&);
    void GetScale(const std::string&);

#pragma endregion

#pragma region Components
    
    void AddTransformComponent(const std::string&);

    void AddRenderComponent(const std::string&);
    
    void AddCollisionComponent(const std::string&);
    
    void AddPhysicComponent(const std::string&);

#pragma endregion

#pragma region Collision
    void SetColisionShape(const std::string&, ColliderComponent::Type newShape);
    void SetCollisionLayer(const std::string&, CollisionLayer newlayer);
#pragma endregion

    //#pragma region InputSystem
    //    void GetMousePos();
    //    void GetKeyPressed();
    //    void GetKeyHold();
    //    void GetKeyRelease();
    //#pragma endregion
    //
    //#pragma region Audio
    //    void Init_AudioEngine();
    //    void PlaySound();
    //    void StopSound();
    //    void PauseSound();
    //    void SetBGM();
    //#pragma endregion










}