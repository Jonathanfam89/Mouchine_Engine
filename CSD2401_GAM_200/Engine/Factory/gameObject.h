/*====================================================================
Jonathan
im just putting my thought here for referance, 

here is where the gameobjects are being stored where the each gameobject
will have a entityID, name , and active-ness and as well as the componment array

was looking around ecs for inspiraion but with about little entities we going to have
so i decided to use the traditional Component system where we have a
core and factory.

as we know the component have many different systems and data and we 
and we need to add componment and remove accordingly, went to look thru the 
 and as well as checking the gameobject what does it has

as i was reseaching i saw the the unqineness of unordermaps and unique_ptr and how was it used
with abit of AI i was able to see create the templates below
for the unordered maps was becuase of its for faster lookup might look into std::maps when 
i need things to be ordered. i feel that i could have used vector but the find() is useful since i have the 
GameObjects named and the components easier.

As for the unique ptrs during my research most of the ecs have these ptrs looking, i realise it useful for creating these system to prevent
memory leaks and auto the parameters and since each gameobject has their own unique typeID and component array.

as for the template that has args... or basically Variadic Templates since different system have different parameters that they take in of course
i didnt know about only after using abit of AI to look at examples.


======================================================================*/

/* Start Header ************************************************************************/
/*!
\file       gameObject.h
\author     Jonathan Fam Qian Wei [100% - 87 lines]
\par        Fam.q@digipen.edu
\date       October, 2, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include "Component.h"
#include "GameManager.h"
#include "MathLib.h"

#include <vector>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <string>

using EntityID = std::uint32_t;
enum class EventType;
struct EventRule;

class EventSystem;
class EventComponent;

class GameObject {
public:

    /**
     * @brief Constructs a GameObject with a unique ID and name.
     * @param id The unique EntityID assigned by the GameObjectFactory.
     * @param name The desired name for the GameObject (defaults to "GameObject").
     */
    GameObject(EntityID id, const std::string& name = "GameObject");

    /**
     * @brief Creates a deep clone of this GameObject, copying all its components.
     * @param newName An optional new name for the cloned GameObject.
     * @return A pointer to the newly created GameObject clone.
     */
    GameObject* Clone(const std::string& newName = "")const;

    /**
     * @brief Destructor for the GameObject.
     */
    ~GameObject();

    // Component management
     /**
     * @brief Adds a component of type T to the GameObject.
     *
     * If a component of type T already exists, it is returned. Otherwise, a new component
     * is created, initialized, and added, taking ownership of the unique_ptr.
     *
     * @tparam T The type of the component to add (must derive from IComponent).
     * @tparam Args The types of the constructor arguments.
     * @param args The constructor arguments forwarded to the component's constructor.
     * @return A raw pointer to the added or existing component.
     */
    template<typename T, typename... Args>
    T* AddComponent(Args&&... args) {
        // Check if component already exists
        if (HasComponent<T>()) {
            return GetComponent<T>();
        }
        //if not then with the new created object add a component or more
        //args... is a way to get the variable number of contstrutor arguments
        //with std::forward it helps to forward  the other args to the component construtor
        auto component = std::make_unique<T>(this, std::forward<Args>(args)...);
        T* componentPtr = component.get();

        //std::type index is to get each of the unique ID for each component type
        std::type_index typeIndex(typeid(T));
        components[typeIndex] = std::move(component);

        // Initialize the component
        componentPtr->Initialize();

        return componentPtr;
    }

    /**
     * @brief Retrieves a component of type T from the GameObject.
     * @tparam T The type of the component to retrieve.
     * @return A raw pointer to the component, or nullptr if not found.
     */
    template<typename T>
    T* GetComponent() {
        std::type_index typeIndex(typeid(T));
        auto it = components.find(typeIndex);
        if (it != components.end()) {
            return static_cast<T*>(it->second.get());
        }
        return nullptr;
    }

    /**
     * @brief Checks if a component of type T exists on the GameObject.
     * @tparam T The type of the component to check for.
     * @return true if the component exists, false otherwise.
     */
    template<typename T>
    bool HasComponent() {
        std::type_index typeIndex(typeid(T));
        return components.find(typeIndex) != components.end();
    }

    /**
     * @brief Removes and destroys a component of type T from the GameObject.
     *
     * The component's Destroy() method is called before it is removed.
     *
     * @tparam T The type of the component to remove.
     */
    template<typename T>
    void RemoveComponent() {
        std::type_index typeIndex(typeid(T));
        auto it = components.find(typeIndex);
        if (it != components.end()) {
            it->second->Destroy();
            components.erase(it);
        }
    }

    void RemoveComponent(const std::string& name);

    /**
     * @brief Adds a pre-existing component to the GameObject, taking ownership.
     * @tparam T The type of the component to add.
     * @param component A unique pointer to the component to add.
     * @return A raw pointer to the newly added component.
     */
    template<typename T>
    T* AddComponent(std::unique_ptr<T> component) {
        // 1. Get the raw pointer to return before moving ownership
        T* raw_ptr = component.get();

        // 2. Insert the component into the map, casting the unique_ptr to the base type
        std::type_index typeIndex(typeid(T));
        components[typeIndex] = std::move(component);

        return raw_ptr;
    }

    /**
     * @brief Gets a vector of raw pointers to all components on the GameObject.
     * @return A vector of IComponent* pointers.
     */
    std::vector<IComponent*> GetAllComponentPointers() {
        std::vector<IComponent*> ComponentArray;
        ComponentArray.reserve(components.size());
        for (auto& pair : components) {
            ComponentArray.push_back(pair.second.get());
        }
        return ComponentArray;
    }

    /**
     * @brief Gets a vector of the names of all components on the GameObject.
     * @return A vector of strings containing the component names.
     */
    std::vector<std::string> GetAllComponentName() {
        std::vector<std::string> ComponentArray;
        ComponentArray.reserve(components.size());
        for (auto& pair : components) {
            ComponentArray.push_back(pair.second->GetName());
        }
        return ComponentArray;
    }

    /**
     * @brief Gets the count of components currently attached to the GameObject.
     * @return The number of components as a size_t.
     */
    size_t GetComponentCount() const {
        return components.size();
    }


    // GameObject lifecycle
    /**
     * @brief Initializes the GameObject and all its components.
     */
    void Initialize();

    /**
     * @brief Updates the GameObject and all its components.
     * @param deltaTime The time elapsed since the last frame.
     */
    void Update(float deltaTime);

    /**
     * @brief Destroys the GameObject and calls Destroy() on all its components.
     */
    void Destroy();

    // Getters
    /**
     * @brief Gets the unique EntityID of the GameObject.
     * @return The EntityID.
     */
    EntityID GetID() const { return entityID; }

    /**
     * @brief Gets the name of the GameObject.
     * @return A constant reference to the GameObject's name string.
     */
    const std::string& GetName() const { return name; }

    /**
     * @brief Sets new name for GameObject.
     * @param A constant reference to the GameObject's new name string.
     */
    void SetName(std::string const& newName) { name = newName; }

    // Enable/disable
    /**
     * @brief Sets the active state of the GameObject.
     * @param active If true, the GameObject is active; otherwise, it's inactive.
     */
    void SetActive(bool active);

    /**
     * @brief Checks if the GameObject is currently active.
     * @return true if the GameObject is active, false otherwise.
     */
    bool IsActive() const { return isActive; }

    void AddChild(GameObject* ChildGO);

    void RemoveChild(GameObject* ChildGO);

    void SetParent(GameObject* ParentGO);

    void RemoveParent();

    GameObject* GetParent() const;

    std::vector<GameObject*> GetAllChildren() const;

    /**
     * @brief Reordering of children - undo
     */
    void ReorderChildren(const std::vector<EntityID>& order);

    /**
     * @brief Execute a gameplay action defined by an EventRule.
     */
    void ExecuteAction(const EventRule& rule, GameObject* other);

    /**
     * @brief Called by the EventSystem when this GameObject receives an event.
     */
    virtual void OnEventTriggered(EventType event, GameObject* other);

    /**
     * @brief Get and Set Current Sound
     */
    const std::string& GetCurrentlyPlayingSound() const { return m_currentlyPlayingSound; }
    void SetCurrentlyPlayingSound(const std::string& s) { m_currentlyPlayingSound = s; }
    void ClearCurrentlyPlayingSound() { m_currentlyPlayingSound.clear(); }

    /**
     * @brief Get Original Postion of gameObject
     */
    vec2 originalPos{ 0, 0 };
    bool originalSaved = false;

private:
    EntityID entityID;
    std::string name;
    bool isActive;
    GameObject* parent;
    std::vector<GameObject*> children;                  
    std::unordered_map<std::type_index, std::unique_ptr<IComponent>> components;

    std::string m_currentlyPlayingSound;
};

void GetFamilyTree(GameObject* parent, std::vector<EntityID>& familyTree);