/* Start Header ************************************************************************/
/*!
\file       gameObjectSystem.cpp
\author     Jonathan Fam Qian Wei [29% - 20 lines]
            Lee Hwee Min [ 71% - 51 lines]
\par        Fam.q@digipen.edu
\date       October, 2, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "GameObjectSystem.h"
#include "GameObject.h"
#include "Factory.h"
#include <iostream>
#include "Message.h"
 GameObjectSystem::GameObjectSystem() : factory(GameObjectFactory::Instance()) {}


 /* brief
    this is to inititalize the gameobjectSystem.

    */
 void GameObjectSystem::Initialize() {
     std::cout << "GameObjectSystem initialized" << std::endl;
 }
 /* brief
   this is the update function that loops together engine loop to check whether the gameobjects are active and update accordingly to the 

   */

 void GameObjectSystem::Update(float timeslice) {
    
     auto& gameObjects = factory.GetAllGameObjects();

     for (auto& obj_unique_ptr : gameObjects) {
         GameObject* obj = obj_unique_ptr.get(); 

        
         if (obj == nullptr) {
             std::cerr << "ERROR: Null GameObject pointer found in factory list!" << std::endl;
             continue; 
         }

         if (obj->IsActive()) { 
             obj->Update(timeslice);
         }
     }
 }


 /**
  * @brief Handles CreateObjectMessage to create new GameObjects.
  *
  * Depending on the message type (Empty, Basic, or Prefab), this function
  * initializes a new GameObject using the GameObjectFactory and attaches
  * necessary components such as Transform, Render, and Collider.
  *
  * @param msg Contains the object name, type, shape, position, scale, rotation, and color.
  */

 void GameObjectSystem::HandleMessage(const CreateObjectMessage& msg)
 {
     std::cout << "[GameObjectSystem] Received CreateObjectMessage for "
         << msg.name << " (Type: " << static_cast<int>(msg.type) << ")\n";

     //GameObjectFactory& factory = GameObjectFactory::Instance();
     GameObject* go = nullptr;

     switch (msg.type)
     {

     case CreateObjectMessage::ObjectType::Empty:
     {
         std::cout << "[GameObjectSystem] Creating EMPTY GameObject: " << msg.name << "\n";

         // Use your existing CreateEmptyGO() exactly as is
         go = factory.CreateEmptyGO(
             msg.name,
             msg.pos[0],
             msg.pos[1],
             msg.scale[0],
             msg.scale[1]
         );

         break;
     }

     case CreateObjectMessage::ObjectType::Basic:
     {
         std::cout << "[GameObjectSystem] Creating BASIC GameObject: " << msg.name << "\n";

         // Use the normal CreateGameObject()
         go = factory.CreateGameObject(msg.name);
         if (!go) break;

         // --- Transform (always required) ---
         auto* transform = go->AddComponent<TransformComponent>();
         if (transform)
         {
             transform->SetPosition(msg.pos[0], msg.pos[1]);
             transform->SetScale(msg.scale[0], msg.scale[1]);
             transform->SetRotation(msg.rotation);
         }

         // --- Render ---
         auto* render = go->AddComponent<RenderComponent>();
         if (render)
         {
             render->SetShapeType(msg.shapeType);
             render->SetColor({ msg.color.x, msg.color.y, msg.color.z, msg.color.w });
         }

         // --- Physics ---
         //go->AddComponent<PhysicsComponent>();

         // --- Collider ---
         auto* collider = go->AddComponent<ColliderComponent>();
         if (collider) 
         {
             collider->setColliShape(msg.colType);
             collider->SetSize(msg.colSize[0], msg.colSize[1]);
             collider->SetLayer(msg.colLayer);
         }

         break;
     }

     case CreateObjectMessage::ObjectType::Prefab:
     {
         std::cout << "[GameObjectSystem] Creating PREFAB GameObject: " << msg.name << "\n";
         break;
     }

     default:
         std::cerr << "[GameObjectSystem] Unknown CreateObjectMessage type!\n";
         break;
     }

     if (go)
     {
         std::cout << "[GameObjectSystem] Created object ptr: " << go << " Name: " << go->GetName() << std::endl;
         std::cout << "   Has Transform: " << (go->GetComponent<TransformComponent>() ? "Yes" : "No") << std::endl;
         std::cout << "   Has Render:    " << (go->GetComponent<RenderComponent>() ? "Yes" : "No") << std::endl;
         std::cout << "   Has Physics:   " << (go->GetComponent<PhysicsComponent>() ? "Yes" : "No") << std::endl;
         std::cout << "   Has Collider:  " << (go->GetComponent<ColliderComponent>() ? "Yes" : "No") << std::endl;
         
         // Update the last undo entry with new ID
#ifdef USE_IMGUI
         LevelEditor* editor = CORE->GetLevelEditor();
         if (editor)
         {
             EditorUndoCommand undocmd;
             undocmd.action = EditorUndoCommand::ActionType::ObjectCreated;
             undocmd.entityID = go->GetID();

             // Save newly created object as JSON
             undocmd.afterJson = Serialization::SerializeGameObject(go);

             editor->RecordUndo(undocmd);
         }
#endif
     }
 }

