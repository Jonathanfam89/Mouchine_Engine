/* Start Header ************************************************************************/
/*!
\file       main.cpp
\author     Lee Hwee Min [100% - 137 lines]
\par        l.hweemin@digipen.edu
\date       October, 2, 2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "main.h"


namespace CrashHandler
{
    std::string GetStackTrace();
    void LogCrash(const char *type, const std::exception *e = nullptr);
}

/**
 * @brief Enables memory leak checking in debug mode.
 * @param breakAlloc, Allocation number to break on for debugging.
 */
void EnableMemoryLeakChecking(int breakAlloc = -1)
{
    // Set the leak checking flag
    int tmpDbgFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
    tmpDbgFlag |= _CRTDBG_LEAK_CHECK_DF;
    _CrtSetDbgFlag(tmpDbgFlag);

    // If a valid break alloc provided set the breakAlloc
    if (breakAlloc != -1)
        _CrtSetBreakAlloc(breakAlloc);
}

/**
 * @brief Main entry point of the program.
 *
 * Initializes the engine, resource manager, audio, and game objects.
 * Loads textures, audio files, sets up the scene, then enters the main loop.
 *
 * @return Program exit code (0 = success).
 */
int main()
{
    try
    {

        EnableMemoryLeakChecking();
        AudioEngine audio;
        CoreEngine engine;

        API::Init_Engine(engine, audio);

        g_UseGameStates = false; // Set the flag to use Game States
        CORE->RequestSceneLoad("../Assets/Scenes/MainMenu.json");

        GameObject *mainCamera = GameObjectFactory::Instance().CreateGameObject("MainCamera");
        auto *cam = mainCamera->AddComponent<CameraComponent>();

        GameObject *player = GameObjectFactory::Instance().FindGameObject("Hero");
        if (player)
        {
            cam->SetMode(CameraComponent::CameraMode::FOLLOW_TARGET);
            // WHEN CAMERA MODE IS FIXED
            // cam->SetPosition(0.f, 0.f);
            cam->SetFollowTarget(player);
            cam->SetFollowOffset(vec2(0.f, 0.f));
            cam->SetSmoothing(true, 2.f);
        }

        CameraManager::Instance().SetActiveCamera(mainCamera);

        GameObject* testObj = GameObjectFactory::Instance().CreateShape(
            "MyTestSquare",                  // <--- NAME MUST MATCH C#
            RenderComponent::ShapeType::Square,
            -200.0f, 400.0f,                      // Center of screen
            100.0f, 100.0f,                  // Size
            vec4(1.0f, 1.0f, 1.0f, 1.0f)     // White (Script will change this to Yellow)
        );

        // 2. Add the ScriptComponent
        if (testObj)
        {
            ScriptComponent* scriptComp = testObj->AddComponent<ScriptComponent>();

            if (scriptComp)
            {
                // 3. Attach the script "SpinningSquare"
                // Ensure the namespace in C# is "GameScripts"
                scriptComp->AddScript("SpinningSquare");

                std::cout << "[Main] Created 'MyTestSquare' and attached 'SpinningSquare'.\n";
            }
        }

        GameObject* Hero = GameObjectFactory::Instance().CreateAnimatedImage(
            "Hero",
            0, 0,
            100, 100,
            "../Assets/Mouch_idle.png", 1, 2, 0, 7, 2.0f, CollisionLayer::Player);

        if (Hero)
        {
            ColliderComponent* colComp = Hero->AddComponent<ColliderComponent>();
            PhysicsComponent* phyComp = Hero->AddComponent<PhysicsComponent>();

            if (colComp && phyComp)
            {
                colComp->SetSize(100.0f, 100.0f);
            }

            ScriptComponent* scriptComp = Hero->AddComponent<ScriptComponent>();

            if (scriptComp)
            {
                // 3. Attach the script "SpinningSquare"
                // Ensure the namespace in C# is "GameScripts"
                scriptComp->AddScript("PlayerMovement");
                scriptComp->AddScript("PlayerMovementStats");

                std::cout << "[Main] Created 'PlayerMovement' and attached 'Hero_cs'.\n";
            }
        }

        GameObject* Platform = GameObjectFactory::Instance().CreateShape(
            "Platform",                  // <--- NAME MUST MATCH C#
            RenderComponent::ShapeType::Square,
            0.0f, -200.0f,                      // Center of screen
            10000.0f, 100.0f,                  // Size
            vec4(1.0f, 1.0f, 1.0f, 0.6f)     // White (Script will change this to Yellow)
        );

        if (Platform)
        {
            ColliderComponent* colComp = Platform->AddComponent<ColliderComponent>();
            PhysicsComponent* phyComp = Platform->AddComponent<PhysicsComponent>();

            if (colComp && phyComp)
            {
                phyComp->SetStatic(true);
                colComp->SetSize(10000.0f, 100.0f);
                colComp->SetLayer(CollisionLayer::Ground);
            }
        }

        GameObject* Wall = GameObjectFactory::Instance().CreateShape(
            "Platform",                  // <--- NAME MUST MATCH C#
            RenderComponent::ShapeType::Square,
            -200.0f, 0.0f,                      // Center of screen
            100.0f, 1000.0f,                  // Size
            vec4(1.0f, 1.0f, 1.0f, 0.6f)     // White (Script will change this to Yellow)
        );

        if (Wall)
        {
            ColliderComponent* colComp = Wall->AddComponent<ColliderComponent>();
            PhysicsComponent* phyComp = Wall->AddComponent<PhysicsComponent>();

            if (colComp && phyComp)
            {
                phyComp->SetStatic(true);
                colComp->SetSize(100.0f, 1000.0f);
                colComp->SetLayer(CollisionLayer::Ground);
            }
        }

        // Initialise Game State Manager (Level1)
        GameManager::Instance().ChangeState(std::make_unique<Level1>());

        API::StartEngineLoop(engine);
    }
    catch (const std::exception &e)
    {
        std::cerr << "[TEST] Exception caught: " << e.what() << "\n";
        CrashHandler::LogCrash("EXCEPTION", &e);
    }
    catch (...)
    {
        CrashHandler::LogCrash("UNKNOWN EXCEPTION");
    }
    return 0;
}

namespace CrashHandler
{

#if defined(_WIN32)
#include <windows.h>
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")
#endif
    /**
     * @brief Captures and returns the current stack trace as a formatted string.
     * @return A string containing the call stack (Windows only).
     */
    std::string GetStackTrace()
    {
#if defined(_WIN32)
        void *stack[62];
        HANDLE process = GetCurrentProcess();
        SymInitialize(process, NULL, TRUE);
        WORD frames = CaptureStackBackTrace(0, 62, stack, NULL);

        std::ostringstream oss;
        for (WORD i = 0; i < frames; ++i)
        {
            DWORD64 address = (DWORD64)(stack[i]);
            char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
            PSYMBOL_INFO symbol = (PSYMBOL_INFO)buffer;
            symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
            symbol->MaxNameLen = MAX_SYM_NAME;

            DWORD64 displacement = 0;
            if (SymFromAddr(process, address, &displacement, symbol))
            {
                oss << "[" << i << "] " << symbol->Name
                    << " - 0x" << std::hex << symbol->Address << std::dec << "\n";
            }
        }
        return oss.str();
#else
        return "Stacktrace not supported on this platform.";
#endif
    }
    /**
     * @brief Logs crash details and stack trace to a file.
     * @param type Type or category of the crash (e.g., "EXCEPTION").
     * @param e Optional exception pointer for additional error details.
     */
    void LogCrash(const char *type, const std::exception *e)
    {
        std::filesystem::create_directories("../Logs");
        std::ofstream log("../Logs/CrashLog.txt", std::ios::app);
        auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        log << "\n[" << type << "] " << std::put_time(std::localtime(&now), "%F %T") << "\n";
        if (e)
            log << e->what() << "\n";
        log << GetStackTrace() << "\n";
        log.close();
    }
} // namespace CrashHandler
