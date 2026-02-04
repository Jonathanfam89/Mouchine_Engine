/* Start Header ************************************************************************/
/*!
\file       main.h
\author     Lee Hwee Min [100% - 23 lines]
\par        l.hweemin@digipen.edu
\date       October, 2, 2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once 

#include "Core.h"
#include "Factory.h"
#include "GameObjectSystem.h"
#include "GraphicsSystem.h"
#include "ImGuiSystem.h"
#include "DebugSystem.h"
#include "Transform.h"
#include "Collision.h"
#include "Collidercomponent.h"
#include "Physics.h"
#include "RenderComponent.h"
#include <iostream>
#include <cmath>
#include "Animator.hpp"
#include "ResourceManager.h"
#include "audio.h"
#include "InputManager.h"
#include "ScriptAPI.h"
#include "ScriptingSystem.h"
#include "Serialization.h"
#include "PrefabManager.h"
#include "LevelEditor.h"
#include "ScriptComponent.h"


#include "GameManager.h"
#include "Level1.h"

/**
 * @brief Initializes all main systems and resources for the game.
 */
void Main_init();

/**
 * @brief Updates all systems and processes logic each frame.
 */
void Main_Update();

/**
 * @brief Renders all game objects and systems each frame.
 */
void Main_Draw();

/**
 * @brief Frees all allocated resources when shutting down the game.
 */
void Main_Free();

/**
 * @brief Unloads resources (textures, audio, etc.) from memory.
 */
void Main_Unload();