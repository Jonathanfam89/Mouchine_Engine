/* Start Header ************************************************************************/
/*!
\file       Forces.h
\author     Khalil Belabadia [100% - 100 lines]
\par        khalil.b@digipen.edu
\date       November, 2, 2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include <memory>
#include <cmath>
#include "MathLib.h"  // For vec2

// Forward declaration
struct Force;

/**
 * @brief Types of forces available in the system
 */
enum class ForceType {
    Jump,
    MoveRight,
    MoveLeft,
    Wind,
    Boost,
    Friction,
    Knockback,
    Dash
};

/**
 * @brief Force constants - all force values defined here
 */
namespace ForceConstants {
    // Gravity
    constexpr float GRAVITY_ACCELERATION = -381.0f;
    // Jump
    constexpr float JUMP_FORCE = 1000.0f;
    constexpr float JUMP_DURATION = 0.4f;

    // Movement
    constexpr float MOVE_FORCE = 200.0f;

    // Drag
    constexpr float DRAG_COEFFICIENT = 30.0f;

    // Friction (ground friction) - force magnitude opposing velocity
    constexpr float FRICTION_COEFFICIENT = 5.0f;  // Force coefficient (will be divided by mass)

    // Knockback (applied when hit by enemy)
    constexpr float KNOCKBACK_FORCE = 800.0f;      // Constant knockback force
    constexpr float KNOCKBACK_DURATION = 0.2f;     // Brief knockback time

    // Dash (horizontal dash ability)
    constexpr float DASH_FORCE = 1800.0f;          // Impulse force for dash
    constexpr float DASH_DURATION = 0.3f;         // How long dash lasts
}

/**
 * @brief Base class for all force types
 * Uses vec2 for cleaner vector math
 */
struct Force {
    bool isActive;
    float lifetime;  // -1 for infinite
    float age;       // Time since creation

    Force() : isActive(true), lifetime(-1.0f), age(0.0f) {}
    virtual ~Force() = default;

    /**
     * @brief Calculate acceleration from force
     * @param mass Object's mass
     * @param velocity Current velocity vector
     * @return Acceleration vector
     */
    virtual vec2 CalculateAcceleration(float mass, const vec2& velocity) = 0;
};

/**
 * @brief Linear directional force with unit direction and magnitude
 */
struct LinearDirectionalForce : public Force {
    vec2 direction;     // Unit direction vector
    float magnitude;
    ForceType type;

    LinearDirectionalForce(float dirX, float dirY, float mag, float life, ForceType t)
        : direction{ dirX, dirY }, magnitude(mag), type(t) {
        lifetime = life;

        // Normalize direction
        float len = std::sqrt(direction.x * direction.x + direction.y * direction.y);
        if (len > 0.0001f) {
            direction.x /= len;
            direction.y /= len;
        }
    }

    vec2 CalculateAcceleration(float mass, const vec2& velocity) override {
        (void)velocity;  // Unused - directional forces don't depend on velocity

        if (!isActive) {
            return { 0.0f, 0.0f };
        }

        vec2 force = { direction.x * magnitude, direction.y * magnitude };
        return { force.x / mass, force.y / mass };
    }
};

/**
 * @brief Drag force opposing motion
 */
struct DragForce : public Force {
    float dragCoefficient;

    DragForce(float drag) : dragCoefficient(drag) {
        lifetime = -1.0f;
    }

    vec2 CalculateAcceleration(float mass, const vec2& velocity) override {
        if (!isActive) {
            return { 0.0f, 0.0f };
        }

        vec2 dragForce = { -dragCoefficient * velocity.x, -dragCoefficient * velocity.y };
        return { dragForce.x / mass, dragForce.y / mass };
    }
};

/**
 * @brief Friction force (only horizontal, only when grounded)
 */
struct FrictionForce : public Force {
    float frictionCoefficient;
    bool* isGrounded;
    ForceType type;

    FrictionForce(float friction, bool* groundedPtr)
        : frictionCoefficient(friction), isGrounded(groundedPtr), type(ForceType::Friction) {
        lifetime = -1.0f;
    }

    vec2 CalculateAcceleration(float mass, const vec2& velocity) override {
        if (!isActive || !isGrounded || !(*isGrounded)) {
            return { 0.0f, 0.0f };
        }

        // Friction only opposes horizontal motion when grounded
        // Calculate friction force: F_friction = -coefficient * velocity
        vec2 frictionForce = { -frictionCoefficient * velocity.x, 0.0f };

        // Convert force to acceleration: a = F / m
        return { frictionForce.x / mass, frictionForce.y / mass };
    }
};