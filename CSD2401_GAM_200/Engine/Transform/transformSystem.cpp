
/* Start Header ************************************************************************/
/*!
\file       TransformSystem.cpp
\author     Jonathan Fam Qian Wei [100% - 21 lines]
\par        Fam.q@digipen.edu
\date       Novenber, 7, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/


#include "transformSystem.h"

TransformSystem::TransformSystem() {

}

TransformSystem::~TransformSystem() {

}
void TransformSystem::Update(float dt) {
	auto& factory = GameObjectFactory::Instance();
	const auto& gameObjects = factory.GetAllGameObjects();
	
	for (const auto& obj : gameObjects) {
		const auto& physics = obj->GetComponent<PhysicsComponent>();
		const auto& transform = obj->GetComponent<TransformComponent>();
	
		if (physics && transform && !physics->IsStatic()) {
			IntegrateVelocity(physics, transform, dt);
		}
	}
};

/**
* @brief Updates position based on velocity (logic moved from PhysicsSystem)
*/
void TransformSystem::IntegrateVelocity(PhysicsComponent* physics, TransformComponent* transform, float deltaTime) {
	transform->Move(physics->velocity.x * deltaTime, physics->velocity.y * deltaTime);
}
