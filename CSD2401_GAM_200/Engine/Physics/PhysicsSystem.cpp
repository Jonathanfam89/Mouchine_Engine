/* Start Header ************************************************************************/
/*!
\file       PhysicsSystem.cpp
\author     Khalil Belabadia [98% - 380 lines]
            Lee Hwee Min [2% - 10 lines]
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
            // Clear directional collision flags
            physics->isCollidingLeft = false;
            physics->isCollidingRight = false;
            physics->isCollidingTop = false;
            physics->isCollidingBottom = false;
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
    vec2 vel1 = physics1->velocity;
    vec2 vel2 = physics2->velocity;

    float relVelX = vel1.x - vel2.x;
    float relVelY = vel1.y - vel2.y;
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
        vel1.x += impulseX / mass1;
        vel1.y += impulseY / mass1;
        physics1->velocity = vel1;
    }

    if (!physics2->IsStatic())
    {
        vel2.x -= impulseX / mass2;
        vel2.y -= impulseY / mass2;
        physics2->velocity = vel2;
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

        // Set collision flags for obj1 (always set, even if kinematic)
        if (direction < 0) {
            physics1->isCollidingRight = true;
            physics1->rightCollisionLayer = col2->GetLayer();
        }
        else {
            physics1->isCollidingLeft = true;
            physics1->leftCollisionLayer = col2->GetLayer();
        }

        // Set collision flags for obj2 (always set, even if kinematic)
        if (direction < 0) {
            physics2->isCollidingLeft = true;
            physics2->leftCollisionLayer = col1->GetLayer();
        }
        else {
            physics2->isCollidingRight = true;
            physics2->rightCollisionLayer = col1->GetLayer();
        }

        // Resolve collision for non-kinematic objects
        bool obj1Kinematic = physics1->IsKinematic();
        bool obj2Kinematic = physics2->IsKinematic();

        if (!physics1->IsStatic() && !obj1Kinematic && transform1)
        {
            transform1->Move(direction * overlapX * 0.5f, 0);
            physics1->velocity.x = 0.0f;
        }
        if (!physics2->IsStatic() && !obj2Kinematic && transform2)
        {
            transform2->Move(-direction * overlapX * 0.5f, 0);
            physics2->velocity.x = 0.0f;
        }

        // If one is kinematic and other isn't, push the non-kinematic one fully
        if (obj1Kinematic && !obj2Kinematic && !physics2->IsStatic() && transform2) {
            transform2->Move(-direction * overlapX, 0);  // Push full amount
        }
        if (obj2Kinematic && !obj1Kinematic && !physics1->IsStatic() && transform1) {
            transform1->Move(direction * overlapX, 0);   // Push full amount
        }

        // CRITICAL: Push kinematic objects when colliding with static objects
        if (obj1Kinematic && physics2->IsStatic() && transform1) {
            transform1->Move(direction * overlapX, 0);  // Push kinematic away from static
        }
        if (obj2Kinematic && physics1->IsStatic() && transform2) {
            transform2->Move(-direction * overlapX, 0);  // Push kinematic away from static
        }
    }
    else
    {
        // Resolve on Y axis
        float direction = (aabb1.min.y + aabb1.max.y) < (aabb2.min.y + aabb2.max.y) ? -1.0f : 1.0f;

        auto* transform1 = obj1->GetComponent<TransformComponent>();
        auto* transform2 = obj2->GetComponent<TransformComponent>();

        // Set collision flags for obj1 (always set, even if kinematic)
        if (direction > 0) {
            physics1->isCollidingBottom = true;
            physics1->bottomCollisionLayer = col2->GetLayer();
            physics1->SetGrounded(true);
        }
        else {
            physics1->isCollidingTop = true;
            physics1->topCollisionLayer = col2->GetLayer();
        }

        // Set collision flags for obj2 (always set, even if kinematic)
        if (direction < 0) {
            physics2->isCollidingBottom = true;
            physics2->bottomCollisionLayer = col1->GetLayer();
            physics2->SetGrounded(true);
        }
        else {
            physics2->isCollidingTop = true;
            physics2->topCollisionLayer = col1->GetLayer();
        }

        // Resolve collision for non-kinematic objects
        bool obj1Kinematic = physics1->IsKinematic();
        bool obj2Kinematic = physics2->IsKinematic();

        if (!physics1->IsStatic() && !obj1Kinematic && transform1)
        {
            transform1->Move(0, direction * overlapY * 0.5f);
            physics1->velocity.y = 0.0f;
        }
        if (!physics2->IsStatic() && !obj2Kinematic && transform2)
        {
            transform2->Move(0, -direction * overlapY * 0.5f);
            physics2->velocity.y = 0.0f;
        }

        // If one is kinematic and other isn't, push the non-kinematic one fully
        if (obj1Kinematic && !obj2Kinematic && !physics2->IsStatic() && transform2) {
            transform2->Move(0, -direction * overlapY);  // Push full amount
        }
        if (obj2Kinematic && !obj1Kinematic && !physics1->IsStatic() && transform1) {
            transform1->Move(0, direction * overlapY);   // Push full amount
        }

        // CRITICAL: Push kinematic objects when colliding with static objects
        if (obj1Kinematic && physics2->IsStatic() && transform1) {
            transform1->Move(0, direction * overlapY);  // Push kinematic away from static
        }
        if (obj2Kinematic && physics1->IsStatic() && transform2) {
            transform2->Move(0, -direction * overlapY);  // Push kinematic away from static
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

        vec2 circleVel = circlePhysics->velocity;

        // Check if this is a player-enemy collision
        bool isPlayerSquare = (squareCol->GetLayer() == CollisionLayer::Player);
        bool isEnemyCircle = (circleCol->GetLayer() == CollisionLayer::Enemy);

        // Separate
        if (!squarePhysics->IsStatic() && squareTransform)
        {
            // Set directional collision flags for square based on collision normal
            // Normal points from square to circle, so negate for square's perspective
            if (-nx > 0.5f) {
                squarePhysics->isCollidingLeft = true;
                squarePhysics->leftCollisionLayer = circleCol->GetLayer();
            }
            else if (-nx < -0.5f) {
                squarePhysics->isCollidingRight = true;
                squarePhysics->rightCollisionLayer = circleCol->GetLayer();
            }

            if (-ny > 0.5f) {
                squarePhysics->isCollidingBottom = true;
                squarePhysics->bottomCollisionLayer = circleCol->GetLayer();
            }
            else if (-ny < -0.5f) {
                squarePhysics->isCollidingTop = true;
                squarePhysics->topCollisionLayer = circleCol->GetLayer();
            }

            // Only resolve collision (move/stop) if not kinematic
            // But if kinematic and colliding with non-static circle, still push kinematic away
            if (!squarePhysics->IsKinematic()) {
                squareTransform->Move(-nx * penetration * 0.5f, -ny * penetration * 0.5f);
            }
            else if (!circlePhysics->IsStatic()) {
                // Kinematic square colliding with non-static circle - push square fully
                squareTransform->Move(-nx * penetration, -ny * penetration);
            }

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
                    if (squarePhysics->velocity.y < 0) {
                        squarePhysics->velocity.y = 0.0f;
                    }

                    std::cout << "[Grounded] Player standing on enemy!" << std::endl;
                }
                // NOTE: Knockback is now handled in InputManager, not here!
                // PhysicsSystem only detects collision, game script decides response
            }
            else
            {
                // For non-player collisions, zero velocity as before (only if not kinematic)
                if (!squarePhysics->IsKinematic()) {
                    squarePhysics->velocity = { 0.0f, 0.0f };
                }
            }
        }
        if (!circlePhysics->IsStatic() && !circlePhysics->IsKinematic() && circleTransform)
        {
            // Push circle away - if square is kinematic, push full amount
            float pushMultiplier = squarePhysics->IsKinematic() ? 1.0f : 0.5f;
            circleTransform->Move(nx * penetration * pushMultiplier, ny * penetration * pushMultiplier);

            // Apply bounce to circle
            float restitution = 0.9f;
            float dot = circleVel.x * nx + circleVel.y * ny;

            circleVel.x = circleVel.x - (1 + restitution) * dot * nx;
            circleVel.y = circleVel.y - (1 + restitution) * dot * ny;

            circlePhysics->velocity = circleVel;

            // Check for grounding
            if (ny > 0.7f && std::abs(circleVel.y) < 1.0f)
            {
                circlePhysics->velocity.y = 0.0f;
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
    pendingCollisions.push_back({ objA, objB, colA, colB });
}

void PhysicsSystem::ClearCollisionQueue()
{
    pendingCollisions.clear();
    std::cout << "[PhysicsSystem] Collision queue cleared." << std::endl;
}