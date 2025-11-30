/* Start Header ************************************************************************/
/*!
\file       ScriptAPI.h
\author     Jonathan Fam Qian Wei [100% - 184 lines]
\par        Fam.q@digipen.edu
\date       Novenber, 7, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/


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
#include "CollisionSystem.h"
#include "Physics.h"
#include "RenderComponent.h"
#include "TextComponent.h"
#include "Animator.hpp"
#include "ResourceManager.h"
#include "audio.h"
#include "InputManager.h"
#include "gameObject.h"
#include <random>
#include "CameraComponent.h"
#include "CameraManager.h"
#include <functional>
#include "MathLib.h"
#include "InputManager.h"

    using TimerCallback = std::function<void()>;
    using TimerID = unsigned int;


/**
 * @brief Enumeration of shape types for rendering.
 */
enum Shape {
    SQUARE,
    CIRCLE,
    POINT,
    LINE
};

/**
 * @brief Namespace containing API functions accessible from C# scripts.
 */
namespace API {
#pragma region Engine
    /**
     * @brief Initializes the engine with the provided CoreEngine and AudioEngine instances.
     * @param engine Reference to the CoreEngine instance.
     * @param audio Reference to the AudioEngine instance.
     */
    void Init_Engine(CoreEngine& engine, AudioEngine& audio);
    
    /**
     * @brief Starts the main engine game loop.
     * @param engine Reference to the CoreEngine instance.
     */
    void StartEngineLoop(CoreEngine& engine);

#pragma endregion

#pragma region Creations
    /**
     * @brief Creates a shape GameObject with specified properties.
     * @param name The name of the GameObject to create.
     * @param type The shape type to render.
     * @param x The initial X position.
     * @param y The initial Y position.
     * @param width The width of the shape.
     * @param height The height of the shape.
     * @param color The color of the shape (RGBA).
     * @param layer The collision layer for the shape (default: Default).
     * @param hasPhysics Whether the shape should have physics (default: false).
     * @param isStatic Whether the shape is static (default: false).
     * @param hasGravity Whether the shape is affected by gravity (default: false).
     */
    void CreateShape(const std::string& name,
        RenderComponent::ShapeType type,
        float x, float y,
        float width, float height,
        vec4 color,
        CollisionLayer layer = CollisionLayer::Default,
        bool hasPhysics = false,
        bool isStatic = false,
        bool hasGravity = false);

    /**
     * @brief Creates an image GameObject with a texture.
     * @param name The name of the GameObject to create.
     * @param type The shape type for the image.
     * @param x The initial X position.
     * @param y The initial Y position.
     * @param width The width of the image.
     * @param height The height of the image.
     * @param texturePath The file path to the texture image.
     */
    void CreateImage(const std::string& name,
        RenderComponent::ShapeType type,
        float x, float y,
        float width, float height,
        const std::string& texturePath);

    /**
     * @brief Creates an animated image GameObject with sprite sheet animation.
     * @param name The name of the GameObject to create.
     * @param x The initial X position.
     * @param y The initial Y position.
     * @param width The width of the image.
     * @param height The height of the image.
     * @param texturePath The file path to the sprite sheet texture.
     * @param rows The number of rows in the sprite sheet.
     * @param columns The number of columns in the sprite sheet.
     * @param startFrame The starting frame index for the animation.
     * @param endFrame The ending frame index for the animation.
     * @param fps The frames per second for the animation.
     * @param layer The collision layer for the image (default: Default).
     * @param hasPhysics Whether the image should have physics (default: false).
     * @param isStatic Whether the image is static (default: false).
     * @param hasGravity Whether the image is affected by gravity (default: false).
     */
    void CreateAnimatedImage(const std::string& name,
        float x, float y,
        float width, float height,
        const std::string& texturePath,
        int rows, int columns,
        int startRow, int startCol, int endRow, int endCol,
        float fps,
        CollisionLayer layer = CollisionLayer::Default,
        bool hasPhysics = false,
        bool isStatic = false,
        bool hasGravity = false);

#pragma endregion

#pragma region Graphics
    vec4 GetColor(const std::string& objectName);

    /**
     * @brief Sets the shape type for a GameObject's render component.
     * @param objectName The name of the GameObject.
     * @param type The new shape type to set.
     */
    void SetShapeType(const std::string& objectName, RenderComponent::ShapeType);
    
    /**
     * @brief Sets the texture for a GameObject.
     * @param objectName The name of the GameObject.
     * @param filename The file path to the texture image.
     */
    void SetTexture(const std::string& objectName, const char* filename);
    
    /**
     * @brief Sets the color of a GameObject.
     * @param objectName The name of the GameObject.
     * @param color The new color (RGBA).
     */
    void setColor(const std::string& objectName, vec4 color);
    
    /**
     * @brief Sets the alpha (transparency) value for a GameObject.
     * @param objectName The name of the GameObject.
     * @param alpha The alpha value (0.0 to 1.0).
     */
    void SetAlpha(const std::string& objectName,float);
    
    /**
     * @brief Sets the background using a texture image.
     * @param filename The file path to the background texture.
     */
    void SetBackGround(char const*);
    
    /**
     * @brief Sets the background using a solid color.
     * @param color The background color (RGBA).
     */
    void SetBackGround(vec4);
#pragma endregion

#pragma region Physics
    /**
     * @brief Gets the velocity of a GameObject.
     * @param objectName The name of the GameObject.
     */
    vec2 GetVelocity(const std::string&);
    
    /**
     * @brief Sets the velocity of a GameObject.
     * @param objectName The name of the GameObject.
     * @param x The X component of the velocity.
     * @param y The Y component of the velocity.
     */
    void SetVelocity(const std::string&, float, float);

    /**
     * @brief Sets whether a GameObject is static (immovable).
     * @param objectName The name of the GameObject.
     * @param isStatic True if the object should be static, false otherwise.
     */
    void SetStatic(const std::string&,bool);
    
    /**
     * @brief Sets the jump velocity value for a GameObject.
     * @param objectName The name of the GameObject.
     * @param jumpVal The jump velocity value.
     */
    void SetJumpVal(const std::string&, float);
    
    /**
     * @brief Sets whether a GameObject is affected by gravity.
     * @param objectName The name of the GameObject.
     * @param hasGravity True if the object should be affected by gravity, false otherwise.
     */
    void SetGravBool(const std::string&,bool);

    void SetFriction(const std::string&, float);
    void SetMass(const std::string&, float);
#pragma endregion

#pragma region Transform
    /**
     * @brief Sets the position of a GameObject.
     * @param objectName The name of the GameObject.
     * @param x The X coordinate.
     * @param y The Y coordinate.
     */
    void SetPos(const std::string&, float, float);

    /**
     * @brief Sets the uniform scale of a GameObject (same for X and Y).
     * @param objectName The name of the GameObject.
     * @param scale The scale value for both X and Y axes.
     */
    void SetScale(const std::string&, float);
    
    /**
     * @brief Sets the scale of a GameObject with separate X and Y values.
     * @param objectName The name of the GameObject.
     * @param scaleX The scale value for the X axis.
     * @param scaleY The scale value for the Y axis.
     */
    void SetScale(const std::string&, float, float);

    /**
     * @brief Sets the rotation of a GameObject.
     * @param objectName The name of the GameObject.
     * @param rotation The rotation angle in radians.
     */
    void SetRot(const std::string&, float);
    
    /**
     * @brief Adds to the current rotation of a GameObject.
     * @param objectName The name of the GameObject.
     * @param rotation The rotation angle to add (in radians).
     */
    void AddRot(const std::string&, float);

    /**
     * @brief Gets the rotation of a GameObject.
     * @param objectName The name of the GameObject.
     */
        void GetPosition(const std::string&, float* x, float* y);
        
        /**
         * @brief Gets the scale of a GameObject.
         * @param objectName The name of the GameObject.
         */
        void GetScale(const std::string&, float* x, float* y);
    float GetRotation(const std::string&);

#pragma endregion

#pragma region Text
    /**
     * @brief Sets the text content of a GameObject's TextComponent.
     * @param objectName The name of the GameObject.
     * @param text The text content to set.
     */
    void SetText(const std::string&, const std::string&);

    /**
     * @brief Gets the text content of a GameObject's TextComponent.
     * @param objectName The name of the GameObject.
     */
    std::string GetText(const std::string&);

    /**
     * @brief Sets the text color of a GameObject's TextComponent.
     * @param objectName The name of the GameObject.
     * @param color The color (RGBA).
     */
    void SetTextColor(const std::string&, vec4);

    /**
     * @brief Gets the text color of a GameObject's TextComponent.
     * @param objectName The name of the GameObject.
     */
    vec4 GetTextColor(const std::string&);

    /**
     * @brief Sets the text scale of a GameObject's TextComponent.
     * @param objectName The name of the GameObject.
     * @param scale The scale factor.
     */
    void SetTextScale(const std::string&, float);

    /**
     * @brief Gets the text scale of a GameObject's TextComponent.
     * @param objectName The name of the GameObject.
     */
    float GetTextScale(const std::string&);

    /**
     * @brief Sets whether the text is rendered in screen space.
     * @param objectName The name of the GameObject.
     * @param screenSpace True for screen space, false for world space.
     */
    void SetTextScreenSpace(const std::string&, bool);

    /**
     * @brief Gets whether the text is rendered in screen space.
     * @param objectName The name of the GameObject.
     */
    bool GetTextScreenSpace(const std::string&);

#pragma endregion

#pragma region Components
    void SetComponentActive(const std::string& objectName, const std::string& componentName, bool active);
    void RemoveComponent(const std::string& objectName, const std::string& componentName);

    /**
     * @brief Adds a Transform component to a GameObject.
     * @param objectName The name of the GameObject.
     */
    void AddTransformComponent(const std::string&);

    /**
     * @brief Adds a Render component to a GameObject.
     * @param objectName The name of the GameObject.
     */
    void AddRenderComponent(const std::string&);
    
    /**
     * @brief Adds a Collision component to a GameObject.
     * @param objectName The name of the GameObject.
     */
    void AddCollisionComponent(const std::string&);
    
    /**
     * @brief Adds a Physics component to a GameObject.
     * @param objectName The name of the GameObject.
     */
    void AddPhysicComponent(const std::string&);

    /**
     * @brief Adds a Text component to a GameObject.
     * @param objectName The name of the GameObject.
     * @param fontPath The file path to the font.
     * @param fontSize The size of the font.
     * @param text The initial text content.
     * @param color The text color (RGBA).
     * @param screenSpace Whether to render in screen space.
     */
    void AddTextComponent(const std::string&, const std::string&, unsigned int, const std::string&, vec4, bool);

#pragma endregion

#pragma region Collision
    bool Raycast(float x, float y, float dx, float dy, float distance, MonoString* selfName, MonoString** hitObjectName, float* hitDistance);
    int GetCollisionLayer(const std::string& objectName);
    int GetCollisionShape(const std::string& objectName);

    /**
     * @brief Sets the collision shape type for a GameObject.
     * @param objectName The name of the GameObject.
     * @param newShape The new collision shape type.
     */
    void SetColisionShape(const std::string&, ColliderComponent::Type newShape);
    
    /**
     * @brief Sets the collision layer for a GameObject.
     * @param objectName The name of the GameObject.
     * @param newlayer The new collision layer.
     */
    void SetCollisionLayer(const std::string&, CollisionLayer newlayer);

    bool OnTriggerEnter(const std::string, const std::string);
    bool OnTriggerExit(const std::string, const std::string);
    bool OnTriggerStay(const std::string, const std::string);

#pragma endregion

#pragma region Audio
    void AddSound(const std::string, const std::string, bool, float, float,std::string);
    void PlaySound(const std::string,const std::string);
    void StopSound(const std::string);
    void StopAllSound();
    void RemoveSound(const std::string);

    void TriggerEvent(const std::string);
    void AddEventSound(const std::string, const std::string);
   
    void SetVolume(const std::string,float);
    void SetCategoryVolume(const std::string,float);
#pragma endregion


#pragma region Timer
    TimerID WaitForSeconds(float duration, TimerCallback callback);
    TimerID Delay(float duration, TimerCallback callback);
    TimerID InvokeRepeating(float interval, TimerCallback callback);
    TimerID Invoke(float delay, void (*functionToCall)());
    TimerID Schedule(float delay, std::function<void()> callback);
    void CancelTimer(TimerID id);
    void CancelTimerByName(std::string name);
    void PauseTimer(TimerID id);
    void ResumeTimer(TimerID id);
#pragma endregion

#pragma region Camera
    vec2 GetCameraPosition();
    float GetCameraZoom();
    int GetCameraMode();
    void setCameraPos(vec2);
    void setCameraZoom(float);
    void setCameraMode(CameraComponent::CameraMode);

#pragma endregion

#pragma region Animator
    void PlayAnimation(const std::string& objectName, const std::string& animationName);
    void AddAnimationToQueue(const std::string& objectName, const std::string& animationName);
    void RegisterAnimation(const std::string& objectName, const std::string& animationName, const std::string& texturePath, int rows, int cols, int startRow, int startCol, int endRow, int endCol, float fps);
    void SetAnimation(const std::string& objectName, int startFrame, int endFrame, float fps);
    std::string GetCurrentAnimationName(const std::string& objectName);
#pragma endregion


#pragma region Misc
    double GetDelta_Time();
    double GetFixedDelta_time();
    double GetFramePerSecond();

    int RandomNumberGen(int, int);
    float fLerp(float a, float b, float t);
    float fInverseLerp(float a, float b, float value);
    float fClamp(float value, float min, float max);
    int iClamp(int value, int min, int max);

#pragma endregion

#pragma region Scripting
    bool AttachScript(const std::string& objectName, const std::string& scriptClassName);
#pragma endregion

#pragma region Input
    bool IsKeyPressed(int key);
    bool IsKeyReleased(int key);
    bool IsKeyHeld(int key);
    bool IsMouseButtonPressed(int button);
    bool IsMouseButtonReleased(int button);
    bool IsMouseButtonHeld(int button);
    float GetMouseX();
    float GetMouseY();
    float GetMouseWorldX();
    float GetMouseWorldY();
#pragma endregion

}