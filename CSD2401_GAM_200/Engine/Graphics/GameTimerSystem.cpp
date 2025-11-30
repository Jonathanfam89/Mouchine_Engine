/* Start Header ************************************************************************/
/*!
\file       GameTimerSystem.cpp
\author     Leck Xin Yee [100% - 118 lines]
\par        xinyee.leck@digipen.edu
\date       November, 26, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "GameTimerSystem.h"
#include "WinLoseLogic.h"

#include <cstdio>
#include <iostream>

/**
 * @brief Initializes the countdown timer and updates initial display.
 */
void GameTimerSystem::Initialize()
{
    remainingTime = 30.0f;
    lastDisplayedSeconds = -1;
    hasTriggeredTimeUp = false;
    UpdateText(30);
}

/**
 * @brief Updates timer logic, handles editor/pause conditions, hides timer during win/lose,
 *        and triggers lose state when time reaches zero.
 */
void GameTimerSystem::Update(float dt)
{
    auto& factory = GameObjectFactory::Instance();
    GameObject* timerGO = factory.FindGameObject(timerObjectName);

    // Check if we're in the main menu and hide/stop timer
    // Main menu has a "Title" GameObject
    GameObject* mainMenuCheck = factory.FindGameObject("Title");
    if (mainMenuCheck && mainMenuCheck->IsActive())
    {
        if (timerGO)
        {
            timerGO->SetActive(false);
        }
        // Reset timer when in main menu to prevent lose condition from triggering
        if (remainingTime != 30.0f)
        {
            remainingTime = 30.0f;
            lastDisplayedSeconds = -1;
            hasTriggeredTimeUp = false;
        }
        return;
    }

    // Check if win/lose screen is active and hide timer (but keep visible during pause menu)
    WinLoseLogicSystem* winLoseLogic = CORE ? CORE->GetWinLoseLogic() : nullptr;
    if (winLoseLogic)
    {
        if (winLoseLogic->IsWinScreenActive() || winLoseLogic->IsLoseScreenActive())
        {
            // Hide timer during win/lose screens
            if (timerGO)
            {
                timerGO->SetActive(false);
            }
            return;
        }
    }

    // Show timer when game is active (including during pause menu)
    if (timerGO)
    {
        timerGO->SetActive(true);
    }

    // Check if game is paused (pause menu or win/lose) - stop counting but keep visible if pause menu
    if (CORE && CORE->IsGamePaused())
    {
        return;
    }

#ifdef USE_IMGUI
    LevelEditor* editor = CORE ? CORE->GetLevelEditor() : nullptr;
    if (!editor)
    {
        return;
    }

    EditorMode mode = editor->GetEditorMode();

    // check if timer object exists in current scene, if not don't run timer
    auto& factory = GameObjectFactory::Instance();
    GameObject* timerGO = factory.FindGameObject(timerObjectName);
    if (!timerGO)
    {
        // timer doesn't exist in this scene (e.g. main menu), don't run
        return;
    }

    // hide timer if win or lose screen is active
    WinLoseLogicSystem* winLoseLogic = CORE->GetWinLoseLogic();
    if (winLoseLogic && (winLoseLogic->IsWinScreenActive() || winLoseLogic->IsLoseScreenActive()))
    {
        if (timerGO->IsActive())
        {
            timerGO->SetActive(false);
        }
        return;
    }

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

    // keep timer frozen in pause mode
    if (mode == EditorMode::Pause)
    {
        return;
    }

    // only decrease time in play mode
    if (mode == EditorMode::Play)
    {
#endif
        /*std::cout << "[GameTimer] Decreasing time by " << dt << std::endl;*/
        remainingTime -= dt;

        // check for lose condition aka time ran out
        if (remainingTime <= 0.0f && !hasTriggeredTimeUp)
        {
            remainingTime = 0.0f;
            UpdateText(0);
            hasTriggeredTimeUp = true;

            // Only trigger lose screen if NOT in main menu
            GameObject* mainMenuCheck = factory.FindGameObject("Title");
            bool isInMainMenu = mainMenuCheck && mainMenuCheck->IsActive();

            if (!isInMainMenu)
            {
                // LOSE CONDITION - trigger lose screen
                //std::cout << "TIME'S UP! Triggering lose screen..." << std::endl;
                WinLoseLogicSystem* winLoseLogic = CORE->GetWinLoseLogic();
                if (winLoseLogic)
                {
                    winLoseLogic->TriggerLose();
                }
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

/**
 * @brief Resets timer and updates displayed text.
 */
void GameTimerSystem::ResetTimer()
{
    remainingTime = 30.0f;
    lastDisplayedSeconds = -1;
    hasTriggeredTimeUp = false;
    UpdateText(30);
}

/**
 * @brief Decreases the remaining timer value by a given amount.
 */
void GameTimerSystem::DecreaseTime(float amount)
{
    remainingTime -= amount;
    if (remainingTime < 0.0f)
    {
        remainingTime = 0.0f;
    }
}

/**
 * @brief Updates the formatted timer text (mm:ss) on the assigned TextComponent.
 */
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

    // check if win or lose screen is active, if so hide the timer
    WinLoseLogicSystem* winLoseLogic = CORE->GetWinLoseLogic();
    if (winLoseLogic && (winLoseLogic->IsWinScreenActive() || winLoseLogic->IsLoseScreenActive()))
    {
        textComp->SetText("");
        timerGO->SetActive(false);
        return;
    }

    // make sure timer is active when not on win/lose screen
    if (!timerGO->IsActive())
    {
        timerGO->SetActive(true);
    }

    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;

    char buffer[16];
    std::snprintf(buffer, sizeof(buffer), "%02d:%02d", minutes, seconds);

    textComp->SetText(buffer);
}
