#include "ScriptAPI.h"

namespace API {
#pragma region Engine
    void Init_Engine(CoreEngine& engine, AudioEngine& audio) {
        engine.SetAudioEngine(&audio);
        std::map<std::string, double> g_systemTimings; // ms per system
        std::cout << "\n--- Initializing Engine ---" << std::endl;

        engine.Init();

        // Initialise Audio System
        std::cout << "\n--- Initialising Audio System ---" << std::endl;

        if (!audio.initialiseAudio())
        {
            std::cout << "Audio system failed to initialise, continuing without audio" << std::endl;
        }
        else
        {
            std::cout << "Audio system initialised successfully" << std::endl;
        }

        // Initialise ResourceManager
        std::cout << "\n--- Initialising Resource Manager ---" << std::endl;
        auto& resourceMgr = ResourceManager::Instance();
        resourceMgr.SetAudioEngine(&audio);

        // load textures
       // resourceMgr.LoadTexture("Hero", "./Assets/Mouch_idle.png");
        //resourceMgr.LoadTexture("bgImage", "./Assets/wallpaper_transparent.png");

        // load audio files
        //resourceMgr.LoadAudio("bgm", "./Assets/audio/level_1.wav", true); // looping
        //resourceMgr.LoadAudio("jump_sfx", "./Assets/audio/jump.wav", false); // when jumping only

        // print loaded resources
        //resourceMgr.PrintLoadedResources();

        // register the sound with audio engine
        //std::cout << "\n--- Starting Background Music ---" << std::endl;
        //audio.playSound("bgm", "bgm_channel");
        //audio.setVolume("bgm_channel", 0.5f);
    }

    void StartEngineLoop(CoreEngine& engine)
    {
        std::cout << "\n--- Starting Engine Gameloop ---" << std::endl;
        engine.GameLoop();
    }

#pragma endregion

#pragma region Creation

    void CreateShape(const std::string& name,
        RenderComponent::ShapeType type,
        float x, float y,
        float width, float height,
        vec4 color,
        CollisionLayer layer,
        bool hasPhysics,
        bool isStatic,
        bool hasGravity)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.CreateShape(name, type, x, y, width, height, color, layer);
        if (hasPhysics && obj)
        {
            auto* physics = obj->AddComponent<PhysicsComponent>(1.0f, isStatic);
            physics->SetStatic(isStatic);

            if (hasGravity)
            {
                physics->SetGravity(true);
            }
        }
    }

    void CreateImage(const std::string& name,
        RenderComponent::ShapeType type,
        float x, float y,
        float width, float height,
        const std::string& texturePath)
    {
        auto& factory = GameObjectFactory::Instance();
        factory.CreateImage(name, type, x, y, width, height, texturePath.c_str());
    }

    void CreateAnimatedImage(const std::string& name,
        float x, float y,
        float width, float height,
        const std::string& texturePath,
        int rows, int columns,
        int startFrame, int endFrame,
        float fps,
        CollisionLayer layer,
        bool hasPhysics,
        bool isStatic,
        bool hasGravity)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.CreateAnimatedImage(name, x, y, width, height, texturePath.c_str(),
            rows, columns, startFrame, endFrame, fps, layer);

        if (hasPhysics && obj)
        {
            auto* physics = obj->AddComponent<PhysicsComponent>(1.0f, isStatic);
            physics->SetStatic(isStatic);

            if (hasGravity)
            {
                physics->SetGravity(true);
            }
        }
    }

#pragma endregion

#pragma region Graphics
    void SetShapeType(const std::string& objectName, RenderComponent::ShapeType newShape) {
        auto& factory = GameObjectFactory::Instance();

        GameObject* obj = factory.FindGameObject(objectName);

        if (obj)
        {
            auto* render = obj->GetComponent<RenderComponent>();
            if (render)
            {
                render->SetShapeType(newShape);
            }
        }
    }
    void SetTexture(const std::string& objectName, const char* filename)
    {
        auto& factory = GameObjectFactory::Instance();

        GameObject* obj = factory.FindGameObject(objectName);

        if (obj)
        {
            auto* render = obj->GetComponent<RenderComponent>();
            if (render)
            {
                GLuint newTextID = GraphicsSystem::LoadTexture(filename);
                render->SetTextureID(newTextID);
            }
        }
    }
    void setColor(const std::string& objectName, vec4 color) {
        auto& factory = GameObjectFactory::Instance();

        GameObject* obj = factory.FindGameObject(objectName);

        if (obj)
        {
            auto* render = obj->GetComponent<RenderComponent>();
            if (render)
            {
                render->SetColor(color);
            }
        }
    }
    void SetAlpha(const std::string& objectName, float val) {
        auto& factory = GameObjectFactory::Instance();

        GameObject* obj = factory.FindGameObject(objectName);

        if (obj)
        {
            auto* render = obj->GetComponent<RenderComponent>();
            if (render)
            {
                render->setAlpha(val);
            }
        }
    }
#pragma endregion

#pragma region Physics
    void SetVelocity(const std::string& objectName, float vx, float vy)
    {

        auto& factory = GameObjectFactory::Instance();

        GameObject* obj = factory.FindGameObject(objectName);

        if (obj)
        {
            auto* physics = obj->GetComponent<PhysicsComponent>();
            if (physics)
            {
                physics->SetVelocity(vx, vy);
            }
        }
    }

    void SetJumpVal(const std::string& objectName, float val)
    {

        auto& factory = GameObjectFactory::Instance();

        GameObject* obj = factory.FindGameObject(objectName);

        if (obj)
        {
            auto* physics = obj->GetComponent<PhysicsComponent>();
            if (physics)
            {
                physics->GetForcesManager().AddForce(ForceType::Jump, physics->GetMass());
            }
        }
    }

    void SetGravBool(const std::string& objectName, bool val)
    {
        auto& factory = GameObjectFactory::Instance();

        GameObject* obj = factory.FindGameObject(objectName);

        if (obj)
        {
            auto* physics = obj->GetComponent<PhysicsComponent>();
            if (physics)
            {
                physics->SetGravity(val);
            }
        }
    }

    //void SetGravVal(const std::string& objectName, float x, float y)
    //{
    //    auto& factory = GameObjectFactory::Instance();

    //    GameObject* obj = factory.FindGameObject(objectName);

    //    if (obj)
    //    {
    //        auto* physics = obj->GetComponent<PhysicsComponent>();
    //        if (physics)
    //        {
    //            // Remove existing gravity and add new one with custom direction
    //            physics->GetForcesManager().RemoveForcesByType(ForceType::Gravity);

    //            physics->GetForcesManager().AddForce(ForceType::Gravity, physics->GetMass());
    //        }
    //    }
    //}

    void SetStatic(const std::string& objectName, bool val)
    {
        auto& factory = GameObjectFactory::Instance();

        GameObject* obj = factory.FindGameObject(objectName);

        if (obj)
        {
            auto* physics = obj->GetComponent<PhysicsComponent>();
            if (physics)
            {
                physics->SetStatic(val);
            }
        }
    }


#pragma endregion

#pragma region Transform
    void SetPos(const std::string& objectName, float x, float y) {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* transform = obj->GetComponent<TransformComponent>();
            if (transform)
            {
                transform->SetPosition(x, y);
            }
        }
    }

    void SetRot(const std::string& objectName, float val) {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* transform = obj->GetComponent<TransformComponent>();
            if (transform)
            {
                transform->SetRotation(val);
            }
        }
    }
    void AddRot(const std::string& objectName, float val) {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* transform = obj->GetComponent<TransformComponent>();
            if (transform)
            {
                transform->Rotate(val);
            }
        }
    }

    void SetScale(const std::string& objectName, float val) {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* transform = obj->GetComponent<TransformComponent>();
            if (transform)
            {
                transform->SetScale(val);
            }
        }
    }

    void SetScale(const std::string& objectName, float x, float y) {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* transform = obj->GetComponent<TransformComponent>();
            if (transform)
            {
                transform->SetScale(x, y);
            }
        }
    }

#pragma endregion

#pragma region Components

    void AddTransformComponent(const std::string& objectName) {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj) {

            if (!(obj->GetComponent<TransformComponent>())) {
                obj->AddComponent<TransformComponent>();
            }
            else {
                throw std::runtime_error("Attempted to add duplicate TransformComponent to GameObject: " + objectName);
            }
        }
        else {
            throw std::runtime_error("GameObject not found: " + objectName);
        }

    }

    void AddRenderComponent(const std::string& objectName) {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj) {

            if (!(obj->GetComponent<RenderComponent>())) {
                obj->AddComponent<RenderComponent>();
            }
            else {
                throw std::runtime_error("Attempted to add duplicate RenderComponent to GameObject: " + objectName);
            }
        }
    }

    void AddCollisionComponent(const std::string& objectName) {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj) {

            if (!obj->GetComponent<ColliderComponent>() && obj->GetComponent<TransformComponent>()) {
                obj->AddComponent<ColliderComponent>();
            }
            else if (!obj->GetComponent<TransformComponent>()) {
                throw std::runtime_error("This GameObject " + objectName + " do not have a TransformComponent");
            }
            else {
                throw std::runtime_error("Attempted to add duplicate ColliderComponent to GameObject: " + objectName);
            }
        }
    }

    void AddPhysicComponent(const std::string& objectName) {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj) {

            if (!obj->GetComponent<PhysicsComponent>()) {
                obj->AddComponent<PhysicsComponent>();
            }
            else {
                throw std::runtime_error("Attempted to add duplicate PhysicsComponent to GameObject: " + objectName);
            }
        }
    }
#pragma endregion

#pragma region Collision
    void SetColisionShape(const std::string& objectName, ColliderComponent::Type newShape) {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);

        if (obj)
        {
            auto* colli = obj->GetComponent<ColliderComponent>();
            if (colli)
            {
                colli->setColliShape(newShape);
            }
        }
    }
    void SetCollisionLayer(const std::string& objectName, CollisionLayer newlayer) {
        auto& factory = GameObjectFactory::Instance();

        GameObject* obj = factory.FindGameObject(objectName);

        if (obj)
        {
            auto* colli = obj->GetComponent<ColliderComponent>();
            if (colli)
            {
                colli->SetLayer(newlayer);
            }
        }
    }
#pragma endregion

}