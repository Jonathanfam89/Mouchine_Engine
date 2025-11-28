/* Start Header ************************************************************************/
/*!
\file       gameObject.cpp
\author     Jonathan Fam Qian Wei [73% - 68 lines]
\co-author	Muhammad Ashfer [27% - 25 lines]
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
    for (auto& [type, component] : components) {
        component->Destroy();
    }
    components.clear();
}

void GameObject::SetActive(bool active)
{
    isActive = active; 

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

void GameObject::OnEventTriggered(EventType eventType, GameObject* other)
{
    std::cout << "[OnEventTriggered] object=" << GetName()
        << " | event=" << EventTypeToString(eventType)
        << "\n";

    auto* ec = GetComponent<EventComponent>();

    std::cout << "[EventComponent] Rules count = " << ec->rules.size() << "\n";

    if (!ec) return;

    for (auto& rule : ec->rules)
    {
        std::cout << "Comparing: rule="
            << EventTypeToString(rule.eventType)
            << " vs event="
            << EventTypeToString(eventType)
            << "\n";

        // EVENT TYPE MATCH?
        if (rule.eventType != eventType)
            continue;

        // TARGET FILTER MATCH?
        if (!rule.targetFilter.empty())
        {
            if (!other) continue;
            if (other->GetName() != rule.targetFilter)
                continue;
        }

        // Execute Action
        std::cout << "Action = " << (int)rule.action << std::endl;
        std::cout << "Action = " << ActionTypeToString(rule.action) << std::endl;

        auto* t = GetComponent<TransformComponent>();

        switch (rule.action)
        {
        case ActionType::Shrink:        
            break;

        case ActionType::Grow:
            break;

        case ActionType::DestroyObject:
            SetActive(false);
            break;

        case ActionType::DisableObject:
            SetActive(false);
            break;

        case ActionType::EnableObject:
            SetActive(true);
            break;

        case ActionType::PlaySound:
            //CORE->GetAudioManager()->PlaySound("");
            break;

        case ActionType::LoadNextLevel:
     
            break;

        case ActionType::Respawn:
            if (t)
                t->SetPosition(0, 0);
            break;

        default:
            break;
        }
    }
}
