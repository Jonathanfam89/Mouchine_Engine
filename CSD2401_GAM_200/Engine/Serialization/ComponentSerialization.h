/* Start Header ************************************************************************/
/*!
\file       ComponentSerialization.h
\author     Lee Hwee Min [100% - 31 lines]
\par        l.hweemin@digipen.edu
\date       November, 3, 2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/


#pragma once
#include <nlohmann/json.hpp>
#include "GameObject.h"
#include "Transform.h"
#include "RenderComponent.h"
#include "TextComponent.h"
#include "CameraComponent.h"
#include "Physics.h"
#include "Animator.hpp"
#include "ColliderComponent.h"
#include "EnumConverters.h"
#include "Serialization.h"
#include "logic.h"

class Spritesheet;
using json = nlohmann::ordered_json;

extern bool DND_firstObject;
extern float DND_rootJsonX;
extern float DND_rootJsonY;

/**
* @brief Declares per-component serialization and deserialization functions.
*    Each component knows how to read and write its own data to/from JSON.
*/
namespace ComponentSerialization
{
    // ----------- Serialize Individual Components -----------------
    void SerializeTransform(json& j, const TransformComponent* t);
    void SerializeRender(json& j, RenderComponent* r);
    void SerializeSpritesheet(json& j, Spritesheet* s);
    void SerializePhysics(json& j, PhysicsComponent* p);
    void SerializeCollider(json& j, ColliderComponent* c);
    void SerializeText(json& j, TextComponent* tc);
    void SerializeCamera(json& j, CameraComponent* cam);
    void SerializeEvent(json& j, EventComponent* ec);

    // ----------- Deserialize Individual Components ----------------
    void DeserializeTransform(const json& j, GameObject* go);
    void DeserializeRender(const json& j, GameObject* go);
    void DeserializeSpritesheet(const json& j, GameObject* go);
    void DeserializePhysics(const json& j, GameObject* go);
    void DeserializeCollider(const json& j, GameObject* go);
    void DeserializeText(const json& j, GameObject* go);
    void DeserializeCamera(const json& j, GameObject* go);
    void DeserializeEvent(const json& j, GameObject* go);

    // ----------- Convenience Wrappers ------------------------------
    void SerializeAllComponents(GameObject* go, json& j);
    void DeserializeAllComponents(GameObject* go, const json& j);
}
