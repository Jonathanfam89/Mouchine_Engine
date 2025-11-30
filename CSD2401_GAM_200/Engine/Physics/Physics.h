/* Start Header ************************************************************************/
/*!
\file       Physics.h
\author     Khalil Belabadia [100% - 167 lines]
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
#include "Forces.h"
#include <memory>
#include <vector>
#include <string>
#include <algorithm>
#include "transform.h"
#include "GameObject.h"
#include <iostream>
#include <cmath>

// Forward declarations
class TransformComponent;
enum class CollisionLayer;  // Forward declare collision layer

/**
 * @brief Manages all forces acting on a physics object
 */
class ForcesManager {
public:
    ForcesManager() = default;

    /**
     * @brief Add a force by type with optional custom parameters
     * @param type The type of force to add
     * @param forceMagnitude Optional custom force magnitude (uses default if < 0)
     * @param duration Optional custom duration (uses default if <= -999.0f)
     * @param directionX Optional X direction for Move force (default 0)
     * @param directionY Optional Y direction for Move force (default 0)
     * @param groundedPtr Optional pointer for friction (only used for Friction type)
     */
    void AddForce(ForceType type, float forceMagnitude = -1.0f, float duration = -999.0f,
        float directionX = 0.0f, float directionY = 0.0f, bool* groundedPtr = nullptr) {
        switch (type) {
        case ForceType::Jump:
            // Only add if not already jumping
            if (!HasForceType(ForceType::Jump)) {
                forces.push_back(std::make_unique<LinearDirectionalForce>(
                    0.0f, 1.0f,
                    (forceMagnitude < 0.0f) ? ForceConstants::JUMP_FORCE : forceMagnitude,
                    (duration <= -999.0f) ? ForceConstants::JUMP_DURATION : duration,
                    ForceType::Jump
                ));
            }
            break;

        case ForceType::Move:
            if (!HasForceType(ForceType::Move)) {
                forces.push_back(std::make_unique<LinearDirectionalForce>(
                    directionX, directionY,
                    (forceMagnitude < 0.0f) ? ForceConstants::MOVE_FORCE : forceMagnitude,
                    -1.0f,  // Movement forces are permanent
                    ForceType::Move
                ));
            }
            break;

        case ForceType::Friction:
            forces.push_back(std::make_unique<FrictionForce>(
                (forceMagnitude < 0.0f) ? ForceConstants::FRICTION_COEFFICIENT : forceMagnitude,
                groundedPtr
            ));
            break;

        //case ForceType::Wind:
        //case ForceType::Boost:
        //    break;
        }
    }

    /**
     * @brief Add a custom force (for advanced usage)
     */
    void AddCustomForce(std::unique_ptr<Force> force) {
        forces.push_back(std::move(force));
    }

    /**
     * @brief Apply knockback force in a specific direction
     * @param dirX X direction (will be normalized)
     * @param dirY Y direction (will be normalized)
     * @param forceMagnitude Knockback force magnitude (default uses constant)
     */
    void ApplyKnockback(float dirX, float dirY, float forceMagnitude = ForceConstants::KNOCKBACK_FORCE) {
        // Remove any existing knockback forces first
        RemoveForcesByType(ForceType::Knockback);

        // Add new knockback force
        forces.push_back(std::make_unique<LinearDirectionalForce>(
            dirX, dirY,
            forceMagnitude,
            ForceConstants::KNOCKBACK_DURATION,
            ForceType::Knockback
        ));
    }

    /**
     * @brief Remove a force by index
     * @param index Index of force to remove
     */
    void RemoveForce(size_t index) {
        if (index < forces.size()) {
            forces.erase(forces.begin() + index);
        }
    }

    /**
     * @brief Activate or deactivate a force
     * @param index Index of force
     * @param active New active state
     */
    void ActivateForce(size_t index, bool active) {
        if (index < forces.size()) {
            forces[index]->isActive = active;
        }
    }

    /**
     * @brief Update force ages and remove expired forces
     * @param deltaTime Time elapsed
     */
    void ValidateAge(float deltaTime) {
        for (auto it = forces.begin(); it != forces.end();) {
            (*it)->age += deltaTime;

            // Remove if lifetime expired (lifetime >= 0 means it has a limit)
            if ((*it)->lifetime >= 0.0f && (*it)->age >= (*it)->lifetime) {
                it = forces.erase(it);
            }
            else {
                ++it;
            }
        }
    }

    /**
     * @brief Set the lifetime of a force
     * @param index Index of force
     * @param lifetime New lifetime (-1 for infinite)
     */
    void SetLifeTime(size_t index, float lifetime) {
        if (index < forces.size()) {
            forces[index]->lifetime = lifetime;
        }
    }

    /**
     * @brief Calculate total acceleration from all active forces
     * @param mass Object's mass
     * @param velocity Current velocity vector
     * @return Acceleration vector
     */
    vec2 CalculateTotalAcceleration(float mass, const vec2& velocity) {
        vec2 totalAccel = { 0.0f, 0.0f };

        for (const auto& force : forces) {
            vec2 accel = force->CalculateAcceleration(mass, velocity);
            totalAccel.x += accel.x;
            totalAccel.y += accel.y;
        }

        return totalAccel;
    }

    /**
     * @brief Get number of forces
     */
    size_t GetForceCount() const { return forces.size(); }

    /**
     * @brief Clear all forces
     */
    void ClearForces() { forces.clear(); }

    /**
     * @brief Get force at index (for inspection/debugging)
     */
    Force* GetForce(size_t index) {
        if (index < forces.size()) {
            return forces[index].get();
        }
        return nullptr;
    }

    /**
     * @brief Remove all forces of a specific type
     * @param type The ForceType to remove
     */
    void RemoveForcesByType(ForceType type) {
        forces.erase(
            std::remove_if(forces.begin(), forces.end(),
                [type](const std::unique_ptr<Force>& force) {
                    // Check if it's a LinearDirectionalForce
                    auto* linearForce = dynamic_cast<LinearDirectionalForce*>(force.get());
                    if (linearForce && linearForce->type == type) {
                        return true;
                    }

                    // Check if it's a FrictionForce
                    auto* frictionForce = dynamic_cast<FrictionForce*>(force.get());
                    if (frictionForce && frictionForce->type == type) {
                        return true;
                    }

                    return false;
                }),
            forces.end()
        );
    }

    /**
     * @brief Find the index of the first force of a specific type
     * @param type The ForceType to find
     * @return Index of the force, or -1 if not found
     */
    int FindForceByType(ForceType type) const {
        for (size_t i = 0; i < forces.size(); ++i) {
            // Check if it's a LinearDirectionalForce
            auto* linearForce = dynamic_cast<LinearDirectionalForce*>(forces[i].get());
            if (linearForce && linearForce->type == type) {
                return static_cast<int>(i);
            }

            // Check if it's a FrictionForce
            auto* frictionForce = dynamic_cast<FrictionForce*>(forces[i].get());
            if (frictionForce && frictionForce->type == type) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }

    /**
     * @brief Check if a force of a specific type exists
     * @param type The ForceType to check
     * @return True if force type exists
     */
    bool HasForceType(ForceType type) const {
        return FindForceByType(type) != -1;
    }

private:
    std::vector<std::unique_ptr<Force>> forces;
};

class PhysicsComponent : public IComponent {
public:
    /**
     * @brief Constructs a physics component with mass and static properties
     * @param owner GameObject that owns this component
     * @param mass Object mass for physics calculations (default 1.0)
     * @param isStatic Whether object is immovable (default false)
     */
    PhysicsComponent(GameObject* owner, float mass = 1.0f, bool isStatic = false);

    /**
     * @brief Gets the component name
     * @return String identifier for this component type
     */
    std::string GetName() const override { return "PhysicsComponent"; }

    /**
     * @brief Creates a deep copy of this component
     * @param NewClone GameObject that will own the clone
     * @return Unique pointer to the cloned component
     */
    std::unique_ptr<IComponent>Clone(GameObject* NewClone) const override;

    // ===== Unity-Style Velocity API =====

    /**
     * @brief Adds to the current velocity (Unity-style)
     * @param deltaVel Velocity delta to add
     */
    void AddVelocity(const vec2& deltaVel);

    /**
     * @brief Sets the velocity of the object (legacy support)
     * @param x Horizontal velocity
     * @param y Vertical velocity
     */
    void SetVelocity(float x, float y);

    /**
     * @brief Gets the current velocity (legacy support)
     * @param x Output horizontal velocity
     * @param y Output vertical velocity
     */
    void GetVelocity(float& x, float& y) const;

    /**
    * @brief Sets the friction coefficient value
    * @param frictionCoeff New friction coefficient
    */
    void SetFrictionCoefficient(float frictionCoeff) {
        // Find the friction force if it exists
        int frictionIndex = forcesManager.FindForceByType(ForceType::Friction);

        if (frictionIndex != -1) {
            // Get the friction force and update its coefficient
            Force* force = forcesManager.GetForce(frictionIndex);
            if (force) {
                FrictionForce* frictionForce = dynamic_cast<FrictionForce*>(force);
                if (frictionForce) {
                    frictionForce->frictionCoefficient = frictionCoeff;
                }
            }
        }
    }

    /**
     * @brief Sets the grounded state
     * @param grounded Whether object is on ground
     */
    void SetGrounded(bool grounded) { isGrounded = grounded; }

    /**
     * @brief Checks if object is on ground
     * @return True if grounded
     */
    bool IsGrounded() const { return isGrounded; }

    /**
     * @brief Checks if object is falling
     * @return True if moving downward and not grounded
     */
    bool IsFalling() const { return velocity.y < 0 && !isGrounded; }

    /**
     * @brief Checks if object is rising
     * @return True if moving upward
     */
    bool IsRising() const { return velocity.y > 0; }

    // Properties
    /**
     * @brief Sets the object mass
     * @param m New mass value
     */
    void SetMass(float m) { mass = m; }

    /**
     * @brief Gets the object mass
     * @return Current mass
     */
    float GetMass() const { return mass; }

    /**
     * @brief Sets whether object is static
     * @param static_ Static state
     */
    void SetStatic(bool static_) { isStatic = static_; }

    /**
     * @brief Checks if object is static
     * @return True if static
     */
    bool IsStatic() const { return isStatic; }

    /**
     * @brief Sets whether object is kinematic (script-controlled movement)
     * @param kinematic Kinematic state
     */
    void SetKinematic(bool kinematic) { isKinematic = kinematic; }

    /**
     * @brief Checks if object is kinematic
     * @return True if kinematic (movement controlled by script, not physics)
     */
    bool IsKinematic() const { return isKinematic; }

    /**
     * @brief Enables or disables gravity (Unity-style)
     * @param useGrav Whether to apply gravity
     */
    void SetGravity(bool useGrav);

    /**
     * @brief Sets the gravity acceleration value
     * @param gravityAccel Gravity acceleration (typically negative Y value like -381.0f)
     */
    void SetGravityValue(float gravityAccel);

    /**
     * @brief Gets the gravity acceleration value
     * @return Gravity acceleration
     */
    float GetGravity() const;

    /**
     * @brief Applies a dash impulse (instant velocity change)
     * @param directionX Direction to dash (-1 for left, 1 for right)
     */
    void ApplyDashImpulse(float directionX) {
        // Remove any existing dash tracking force
        forcesManager.RemoveForcesByType(ForceType::Dash);
        forcesManager.RemoveForcesByType(ForceType::Jump);
        forcesManager.RemoveForcesByType(ForceType::Move);

        // Apply impulse directly to velocity (impulse = velocity change)
        float impulse = directionX * ForceConstants::DASH_FORCE;
        velocity.x = impulse / mass;  // Set velocity directly
        velocity.y = 0.0f;  // Zero out vertical velocity

        // Add a dummy dash "force" with 0 magnitude just to track that we're dashing
        // This allows HasForceType(Dash) to work for checking dash state
        forcesManager.AddCustomForce(std::make_unique<LinearDirectionalForce>(
            directionX, 0.0f,
            0.0f,  // No actual force applied
            ForceConstants::DASH_DURATION,
            ForceType::Dash
        ));
    }

    // ===== Forces Manager Access =====
    /**
     * @brief Get the forces manager
     * @return Reference to forces manager
     */
    ForcesManager& GetForcesManager() { return forcesManager; }

    // 2D specific helpers
    /**
     * @brief Calculates the speed (magnitude of velocity)
     * @return Current speed
     */
    float GetSpeed() const;

    /**
     * @brief Gets direction of movement
     * @return Direction in radians
     */
    float GetDirection() const;

    /**
     * @brief Sets velocity from angle and speed
     * @param angle Direction in radians
     * @param speed Movement speed
     */
    void SetDirection(float angle, float speed);

    /**
     * @brief Integrates forces into motion (velocity and position)
     * @param dt Delta time
     */
    void Integrate(float dt);

public:
    // Linear motion (vec2 velocity)
    vec2 velocity;

    // Properties
    float mass;
    bool isStatic;
    bool useGravity;
    bool isKinematic;
    float gravityAcceleration;

    // State Flags
    bool isGrounded;
    bool wasGrounded;
    bool isJumping;

    // Directional collision detection
    bool isCollidingLeft;
    bool isCollidingRight;
    bool isCollidingTop;
    bool isCollidingBottom;

    // Collision layer information for each direction
    CollisionLayer leftCollisionLayer;
    CollisionLayer rightCollisionLayer;
    CollisionLayer topCollisionLayer;
    CollisionLayer bottomCollisionLayer;

private:
    // Forces manager
    ForcesManager forcesManager;
};