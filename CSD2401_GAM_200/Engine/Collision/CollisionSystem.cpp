/* Start Header ************************************************************************/
/*!
\file       CollisionSystem.cpp
\author     Khalil Belabadia [96% - 275 lines]
            Lee Hwee Min [4% - 5 lines]
\par        khalil.b@digipen.edu
\date       October, 2, 2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "CollisionSystem.h"
#include "PhysicsSystem.h"
#include "GameObject.h"
#include "Factory.h"
#include "ColliderComponent.h"
#include "Collision.h"
#include <iostream>
#include <algorithm>

/**
 * @brief Constructs collision system with null physics reference
 */
CollisionSystem::CollisionSystem() : physicsSystem(nullptr), spatialGrid()
{
}

/**
 * @brief Destructor
 */
CollisionSystem::~CollisionSystem()
{
}

/**
 * @brief Initializes the collision system
 */
void CollisionSystem::Initialize()
{
    std::cout << "Initializing system: CollisionSystem" << std::endl;
}

/**
 * @brief Determines if two collision layers should interact based on layer rules
 * Layer interaction matrix:
 * - Default collides with everything
 * - Player collides with Ground, Enemy, Platform
 * - Enemy collides with Player, Ground, Platform, other Enemies
 * - Ground doesn't collide with other Ground
 * - Platforms only collide with Player and Enemy
 */
bool CollisionSystem::ShouldCollide(CollisionLayer layer1, CollisionLayer layer2)
{
    if (layer1 == CollisionLayer::Default || layer2 == CollisionLayer::Default)
    {
        return true;
    }

    if (layer1 == CollisionLayer::Player || layer2 == CollisionLayer::Player)
    {
        CollisionLayer other = (layer1 == CollisionLayer::Player) ? layer2 : layer1;
        return (other == CollisionLayer::Ground ||
            other == CollisionLayer::Enemy ||
            other == CollisionLayer::Platform ||
            other == CollisionLayer::Player);
    }

    if (layer1 == CollisionLayer::Enemy || layer2 == CollisionLayer::Enemy)
    {
        CollisionLayer other = (layer1 == CollisionLayer::Enemy) ? layer2 : layer1;
        return (other == CollisionLayer::Player ||
            other == CollisionLayer::Ground ||
            other == CollisionLayer::Platform ||
            other == CollisionLayer::Enemy);
    }

    if (layer1 == CollisionLayer::Ground && layer2 == CollisionLayer::Ground)
    {
        return false;
    }

    if (layer1 == CollisionLayer::Platform || layer2 == CollisionLayer::Platform)
    {
        CollisionLayer other = (layer1 == CollisionLayer::Platform) ? layer2 : layer1;
        return (other == CollisionLayer::Player || other == CollisionLayer::Enemy);
    }

    return true;
}

/**
 * @brief Main update loop - detects collisions and reports them to physics system
 * Performs broad-phase collision detection for all game objects with colliders,
 * then narrow-phase testing based on shape types
 */
void CollisionSystem::Update(float deltaTime)
{
    (void)deltaTime;
    auto& factory = GameObjectFactory::Instance();
    const auto& gameObjects = factory.GetAllGameObjects();

    // Clear and rebuild spatial grid each frame
    spatialGrid.Clear();

    // Insert all objects with colliders into the grid
    for (const auto& obj : gameObjects)
    {
        auto* collider = obj->GetComponent<ColliderComponent>();
        if (collider)
        {
            spatialGrid.Insert(obj.get(), collider);
        }
    }

    // Use spatial grid for broad-phase collision detection
    for (const auto& obj1 : gameObjects)
    {
        auto* col1 = obj1->GetComponent<ColliderComponent>();
        if (!col1)
            continue;

        // Query nearby objects from spatial grid (MUCH FASTER than checking all)
        std::vector<GameObject*> nearbyObjects = spatialGrid.Query(obj1.get(), col1);

        // Check collisions only with nearby objects
        for (GameObject* obj2 : nearbyObjects)
        {
            auto* col2 = obj2->GetComponent<ColliderComponent>();
            if (!col2)
                continue;


            // OPTIMIZATION: Check layer collision rules FIRST (cheap)
            // Skip expensive shape collision if layers should not collide
            if (!ShouldCollide(col1->GetLayer(), col2->GetLayer()))
            {
                continue;  // Skip this pair entirely
            }
            bool isColliding = false;

            // Narrow-phase: Detect collision based on shape types
            if (col1->GetType() == ColliderComponent::Type::Circle &&
                col2->GetType() == ColliderComponent::Type::Circle)
            {
                isColliding = CollisionIntersection_CircleCircle_Static(
                    col1->GetCircle(), col2->GetCircle());
            }
            else if (col1->GetType() == ColliderComponent::Type::Square &&
                col2->GetType() == ColliderComponent::Type::Square)
            {
                isColliding = CollisionIntersection_RectRect_Static(
                    col1->GetAABB(), col2->GetAABB());
            }
            else
            {
                if (col1->GetType() == ColliderComponent::Type::Square)
                {
                    isColliding = CollisionIntersection_RectCircle_Static(
                        col1->GetAABB(), col2->GetCircle());
                }
                else
                {
                    isColliding = CollisionIntersection_RectCircle_Static(
                        col2->GetAABB(), col1->GetCircle());
                }
            }

            if (isColliding)
            {
                bool wasActive = false;
                for (auto& pair : activeCollisions)
                {
                    if ((pair.first == obj1.get() && pair.second == obj2) ||
                        (pair.first == obj2 && pair.second == obj1.get()))
                    {
                        wasActive = true;
                        break;
                    }
                }

                // If it's a new collision, publish a message and log it
                if (!wasActive)
                {
                    CollisionMessage msg(obj1.get(), obj2);
                    CORE->GetMessageBus().Publish(msg);
                    std::cout << "Collision: " << obj1->GetName() << " <-> "
                        << obj2->GetName() << std::endl;
                    activeCollisions.push_back({ obj1.get(), obj2 });
                }

                // Report collision to PhysicsSystem for resolution
                if (physicsSystem)
                {
                    physicsSystem->ReportCollision(obj1.get(), obj2, col1, col2);
                }
            }
            else
            {
                if (JustStopped(obj1.get(), obj2)) {
                    auto RaiseIfNeeded = [&](GameObject* receiver, GameObject* other)
                        {
                            if (receiver->GetComponent<EventComponent>())
                                EventSystem::Raise(EventType::OnCollisionExit, receiver, other);
                        };

                    RaiseIfNeeded(obj1.get(), obj2);
                    RaiseIfNeeded(obj2, obj1.get());
                }

                // Remove from active collisions
                activeCollisions.erase(
                    std::remove_if(activeCollisions.begin(), activeCollisions.end(),
                        [&](const auto& pair)
                        {
                            return (pair.first == obj1.get() && pair.second == obj2) ||
                                (pair.first == obj2 && pair.second == obj1.get());
                        }),
                    activeCollisions.end());
            }
        }
    }

    // Update collision tracking for helper functions
    previousCollisions = currentCollisions;
    currentCollisions.clear();

    // Rebuild current collisions set
    for (const auto& obj1 : gameObjects) {
        auto* col1 = obj1->GetComponent<ColliderComponent>();
        if (!col1) continue;

        std::vector<GameObject*> nearbyObjects = spatialGrid.Query(obj1.get(), col1);

        for (GameObject* obj2 : nearbyObjects) {
            auto* col2 = obj2->GetComponent<ColliderComponent>();
            if (!col2) continue;

            if (AreColliding(obj1.get(), obj2)) {
                std::string key = GetCollisionKey(obj1.get(), obj2);
                currentCollisions.insert(key);

                if (StillColliding(obj1.get(), obj2)) {
                    auto RaiseIfNeeded = [&](GameObject* receiver, GameObject* other)
                        {
                            if (receiver->GetComponent<EventComponent>())
                                EventSystem::Raise(EventType::OnCollisionStay, receiver, other);
                        };

                    RaiseIfNeeded(obj1.get(), obj2);
                    RaiseIfNeeded(obj2, obj1.get());
                }

            }
        }
    }
}
// ===== Helper Functions for Specific Object Collision Queries =====

std::string CollisionSystem::GetCollisionKey(GameObject* obj1, GameObject* obj2) {

    // GetCollisionKey(player, enemy) and GetCollisionKey(enemy, player) 
    // both become GetCollisionKey(player, enemy) after this line
    if (obj1 > obj2) std::swap(obj1, obj2);

    //   Hero vs Enemy1 ? "Hero_Enemy_100_200"
    //   Hero vs Enemy2 ? "Hero_Enemy_100_300" (different address for Enemy2)
    return obj1->GetName() + "_" + obj2->GetName() + "_" +
        std::to_string(reinterpret_cast<uintptr_t>(obj1)) + "_" +
        std::to_string(reinterpret_cast<uintptr_t>(obj2));
}

bool CollisionSystem::AreColliding(GameObject* obj1, GameObject* obj2) {
    if (!obj1 || !obj2) return false;

    auto* col1 = obj1->GetComponent<ColliderComponent>();
    auto* col2 = obj2->GetComponent<ColliderComponent>();

    if (!col1 || !col2) return false;

    if (col1->GetType() == ColliderComponent::Type::Circle &&
        col2->GetType() == ColliderComponent::Type::Circle) {
        return CollisionIntersection_CircleCircle_Static(col1->GetCircle(), col2->GetCircle());
    }
    else if (col1->GetType() == ColliderComponent::Type::Square &&
        col2->GetType() == ColliderComponent::Type::Square) {
        return CollisionIntersection_RectRect_Static(col1->GetAABB(), col2->GetAABB());
    }
    else {
        if (col1->GetType() == ColliderComponent::Type::Square) {
            return CollisionIntersection_RectCircle_Static(col1->GetAABB(), col2->GetCircle());
        }
        else {
            return CollisionIntersection_RectCircle_Static(col2->GetAABB(), col1->GetCircle());
        }
    }
}

bool CollisionSystem::JustCollided(GameObject* obj1, GameObject* obj2) {
    std::string key = GetCollisionKey(obj1, obj2);
    bool collidingNow = currentCollisions.find(key) != currentCollisions.end();
    bool collidingBefore = previousCollisions.find(key) != previousCollisions.end();
    return collidingNow && !collidingBefore;
}

bool CollisionSystem::StillColliding(GameObject* obj1, GameObject* obj2) {
    std::string key = GetCollisionKey(obj1, obj2);
    bool collidingNow = currentCollisions.find(key) != currentCollisions.end();
    bool collidingBefore = previousCollisions.find(key) != previousCollisions.end();
    return collidingNow && collidingBefore;
}

bool CollisionSystem::JustStopped(GameObject* obj1, GameObject* obj2) {
    std::string key = GetCollisionKey(obj1, obj2);
    bool collidingNow = currentCollisions.find(key) != currentCollisions.end();
    bool collidingBefore = previousCollisions.find(key) != previousCollisions.end();
    return !collidingNow && collidingBefore;
}

// ===== Raycast Implementation =====

/**
 * @brief Casts a ray and returns the first object hit (optionally filtered by layer)
 */
bool CollisionSystem::Raycast(const vec2& origin, const vec2& direction, float distance,
    RaycastHit& hit, GameObject* self, CollisionLayer layerMask)
{
    // Normalize direction
    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (length < 0.0001f) return false;

    vec2 normDir = { direction.x / length, direction.y / length };

    auto& factory = GameObjectFactory::Instance();
    const auto& gameObjects = factory.GetAllGameObjects();

    float closestDistance = distance;
    bool foundHit = false;

    // Check all objects with colliders
    for (const auto& obj : gameObjects)
    {
        if (obj.get() == self) {
            continue; // Skip the object that is casting the ray
        }

        auto* collider = obj->GetComponent<ColliderComponent>();
        if (!collider) continue;

        // Filter by layer if not Default (Default means check all layers)
        if (layerMask != CollisionLayer::Default &&
            collider->GetLayer() != layerMask) {
            continue;
        }

        float hitDist = 0.0f;
        bool intersects = false;

        // Check based on collider type
        if (collider->GetType() == ColliderComponent::Type::Square)
        {
            AABB aabb = collider->GetAABB();
            intersects = IntersectRayAABB(origin, normDir, aabb, closestDistance, hitDist);
        }
        else if (collider->GetType() == ColliderComponent::Type::Circle)
        {
            Circle circle = collider->GetCircle();
            intersects = IntersectRayCircle(origin, normDir, circle, closestDistance, hitDist);
        }

        // If this is closer than previous hits, record it
        if (intersects && hitDist < closestDistance)
        {
            closestDistance = hitDist;
            foundHit = true;

            hit.gameObject = obj.get();
            hit.collider = collider;
            hit.distance = hitDist;
            hit.point = { origin.x + normDir.x * hitDist, origin.y + normDir.y * hitDist };

            // Calculate normal based on hit point
            if (collider->GetType() == ColliderComponent::Type::Square)
            {
                // For AABB, determine which face was hit
                AABB aabb = collider->GetAABB();
                vec2 center = { (aabb.min.x + aabb.max.x) * 0.5f, (aabb.min.y + aabb.max.y) * 0.5f };
                vec2 toHit = { hit.point.x - center.x, hit.point.y - center.y };

                // Determine which axis has larger offset
                if (std::abs(toHit.x) > std::abs(toHit.y)) {
                    hit.normal = { toHit.x > 0 ? 1.0f : -1.0f, 0.0f };
                }
                else {
                    hit.normal = { 0.0f, toHit.y > 0 ? 1.0f : -1.0f };
                }
            }
            else
            {
                // For circle, normal points from center to hit point
                Circle circle = collider->GetCircle();
                vec2 toHit = { hit.point.x - circle.center.x, hit.point.y - circle.center.y };
                float len = std::sqrt(toHit.x * toHit.x + toHit.y * toHit.y);
                if (len > 0.0001f) {
                    hit.normal = { toHit.x / len, toHit.y / len };
                }
                else {
                    hit.normal = { 0.0f, 1.0f };
                }
            }
        }
    }

    return foundHit;
}