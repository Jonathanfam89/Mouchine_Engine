/* Start Header ************************************************************************/
/*!
\file       ColliderComponent.h
\author     Khalil Belabadia [100% - 41 lines]
\par        khalil.b@digipen.edu
\date       October, 2, 2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include "Component.h"
#include "collision.h" // for AABB, Circle, Vec2
#include "transform.h"

class TransformComponent;

// Collision layers enum
enum class CollisionLayer
{
    Player,
    Ground, // Ground/Wall - used for all static terrain
    Enemy,
    Platform,
    Default // For objects without a specific layer
};

class ColliderComponent : public IComponent
{
public:
    enum class Type
    {
        Square,
        Circle
    };

    ColliderComponent(GameObject *own);
    /**
     * @brief Constructs a ColliderComponent
     * @param owner GameObject that owns this component
     * @param type Shape type (Square or Circle)
     * @param w Width for squares or diameter for circles
     * @param h Height for squares (ignored for circles)
     * @param layer Collision layer for filtering
     */
    ColliderComponent(GameObject *owner, Type type, float w, float h = 0.0f,
                      CollisionLayer layer = CollisionLayer::Default);

    /**
     * @brief Gets the component name
     * @return String identifier for this component type
     */
    std::string GetName() const override { return "ColliderComponent"; }

    /**
     * @brief Gets the Axis-Aligned Bounding Box for this collider
     * @return AABB structure with min and max points
     */
    AABB GetAABB() const;

    /**
     * @brief Gets the Circle shape for this collider
     * @return Circle structure with center and radius
     */
    Circle GetCircle() const;

    /**
     * @brief Gets the collision layer
     * @return Current collision layer
     */
    CollisionLayer GetLayer() const { return layer; }

    /**
     * @brief Sets the collision layer
     * @param newLayer New collision layer to assign
     */
    void SetLayer(CollisionLayer newLayer) { layer = newLayer; }

    void setColliShape(ColliderComponent::Type newShape) { type = newShape; }
    /**
     * @brief Creates a deep copy of this component
     * @param newOwner GameObject that will own the clone
     * @return Unique pointer to the cloned component
     */
    std::unique_ptr<IComponent> Clone(GameObject *newOwner) const override;

    /**
     * @brief Gets the collider shape type
     * @return Type enum (Square or Circle)
     */
    Type GetType() const { return type; }

    void SetSize(float w, float h);

    vec2 GetSize() const { return vec2(width, height); }

    /*   void SetRotation(float r);*/

private:
    Type type;
    float width;
    float height;
    float radius;
    // float rotation;
    CollisionLayer layer;
};