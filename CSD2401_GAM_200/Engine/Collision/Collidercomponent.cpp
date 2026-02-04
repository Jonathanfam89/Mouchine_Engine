/* Start Header ************************************************************************/
/*!
\file       ColliderComponent.cpp
\author     Khalil Belabadia [100% - 37 lines]
\par        khalil.b@digipen.edu
\date       October, 2, 2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "ColliderComponent.h"
#include "GameObject.h"

ColliderComponent::ColliderComponent(GameObject* own)
    : IComponent(own), type(Type::Square), width(0), height(0), radius(0), layer(CollisionLayer::Default)
{
}
/**
 * @brief Constructs a ColliderComponent with specified parameters
 */
ColliderComponent::ColliderComponent(GameObject* owner, Type type, float w, float h, CollisionLayer layer)
    : IComponent(owner), type(type), width(w), height(h), radius(w * 0.5f), layer(layer) {
}

/**
 * @brief Converts the transform position to an Axis-Aligned Bounding Box
 */
AABB ColliderComponent::GetAABB() const {
    auto* transform = owner->GetComponent<TransformComponent>();
    if (!transform) return { {0,0}, {0,0} };

    vec2 pos{ transform->posX, transform->posY };
    vec2 half{ width * 0.5f, height * 0.5f };

    return { {pos.x - half.x, pos.y - half.y}, {pos.x + half.x, pos.y + half.y} };
}

/**
 * @brief Converts the transform position to a Circle shape
 */
Circle ColliderComponent::GetCircle() const {
    auto* transform = owner->GetComponent<TransformComponent>();
    if (!transform) return { {0,0}, radius };

    vec2 pos{ transform->posX, transform->posY };
    return { pos, radius };
}

void ColliderComponent::SetSize(float w, float h) {
    width = w;
    height = h;
    radius = w * 0.5f;
}

//void ColliderComponent::SetRotation(float r)
//{
//    rotation = r;
//}

/**
 * @brief Creates a deep copy of this collider component
 */
std::unique_ptr<IComponent> ColliderComponent::Clone(GameObject* newOwner) const {
    auto cloned = std::make_unique<ColliderComponent>(
        newOwner,
        this->type,
        this->width,
        this->height,
        this->layer);

    cloned->SetEnabled(this->IsEnabled());
    return cloned;
}