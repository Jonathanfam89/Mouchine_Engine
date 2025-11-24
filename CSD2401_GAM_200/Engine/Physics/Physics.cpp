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
#include <iostream>
#include <cmath>

/**
 * @brief Constructs physics component with initial settings
 */
PhysicsComponent::PhysicsComponent(GameObject* owner, float m, bool static_)
    : IComponent(owner),
    velocityX(0), velocityY(0),
    mass(m), isStatic(static_), useGravity(true),
    isGrounded(true), isJumping(false), wasGrounded(true)
{
    // Gravity is now a constant acceleration in Integrate(), not a force
}

/**
 * @brief Sets the velocity directly
 */
void PhysicsComponent::SetVelocity(float x, float y)
{
    velocityX = x;
    velocityY = y;
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

    clonedComponent->velocityX = this->velocityX;
    clonedComponent->velocityY = this->velocityY;

    // Note: Forces are not deep-copied since they're time-dependent
    // If needed, implement force cloning logic here

    clonedComponent->SetEnabled(this->IsEnabled());

    return clonedComponent;
}

/**
 * @brief Gets the current velocity values
 */
void PhysicsComponent::GetVelocity(float& x, float& y) const
{
    x = velocityX;
    y = velocityY;
}

/**
 * @brief Calculates the speed magnitude from velocity components
 */
float PhysicsComponent::GetSpeed() const
{
    return std::sqrt(velocityX * velocityX + velocityY * velocityY);
}

/**
 * @brief Calculates the direction angle from velocity components
 */
float PhysicsComponent::GetDirection() const
{
    return std::atan2(velocityY, velocityX);
}

/**
 * @brief Sets velocity from polar coordinates
 */
void PhysicsComponent::SetDirection(float angle, float speed)
{
    velocityX = std::cos(angle) * speed;
    velocityY = std::sin(angle) * speed;
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
    vec2 velocity = { velocityX, velocityY };
    vec2 accel = forcesManager.CalculateTotalAcceleration(mass, velocity);

    // Add constant gravity acceleration if enabled
    if (useGravity)
    {
        accel.y += ForceConstants::GRAVITY_ACCELERATION;
    }

    // Integrate velocity: v = v0 + a*dt
    velocityX += accel.x * dt;
    velocityY += accel.y * dt;

    // Integrate position: p = p0 + v*dt
    TransformComponent* transform = owner->GetComponent<TransformComponent>();
    if (transform)
    {
        transform->Move(velocityX * dt, velocityY * dt);
    }

    // Reset jump state if grounded
    if (isGrounded)
        isJumping = false;
}