/* Start Header ************************************************************************/
/*!
\file       tranform.h
\author     Leck Xin Yee [100%]
\par        xinyee.leck@digipen.edu
\date        October, 3, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include "Component.h"
#include "MathLib.h"

/**
 * @class TransformComponent
 * @brief Component that handles position, rotation, and scale of a GameObject in 2D space
 */
class TransformComponent : public IComponent
{
public:
    /**
     * @brief Constructs a TransformComponent with initial position
     * @param owner GameObject that owns this component
     * @param x Initial X position
     * @param y Initial Y position
     */
    TransformComponent(GameObject *owner, float x = 0, float y = 0);


    /**
     * @brief Gets the name of this component type
     * @return String "TransformComponent"
     */
    std::string GetName() const override { return "TransformComponent"; }

    /**
     * @brief Creates a clone of this component for a new GameObject
     * @param NewClone GameObject that will own the cloned component
     * @return Unique pointer to the cloned TransformComponent
     */
    std::unique_ptr<IComponent> Clone(GameObject *NewClone) const override;

    // Position
    /**
     * @brief Sets the position of the transform
     * @param x X coordinate
     * @param y Y coordinate
     */
    void SetPosition(float x, float y);

    /**
     * @brief Gets the current position of the transform
     * @param x Reference to store X coordinate
     * @param y Reference to store Y coordinate
     */
    void GetPosition(float &x, float &y) const;

    // Rotation (single angle in 2D)
    /**
     * @brief Sets the rotation angle
     * @param angle Rotation angle in radians
     */
    void SetRotation(float angle);

    /**
     * @brief Gets the current rotation angle
     * @return Rotation angle in radians
     */
    float GetRotation() const;

    // Scale
    /**
     * @brief Sets the scale factors for X and Y axes
     * @param x Scale factor for X axis
     * @param y Scale factor for Y axis
     */
    void SetScale(float x, float y);

    /**
     * @brief Sets uniform scale for both X and Y axes
     * @param uniformScale Scale factor to apply to both axes
     */
    void SetScale(float uniformScale); // Uniform scaling

    /**
     * @brief Gets the current scale factors
     * @param x Reference to store X scale factor
     * @param y Reference to store Y scale factor
     */
    void GetScale(float &x, float &y) const;

    // Movement
    /**
     * @brief Moves the transform by a delta amount
     * @param deltaX Amount to move along X axis
     * @param deltaY Amount to move along Y axis
     */
    void Move(float deltaX, float deltaY);

    /**
     * @brief Rotates the transform by a delta angle
     * @param deltaAngle Amount to rotate in radians
     */
    void Rotate(float deltaAngle);

    // 2D specific helpers
    /**
     * @brief Rotates the transform to face a target position
     * @param targetX Target X coordinate
     * @param targetY Target Y coordinate
     */
    void LookAt(float targetX, float targetY);

    /**
     * @brief Moves the transform forward in its current rotation direction
     * @param distance Distance to move forward
     */
    void MoveForward(float distance);

    /**
     * @brief Moves the transform right relative to its current rotation
     * @param distance Distance to move right
     */
    void MoveRight(float distance);

    /**
     * @brief Gets mat4 for translate, scale, rotation
     */
    mat4 GetMatrix();

    void test() 
    {
        _propertySlot.push_back({ "aa", "aa" }); 
        for (auto& ass : GetPropertySlots())
        {
            // WIP but i cant call renderimgui for some fucking reason im dedass bout to eat my whole keyboard this is a nightmare
        }
    };

public:
    float posX, posY;     // Position
    float rotation;       // Rotation angle in radians
    float scaleX, scaleY; // Scale factors
};
