/* Start Header ************************************************************************/
/*!
\file       gameObject.cpp
\author     Jonathan Fam Qian Wei [44% - 68 lines]
\co-author	Muhammad Ashfer [16% - 25 lines]
\co-author  Lee Hwee Min [39% - 60 lines]
\par        Fam.q@digipen.edu
\date       October, 2, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "GameObject.h"
#include "factory.h"
#include "transform.h"
#include "RenderComponent.h"
#include "Physics.h"
#include "ColliderComponent.h"
#include "ScriptComponent.h"
#include "CameraComponent.h"
#include "logic.h"


#include <iostream>
#include <sstream>

 /* brief
   the constructor for the gameobejcts

    */
GameObject::GameObject(EntityID id, const std::string& name)
    : entityID(id), name(name), isActive(true), parent(nullptr) {
}

/* brief
  this function is the cloning of the gamebjects copying the parameters and values from a gameobject 
  to a new a new with the same values and parameters

   */
GameObject* GameObject::Clone(const std::string& newName) const {
    GameObjectFactory& factory = GameObjectFactory::Instance();

    // Ensure we use the unique name passed by the factory, with a fallback
    std::string finalName = newName.empty() ? (this->name + " (FallbackClone)") : newName;

    // 1. Create the new GameObject using the unique name
    GameObject* clone = factory.CreateGameObject(finalName);

    // 2. Iterate over all components and clone them
    for (const auto& pair : components) {
        const std::unique_ptr<IComponent>& originalComponent = pair.second;

        // Call the component's virtual Clone method
        std::unique_ptr<IComponent> clonedComponent = originalComponent->Clone(clone);

        // Move the cloned component into the new GameObject's map
        clone->components[pair.first] = std::move(clonedComponent);
    }

    // 3. Copy state and initialize
    clone->isActive = this->isActive;
    if (this->GetParent())
    {
        clone->SetParent(this->GetParent());
    }
    clone->Initialize();

    return clone;
}
/* brief
  deconstructor

   */
GameObject::~GameObject() {
    Destroy();
}
/* brief
   initializes the components a gameobject

   */
void GameObject::Initialize() {
    for (auto& [type, component] : components) {
        component->Initialize();
    }
}
/* brief
   update the components a gameobject if it is enabled  call the updates

   */
void GameObject::Update(float deltaTime) {
    if (!isActive) return;

    for (auto& [type, component] : components) {
        if (component->IsEnabled()) {
            component->Update(deltaTime);
        }
    }
}
/* brief
   destorys the gameobject and calls the destroy function for the components

   */
void GameObject::Destroy() {
    EventSystem::Raise(EventType::OnDestroy, this, nullptr);

    for (auto& [type, component] : components) {
        component->Destroy();
    }
    components.clear();
}

void GameObject::SetActive(bool active)
{
    if (isActive == active)
        return;

    isActive = active; 

    // Enable/disable all components
    for (auto& [type, component] : components) {
        component->SetEnabled(active);
    }

    if (active)
    {
        EventSystem::Raise(EventType::OnEnable, this, nullptr);
    }
    else
    {
        EventSystem::Raise(EventType::OnDisable, this, nullptr);
    }

    std::vector<EntityID> familyTree{};
    GetFamilyTree(this, familyTree);

    for (auto& childID : familyTree)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* child = factory.GetGameObject(childID);
        child->SetActive(active);
    }
}

void GameObject::AddChild(GameObject* ChildGO)
{
    ChildGO->parent = this;
    children.push_back(ChildGO);
    std::cout << this->GetName() <<" is the parent of: \n";
    for (auto& child : children)
    {
        std::cout << '\t' << child->GetName() << std::endl;
    }
}

void GameObject::RemoveChild(GameObject* ChildGO)
{
    auto it = std::find(children.begin(), children.end(), ChildGO);
    if (it != children.end())
    {
        children.erase(it);
        //RemoveParent();
        ChildGO->RemoveParent();
        std::cout << "Removed " << ChildGO->GetName() << " from " << this->GetName() << "'s children\n";
    }
}

void GameObject::SetParent(GameObject* ParentGO)
{
    if (GetParent())
    {
        GetParent()->RemoveChild(this);
    }
    ParentGO->AddChild(this);
}

void GameObject::RemoveParent()
{
    parent = nullptr;
}

GameObject* GameObject::GetParent() const
{
    if (!parent) return nullptr;
    return parent;
}

std::vector<GameObject*> GameObject::GetAllChildren() const
{
    return children;
}

void GetFamilyTree(GameObject* parent, std::vector<EntityID>& familyTree)
{
    for (auto& child : parent->GetAllChildren()) {
        familyTree.push_back(child->GetID());
        GetFamilyTree(child, familyTree); // Recursively get grandchildren
    }
}

/**
 * @brief Reorders this GameObject's children based on a new ordering of IDs.
 */
void GameObject::ReorderChildren(const std::vector<EntityID>& newOrder) {
    if (children.empty()) return;

    // map id to gameObject*
    std::unordered_map<EntityID, GameObject*> lookup;
    lookup.reserve(children.size());

    for (GameObject* c : children) {
        if (c)
            lookup[c->GetID()] = c;
    }

    std::vector<GameObject*> reordered;
    reordered.reserve(newOrder.size());

    // Rebuild list in correct order
    for (EntityID id : newOrder) {
        auto it = lookup.find(id);
        if (it != lookup.end()) {
            reordered.push_back(it->second);
        }
    }
    // Reassign back
    children = reordered;
}

/**
 * @brief Execute a gameplay action defined by an EventRule.
 */
void GameObject::ExecuteAction(const EventRule& rule, GameObject* other) {
    (void)other;
    switch (rule.action)
    {
    case ActionType::EnableObject:
        SetActive(true);
        break;

    case ActionType::DisableObject:
        SetActive(false);
        break;

    case ActionType::DestroyObject:
        Destroy();
        break;

    case ActionType::PlaySound:
        {
            auto* audioMgr = CORE->GetAudioManager();
            if (!audioMgr) break;

            // rule.stringValue contains the sound name
            if (!rule.stringValue.empty())
            {
                audioMgr->PlaySound(rule.stringValue, "sfx");
                //currentlyPlayingSound = rule.stringValue;
                SetCurrentlyPlayingSound(rule.stringValue);

            }
        }
        break;

    case ActionType::StopSound:
        {
            auto* audio = CORE->GetAudioManager();
            if (!audio) break;

            const std::string& sound = this->GetCurrentlyPlayingSound();

            if (!sound.empty())
            {
                audio->StopSound(sound);

                this->ClearCurrentlyPlayingSound();
            }
            else
            {
                std::cout << "[StopSound] Nothing to stop for this object\n";
            }
        }
        break;

    case ActionType::Respawn:
    {
        auto* tc = GetComponent<TransformComponent>();
        if (tc) {
            // Respawn here
            tc->SetPosition(this->originalPos.x, this->originalPos.y);
            //tc->SetPosition(0,0);
        }
        break;
    }

    default:
        break;
    }
}

/**
 * @brief Called by the EventSystem when this GameObject receives an event.
 */
void GameObject::OnEventTriggered(EventType type, GameObject* other) {
    // Get this object's eventComponent
    auto* ec = GetComponent<EventComponent>();
    if (!ec) return;

    for (auto& rule : ec->rules) {
        if (rule.eventType != type) continue;

        if (rule.targetObjectId != -1)
        {
            if (!other) continue;
            if (other->GetID() != static_cast<unsigned int>(rule.targetObjectId))
                continue;
        }
        ExecuteAction(rule, other);
    }
}

void GameObject::RemoveComponent(const std::string& name) {
    if (name == "TransformComponent") {
        RemoveComponent<TransformComponent>();
    } else if (name == "RenderComponent") {
        RemoveComponent<RenderComponent>();
    } else if (name == "PhysicsComponent") {
        RemoveComponent<PhysicsComponent>();
    } else if (name == "ColliderComponent") {
        RemoveComponent<ColliderComponent>();
    } else if (name == "ScriptComponent") {
        RemoveComponent<ScriptComponent>();
    } else if (name == "CameraComponent") {
        RemoveComponent<CameraComponent>();
    } else if (name == "EventComponent") {
        RemoveComponent<EventComponent>();
    }
}
