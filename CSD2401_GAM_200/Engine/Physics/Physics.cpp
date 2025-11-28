/* Start Header ************************************************************************/
/*!
\file       Physics.cpp
\author     Khalil Belabadia [100% - 67 lines]
\par        khalil.b@digipen.edu
\date       October, 2, 2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "Physics.h"
#include "Transform.h"
#include "GameObject.h"
#include "ColliderComponent.h"
#include <iostream>
#include <cmath>

/**
 * @brief Constructs physics component with initial settings
 */
PhysicsComponent::PhysicsComponent(GameObject* owner, float m, bool static_)
    : IComponent(owner),
    velocity{ 0.0f, 0.0f },
    mass(m), isStatic(static_), useGravity(true), isKinematic(false),
    gravityAcceleration(ForceConstants::GRAVITY_ACCELERATION),
    isGrounded(true), isJumping(false), wasGrounded(true),
    isCollidingLeft(false), isCollidingRight(false),
    isCollidingTop(false), isCollidingBottom(false),
    leftCollisionLayer(CollisionLayer::Default),
    rightCollisionLayer(CollisionLayer::Default),
    topCollisionLayer(CollisionLayer::Default),
    bottomCollisionLayer(CollisionLayer::Default)
{
    // Gravity is now a modifiable acceleration value
}

/**
 * @brief Creates a deep copy of this physics component
 */
std::unique_ptr<IComponent> PhysicsComponent::Clone(GameObject* NewClone) const
{
    auto clonedComponent = std::make_unique<PhysicsComponent>(
        NewClone,
        this->mass,
        this->isStatic
    );

    clonedComponent->velocity = this->velocity;
    clonedComponent->gravityAcceleration = this->gravityAcceleration;

    // Note: Forces are not deep-copied since they're time-dependent
    // If needed, implement force cloning logic here

    clonedComponent->SetEnabled(this->IsEnabled());

    return clonedComponent;
}

/**
 * @brief Adds to the current velocity
 */
void PhysicsComponent::AddVelocity(const vec2& deltaVel)
{
    velocity.x += deltaVel.x;
    velocity.y += deltaVel.y;
}

/**
 * @brief Sets the velocity of the object (legacy)
 */
void PhysicsComponent::SetVelocity(float x, float y)
{
    velocity = { x, y };
}

/**
 * @brief Gets the current velocity values (legacy)
 */
void PhysicsComponent::GetVelocity(float& x, float& y) const
{
    x = velocity.x;
    y = velocity.y;
}

/**
 * @brief Sets whether gravity is enabled
 */
void PhysicsComponent::SetGravity(bool useGrav)
{
    useGravity = useGrav;
}

/**
 * @brief Sets the gravity acceleration value
 */
void PhysicsComponent::SetGravityValue(float gravityAccel)
{
    gravityAcceleration = gravityAccel;
}

/**
 * @brief Gets the gravity acceleration value
 */
float PhysicsComponent::GetGravity() const
{
    return gravityAcceleration;
}

/**
 * @brief Calculates the speed magnitude from velocity components
 */
float PhysicsComponent::GetSpeed() const
{
    return std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
}

/**
 * @brief Calculates the direction angle from velocity components
 */
float PhysicsComponent::GetDirection() const
{
    return std::atan2(velocity.y, velocity.x);
}

/**
 * @brief Sets velocity from polar coordinates
 */
void PhysicsComponent::SetDirection(float angle, float speed)
{
    velocity.x = std::cos(angle) * speed;
    velocity.y = std::sin(angle) * speed;
}

/**
 * @brief Integrates forces into motion (velocity and position)
 */
void PhysicsComponent::Integrate(float dt)
{
    if (isStatic) return; // Static objects don't move

    // Validate and remove expired forces
    forcesManager.ValidateAge(dt);

    // Calculate total acceleration from all forces
    vec2 accel = forcesManager.CalculateTotalAcceleration(mass, velocity);

    // Add gravity acceleration if enabled AND not grounded
    if (useGravity && !isGrounded)
    {
        accel.y += gravityAcceleration;
    }

    // Integrate velocity: v = v0 + a*dt
    velocity.x += accel.x * dt;
    velocity.y += accel.y * dt;

    // Integrate position: p = p0 + v*dt
    TransformComponent* transform = owner->GetComponent<TransformComponent>();
    if (transform)
    {
        transform->Move(velocity.x * dt, velocity.y * dt);
    }

    // Reset jump state if grounded
    if (isGrounded)
        isJumping = false;
}