/* Start Header ************************************************************************/
/*!
\file       Message.h
\author     Lee Hwee Min [100% - 116 lines]
\par        l.hweemin@digipen.edu.sg
\date       Novemeber, 3, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#pragma once
#include <string>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <MathLib.h>
#include <RenderComponent.h>
#include <typeindex>
#include <ColliderComponent.h>

// Forward declaration
class IMessageHandler;

class Message {
public:
    /* @brief Message identifiers used to classify message types.
    */
    enum class Id {
        UNKNOWN,
        KEY_INPUT,
        PLAY_SOUND,
        PAUSE_SOUND,
        COLLISION,
        SCENE_CHANGE,
        CREATE_OBJECT,
        PLAY_ANIMATION
    };

    virtual ~Message() = default;

    /* @brief Message identifiers used to classify message types.
    *  @returns the message type
    */
    Id GetType() const { return type; }

protected:
    Id type;

    /* @brief Protected constructor to enforce type-safe creation
    */
    explicit Message(Id type) : type(type) {}
};

/**
 * @brief Message for keyboard input events.
 */
class KeyMessage : public Message {
public:
    int keyCode;
    bool isPressed;

    KeyMessage(int key, bool pressed)
        : Message(Id::KEY_INPUT), keyCode(key), isPressed(pressed) {
    }
};

/**
 * @brief Message for playing a sound.
 */
class PlaySoundMessage : public Message {
public:
    std::string soundName;
    explicit PlaySoundMessage(const std::string& sound)
        : Message(Id::PLAY_SOUND), soundName(sound) {
    }
};

/**
 * @brief Message for pausing or resuming a sound channel.
 */
class PauseSoundMessage : public Message {
public:
    std::string channelName;
    bool pause;

    PauseSoundMessage(const std::string& channel, bool pauseState)
        : Message(Id::PAUSE_SOUND), channelName(channel), pause(pauseState) {
    }
};


/**
 * @brief Message representing a collision event between two objects.
 */
class CollisionMessage : public Message {
public:
    void* objectA;
    void* objectB;

    CollisionMessage(void* a, void* b)
        : Message(Id::COLLISION), objectA(a), objectB(b) {
    }
};


/**
 * @brief Message for creating a new GameObject in the scene.
 */
class CreateObjectMessage : public Message {
public:
    enum class ObjectType { Empty, Basic, Prefab };

    std::string name;
    ObjectType type;
    RenderComponent::ShapeType shapeType;
    float pos[2];
    float scale[2];
    float rotation;
    vec4 color;
    bool addCollider;
    ColliderComponent::Type colType;
    float colSize[2] = { 100.0f, 100.0f };
    CollisionLayer colLayer;

    CreateObjectMessage(const std::string& objName,
        ObjectType objType,
        RenderComponent::ShapeType shape,
        const float position[2],
        const float scaleVal[2],
        float rot,
        const vec4& col,
        bool hasCollider = false,
        ColliderComponent::Type cType = ColliderComponent::Type::Square,
        float cw = 100.0f,
        float ch = 100.0f,
        CollisionLayer cl = CollisionLayer::Default)

        : Message(Id::CREATE_OBJECT),
        name(objName),
        type(objType),
        shapeType(shape),
        rotation(rot),
        color(col),
        addCollider(hasCollider),
        colType(cType),
        colLayer(cl)

    {
        pos[0] = position[0];   pos[1] = position[1];
        scale[0] = scaleVal[0]; scale[1] = scaleVal[1];
        colSize[0] = cw;    colSize[1] = ch;
    }

};


/**
 * @brief Central hub for subscribing to and publishing messages.
 *
 * The MessageBus enables communication between systems without tight coupling.
 * Systems can subscribe to message types and react when messages are published.
 */
class MessageBus {
private:
    std::unordered_map<std::type_index, std::vector<IMessageHandler*>> subscribers;

public:
    
    /**
     * @brief Subscribes a handler to a specific message type.
     * @tparam T Message type to subscribe to.
     * @param handler Pointer to the subscribing IMessageHandler.
     */
    template<typename T>
    void Subscribe(IMessageHandler* handler) {
        subscribers[typeid(T)].push_back(handler);
    }

    /**
    * @brief Unsubscribes a handler from a specific message type.
    * @tparam T Message type to unsubscribe from.
    * @param handler Pointer to the handler to remove.
    */
    template<typename T>
    void Unsubscribe(IMessageHandler* handler) {
        auto it = subscribers.find(typeid(T));
        if (it != subscribers.end()) {
            auto& vec = it->second;
            vec.erase(std::remove(vec.begin(), vec.end(), handler), vec.end());
        }
    }

    void UnsubscribeAll(IMessageHandler* handler)
    {
        for (auto& entry : subscribers)
        {
            auto& list = entry.second;
            list.erase(std::remove(list.begin(), list.end(), handler), list.end());
        }
    }

    /**
    * @brief Publishes a message to all subscribed handlers of its type.
    * @tparam T Message type to publish.
    * @param message Reference to the message instance.
    */
    template<typename T>
    void Publish(const T& message) {
        auto it = subscribers.find(typeid(T));
        if (it == subscribers.end()) return;

        auto& handlers = it->second;
        for (IMessageHandler* handler : handlers) {
            if (handler)
                handler->HandleMessage(message);
        }
        std::cout << "[MessageBus] Finished publishing \n";
    }

    // Clear all subscribers
    void Clear() { subscribers.clear(); }
};
