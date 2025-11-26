/* Start Header ************************************************************************/
/*!
\file       ScriptAPI.cpp
\author     Jonathan Fam Qian Wei [100% - 386 lines]
\par        Fam.q@digipen.edu
\date       Novenber, 7, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "ScriptAPI.h"

namespace API
{
#pragma region Engine
    void Init_Engine(CoreEngine &engine, AudioEngine &audio)
    {
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
        

        // Initialise Resource Manager
        std::cout << "\n--- Initialising Resource Manager ---" << std::endl;
        auto &resourceMgr = ResourceManager::Instance();
        resourceMgr.SetAudioEngine(&audio);

        // load audio files
        //resourceMgr.LoadAudio("bgm", "../Assets/audio/level_1.wav", true);            // looping
        //resourceMgr.LoadAudio("jump_sfx", "../Assets/audio/jump.wav", false);         // when jumping only
        //resourceMgr.LoadAudio("footstep_sfx", "../Assets/audio/footstep.wav", false); // footstep sound

        // load common fonts
        std::cout << "\n--- Loading Fonts ---" << std::endl;
        resourceMgr.LoadFont("default_font", "../Assets/PinyonScript-Regular.ttf", 48);
        resourceMgr.LoadFont("small_font", "../Assets/FrederickatheGreat-Regular.ttf", 24);
        resourceMgr.LoadFont("large_font", "../Assets/FrederickatheGreat-Regular.ttf", 64);

        // print loaded resources
        resourceMgr.PrintLoadedResources();
        std::cout << "\n--- Audio loaded, music will start in play mode ---" << std::endl;
    }

    void StartEngineLoop(CoreEngine &engine)
    {
        std::cout << "\n--- Starting Engine Gameloop ---" << std::endl;
        engine.GameLoop();
    }

#pragma endregion

#pragma region Creation

    void CreateShape(const std::string &name,
                     RenderComponent::ShapeType type,
                     float x, float y,
                     float width, float height,
                     vec4 color,
                     CollisionLayer layer,
                     bool hasPhysics,
                     bool isStatic,
                     bool hasGravity)
    {
        auto &factory = GameObjectFactory::Instance();
        GameObject *obj = factory.CreateShape(name, type, x, y, width, height, color, layer);
        if (hasPhysics && obj)
        {
            auto *physics = obj->AddComponent<PhysicsComponent>(1.0f, isStatic);
            physics->SetStatic(isStatic);

            if (hasGravity)
            {
                physics->SetGravity(true);
            }
        }
    }

    void CreateImage(const std::string &name,
                     RenderComponent::ShapeType type,
                     float x, float y,
                     float width, float height,
                     const std::string &texturePath)
    {
        auto &factory = GameObjectFactory::Instance();
        factory.CreateImage(name, type, x, y, width, height, texturePath.c_str());
    }

    void CreateAnimatedImage(const std::string &name,
                             float x, float y,
                             float width, float height,
                             const std::string &texturePath,
                             int rows, int columns,
                             int startFrame, int endFrame,
                             float fps,
                             CollisionLayer layer,
                             bool hasPhysics,
                             bool isStatic,
                             bool hasGravity)
    {
        auto &factory = GameObjectFactory::Instance();
        GameObject *obj = factory.CreateAnimatedImage(name, x, y, width, height, texturePath.c_str(),
                                                      rows, columns, startFrame, endFrame, fps, layer);

        if (hasPhysics && obj)
        {
            auto *physics = obj->AddComponent<PhysicsComponent>(1.0f, isStatic);
            physics->SetStatic(isStatic);

            if (hasGravity)
            {
                physics->SetGravity(true);
            }
        }
    }

#pragma endregion

#pragma region Graphics
    void SetShapeType(const std::string &objectName, RenderComponent::ShapeType newShape)
    {
        auto &factory = GameObjectFactory::Instance();

        GameObject *obj = factory.FindGameObject(objectName);

        if (obj)
        {
            auto *render = obj->GetComponent<RenderComponent>();
            if (render)
            {
                render->SetShapeType(newShape);
            }
        }
    }
    void SetTexture(const std::string &objectName, const char *filename)
    {
        auto &factory = GameObjectFactory::Instance();

        GameObject *obj = factory.FindGameObject(objectName);

        if (obj)
        {
            auto *render = obj->GetComponent<RenderComponent>();
            if (render)
            {
                GLuint newTextID = GraphicsSystem::LoadTexture(filename);
                render->SetTextureID(newTextID);
            }
        }
    }
    void setColor(const std::string &objectName, vec4 color)
    {
        auto &factory = GameObjectFactory::Instance();

        GameObject *obj = factory.FindGameObject(objectName);

        if (obj)
        {
            auto *render = obj->GetComponent<RenderComponent>();
            if (render)
            {
                render->SetColor(color);
            }
        }
    }
    void SetAlpha(const std::string &objectName, float val)
    {
        auto &factory = GameObjectFactory::Instance();

        GameObject *obj = factory.FindGameObject(objectName);

        if (obj)
        {
            auto *render = obj->GetComponent<RenderComponent>();
            if (render)
            {
                render->setAlpha(val);
            }
        }
    }
    void SetBackGround(char const *filename)
    {
        auto &factory = GameObjectFactory::Instance();
        GameObject *background = factory.FindGameObject("bgImage");

        if (background)
        {
            unsigned int newID = GraphicsSystem::LoadTexture(filename);
            auto *render = background->GetComponent<RenderComponent>();
            render->SetTextureID(newID);
        }
    }

#pragma endregion

#pragma region Physics
    void SetVelocity(const std::string &objectName, float vx, float vy)
    {

        auto &factory = GameObjectFactory::Instance();

        GameObject *obj = factory.FindGameObject(objectName);

        if (obj)
        {
            auto *physics = obj->GetComponent<PhysicsComponent>();
            if (physics)
            {
                physics->SetVelocity(vx, vy);
            }
        }
    }

    void SetJumpVal(const std::string &objectName, float val)
    {

        auto &factory = GameObjectFactory::Instance();

        GameObject *obj = factory.FindGameObject(objectName);

        if (obj)
        {
            auto *physics = obj->GetComponent<PhysicsComponent>();
            if (physics)
            {
                physics->GetForcesManager().AddForce(ForceType::Jump);
            }
        }
    }

    void SetGravBool(const std::string &objectName, bool val)
    {
        auto &factory = GameObjectFactory::Instance();

        GameObject *obj = factory.FindGameObject(objectName);

        if (obj)
        {
            auto *physics = obj->GetComponent<PhysicsComponent>();
            if (physics)
            {
                physics->SetGravity(val);
            }
        }
    }


    void SetStatic(const std::string &objectName, bool val)
    {
        auto &factory = GameObjectFactory::Instance();

        GameObject *obj = factory.FindGameObject(objectName);

        if (obj)
        {
            auto *physics = obj->GetComponent<PhysicsComponent>();
            if (physics)
            {
                physics->SetStatic(val);
            }
        }
    }

#pragma endregion

#pragma region Transform
    void SetPos(const std::string &objectName, float x, float y)
    {
        auto &factory = GameObjectFactory::Instance();
        GameObject *obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto *transform = obj->GetComponent<TransformComponent>();
            if (transform)
            {
                transform->SetPosition(x, y);
            }
        }
    }

    void SetRot(const std::string &objectName, float val)
    {
        auto &factory = GameObjectFactory::Instance();
        GameObject *obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto *transform = obj->GetComponent<TransformComponent>();
            if (transform)
            {
                transform->SetRotation(val);
            }
        }
    }
    void AddRot(const std::string &objectName, float val)
    {
        auto &factory = GameObjectFactory::Instance();
        GameObject *obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto *transform = obj->GetComponent<TransformComponent>();
            if (transform)
            {
                transform->Rotate(val);
            }
        }
    }

    void SetScale(const std::string &objectName, float val)
    {
        auto &factory = GameObjectFactory::Instance();
        GameObject *obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto *transform = obj->GetComponent<TransformComponent>();
            if (transform)
            {
                transform->SetScale(val);
            }
        }
    }

    void SetScale(const std::string &objectName, float x, float y)
    {
        auto &factory = GameObjectFactory::Instance();
        GameObject *obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto *transform = obj->GetComponent<TransformComponent>();
            if (transform)
            {
                transform->SetScale(x, y);
            }
        }
    }

#pragma endregion

#pragma region Components

    void AddTransformComponent(const std::string &objectName)
    {
        auto &factory = GameObjectFactory::Instance();
        GameObject *obj = factory.FindGameObject(objectName);
        if (obj)
        {

            if (!(obj->GetComponent<TransformComponent>()))
            {
                obj->AddComponent<TransformComponent>();
            }
            else
            {
                throw std::runtime_error("Attempted to add duplicate TransformComponent to GameObject: " + objectName);
            }
        }
        else
        {
            throw std::runtime_error("GameObject not found: " + objectName);
        }
    }

    void AddRenderComponent(const std::string &objectName)
    {
        auto &factory = GameObjectFactory::Instance();
        GameObject *obj = factory.FindGameObject(objectName);
        if (obj)
        {

            if (!(obj->GetComponent<RenderComponent>()))
            {
                obj->AddComponent<RenderComponent>();
            }
            else
            {
                throw std::runtime_error("Attempted to add duplicate RenderComponent to GameObject: " + objectName);
            }
        }
    }

    void AddCollisionComponent(const std::string &objectName)
    {
        auto &factory = GameObjectFactory::Instance();
        GameObject *obj = factory.FindGameObject(objectName);
        if (obj)
        {

            if (!obj->GetComponent<ColliderComponent>() && obj->GetComponent<TransformComponent>())
            {
                obj->AddComponent<ColliderComponent>();
            }
            else if (!obj->GetComponent<TransformComponent>())
            {
                throw std::runtime_error("This GameObject " + objectName + " do not have a TransformComponent");
            }
            else
            {
                throw std::runtime_error("Attempted to add duplicate ColliderComponent to GameObject: " + objectName);
            }
        }
    }

    void AddPhysicComponent(const std::string &objectName)
    {
        auto &factory = GameObjectFactory::Instance();
        GameObject *obj = factory.FindGameObject(objectName);
        if (obj)
        {

            if (!obj->GetComponent<PhysicsComponent>())
            {
                obj->AddComponent<PhysicsComponent>();
            }
            else
            {
                throw std::runtime_error("Attempted to add duplicate PhysicsComponent to GameObject: " + objectName);
            }
        }
    }
#pragma endregion

#pragma region Collision
    void SetColisionShape(const std::string &objectName, ColliderComponent::Type newShape)
    {
        auto &factory = GameObjectFactory::Instance();
        GameObject *obj = factory.FindGameObject(objectName);

        if (obj)
        {
            auto *colli = obj->GetComponent<ColliderComponent>();
            if (colli)
            {
                colli->setColliShape(newShape);
            }
        }
    }
    void SetCollisionLayer(const std::string &objectName, CollisionLayer newlayer)
    {
        auto &factory = GameObjectFactory::Instance();

        GameObject *obj = factory.FindGameObject(objectName);

        if (obj)
        {
            auto *colli = obj->GetComponent<ColliderComponent>();
            if (colli)
            {
                colli->SetLayer(newlayer);
            }
        }
    }
    bool OnTriggerEnter(const std::string obj1_name,const std::string obj2_name)
    {
        auto& factory = GameObjectFactory::Instance();

        GameObject* obj1 = factory.FindGameObject(obj1_name);
        GameObject* obj2 = factory.FindGameObject(obj2_name);
        CollisionSystem* collisys = CORE->GetCollisionSystem();
        if (collisys->CollisionSystem::JustCollided(obj1, obj2)) {
            return true;
        }
        return false;
    }
    bool OnTriggerExit(const std::string obj1_name, const std::string obj2_name)
    {
        auto& factory = GameObjectFactory::Instance();

        GameObject* obj1 = factory.FindGameObject(obj1_name);
        GameObject* obj2 = factory.FindGameObject(obj2_name);
        CollisionSystem* collisys = CORE->GetCollisionSystem();
        if (collisys->CollisionSystem::JustStopped(obj1, obj2)) {
            return true;
        }
        return false;
    }
    bool OnTriggerStay(const std::string obj1_name, const std::string obj2_name)
    {
        auto& factory = GameObjectFactory::Instance();

        GameObject* obj1 = factory.FindGameObject(obj1_name);
        GameObject* obj2 = factory.FindGameObject(obj2_name);
        CollisionSystem* collisys = CORE->GetCollisionSystem();
        if (collisys->CollisionSystem::StillColliding(obj1, obj2)) {
            return true;
        }
        return false;
    }

    

#pragma endregion

#pragma region Camera

    void setCameraPos(vec2 pos)
    {
        auto& cam = CameraManager::Instance();
        CameraComponent* camtemp = cam.GetActiveCamera();
        if (!camtemp) {
            return;
        }

        camtemp->SetPosition(pos.x, pos.y);
    }


    void setCameraZoom(float val)
    {
        auto& cam = CameraManager::Instance();
        CameraComponent* camtemp = cam.GetActiveCamera();
        if (!camtemp) {
            return;
        }
        camtemp->SetZoom(val);
    }

    void setCameraMode(CameraComponent::CameraMode val)
    {
        auto& cam = CameraManager::Instance();
        CameraComponent* camtemp = cam.GetActiveCamera();
        if (!camtemp) {
            return;
        }
        camtemp->SetMode(val);
    }
#pragma endregion

#pragma region Misc
    int RandomNumberGen(int max, int min)
    {
        int range = max - min + 1;
        int result = rand() % range + min;
        return result;
    }
    float GetDelta_Time()
    {
        return CORE->GetDeltaTime();
    }
    float GetFixedDelta_time()
    {
        return CORE->GetFixedDeltaTime();
    }
    float GetFramePerSecond() {
        return CORE->GetFPS();
    }
#pragma endregion

#pragma region Audio

    void AddSound(const std::string name, const std::string filepath,
        bool loop, float volume,float pitch, std::string category) {
        SoundData newSound{ name,filepath,loop,volume,pitch,category };
        AudioManager* audioManager = CORE->GetAudioManager();
        if (audioManager)
        {
            audioManager->AddSound(newSound);
        }
    }

    void PlaySound(const std::string soundName, const std::string channel)
    {
        AudioManager* audioManager = CORE->GetAudioManager();
        if (audioManager)
        {
            audioManager->PlaySound(soundName, channel);
        }

    }

    void StopSound(const std::string soundName)
    {
        AudioManager* audioManager = CORE->GetAudioManager();
        if (audioManager)
        {
            audioManager->StopSound(soundName);
        }
    }

    void StopAllSound()
    {
        AudioManager* audioManager = CORE->GetAudioManager();
        if (audioManager)
        {
            audioManager->StopAllSounds();
        }
    }

    void RemoveSound(const std::string soundName)
    {
        AudioManager* audioManager = CORE->GetAudioManager();
        if (audioManager)
        {
            audioManager->RemoveSound(soundName);
        }
    }

    void TriggerEvent(const std::string eventname)
    {
        AudioManager* audioManager = CORE->GetAudioManager();
        if (audioManager)
        {
            audioManager->TriggerEvent(eventname);
        }
    }

    void AddEventSound(const std::string eventName, const std::string soundName)
    {
        AudioManager* audioManager = CORE->GetAudioManager();
        if (audioManager)
        {
            audioManager->AddEventMapping(eventName, soundName);
        }
    }

    void SetVolume(const std::string soundName, float volume)
    {
        AudioManager* audioManager = CORE->GetAudioManager();
        if (audioManager)
        {
            audioManager->SetSoundVolume(soundName, volume);
        }
    }

    void SetCategoryVolume(const std::string CatName, float vol)
    {
        AudioManager* audioManager = CORE->GetAudioManager();
        if (audioManager)
        {
            audioManager->SetCategoryVolume(CatName, vol);
        }
    }
#pragma endregion

#pragma region Timer
    TimerID WaitForSeconds(float duration, TimerCallback callback)
    {
        TimerSystem* ts = CORE->GetTimerSystem();
        if (ts)
        {
            return ts->AddTimer(duration, callback, false, "WaitForSeconds");
        }
        return 0;
    }

    TimerID Delay(float duration, TimerCallback callback)
    {
        return WaitForSeconds(duration, callback);
    }

    TimerID InvokeRepeating(float interval, TimerCallback callback)
    {
        TimerSystem* ts = CORE->GetTimerSystem();
        if (ts)
        {
            return ts->AddTimer(interval, callback, true, "InvokeRepeating");
        }
        return 0;
    }

    TimerID Invoke(float delay, void (*functionToCall)())
    {
        TimerSystem* ts = CORE->GetTimerSystem();
        if (ts)
        {
            return ts->AddTimer(delay, functionToCall, false, "Invoke");
        }
        return 0;
    }

    TimerID Schedule(float delay, std::function<void()> callback)
    {
        TimerSystem* ts = CORE->GetTimerSystem();
        if (ts)
        {
            return ts->AddTimer(delay, callback, false, "Schedule");
        }
        return 0;
    }

    void CancelTimer(TimerID id)
    {
        TimerSystem* ts = CORE->GetTimerSystem();
        if (ts) ts->CancelTimer(id);
    }

    void CancelTimerByName(std::string name)
    {
        TimerSystem* ts = CORE->GetTimerSystem();
        if (ts) ts->CancelTimerByName(name);
    }

    void PauseTimer(TimerID id)
    {
        TimerSystem* ts = CORE->GetTimerSystem();
        if (ts) ts->PauseTimer(id);
    }

    void ResumeTimer(TimerID id)
    {
        TimerSystem* ts = CORE->GetTimerSystem();
        if (ts) ts->ResumeTimer(id);
    }
#pragma endregion


#pragma region Input
    bool IsKeyDown(int key)
    {
        GLFWwindow* window = CORE->GetGraphicsSystem()->getWindow();
        if (window)
        {
            return glfwGetKey(window, key) == GLFW_PRESS;
        }
        return false;
    }

    bool IsMouseButtonDown(int button)
    {
        GLFWwindow* window = CORE->GetGraphicsSystem()->getWindow();
        if (window)
        {
            return glfwGetMouseButton(window, button) == GLFW_PRESS;
        }
        return false;
    }

    float GetMouseX()
    {
        InputManager* input = CORE->GetInputManager();
        if (input) return (float)input->GetMouseX();
        return 0.0f;
    }

    float GetMouseY()
    {
        InputManager* input = CORE->GetInputManager();
        if (input) return (float)input->GetMouseY();
        return 0.0f;
    }

    float GetMouseWorldX()
    {
        // InputManager static accessor from your previous code
        return (float)InputManager::GetMouseWorldX();
    }

    float GetMouseWorldY()
    {
        return (float)InputManager::GetMouseWorldY();
    }
#pragma endregion

}