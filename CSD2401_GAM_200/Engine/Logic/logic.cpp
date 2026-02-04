/* Start Header ************************************************************************/
/*!
\file       logic.cpp
\author     Lee Hwee Min [100% - 82 lines]
\par        l.hweemin@digipen.edu
\date       November, 29, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "logic.h"
#include "gameObject.h"
#include "factory.h"

#include <iostream>

std::vector<QueuedEvent> EventSystem::events;

const char* EventTypeToString(EventType e)
{
    switch (e)
    {
    case EventType::None: return "None";
    case EventType::OnEnable: return "OnEnable";
    case EventType::OnDisable: return "OnDisable";
    case EventType::OnDestroy: return "OnDestroy";
    case EventType::OnCollisionEnter: return "OnCollisionEnter";
    case EventType::OnCollisionStay: return "OnCollisionStay";
    case EventType::OnCollisionExit: return "OnCollisionExit";
    }
    return "Unknown";
}

EventType StringToEventType(const std::string& s)
{
    if (s == "None") return EventType::None;
    if (s == "OnEnable") return EventType::OnEnable;
    if (s == "OnDisable") return EventType::OnDisable;
    if (s == "OnDestroy") return EventType::OnDestroy;
    if (s == "OnCollisionEnter") return EventType::OnCollisionEnter;
    if (s == "OnCollisionStay") return EventType::OnCollisionStay;
    if (s == "OnCollisionExit") return EventType::OnCollisionExit;

    return EventType::None;
}

const char* ActionTypeToString(ActionType a)
{
    switch (a)
    {
    case ActionType::None: return "None";
    case ActionType::EnableObject: return "EnableObject";
    case ActionType::DisableObject: return "DisableObject";
    case ActionType::DestroyObject: return "DestroyObject";
    case ActionType::PlaySound: return "PlaySound";
    case ActionType::StopSound: return "StopSound";
    case ActionType::Respawn: return "Respawn";

    }
    return "Unknown";
}

ActionType StringToActionType(const std::string& s)
{
    if (s == "None") return ActionType::None;
    if (s == "EnableObject") return ActionType::EnableObject;
    if (s == "DisableObject") return ActionType::DisableObject;
    if (s == "DestroyObject") return ActionType::DestroyObject;
    if (s == "PlaySound") return ActionType::PlaySound;
    if (s == "StopSound") return ActionType::StopSound;
    if (s == "Respawn") return ActionType::Respawn;

    return ActionType::None;
}

EventComponent::EventComponent(GameObject* owner) : IComponent(owner) {}

EventComponent::EventComponent() : IComponent(nullptr) {}

/**
 * @brief Clone this component when duplicating a prefab or GameObject.
 *        All rules are copied to the new instance.
 */
std::unique_ptr<IComponent> EventComponent::Clone(GameObject* newClone) const {
    auto clone = std::make_unique<EventComponent>(newClone);
    clone->rules = rules;
    return clone;
}

/**
 * @brief Clear all pending events.
 *        Used when switching scenes or resetting the editor.
 */
void EventSystem::Clear() {
    events.clear();
}

/**
 * @brief Queue a new event to be processed next frame.
 */
void EventSystem::Raise(EventType type, GameObject* target, GameObject* other) {    
    events.push_back({ type, target, other });
}

/**
 * @brief Safely process all queued gameplay events.
 */
void EventSystem::ProcessEvents() {
    auto queue = events;
    EventSystem::Clear();

    auto& factory = GameObjectFactory::Instance();

    for (auto& e : queue) {
        // Validate target pointer
        if (!e.target || !factory.IsAlive(e.target))
            continue;

        // Validate "other" pointer
        if (e.other && !factory.IsAlive(e.other))
            e.other = nullptr;
        if (e.target) {
            e.target->OnEventTriggered(e.type, e.other);
        }
    }
}