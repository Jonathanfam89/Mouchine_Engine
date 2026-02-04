/* Start Header ************************************************************************/
/*!
\file       factory.cpp
\author     Jonathan Fam Qian Wei [80% - 392 lines]
\co-author  Lee Hwee Min [ 20% - 100 lines]
\par        Fam.q@digipen.edu
\date       October, 2, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "Factory.h"
#include <queue>

/*
    /brief
    Creation of the Gameobjects. it the single argument constructor create and
    pushes the gameobject id in to the gameobject system.
*/
GameObject *GameObjectFactory::CreateGameObject(const std::string &name)
{
    EntityID id = nextEntityID++;
    auto gameObject = std::make_unique<GameObject>(id, name);
    GameObject *ptr = gameObject.get();

    entityToIndex[id] = gameObjects.size();
    gameObjects.push_back(std::move(gameObject));

    return ptr;
}
/*
    /brief
     This is the blueprint for the player creation where
     it has 2 overloads with the sprite and the color,
     color overload is for the creating the shape and the sprite is for the texture.
     in the future will create a hidden variable for player as the camera and inputs shld be on the player gameobject only.

*/

GameObject *GameObjectFactory::CreatePlayer(const std::string &name, RenderComponent::ShapeType shape, float x, float y, float w, float h, const vec4 color, CollisionLayer)
{
    GameObject *player = CreateGameObject(name);

    auto *transform = player->AddComponent<TransformComponent>(x, y);
    auto *physics = player->AddComponent<PhysicsComponent>(1.0f, false);
    player->AddComponent<RenderComponent>(shape, color);
    transform->SetScale(w, h);
    physics->SetVelocity(0.f, 0.f);

    // No gravity for this player variant

    std::cout << "Created Player: " << name << " at (" << x << ", " << y << ")" << std::endl;

    return player;
}

GameObject *GameObjectFactory::CreatePlayer(const std::string &name, RenderComponent::ShapeType shape, float x, float y, float w, float h, const char *filename, CollisionLayer layer)
{
    GameObject *player = CreateGameObject(name);
    auto *transform = player->AddComponent<TransformComponent>(x, y);
    auto *physics = player->AddComponent<PhysicsComponent>(1.0f, false);
    GLuint texID = GraphicsSystem::LoadTexture(filename);
    ResourceManager::Instance().PrintLoadedResources();
    player->AddComponent<RenderComponent>(texID);
    physics->SetStatic(false);
    transform->SetScale(w, h);
    physics->SetVelocity(0.f, 0.f);

    // Enable gravity
    physics->SetGravity(true);

    if (shape == RenderComponent::ShapeType::Circle)
    {
        player->AddComponent<ColliderComponent>(ColliderComponent::Type::Circle, w, 0.0f, layer);
        std::cout << "Added Circle collider with radius: " << (w * 0.5f) << std::endl;
    }
    else if (shape == RenderComponent::ShapeType::Square)
    {
        player->AddComponent<ColliderComponent>(ColliderComponent::Type::Square, w, h, layer);
        std::cout << "Added Square collider with size: " << w << "x" << h << std::endl;
    }

    std::cout << "Created Player: " << name << " at (" << x << ", " << y << ")" << std::endl;

    return player;
}

/*
    /brief
     This is the blueprint for the creation of empty GOs that do not require shapes or textures. Will largely be used for parenting and organising.
*/
GameObject *GameObjectFactory::CreateEmptyGO(const std::string &name, float x, float y, float w, float h)
{
    (void)w;
    (void)h;

    GameObject *obj = CreateGameObject(name);

    auto *transform = obj->AddComponent<TransformComponent>(x, y);
    transform->SetScale(w, h);
    obj->AddComponent<PhysicsComponent>(0.0f, false);

    return obj;
}

/*
    /brief
     This is the blueprint for the normal gameobject creation where
     it has 3 overloads with the sprite and the color and one more for the collsions,
     color overload is for the creating the shape and the sprite is for the texture and the last one is the collision layer.

*/

GameObject *GameObjectFactory::CreateShape(const std::string &name, RenderComponent::ShapeType shape, float x, float y, float w, float h, const vec4 &color, CollisionLayer layer)
{
    GameObject *obj = CreateGameObject(name);

    auto *transform = obj->AddComponent<TransformComponent>(x, y);

    auto render = obj->AddComponent<RenderComponent>(shape, color);

    if (shape == RenderComponent::ShapeType::Line)
    {
        render->SetLine(vec2(x, y), vec2(w, h));
    }
    else if (shape == RenderComponent::ShapeType::Point)
    {
        render->SetPoint(vec2(x, y), w);
    }
    else
    {
        transform->SetScale(w, h);

        // Attach collider automatically
        if (shape == RenderComponent::ShapeType::Circle)
        {
            obj->AddComponent<ColliderComponent>(ColliderComponent::Type::Circle, w, 0.0f, layer);
        }
        else if (shape == RenderComponent::ShapeType::Square)
        {
            obj->AddComponent<ColliderComponent>(ColliderComponent::Type::Square, w, h, layer);
        }
    }
    // testing TO BE REMOVED
    std::cout << "Created "
              << (shape == RenderComponent::ShapeType::Circle ? "Circle" : shape == RenderComponent::ShapeType::Square ? "Square"
                                                                       : shape == RenderComponent::ShapeType::Line     ? "Line"
                                                                                                                       : "Point")
              << ": " << name << " at (" << x << "," << y << ")\n";

    return obj;
}

GameObject *GameObjectFactory::CreateShape(const std::string &name, RenderComponent::ShapeType shape, float x, float y, float w, float h, const vec4 &color)
{
    GameObject *obj = CreateGameObject(name);

    auto *transform = obj->AddComponent<TransformComponent>(x, y);

    auto render = obj->AddComponent<RenderComponent>(shape, color);

    if (shape == RenderComponent::ShapeType::Line)
    {
        render->SetLine(vec2(x, y), vec2(w, h));
    }
    else if (shape == RenderComponent::ShapeType::Point)
    {
        render->SetPoint(vec2(x, y), w);
    }
    else
    {
        transform->SetScale(w, h);
    }
    // testing TO BE REMOVED
    std::cout << "Created "
              << (shape == RenderComponent::ShapeType::Circle ? "Circle" : shape == RenderComponent::ShapeType::Square ? "Square"
                                                                       : shape == RenderComponent::ShapeType::Line     ? "Line"
                                                                                                                       : "Point")
              << ": " << name << " at (" << x << "," << y << ")\n";

    return obj;
}

GameObject *GameObjectFactory::CreateShape(const std::string &name, RenderComponent::ShapeType shape, float x, float y, float w, float h, const char *filename)
{
    GameObject *obj = CreateGameObject(name);

    auto *transform = obj->AddComponent<TransformComponent>(x, y);
    GLuint texID = GraphicsSystem::LoadTexture(filename);
    auto render = obj->AddComponent<RenderComponent>(texID);

    if (shape == RenderComponent::ShapeType::Line)
    {
        render->SetLine(vec2(x, y), vec2(w, h));
    }
    else if (shape == RenderComponent::ShapeType::Point)
    {
        render->SetPoint(vec2(x, y), w);
    }
    else
    {
        transform->SetScale(w, h);
    }

    return obj;
}

/*
    /brief
     This is the blueprint for the normal image gameobject, wer would this function to be images that we wanna like the background
*/

GameObject *GameObjectFactory::CreateImage(const std::string &name, RenderComponent::ShapeType, float x, float y, float w, float h, char const *filename)
{
    GameObject *obj = CreateGameObject(name);

    auto *transform = obj->AddComponent<TransformComponent>(x, y);
    // GLint texID = GraphicsSystem::LoadTexture(filename);
    TextureResource *texResource = ResourceManager::Instance().GetTexture(name);
    GLuint texID = 0;

    if (texResource && texResource->IsLoaded())
    {
        // Use existing texture from ResourceManager
        texID = texResource->GetTextureID();
        std::cout << "Using texture from ResourceManager: " << name << std::endl;
    }
    else
    {
        // Fallback: load directly if not in ResourceManager
        texID = GraphicsSystem::LoadTexture(filename);
        std::cout << "Warning: Texture not in ResourceManager, loading directly: " << filename << std::endl;
    }

    obj->AddComponent<RenderComponent>(texID);
    transform->SetScale(w, h);
    return obj;
}

GameObject *GameObjectFactory::CreateAnimatedImage(const std::string &name, float x, float y, float w, float h,
                                                   char const *filename, int numRows, int numCols,
                                                   int startFrame, int endFrame, float fps, CollisionLayer)
{
    GameObject *obj = CreateGameObject(name);

    auto *transform = obj->AddComponent<TransformComponent>(x, y);
    transform->SetScale(w, h);

    auto spritesheetComp = std::make_unique<Spritesheet>(
        obj,
        filename,
        numRows,
        numCols);

    GLuint textureID = spritesheetComp->GetTextureID();

    obj->AddComponent<RenderComponent>(textureID);
    Spritesheet *anim = spritesheetComp.get();
    obj->AddComponent(std::move(spritesheetComp));

    anim->SetAnimation(startFrame, endFrame, fps);

    return obj;
}

/*
    /brief
     This is the blueprint for the Cloning of the gameobjects where we take in the id and newname for the game Object the we clone if we need to.
*/

GameObject *GameObjectFactory::CloneGameObject(EntityID id, const std::string &newName)
{
    GameObject *original = GetGameObject(id);

    if (original)
    {
        std::string finalName;

        if (newName.empty())
        {
            std::stringstream ss;

            size_t length = original->GetName().find_first_of('(');
            if (length == std::string::npos)
            {
                length = original->GetName().length();
            }

            ++cloneCounter;
            ss << original->GetName().substr(0, length) << "(" << cloneCounter << ")";
            finalName = ss.str();
        }
        else
        {
            finalName = newName;
        }

        return original->Clone(finalName);
    }
    return nullptr;
}

/*
    /brief
     This is the blueprint for the Cloning of the gameobjects where we take in the id and newname for the game Object the we clone if we need to.
*/

GameObject *GameObjectFactory::ClonePlayer(EntityID id, const std::string &newName)
{
    GameObject *original = GetGameObject(id);

    if (original)
    {
        std::string finalName;

        if (newName.empty())
        {
            std::stringstream ss;
            size_t length = original->GetName().find_first_of('(');
            if (length == std::string::npos)
            {
                length = original->GetName().length();
            }

            ++cloneCounter;
            ss << original->GetName().substr(0, length) << "(" << cloneCounter << ")";
            finalName = ss.str();
        }
        else
        {
            finalName = newName;
        }

        GameObject *clone = original->Clone(finalName);

        std::cout << "Cloned Player: " << finalName << " from " << original->GetName() << std::endl;

        return clone;
    }
    return nullptr;
}

GameObject *GameObjectFactory::CloneShape(EntityID id, const std::string &newName)
{
    GameObject *original = GetGameObject(id);

    if (original)
    {
        std::string finalName;

        if (newName.empty())
        {
            std::stringstream ss;
            size_t length = original->GetName().find_first_of('(');
            if (length == std::string::npos)
            {
                length = original->GetName().length();
            }

            ++cloneCounter;
            ss << original->GetName().substr(0, length) << "(" << cloneCounter << ")";
            finalName = ss.str();
        }
        else
        {
            finalName = newName;
        }

        GameObject *clone = original->Clone(finalName);

        std::cout << "Cloned Shape: " << finalName << " from " << original->GetName() << std::endl;

        return clone;
    }
    return nullptr;
}

GameObject *GameObjectFactory::CloneImage(EntityID id, const std::string &newName)
{
    GameObject *original = GetGameObject(id);

    if (original)
    {
        std::string finalName;

        if (newName.empty())
        {
            std::stringstream ss;
            size_t length = original->GetName().find_first_of('(');
            if (length == std::string::npos)
            {
                length = original->GetName().length();
            }

            ++cloneCounter;
            ss << original->GetName().substr(0, length) << "(" << cloneCounter << ")";
            finalName = ss.str();
        }
        else
        {
            finalName = newName;
        }

        GameObject *clone = original->Clone(finalName);

        std::cout << "Cloned Image: " << finalName << " from " << original->GetName() << std::endl;

        return clone;
    }
    return nullptr;
}

GameObject *GameObjectFactory::CloneAnimatedImage(EntityID id, const std::string &newName)
{
    GameObject *original = GetGameObject(id);

    if (original)
    {
        std::string finalName;

        if (newName.empty())
        {
            std::stringstream ss;
            size_t length = original->GetName().find_first_of('(');
            if (length == std::string::npos)
            {
                length = original->GetName().length();
            }

            ++cloneCounter;
            ss << original->GetName().substr(0, length) << "(" << cloneCounter << ")";
            finalName = ss.str();
        }
        else
        {
            finalName = newName;
        }

        GameObject *clone = original->Clone(finalName);

        std::cout << "Cloned Animated Image: " << finalName << " from " << original->GetName() << std::endl;

        return clone;
    }
    return nullptr;
}

/*
    /brief
     This is the function that calls the getGameObject to return the gameobject.

*/

GameObject *GameObjectFactory::GetGameObject(EntityID id)
{
    auto it = entityToIndex.find(id);
    if (it != entityToIndex.end() && it->second < gameObjects.size())
    {
        return gameObjects[it->second].get();
    }
    return nullptr;
}

/*
/brief
 This is the function that calls the findGameObject to find the gameobject.

*/
GameObject *GameObjectFactory::FindGameObject(const std::string &name)
{
    for (auto &obj : gameObjects)
    {
        if (obj->GetName() == name)
        {
            return obj.get();
        }
    }
    return nullptr;
}

const std::vector<std::unique_ptr<GameObject>> &GameObjectFactory::GetAllGameObjects() const
{
    return gameObjects;
}
/*
/brief
This is the function that calls the destroyGameObject to destroy/ delete/ set unactive the gameobject.

*/

void GameObjectFactory::DestroyGameObject(EntityID id)
{
    auto it = entityToIndex.find(id);
    if (it == entityToIndex.end())
        return;

    GameObject *gameObject = GetGameObject(id);
    if (!gameObject)
        return;

    // remove parenting
    GameObject *goParent = gameObject->GetParent();
    if (goParent)
    {
        std::cout << goParent->GetName() << " is removing child: " << gameObject->GetName() << std::endl;
        goParent->RemoveChild(gameObject);
    }

    // swap and pop for factory side
    size_t index = it->second;
    if (index < gameObjects.size() - 1)
    {
        std::swap(gameObjects[index], gameObjects.back());
        entityToIndex[gameObjects[index]->GetID()] = index;
    }

    gameObjects.pop_back();
    entityToIndex.erase(it);
}

void GameObjectFactory::DestroyAllRelatedGameObjects(EntityID parentID)
{
    std::vector<EntityID> goDestroyList; // vector to gather all items to destroy - cant use getfamilytree because we need the order
    std::queue<EntityID> queue;          // queue to sort by which needs to go first (bottom to up)
    queue.push(parentID);

    // the gist - add everything to queue, then add items in queue to vector
    // starting from the parent, dig all the way down and find all children
    while (!queue.empty())
    {
        EntityID currID = queue.front();
        queue.pop(); // pop and add to the vector
        goDestroyList.push_back(currID);

        // if valid pointer, get all children, GET EVERYTHING
        if (GetGameObject(currID))
        {
            std::vector<GameObject *> children = GetGameObject(currID)->GetAllChildren();
            for (auto const &child : children)
            {
                queue.push(child->GetID());
            }
        }
    }
    // since queue was ordered in sequence, reverse iterator to get from deepest in to out
    auto begin = goDestroyList.rbegin();
    auto end = goDestroyList.rend();
    while (begin != end)
    {
        DestroyGameObject(*begin); // destroy one by one
        begin++;
    }
}

// destroys all gameobjects without resetting the EntityID counter - used by level editor
void GameObjectFactory::DestroyAllGameObjects()
{
    std::cout << "Factory: Destroying all " << gameObjects.size() << " GameObjects..." << std::endl;

    gameObjects.clear();
    entityToIndex.clear();

    std::cout << "Factory: All game objects destroyed. Next ID will be: " << nextEntityID << std::endl;
}

void GameObjectFactory::Clear()
{
    gameObjects.clear();
    entityToIndex.clear();
    nextEntityID = 1;
}

GameObject *GameObjectFactory::CreateGameObjectWithID(EntityID id, const std::string &name)
{
    auto gameObject = std::make_unique<GameObject>(id, name);
    GameObject *ptr = gameObject.get();

    entityToIndex[id] = gameObjects.size();
    gameObjects.push_back(std::move(gameObject));

    if (id >= nextEntityID)
    {
        nextEntityID = id + 1;
    }

    return ptr;
}

void GameObjectFactory::SetNextEntityID(EntityID id)
{
    nextEntityID = id;
}

EntityID GameObjectFactory::GetNextEntityID() const
{
    return nextEntityID;
}

EntityID GameObjectFactory::GetSmallestAvailableID() const
{
    EntityID candidate = 1;

    while (const_cast<GameObjectFactory *>(this)->GetGameObject(candidate) != nullptr)
        candidate++;

    return candidate;
}

GameObject *GameObjectFactory::FindByName(const std::string &name)
{
    for (auto &go : gameObjects)
    {
        if (go && go->GetName() == name)
            return go.get();
    }
    return nullptr;
}

void GameObjectFactory::ReorderRootObjects(const std::vector<EntityID> &order)
{
    if (order.empty())
        return;

    std::unordered_map<EntityID, std::unique_ptr<GameObject>> moved;
    moved.reserve(gameObjects.size());

    for (auto &u : gameObjects)
    {
        moved[u->GetID()] = std::move(u);
    }

    std::vector<std::unique_ptr<GameObject>> newList;
    newList.reserve(order.size());

    for (EntityID id : order)
    {
        auto it = moved.find(id);
        if (it != moved.end())
        {
            newList.push_back(std::move(it->second));
        }
    }

    gameObjects = std::move(newList);

    entityToIndex.clear();
    for (size_t i = 0; i < gameObjects.size(); ++i)
    {
        if (gameObjects[i])
        {
            entityToIndex[gameObjects[i]->GetID()] = i;
        }
    }
}

// Check if there is dangling pointers
bool GameObjectFactory::IsAlive(GameObject *ptr) const
{
    auto &allGOs = GameObjectFactory::GetAllGameObjects();
    // Loop through all alive game objects
    for (const auto &obj : allGOs)
    {
        if (obj.get() == ptr)
            return true; // pointer is valid and alive
    }

    return false; // pointer not found = invalid/destroyed
}

#pragma region M1 Showcase - 2500 GOs Test

// Vector to store all GOs made by test function
std::vector<GameObject *> vecGOs;

/**
* @brief Spawns 2500 Circle Game Objects on screen randomly within the 1920x1080 resolution window.
            (Can spawn more - at your own risk!)
*/
void DemoM1_SpawnGameObjects()
{
    auto &factory = GameObjectFactory::Instance();

    std::random_device randomDevice;
    std::mt19937 gen(randomDevice());
    std::uniform_int_distribution<int> distributionX(-960, 960);
    std::uniform_int_distribution<int> distributionY(-540, 540);

    // THIS IS ANIMATED OBJECT FOR COMBI 1 WE JUST NEED A STATIC TEXTURE, STILL GOOD TO GET LOAD TEX ONCE THOUGH
    GLuint sharedTextureID = GraphicsSystem::LoadTexture("../Assets/Mouch_idle.png");

    // spawn 2.5k small circles
    for (size_t i = 0; i < 1000; i++)
    {
        int intX{distributionX(gen)}, intY{distributionY(gen)};
        std::string uniqueName = "Demo Circle " + std::to_string(i);
        // GameObject *newGO = factory.CreateShape(uniqueName, RenderComponent::ShapeType::Circle, static_cast<float>(intX), static_cast<float>(intY), 40.f, 40.f, vec4(1.f, 0.f, 0.f, 1.f));
        // GameObject* newGO = factory.CreateAnimatedImage(uniqueName, static_cast<float>(intX), static_cast<float>(intY), 40.0, 40.0, "../Assets/Mouch_idle.png", 1, 2, 0, 7, 2.0f, CollisionLayer::Ground);

        GameObject *newGO = factory.CreateGameObject(uniqueName);

        newGO->AddComponent<TransformComponent>(
            static_cast<float>(intX),
            static_cast<float>(intY));
        auto *transform = newGO->GetComponent<TransformComponent>();
        auto *physics = newGO->AddComponent<PhysicsComponent>(0.f, false);
        physics->SetGravity(true); // Enable/Disable gravity
        physics->SetStatic(false);
        transform->SetScale(40.f, 40.f);
        physics->SetVelocity(0.f, 0.f);
        newGO->AddComponent<ColliderComponent>(ColliderComponent::Type::Square, 5.f, 5.f, CollisionLayer::Ground);

        auto *render = newGO->AddComponent<RenderComponent>(sharedTextureID);
        render->SetLayer(0);

        auto *animator = newGO->AddComponent<Spritesheet>(
            sharedTextureID,
            1,
            2);
        animator->SetAnimation(0, 7, 2.f);
        vecGOs.push_back(newGO);
    }
}

/**
* @brief Function that removes all Circle Game Objects spawned from the function above.
            (Yes even if you chose to spawn more than 2500)
*/
void DemoM1_DestroyGameObjects()
{
    auto &factory = GameObjectFactory::Instance();

    for (GameObject *&GO : vecGOs)
    {
        factory.DestroyGameObject(GO->GetID());
    }
    vecGOs.clear();
}

#pragma endregion

#pragma region M2 Showcase - 1000 GOs Test

static std::vector<GameObject *> stressObjects;
static bool stressSpawned = false;
static bool stressInitPhysics = false;
static int stressIndex = 0;

void SpawnStressObjects()
{
    if (stressSpawned)
        return;
    stressSpawned = true;

    auto &factory = GameObjectFactory::Instance();
    auto &resourceMgr = ResourceManager::Instance();

    stressObjects.clear();
    stressObjects.reserve(1000);

    const int totalObjects = 1000;
    const float spawnW = 32.f;
    const float spawnH = 32.f;

    // Load shared texture once
    const std::string spritePath = "../Assets/protosprite.png";
    const std::string textureKey = "stress_shared";

    TextureResource *texRes = resourceMgr.GetTexture(textureKey);
    if (!texRes)
    {
        resourceMgr.LoadTexture(textureKey, spritePath);
        texRes = resourceMgr.GetTexture(textureKey);
    }
    if (!texRes || !texRes->IsLoaded())
    {
        std::cerr << "[STRESS] Texture load failed.\n";
        return;
    }

    GLuint texID = texRes->GetTextureID();

    // ---------------- Spawn only -----------------
    for (int i = 0; i < totalObjects; ++i)
    {
        std::string objName = "StressObj_" + std::to_string(i);
        GameObject *go = factory.CreateGameObject(objName);
        stressObjects.push_back(go);

        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<int> distX(-960, 960);
        static std::uniform_int_distribution<int> distY(-540, 540);

        if (auto *t = go->AddComponent<TransformComponent>())
        {
            float x = static_cast<float>(distX(gen));
            float y = static_cast<float>(distY(gen));

            t->SetPosition(x, y);
            t->SetScale(spawnW, spawnH);
        }

        // Render
        auto *render = go->AddComponent<RenderComponent>();
        render->SetTextureID(texID);
        render->SetTexturePath(spritePath);
        render->SetShapeType(RenderComponent::ShapeType::Square);
        render->SetColor({1, 1, 1, 1});
    }

    // Mark physics batching to begin next frame
    stressInitPhysics = true;
    stressIndex = 0;

    std::cout << "[STRESS TEST] Spawned 1000 objects.\n";
}

void SpawnTest_Update()
{
    if (!stressInitPhysics)
        return;

    const int batch = 50;

    for (int i = 0; i < batch && stressIndex < stressObjects.size(); ++i, ++stressIndex)
    {
        GameObject *go = stressObjects[stressIndex];
        if (!go)
            continue;

        // const float spawnW = 32.f;
        // const float spawnH = 32.f;

        // Collider
        if (!go->GetComponent<ColliderComponent>())
        {
            // go->AddComponent<ColliderComponent>(
            //     ColliderComponent::Type::Circle,
            //     spawnW, spawnH,
            //     CollisionLayer::StressTest
            //);
        }

        // Physics
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<float> velDist(-350.f, 350.f);

        auto *p = go->AddComponent<PhysicsComponent>();
        p->SetGravity(true);
        p->SetStatic(false);

        float vx = velDist(gen);
        float vy = velDist(gen);

        p->SetVelocity(vx, vy);
        p->SetMass(1.f);
    }

    if (stressIndex >= stressObjects.size())
    {
        stressInitPhysics = false;
        std::cout << "[STRESS TEST] Finished initializing physics.\n";
    }
}

void DestroyStressTestGameObjects()
{
    auto &factory = GameObjectFactory::Instance();

    for (GameObject *&GO : stressObjects)
    {
        factory.DestroyGameObject(GO->GetID());
    }

    stressSpawned = false;
    stressObjects.clear();
}

#pragma endregion