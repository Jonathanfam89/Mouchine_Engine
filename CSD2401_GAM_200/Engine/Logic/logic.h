/* Start Header ************************************************************************/
/*!
\file       logic.h
\author     Lee Hwee Min [100% - 54 lines]
\par        l.hweemin@digipen.edu
\date       November, 29, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include "Component.h"
#include "MathLib.h"

#include <string>
#include <vector>
#include <memory>

class GameObject;
class GameObjectFactory;

/**
 * @brief All event types that a GameObject may react to.
 */
enum class EventType {
    None = 0,
    OnEnable, OnDisable, OnDestroy,                          // Lifecycle Events
    OnCollisionEnter, OnCollisionStay, OnCollisionExit,      // Physics Events
    //OnGoalReached                                          // Gameplay Events
};  

/**
 * @brief Actions executed when an EventRule triggers.
 */
enum class ActionType {
    None = 0,
    EnableObject, DisableObject, DestroyObject,             // Object State Action
    PlaySound, StopSound,                                   // Audio Action                                 
    Respawn,
};

/**
 * @brief Convert EventType to string for JSON serialization.
 */
const char* EventTypeToString(EventType e);
EventType StringToEventType(const std::string& s);

/**
 * @brief Convert ActionType to string for JSON serialization.
 */
const char* ActionTypeToString(ActionType a);;
ActionType StringToActionType(const std::string& s);

/**
 * @brief Each GameObject with an EventComponent holds multiple rules.
 */
struct EventRule {
    EventType eventType = EventType::None;

    // The ID of the OTHER object required to trigger this rule.
    int targetObjectId = -1; // -1 = Any
    ActionType action = ActionType::None;

    // Action Parameters
    float floatValue = 0.f;
    float floatValue2 = 0.f;
    float floatValue3 = 0.f;

    std::string stringValue = "";
    
    vec2 vec2Value = { 0,0 };
    vec4 colorValue = { 1, 1, 1, 1 };
};

/**
 * @brief This component stores a list of controlled rules.
 * Whenever a GameObject receives an event, it checks these rules
 * to determine which actions to perform.
 */
class EventComponent : public IComponent {
public:
    std::vector<EventRule> rules;

    EventComponent(GameObject* owner);
    EventComponent();

    std::unique_ptr<IComponent> Clone(GameObject* newClone) const override;
    
    std::string GetName() const override { return "EventComponent"; }
};
/**
 * @brief A queued gameplay event waiting to be processed.
 */
struct QueuedEvent {
    EventType type;
    GameObject* target;
    GameObject* other;
};

/**
 * @brief The EventSystem acts as a dispatcher
 */
class EventSystem {
public:
    static std::vector<QueuedEvent> events;

    static void Raise(EventType type, GameObject* target, GameObject* other = nullptr);
    static void ProcessEvents();
    static void Clear();
};
