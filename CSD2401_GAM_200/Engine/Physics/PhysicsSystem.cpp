/* Start Header ************************************************************************/
/*!
\file       PhysicsSystem.cpp
\author     Khalil Belabadia [96% - 222 lines]
            Lee Hwee Min [4% - 10 lines]
\par        khalil.b@digipen.edu
\date       October, 2, 2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "PhysicsSystem.h"
#include "GameObject.h"
#include "Factory.h"
#include "Physics.h"
#include "transform.h"
#include "Collision.h"
#include <iostream>
#include <cmath>
#include "MathLib.h"
#include "logic.h"

#include "Message.h"
#include "IMessageHandler.h"

/**
 * @brief Default constructor
 */
PhysicsSystem::PhysicsSystem()
{
}

/**
 * @brief Destructor
 */
PhysicsSystem::~PhysicsSystem()
{
}

/**
 * @brief Initializes the physics system
 */
void PhysicsSystem::Initialize()
{
    std::cout << "Initializing system: PhysicsSystem" << std::endl;
    //CORE->GetMessageBus().Subscribe<CollisionMessage>(this);
}

/**
 * @brief Main physics update loop
 * Integrates physics for all objects, then resolves collisions
 */
void PhysicsSystem::Update(float deltaTime)
{
    auto& factory = GameObjectFactory::Instance();
    const auto& gameObjects = factory.GetAllGameObjects();

    // Integrate physics for all objects
    for (const auto& obj : gameObjects)
    {
        auto* physics = obj->GetComponent<PhysicsComponent>();
        if (physics && physics->IsEnabled())
        {
            physics->Integrate(deltaTime);
        }
    }

    // Clear grounded state for ALL objects before resolving collisions
    for (const auto& obj : gameObjects)
    {
        auto* physics = obj->GetComponent<PhysicsComponent>();
        if (physics && !physics->IsStatic())
        {
            physics->SetGrounded(false);
        }
    }

    // Resolve pending collisions
    for (const auto& collision : pendingCollisions)
    {
        auto* col1 = collision.col1;
        auto* col2 = collision.col2;

        if (col1->GetType() == ColliderComponent::Type::Circle &&
            col2->GetType() == ColliderComponent::Type::Circle)
        {
            ResolveCircleCircle(collision.obj1, collision.obj2, col1, col2);
        }
        else if (col1->GetType() == ColliderComponent::Type::Square &&
            col2->GetType() == ColliderComponent::Type::Square)
        {
            ResolveSquareSquare(collision.obj1, collision.obj2, col1, col2);
        }
        else
        {
            ResolveMixed(collision.obj1, collision.obj2, col1, col2);
        }
    }

    pendingCollisions.clear();
}

/**
 * @brief Stores collision for later resolution in physics update
 */
void PhysicsSystem::ReportCollision(GameObject* obj1, GameObject* obj2,
    ColliderComponent* col1, ColliderComponent* col2)
{
    pendingCollisions.push_back({ obj1, obj2, col1, col2 });
}

/**
 * @brief Resolves elastic collision between two circles
 * Handles separation and momentum transfer
 */
void PhysicsSystem::ResolveCircleCircle(GameObject* obj1, GameObject* obj2,
    ColliderComponent* col1, ColliderComponent* col2)
{
    auto* physics1 = obj1->GetComponent<PhysicsComponent>();
    auto* physics2 = obj2->GetComponent<PhysicsComponent>();

    if (!physics1 || !physics2)
        return;
    if (physics1->IsStatic() && physics2->IsStatic())
        return;

    Circle c1 = col1->GetCircle();
    Circle c2 = col2->GetCircle();

    float dx = c2.center.x - c1.center.x;
    float dy = c2.center.y - c1.center.y;
    float distSq = dx * dx + dy * dy;
    float distance = std::sqrt(distSq);

    // CRITICAL: If circles are on top of each other, separate them manually
    if (distance < 0.1f)
    {
        // Generate a separation direction (prefer vertical to prevent sticking)
        distance = 0.1f;
        dx = 0.0f;
        dy = 0.1f;

        // Separate them immediately
        auto* transform1 = obj1->GetComponent<TransformComponent>();
        auto* transform2 = obj2->GetComponent<TransformComponent>();
        if (transform1 && transform2)
        {
            if (!physics1->IsStatic())
                transform1->Move(0, 1.0f);
            if (!physics2->IsStatic())
                transform2->Move(0, -1.0f);
        }
        return; // Skip physics resolution this frame
    }

    float nx = dx / distance;
    float ny = dy / distance;

    // Separate circles
    float radiusSum = c1.radius + c2.radius;
    float overlap = radiusSum - distance;

    // Safety: ignore collision if overlap is unreasonably large
    if (overlap < 0 || overlap > 100.0f)
        return;

    if (overlap > 0)
    {
        auto* transform1 = obj1->GetComponent<TransformComponent>();
        auto* transform2 = obj2->GetComponent<TransformComponent>();

        if (transform1 && transform2)
        {
            float separationX = nx * overlap * 0.5f;
            float separationY = ny * overlap * 0.5f;

            if (!physics1->IsStatic())
            {
                transform1->Move(-separationX, -separationY);
            }
            if (!physics2->IsStatic())
            {
                transform2->Move(separationX, separationY);
            }
        }
    }

    // Apply bounce
    float vx1, vy1, vx2, vy2;
    physics1->GetVelocity(vx1, vy1);
    physics2->GetVelocity(vx2, vy2);

    float relVelX = vx1 - vx2;
    float relVelY = vy1 - vy2;
    float velAlongNormal = relVelX * nx + relVelY * ny;

    // Check if either is a player - players don't bounce as much
    bool isPlayer1 = (col1->GetLayer() == CollisionLayer::Player);
    bool isPlayer2 = (col2->GetLayer() == CollisionLayer::Player);
    float restitution = (isPlayer1 || isPlayer2) ? 0.2f : 0.8f;

    float mass1 = physics1->GetMass();
    float mass2 = physics2->GetMass();

    // Safety: prevent division by zero
    if (mass1 < 0.01f)
        mass1 = 0.01f;
    if (mass2 < 0.01f)
        mass2 = 0.01f;

    float impulse = -(1 + restitution) * velAlongNormal;
    impulse /= (1.0f / mass1 + 1.0f / mass2);

    float impulseX = impulse * nx;
    float impulseY = impulse * ny;

    if (!physics1->IsStatic())
    {
        vx1 += impulseX / mass1;
        vy1 += impulseY / mass1;
        physics1->SetVelocity(vx1, vy1);
    }

    if (!physics2->IsStatic())
    {
        vx2 -= impulseX / mass2;
        vy2 -= impulseY / mass2;
        physics2->SetVelocity(vx2, vy2);
    }
}

/**
 * @brief Resolves collision between two AABBs
 * Separates on shortest axis and handles grounding
 */
void PhysicsSystem::ResolveSquareSquare(GameObject* obj1, GameObject* obj2,
    ColliderComponent* col1, ColliderComponent* col2)
{
    auto* physics1 = obj1->GetComponent<PhysicsComponent>();
    auto* physics2 = obj2->GetComponent<PhysicsComponent>();

    if (!physics1 || !physics2)
        return;
    if (physics1->IsStatic() && physics2->IsStatic())
        return;

    AABB aabb1 = col1->GetAABB();
    AABB aabb2 = col2->GetAABB();

    float overlapX = std::min(aabb1.max.x - aabb2.min.x, aabb2.max.x - aabb1.min.x);
    float overlapY = std::min(aabb1.max.y - aabb2.min.y, aabb2.max.y - aabb1.min.y);

    if (overlapX < overlapY)
    {
        // Resolve on X axis
        float direction = (aabb1.min.x + aabb1.max.x) < (aabb2.min.x + aabb2.max.x) ? -1.0f : 1.0f;

        auto* transform1 = obj1->GetComponent<TransformComponent>();
        auto* transform2 = obj2->GetComponent<TransformComponent>();

        if (!physics1->IsStatic() && transform1)
        {
            transform1->Move(direction * overlapX * 0.5f, 0);
            physics1->SetVelocity(0, physics1->velocityY);
        }
        if (!physics2->IsStatic() && transform2)
        {
            transform2->Move(-direction * overlapX * 0.5f, 0);
            physics2->SetVelocity(0, physics2->velocityY);
        }
    }
    else
    {
        // Resolve on Y axis
        float direction = (aabb1.min.y + aabb1.max.y) < (aabb2.min.y + aabb2.max.y) ? -1.0f : 1.0f;

        auto* transform1 = obj1->GetComponent<TransformComponent>();
        auto* transform2 = obj2->GetComponent<TransformComponent>();

        if (!physics1->IsStatic() && transform1)
        {
            transform1->Move(0, direction * overlapY * 0.5f);
            physics1->SetVelocity(physics1->velocityX, 0);
            // obj1 is ABOVE obj2 when direction > 0, so it's landing on top
            if (direction > 0 && physics1->velocityY <= 0)
            {
                physics1->SetGrounded(true);
            }
        }
        if (!physics2->IsStatic() && transform2)
        {
            transform2->Move(0, -direction * overlapY * 0.5f);
            physics2->SetVelocity(physics2->velocityX, 0);
            // obj2 is ABOVE obj1 when direction < 0, so it's landing on top
            if (direction < 0 && physics2->velocityY <= 0)
            {
                physics2->SetGrounded(true);
            }
        }
    }
}

/**
 * @brief Resolves collision between circle and square
 * Handles separation and bounce physics
 * Applies knockback to player (square) when hit by enemy (circle)
 */
void PhysicsSystem::ResolveMixed(GameObject* obj1, GameObject* obj2,
    ColliderComponent* col1, ColliderComponent* col2)
{
    ColliderComponent* squareCol = (col1->GetType() == ColliderComponent::Type::Square) ? col1 : col2;
    ColliderComponent* circleCol = (col1->GetType() == ColliderComponent::Type::Circle) ? col1 : col2;
    GameObject* squareObj = (col1->GetType() == ColliderComponent::Type::Square) ? obj1 : obj2;
    GameObject* circleObj = (col1->GetType() == ColliderComponent::Type::Circle) ? obj1 : obj2;

    auto* squarePhysics = squareObj->GetComponent<PhysicsComponent>();
    auto* circlePhysics = circleObj->GetComponent<PhysicsComponent>();

    if (!squarePhysics || !circlePhysics)
        return;

    AABB rect = squareCol->GetAABB();
    Circle circle = circleCol->GetCircle();

    float closestX = std::max(rect.min.x, std::min(circle.center.x, rect.max.x));
    float closestY = std::max(rect.min.y, std::min(circle.center.y, rect.max.y));

    float dx = circle.center.x - closestX;
    float dy = circle.center.y - closestY;
    float distance = std::sqrt(dx * dx + dy * dy);

    if (distance < 0.0001f)
        return;

    float nx = dx / distance;
    float ny = dy / distance;

    float penetration = circle.radius - distance;

    if (penetration > 0)
    {
        auto* squareTransform = squareObj->GetComponent<TransformComponent>();
        auto* circleTransform = circleObj->GetComponent<TransformComponent>();

        float cvx, cvy;
        circlePhysics->GetVelocity(cvx, cvy);

        // Check if this is a player-enemy collision
        bool isPlayerSquare = (squareCol->GetLayer() == CollisionLayer::Player);
        bool isEnemyCircle = (circleCol->GetLayer() == CollisionLayer::Enemy);

        // Separate
        if (!squarePhysics->IsStatic() && squareTransform)
        {
            squareTransform->Move(-nx * penetration * 0.5f, -ny * penetration * 0.5f);

            // Handle player-enemy collision
            if (isPlayerSquare && isEnemyCircle)
            {
                // Check if player is landing on top of the enemy
                // ny < 0 means collision normal points downward (player is above circle)
                bool isLandingOnTop = (ny < -0.5f);  // Normal pointing significantly downward

                if (isLandingOnTop)
                {
                    // Player is on top of enemy - treat as grounded
                    squarePhysics->SetGrounded(true);

                    // Stop downward velocity to prevent phasing through
                    float svx, svy;
                    squarePhysics->GetVelocity(svx, svy);
                    if (svy < 0) {  // If moving downward
                        squarePhysics->SetVelocity(svx, 0);  // Stop vertical movement
                    }

                    std::cout << "[Grounded] Player standing on enemy!" << std::endl;
                }
                else
                {
                    // Player hit from side or below - apply elastic collision knockback
                    // Use same physics as circle-circle collision

                    float svx, svy;
                    squarePhysics->GetVelocity(svx, svy);

                    // Calculate relative velocity
                    float relVelX = svx - cvx;
                    float relVelY = svy - cvy;
                    float velAlongNormal = relVelX * (-nx) + relVelY * (-ny);  // Using -n because we want player's perspective

                    // Calculate elastic collision impulse (same as circle-circle)
                    float restitution = 0.8f;  // High bounce
                    float mass1 = squarePhysics->GetMass();
                    float mass2 = circlePhysics->GetMass();

                    if (mass1 < 0.01f) mass1 = 0.01f;
                    if (mass2 < 0.01f) mass2 = 0.01f;

                    float impulse = -(1 + restitution) * velAlongNormal;
                    impulse /= (1.0f / mass1 + 1.0f / mass2);

                    // Calculate impulse direction (away from circle)
                    float impulseX = impulse * (-nx);  // Opposite of collision normal
                    float impulseY = impulse * (-ny);

                    // Apply impulse as velocity change
                    svx += impulseX / mass1;
                    svy += impulseY / mass1;
                    squarePhysics->SetVelocity(svx, svy);

                    // Also apply constant knockback force for sustained push
                    squarePhysics->GetForcesManager().ApplyKnockback(-nx, -ny);

                    std::cout << "[Knockback] Player hit by enemy! Impulse: ("
                        << impulseX << ", " << impulseY << ")" << std::endl;
                }
            }
            else
            {
                // For non-player collisions, zero velocity as before
                squarePhysics->SetVelocity(0, 0);
            }
        }
        if (!circlePhysics->IsStatic() && circleTransform)
        {
            circleTransform->Move(nx * penetration, ny * penetration);

            // Apply bounce to circle
            float restitution = 0.9f;
            float dot = cvx * nx + cvy * ny;

            cvx = cvx - (1 + restitution) * dot * nx;
            cvy = cvy - (1 + restitution) * dot * ny;

            circlePhysics->SetVelocity(cvx, cvy);

            // Check for grounding
            if (ny > 0.7f && std::abs(cvy) < 1.0f)
            {
                circlePhysics->SetVelocity(cvx, 0);
                circlePhysics->SetGrounded(true);
            }
        }
    }
}

/**
 * @brief Queues collision pairs for physics resolution.
 */
void PhysicsSystem::HandleMessage(const CollisionMessage& msg)
{
    // msg.objectA and msg.objectB are GameObject pointers
    GameObject* objA = static_cast<GameObject*>(msg.objectA);
    GameObject* objB = static_cast<GameObject*>(msg.objectB);

    if (!objA || !objB)
        return;

    auto* colA = objA->GetComponent<ColliderComponent>();
    auto* colB = objB->GetComponent<ColliderComponent>();
    if (!colA || !colB)
        return;

    auto RaiseIfNeeded = [&](GameObject* receiver, GameObject* other)
        {
            if (receiver->GetComponent<EventComponent>())
                EventSystem::Raise(EventType::OnCollisionEnter, receiver, other);
        };

    RaiseIfNeeded(objA, objB);
    RaiseIfNeeded(objB, objA);

    // Queue for resolution during physics update
    pendingCollisions.push_back({objA, objB, colA, colB});
}

void PhysicsSystem::ClearCollisionQueue()
{
    pendingCollisions.clear();
    std::cout << "[PhysicsSystem] Collision queue cleared." << std::endl;
}
