/* Start Header ************************************************************************/
/*!
\file       EnumConverters.h
\author     Lee Hwee Min [100% - 42 lines]
\date       November, 2, 2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#pragma once
#include <string>
#include "ColliderComponent.h"

/**
 *   @Brief Provides helper functions to convert between enumerated engine types
 *    and human-readable string representations. This allows JSON-based
 *    serialization and ImGui editors to display meaningful labels instead
 *    of raw integer enum values.
 */

namespace EnumConverters
{
    /*!
    * @brief Converts a ColliderComponent::Type enum value to its string name.
    * @param type, The collider shape type (Square, Circle, etc.)
    *
    * @return A string representing the enum name.
    */
    inline std::string ToString(ColliderComponent::Type type)
    {
        switch (type)
        {
        case ColliderComponent::Type::Square: return "Square";
        case ColliderComponent::Type::Circle: return "Circle";
        default: return "Unknown";
        }
    }

    /*!
    * @brief Converts a string name to a ColliderComponent::Type enum value.
    * param str, The string ("Square", "Circle", etc.)
    *
    * @return, Corresponding enum value. Defaults to Square if unrecognized.
    */
    inline ColliderComponent::Type ToColliderType(const std::string& str)
    {
        if (str == "Square") return ColliderComponent::Type::Square;
        if (str == "Circle") return ColliderComponent::Type::Circle;
        return ColliderComponent::Type::Square;
    }

    /*!
    * @brief Converts a CollisionLayer enum value to its string name.
    * @param layer, The collision layer (Player, Ground, Wall, etc.)
    *
    *@return, A string representing the layer name.
    */
    inline std::string ToString(CollisionLayer layer)
    {
        switch (layer)
        {
        case CollisionLayer::Player:   return "Player";
        case CollisionLayer::Ground:   return "Ground";
        case CollisionLayer::Enemy:    return "Enemy";
        case CollisionLayer::Platform: return "Platform";
        default:                       return "Default";
        }
    }

    /*!
    * @brief Converts a string name to a CollisionLayer enum value.
    * @param s, The string ("Player", "Ground", etc.)
    *
    * @return Corresponding CollisionLayer. Defaults to CollisionLayer::Default.
    */
    inline CollisionLayer ToCollisionLayer(const std::string& s)
    {
        if (s == "Player")   return CollisionLayer::Player;
        if (s == "Ground")   return CollisionLayer::Ground;
        if (s == "Enemy")    return CollisionLayer::Enemy;
        if (s == "Platform") return CollisionLayer::Platform;
        return CollisionLayer::Default;
    }
} // namespace EnumConverters
