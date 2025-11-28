/* Start Header ************************************************************************/
/*!
\file       Collision.cpp
\author     Khalil Belabadia [100% - 68 lines]
\par        khalil.b@digipen.edu
\date       October, 2, 2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "collision.h"
#include <algorithm> // for std::min, std::max
#include <cmath>     // for sqrtf, fmaxf, fminf

// --------------------------------------------------
// Static Intersections
// --------------------------------------------------

/**
 * @brief Performs AABB vs AABB intersection test using separating axis theorem
 */
bool CollisionIntersection_RectRect_Static(const AABB& aabb1, const AABB& aabb2)
{
    if (aabb1.max.x < aabb2.min.x) return false;
    if (aabb1.min.x > aabb2.max.x) return false;
    if (aabb1.max.y < aabb2.min.y) return false;
    if (aabb1.min.y > aabb2.max.y) return false;
    return true;
}

/**
 * @brief Performs Circle vs Circle intersection test using distance check
 */
bool CollisionIntersection_CircleCircle_Static(const Circle& c1, const Circle& c2)
{
    float dx = c1.center.x - c2.center.x;
    float dy = c1.center.y - c2.center.y;
    float distSq = dx * dx + dy * dy;
    float radiusSum = c1.radius + c2.radius;
    return distSq <= radiusSum * radiusSum;
}

/**
 * @brief Performs AABB vs Circle intersection test by finding closest point on rect
 */
bool CollisionIntersection_RectCircle_Static(const AABB& rect, const Circle& circle)
{
    // Clamp circle center to the edges of the rectangle
    float closestX = std::max(rect.min.x, std::min(circle.center.x, rect.max.x));
    float closestY = std::max(rect.min.y, std::min(circle.center.y, rect.max.y));

    float dx = circle.center.x - closestX;
    float dy = circle.center.y - closestY;

    return (dx * dx + dy * dy) <= (circle.radius * circle.radius);
}

// --------------------------------------------------
// Dynamic Intersections
// --------------------------------------------------

/**
 * @brief Performs swept AABB collision detection for moving objects
 * Uses relative velocity and separating axis theorem to find collision time
 */
bool CollisionIntersection_RectRect_Dynamic(const AABB& aabb1,
    const vec2& vel1,
    const AABB& aabb2,
    const vec2& vel2,
    float& firstTimeOfCollision)
{
    // Relative velocity of B wrt A
    vec2 relVel = { vel2.x - vel1.x, vel2.y - vel1.y };

    // Initialize times
    float tFirst = 0.0f;
    float tLast = 1.0f; // normalized to frame [0,1]

    // For both axes
    for (int axis = 0; axis < 2; ++axis) {
        float v = (axis == 0 ? relVel.x : relVel.y);
        float aMin = (axis == 0 ? aabb1.min.x : aabb1.min.y);
        float aMax = (axis == 0 ? aabb1.max.x : aabb1.max.y);
        float bMin = (axis == 0 ? aabb2.min.x : aabb2.min.y);
        float bMax = (axis == 0 ? aabb2.max.x : aabb2.max.y);

        if (v < 0.0f) {
            if (aMin > bMax) return false; // moving away
            float tEnter = (aMax - bMin) / v;
            float tExit = (aMin - bMax) / v;
            tFirst = std::max(tFirst, tEnter);
            tLast = std::min(tLast, tExit);
        }
        else if (v > 0.0f) {
            if (aMax < bMin) return false; // moving away
            float tEnter = (aMin - bMax) / v;
            float tExit = (aMax - bMin) / v;
            tFirst = std::max(tFirst, tEnter);
            tLast = std::min(tLast, tExit);
        }
        else {
            // v == 0
            if (aMax < bMin || aMin > bMax) {
                return false; // no movement, no overlap
            }
        }
    }

    if (tFirst > tLast) {
        return false;
    }

    firstTimeOfCollision = tFirst;
    return true;
}

// --------------------------------------------------
// Ray Intersections
// --------------------------------------------------

/**
 * @brief Tests if a ray intersects an AABB using slab method
 */
bool IntersectRayAABB(const vec2& origin, const vec2& direction,
    const AABB& aabb, float maxDistance, float& hitDistance)
{
    // Initialize intersection distances
    float tMin = 0.0f;
    float tMax = maxDistance;

    // Check X axis
    if (std::abs(direction.x) > 0.0001f) {
        float t1 = (aabb.min.x - origin.x) / direction.x;
        float t2 = (aabb.max.x - origin.x) / direction.x;

        if (t1 > t2) std::swap(t1, t2);

        tMin = std::max(tMin, t1);
        tMax = std::min(tMax, t2);

        if (tMin > tMax) return false;
    }
    else {
        // Ray is parallel to X axis
        if (origin.x < aabb.min.x || origin.x > aabb.max.x) {
            return false;
        }
    }

    // Check Y axis
    if (std::abs(direction.y) > 0.0001f) {
        float t1 = (aabb.min.y - origin.y) / direction.y;
        float t2 = (aabb.max.y - origin.y) / direction.y;

        if (t1 > t2) std::swap(t1, t2);

        tMin = std::max(tMin, t1);
        tMax = std::min(tMax, t2);

        if (tMin > tMax) return false;
    }
    else {
        // Ray is parallel to Y axis
        if (origin.y < aabb.min.y || origin.y > aabb.max.y) {
            return false;
        }
    }

    hitDistance = tMin;
    return true;
}

/**
 * @brief Tests if a ray intersects a circle using quadratic equation
 */
bool IntersectRayCircle(const vec2& origin, const vec2& direction,
    const Circle& circle, float maxDistance, float& hitDistance)
{
    // Vector from ray origin to circle center
    vec2 oc = { origin.x - circle.center.x, origin.y - circle.center.y };

    // Quadratic equation coefficients: at^2 + bt + c = 0
    float a = direction.x * direction.x + direction.y * direction.y;
    float b = 2.0f * (oc.x * direction.x + oc.y * direction.y);
    float c = oc.x * oc.x + oc.y * oc.y - circle.radius * circle.radius;

    float discriminant = b * b - 4.0f * a * c;

    // No intersection if discriminant is negative
    if (discriminant < 0.0f) {
        return false;
    }

    // Calculate the two possible intersection points
    float sqrtDisc = std::sqrt(discriminant);
    float t1 = (-b - sqrtDisc) / (2.0f * a);
    float t2 = (-b + sqrtDisc) / (2.0f * a);

    // Find the closest valid hit
    if (t1 >= 0.0f && t1 <= maxDistance) {
        hitDistance = t1;
        return true;
    }

    if (t2 >= 0.0f && t2 <= maxDistance) {
        hitDistance = t2;
        return true;
    }

    return false;
}