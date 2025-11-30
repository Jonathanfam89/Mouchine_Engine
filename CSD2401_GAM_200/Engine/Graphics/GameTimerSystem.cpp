#include "GameTimerSystem.h"

#include <cstdio>
#include <iostream>

void GameTimerSystem::Initialize()
{
    remainingTime = 30.0f;
    lastDisplayedSeconds = -1;
    hasTriggeredTimeUp = false;
    UpdateText(30);
}

void GameTimerSystem::Update(float dt)
{
#ifdef USE_IMGUI
    LevelEditor* editor = CORE ? CORE->GetLevelEditor() : nullptr;
    if (!editor)
    {
        return;
    }

    EditorMode mode = editor->GetEditorMode();

    // reset timer and text
    if (mode == EditorMode::Stop)
    {
        if (remainingTime != 30.0f || lastDisplayedSeconds != 30)
        {
            remainingTime = 30.0f;
            lastDisplayedSeconds = -1;
            hasTriggeredTimeUp = false;
            UpdateText(30);
        }
        return;
    }

    // keep timer frozen
    if (mode == EditorMode::Pause || CORE->IsGamePaused())
    {
        return;
    }

    // only decrease time in play mode
    if (mode == EditorMode::Play)
    {
#endif
        remainingTime -= dt;

        // check for lose condition aka time ran out
        if (remainingTime <= 0.0f && !hasTriggeredTimeUp)
        {
            remainingTime = 0.0f;
            UpdateText(0);
            hasTriggeredTimeUp = true;

            // LOSE CONDITION - trigger lose screen
            std::cout << "TIME'S UP! Triggering lose screen..." << std::endl;
            WinLoseLogicSystem* winLoseLogic = CORE->GetWinLoseLogic();
            if (winLoseLogic)
            {
                winLoseLogic->TriggerLose();
            }
            return;
        }

        int totalSeconds = static_cast<int>(remainingTime);
        if (totalSeconds != lastDisplayedSeconds)
        {
            lastDisplayedSeconds = totalSeconds;
            UpdateText(totalSeconds);
        }
#ifdef USE_IMGUI
    }
#endif
}

void GameTimerSystem::ResetTimer()
{
    remainingTime = 30.0f;
    lastDisplayedSeconds = -1;
    hasTriggeredTimeUp = false;
    UpdateText(30);
}

void GameTimerSystem::DecreaseTime(float amount)
{
    remainingTime -= amount;
    if (remainingTime < 0.0f)
    {
        remainingTime = 0.0f;
    }
}

void GameTimerSystem::UpdateText(int totalSeconds)
{
    // find timer by name
    auto& factory = GameObjectFactory::Instance();
    GameObject* timerGO = factory.FindGameObject(timerObjectName);

    if (!timerGO)
    {
        //std::cout << "[GameTimerSystem] Timer GameObject \"" << timerObjectName << "\" not found.\n";
        return;
    }

    auto* textComp = timerGO->GetComponent<TextComponent>();
    if (!textComp)
    {
        //std::cout << "[GameTimerSystem] No TextComponent on \"" << timerObjectName << "\".\n";
        return;
    }

    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;

    char buffer[16];
    std::snprintf(buffer, sizeof(buffer), "%02d:%02d", minutes, seconds);

    textComp->SetText(buffer);
}
