/* Start Header ************************************************************************/
/*!
\file       CollisionSystem.h
\author     Khalil Belabadia [100% - 23 lines]
\par        khalil.b@digipen.edu
\date       October, 2, 2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include "System.h"
#include "ColliderComponent.h"
#include "SpatialGrid.h"
#include <vector>
#include <utility>
#include <unordered_set>
#include <string>

class GameObject;
class ColliderComponent;
class PhysicsSystem;

class CollisionSystem : public ISystem {
public:
    /**
     * @brief Default constructor
     */
    CollisionSystem();

    /**
     * @brief Destructor
     */
    ~CollisionSystem() override;

    /**
     * @brief Initializes the collision system
     */
    void Initialize() override;

    /**
     * @brief Updates collision detection each frame
     * @param deltaTime Time elapsed since last update
     */
    void Update(float deltaTime) override;

    /**
     * @brief Gets the system name
     * @return String identifier for this system
     */
    std::string GetName() override { return "CollisionSystem"; }

    /**
     * @brief Sets the physics system for collision resolution
     * @param physSys Pointer to the physics system
     */
    void SetPhysicsSystem(PhysicsSystem* physSys) { physicsSystem = physSys; }

    /**
     * @brief Checks if two collision layers should interact
     * @param layer1 First collision layer
     * @param layer2 Second collision layer
     * @return True if the layers should collide
     */
    static bool ShouldCollide(CollisionLayer layer1, CollisionLayer layer2);

    // ===== Raycast Functions =====

    /**
     * @brief Casts a ray and returns the first object hit
     * @param origin Starting point of the ray
     * @param direction Direction of the ray (will be normalized)
     * @param distance Maximum distance to cast
     * @param hit Output: Information about what was hit
     * @param layerMask Optional: Only detect collisions with this layer (Default = all layers)
     * @return True if something was hit
     */
    bool Raycast(const vec2& origin, const vec2& direction, float distance,
        RaycastHit& hit, CollisionLayer layerMask = CollisionLayer::Default);

    // ===== Helper Functions for Specific Object Collision Queries =====
    bool AreColliding(GameObject* obj1, GameObject* obj2);
    bool JustCollided(GameObject* obj1, GameObject* obj2);
    bool StillColliding(GameObject* obj1, GameObject* obj2);
    bool JustStopped(GameObject* obj1, GameObject* obj2);

private:
    // Track collision pairs to avoid spam
    std::vector<std::pair<GameObject*, GameObject*>> activeCollisions;

    // Reference to physics system for reporting collisions
    PhysicsSystem* physicsSystem;

    // Spatial grid for broad-phase collision detection
    std::string GetCollisionKey(GameObject* obj1, GameObject* obj2);
    std::unordered_set<std::string> previousCollisions;
    std::unordered_set<std::string> currentCollisions;
    SpatialGrid spatialGrid;
};