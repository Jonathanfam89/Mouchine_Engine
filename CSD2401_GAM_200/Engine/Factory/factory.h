/* Start Header ************************************************************************/
/*!
\file       factory.h
\author     Jonathan Fam Qian Wei [100% - 54 lines]
\par        Fam.q@digipen.edu
\date       October, 2, 2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include "GameObject.h"
#include "ColliderComponent.h"
#include <memory>
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include "RenderComponent.h"
#include <string>
#include "Animator.hpp"
#include "GraphicsSystem.h"
#include "transform.h"
#include "collision.h"
#include "physics.h"
#include <iostream>
#include <cmath>
#include <sstream>
#include <random> // For M1 demo, spawning 2.5k GOs
#include "ResourceManager.h"

class GameObjectFactory
{
public:
    /**
     * @brief Gets the singleton instance of the GameObjectFactory.
     * @return A reference to the single GameObjectFactory instance.
     */
    static GameObjectFactory &Instance()
    {
        static GameObjectFactory instance;
        return instance;
    }

    // Create GameObject
    /**
     * @brief Creates a basic, empty GameObject.
     * @param name The desired name for the new GameObject (defaults to "GameObject").
     * @return A pointer to the newly created GameObject.
     */
    GameObject *CreateGameObject(const std::string &name = "GameObject");

    // Create predefined 2D types
    /**
     * @brief Creates a player GameObject with a colored shape.
     * @param name The desired name for the new GameObject.
     * @param shape The shape type for rendering.
     * @param x The initial x-coordinate.
     * @param y The initial y-coordinate.
     * @param w The width.
     * @param h The height.
     * @param color The color of the shape.
     * @return A pointer to the newly created player GameObject.
     */
    GameObject *CreatePlayer(const std::string &name, RenderComponent::ShapeType shape, float x, float y, float w, float h, const vec4 color);

    /**
     * @brief Creates a player GameObject with a sprite texture.
     * @param name The desired name for the new GameObject.
     * @param shape The shape type for rendering.
     * @param x The initial x-coordinate.
     * @param y The initial y-coordinate.
     * @param w The width.
     * @param h The height.
     * @param sprite The path to the sprite file.
     * @return A pointer to the newly created player GameObject.
     */
    GameObject *CreatePlayer(const std::string &name, RenderComponent::ShapeType shape, float x, float y, float w, float h, const char *sprite);

    /**
     * @brief Creates a generic shaped GameObject with a color.
     * @param name The desired name for the new GameObject.
     * @param shape The shape type for rendering.
     * @param x The initial x-coordinate.
     * @param y The initial y-coordinate.
     * @param w The width.
     * @param h The height.
     * @param color The color of the shape.
     * @return A pointer to the newly created shape GameObject.
     */
    GameObject *CreateShape(const std::string &name, RenderComponent::ShapeType shape, float x, float y, float w, float h, const vec4 &color);

    /**
     * @brief Creates a generic shaped GameObject with a texture.
     * @param name The desired name for the new GameObject.
     * @param shape The shape type for rendering.
     * @param x The initial x-coordinate.
     * @param y The initial y-coordinate.
     * @param w The width.
     * @param h The height.
     * @param filename The path to the texture file.
     * @return A pointer to the newly created shape GameObject.
     */
    GameObject *CreateShape(const std::string &name, RenderComponent::ShapeType shape, float x, float y, float w, float h, const char *filename);

    /**
     * @brief Creates an image GameObject (usually a textured square).
     * @param name The desired name for the new GameObject.
     * @param shape The shape type for rendering (likely SQUARE).
     * @param x The initial x-coordinate.
     * @param y The initial y-coordinate.
     * @param w The width.
     * @param h The height.
     * @param filename The path to the image file.
     * @return A pointer to the newly created image GameObject.
     */
    GameObject *CreateImage(const std::string &name, RenderComponent::ShapeType shape, float x, float y, float w, float h, char const *filename);

    /**
     * @brief Creates an animated image GameObject from a sprite sheet.
     * @param name The desired name for the new GameObject.
     * @param x The initial x-coordinate.
     * @param y The initial y-coordinate.
     * @param w The width.
     * @param h The height.
     * @param filename The path to the sprite sheet file.
     * @param numRows The number of rows in the sprite sheet.
     * @param numCols The number of columns in the sprite sheet.
     * @param startFrame The index of the first frame for the animation.
     * @param endFrame The index of the last frame for the animation.
     * @param fps Frames per second for the animation.
     * @return A pointer to the newly created animated image GameObject.
     */
    GameObject *CreateAnimatedImage(const std::string &name, float x, float y, float w, float h,
                                    char const *filename, int numRows, int numCols,
                                    int startFrame, int endFrame, float fps);

    /**
     * @brief Creates a player GameObject with a colored shape and collision layer.
     * @param name The desired name for the new GameObject.
     * @param shape The shape type for rendering.
     * @param x The initial x-coordinate.
     * @param y The initial y-coordinate.
     * @param w The width.
     * @param h The height.
     * @param color The color of the shape.
     * @param layer The collision layer for the object.
     * @return A pointer to the newly created player GameObject.
     */
    GameObject *CreatePlayer(const std::string &name, RenderComponent::ShapeType shape, float x, float y, float w, float h, const vec4 color, CollisionLayer layer);

    /**
     * @brief Creates a player GameObject with a sprite texture and collision layer.
     * @param name The desired name for the new GameObject.
     * @param shape The shape type for rendering.
     * @param x The initial x-coordinate.
     * @param y The initial y-coordinate.
     * @param w The width.
     * @param h The height.
     * @param sprite The path to the sprite file.
     * @param layer The collision layer for the object.
     * @return A pointer to the newly created player GameObject.
     */
    GameObject *CreatePlayer(const std::string &name, RenderComponent::ShapeType shape, float x, float y, float w, float h, const char *sprite, CollisionLayer layer);

    /**
     * @brief Creates an empty GameObject that just has no shape, only Physics and transform.
     * @param name The desired name for the new GameObject.=
     * @param x The initial x-coordinate.
     * @param y The initial y-coordinate.
     * @param w The width.
     * @param h The height.
     */
    GameObject* CreateEmptyGO(const std::string& name, float x, float y, float w, float h);

    /**
     * @brief Creates a generic shaped GameObject with a color and collision layer.
     * @param name The desired name for the new GameObject.
     * @param shape The shape type for rendering.
     * @param x The initial x-coordinate.
     * @param y The initial y-coordinate.
     * @param w The width.
     * @param h The height.
     * @param color The color of the shape.
     * @param layer The collision layer for the object.
     * @return A pointer to the newly created shape GameObject.
     */
    GameObject *CreateShape(const std::string &name, RenderComponent::ShapeType shape, float x, float y, float w, float h, const vec4 &color, CollisionLayer layer);

    /**
     * @brief Creates an animated image GameObject with a collision layer.
     * @param name The desired name for the new GameObject.
     * @param x The initial x-coordinate.
     * @param y The initial y-coordinate.
     * @param w The width.
     * @param h The height.
     * @param filename The path to the sprite sheet file.
     * @param numRows The number of rows in the sprite sheet.
     * @param numCols The number of columns in the sprite sheet.
     * @param startFrame The index of the first frame for the animation.
     * @param endFrame The index of the last frame for the animation.
     * @param fps Frames per second for the animation.
     * @param layer The collision layer for the object.
     * @return A pointer to the newly created animated image GameObject.
     */
    GameObject *CreateAnimatedImage(const std::string &name, float x, float y, float w, float h,
                                    char const *filename, int numRows, int numCols,
                                    int startFrame, int endFrame, float fps, CollisionLayer layer);


    /**
     * @brief Creates a clone of an existing GameObject by its EntityID.
     * @param id The EntityID of the GameObject to clone.
     * @param newName The desired name for the clone (optional).
     * @return A pointer to the newly created GameObject clone.
     */
    GameObject *CloneGameObject(EntityID id, const std::string &newName = "");

    /**
     * @brief Creates a clone of an existing Player GameObject by its EntityID.
     * @param id The EntityID of the Player GameObject to clone.
     * @param newName The desired name for the clone (optional).
     * @return A pointer to the newly created Player GameObject clone.
     */
    GameObject *ClonePlayer(EntityID id, const std::string &newName = "");

    /**
     * @brief Creates a clone of an existing Shape GameObject by its EntityID.
     * @param id The EntityID of the Shape GameObject to clone.
     * @param newName The desired name for the clone (optional).
     * @return A pointer to the newly created Shape GameObject clone.
     */
    GameObject *CloneShape(EntityID id, const std::string &newName = "");

    /**
     * @brief Creates a clone of an existing Image GameObject by its EntityID.
     * @param id The EntityID of the Image GameObject to clone.
     * @param newName The desired name for the clone (optional).
     * @return A pointer to the newly created Image GameObject clone.
     */
    GameObject *CloneImage(EntityID id, const std::string &newName = "");

    /**
     * @brief Creates a clone of an existing Animated Image GameObject by its EntityID.
     * @param id The EntityID of the Animated Image GameObject to clone.
     * @param newName The desired name for the clone (optional).
     * @return A pointer to the newly created Animated Image GameObject clone.
     */
    GameObject *CloneAnimatedImage(EntityID id, const std::string &newName = "");

    // Management
    /**
     * @brief Retrieves a GameObject by its unique EntityID.
     * @param id The unique identifier of the GameObject.
     * @return A pointer to the GameObject, or nullptr if not found.
     */
    GameObject *GetGameObject(EntityID id);

    /**
     * @brief Finds the first GameObject with the specified name.
     * @param name The name of the GameObject to find.
     * @return A pointer to the found GameObject, or nullptr if not found.
     */
    GameObject *FindGameObject(const std::string &name);

    /**
     * @brief Gets a constant reference to the list of all active GameObjects.
     * @return A constant reference to the vector of unique pointers to GameObjects.
     */
    const std::vector<std::unique_ptr<GameObject>> &GetAllGameObjects() const;

    /**
     * @brief Gets the current count of GameObjects that have been cloned.
     * @return The total number of clones created.
     */
    unsigned int GetCloneCounter() { return cloneCounter; }

    /**
     * @brief Destroys a GameObject by its unique EntityID.
     * @param id The unique identifier of the GameObject to destroy.
     */
    void DestroyGameObject(EntityID id);

    /**
     * @brief Destroys all child and further descended game objects in the family tree. 
     */
    void DestroyAllRelatedGameObjects(EntityID parentID);

    /**
     * @brief Destroys all currently managed GameObjects.
     * This method clears all GameObjects but does NOT reset the EntityID counter.
     * Use this for level editor play/stop functionality to avoid ID conflicts.
     */
    void DestroyAllGameObjects();

    /**
     * @brief Destroys all currently managed GameObjects.
     */
    void Clear();

    GameObject* CreateGameObjectWithID(EntityID id, const std::string& name);
    void SetNextEntityID(EntityID id);
    EntityID GetNextEntityID() const;

    GameObject* FindByName(const std::string& name);

    /**
     * @brief Reorder all current GameObjects.
     */
    void ReorderRootObjects(const std::vector<EntityID>& order);

    /**
     * @brief Get the next smallest EntityID
     */
    EntityID GetSmallestAvailableID() const;

    /**
     * @brief Check if gameObject is alive
     */
    bool IsAlive(GameObject* ptr) const;

private:
    GameObjectFactory() : nextEntityID(1), cloneCounter(0) { gameObjects.reserve(1000); }           // HAVE TO PUT THIS HERE IF NOT CRASHES WHEN WE WANT TO CLONE LOL
    EntityID nextEntityID;
    std::vector<std::unique_ptr<GameObject>> gameObjects;
    std::unordered_map<EntityID, size_t> entityToIndex;
    unsigned int cloneCounter;
};

/**
* @brief Spawns 2500 Circle Game Objects on screen randomly within the 1920x1080 resolution window.
         (Can spawn more - at your own risk!)
*/
void DemoM1_SpawnGameObjects();

/**
* @brief Function that removes all Circle Game Objects spawned from the function above.
         (Yes even if you chose to spawn more than 2500)
*/
void DemoM1_DestroyGameObjects();

/**
 * @brief Handles the management of the spawning 1000 Gos
 */
void SpawnStressObjects();
void SpawnTest_Update();
void DestroyStressTestGameObjects();