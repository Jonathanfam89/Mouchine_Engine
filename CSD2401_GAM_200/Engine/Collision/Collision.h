/* Start Header ************************************************************************/
/*!
\file        Collision.h
\author     Khalil Belabadia [100% - 32 lines]
\par        khalil.b@digipen.edu
\date       October, 2, 2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include "MathLib.h"
#include <string>
#include <cmath>

// Forward declarations
class GameObject;
class ColliderComponent;
enum class CollisionLayer;

// --------------------
// Raycast Structures
// --------------------

/**
 * @brief Represents a ray for raycasting
 */
struct Ray {
    vec2 origin;      // Starting point of the ray
    vec2 direction;   // Direction vector (should be normalized)

    Ray() : origin{ 0.0f, 0.0f }, direction{ 0.0f, 0.0f } {}
    Ray(const vec2& orig, const vec2& dir) : origin(orig), direction(dir) {}
};

/**
 * @brief Contains information about a raycast hit
 */
struct RaycastHit {
    GameObject* gameObject;           // The GameObject that was hit
    ColliderComponent* collider;      // The collider that was hit
    vec2 point;                       // The point in world space where the ray hit
    vec2 normal;                      // The normal of the surface hit
    float distance;                   // Distance from ray origin to hit point

    RaycastHit()
        : gameObject(nullptr), collider(nullptr),
        point{ 0.0f, 0.0f }, normal{ 0.0f, 0.0f }, distance(0.0f) {
    }
};

// --------------------
// Collider Types
// --------------------
struct AABB {
    vec2 min; // bottom-left
    vec2 max; // top-right
};

struct Circle {
    vec2 center;
    float radius;
};

// --------------------
// Static Intersection Tests
// --------------------

/**
 * @brief Tests if two AABBs are intersecting
 * @param aabb1 First axis-aligned bounding box
 * @param aabb2 Second axis-aligned bounding box
 * @return True if the AABBs overlap
 */
bool CollisionIntersection_RectRect_Static(const AABB& aabb1, const AABB& aabb2);

/**
 * @brief Tests if two circles are intersecting
 * @param c1 First circle
 * @param c2 Second circle
 * @return True if the circles overlap
 */
bool CollisionIntersection_CircleCircle_Static(const Circle& c1, const Circle& c2);

/**
 * @brief Tests if an AABB and circle are intersecting
 * @param rect Axis-aligned bounding box
 * @param circle Circle to test against
 * @return True if the shapes overlap
 */
bool CollisionIntersection_RectCircle_Static(const AABB& rect, const Circle& circle);

// --------------------
// Dynamic Intersection Tests
// --------------------

/**
 * @brief Tests if two moving AABBs will collide within the frame
 * @param aabb1 First axis-aligned bounding box
 * @param vel1 Velocity of first AABB
 * @param aabb2 Second axis-aligned bounding box
 * @param vel2 Velocity of second AABB
 * @param firstTimeOfCollision Output parameter for normalized collision time [0,1]
 * @return True if the AABBs will collide
 */
bool CollisionIntersection_RectRect_Dynamic(const AABB& aabb1,
    const vec2& vel1,
    const AABB& aabb2,
    const vec2& vel2,
    float& firstTimeOfCollision);

// (Circle vs Circle dynamic can be added later)

// --------------------
// Ray Intersection Tests
// --------------------

/**
 * @brief Tests if a ray intersects an AABB
 * @param origin Ray starting point
 * @param direction Ray direction (normalized)
 * @param aabb Axis-aligned bounding box
 * @param maxDistance Maximum ray distance
 * @param hitDistance Output: distance to hit point if collision occurs
 * @return True if ray intersects the AABB
 */
bool IntersectRayAABB(const vec2& origin, const vec2& direction,
    const AABB& aabb, float maxDistance, float& hitDistance);

/**
 * @brief Tests if a ray intersects a circle
 * @param origin Ray starting point
 * @param direction Ray direction (normalized)
 * @param circle Circle to test against
 * @param maxDistance Maximum ray distance
 * @param hitDistance Output: distance to hit point if collision occurs
 * @return True if ray intersects the circle
 */
bool IntersectRayCircle(const vec2& origin, const vec2& direction,
    const Circle& circle, float maxDistance, float& hitDistance);

// --------------------
// Unified Helpers
// --------------------

/**
 * @brief Combined static and dynamic AABB collision test
 * @param aabb1 First axis-aligned bounding box
 * @param vel1 Velocity of first AABB
 * @param aabb2 Second axis-aligned bounding box
 * @param vel2 Velocity of second AABB
 * @param firstTimeOfCollision Output parameter for normalized collision time
 * @return True if AABBs are colliding or will collide
 */
inline bool CollisionIntersection_RectRect(const AABB& aabb1,
    const vec2& vel1,
    const AABB& aabb2,
    const vec2& vel2,
    float& firstTimeOfCollision)
{
    // Step 1: Static check
    if (CollisionIntersection_RectRect_Static(aabb1, aabb2)) {
        firstTimeOfCollision = 0.0f;
        return true;
    }

    // Step 2: Dynamic test
    return CollisionIntersection_RectRect_Dynamic(aabb1, vel1, aabb2, vel2, firstTimeOfCollision);
}