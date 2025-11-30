/* Start Header ************************************************************************/
/*!
\file       TimerSystem.cpp
\author     Jonathan Fam Qian Wei [100% - 111 lines]
\par        Fam.q@digipen.edu
\date       Novenber, 15, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "TimerSystem.h"
#include <iostream>
#include <algorithm>

void TimerSystem::Initialize()
{
    std::cout << "TimerSystem Initialized" << std::endl;
    m_timers.reserve(20);
}

void TimerSystem::Update(float dt)
{
    // Update all active timers
    for (auto& timer : m_timers)
    {
        if (timer.isFinished || timer.isPaused) continue;

        timer.elapsed += dt;

        if (timer.elapsed >= timer.duration)
        {
            // Execute the callback safely
            if (timer.callback)
            {
                timer.callback();
            }

            if (timer.repeat)
            {
                // Reset for next cycle, keeping the overshoot for accuracy
                timer.elapsed -= timer.duration;
            }
            else
            {
                timer.isFinished = true;
            }
        }
    }

    // Clean up finished timers
    m_timers.erase(
        std::remove_if(m_timers.begin(), m_timers.end(),
            [](const Timer& t) { return t.isFinished; }),
        m_timers.end());
}

TimerID TimerSystem::AddTimer(float duration, TimerCallback callback, bool repeat, const std::string& name)
{
    Timer newTimer;
    newTimer.id = m_nextId++;
    newTimer.duration = duration;
    newTimer.elapsed = 0.0f;
    newTimer.callback = callback;
    newTimer.repeat = repeat;
    newTimer.name = name;
    newTimer.isFinished = false;
    newTimer.isPaused = false;

    m_timers.push_back(newTimer);
    return newTimer.id;
}

void TimerSystem::CancelTimer(TimerID id)
{
    if (id == 0) return;
    for (auto& timer : m_timers)
    {
        if (timer.id == id)
        {
            timer.isFinished = true;
            return;
        }
    }
}

void TimerSystem::CancelTimerByName(const std::string& name)
{
    for (auto& timer : m_timers)
    {
        if (timer.name == name)
        {
            timer.isFinished = true;
        }
    }
}

void TimerSystem::PauseTimer(TimerID id)
{
    if (id == 0) return;
    for (auto& timer : m_timers)
    {
        if (timer.id == id) timer.isPaused = true;
    }
}

void TimerSystem::ResumeTimer(TimerID id)
{
    if (id == 0) return;
    for (auto& timer : m_timers)
    {
        if (timer.id == id) timer.isPaused = false;
    }
}

bool TimerSystem::IsTimerActive(TimerID id)
{
    if (id == 0) return false;
    for (const auto& timer : m_timers)
    {
        if (timer.id == id && !timer.isFinished) return true;
    }
    return false;
}

float TimerSystem::GetTimeRemaining(TimerID id)
{
    for (const auto& timer : m_timers)
    {
        if (timer.id == id && !timer.isFinished)
        {
            return std::max(0.0f, timer.duration - timer.elapsed);
        }
    }
    return -1.0f; // Not found
}

void TimerSystem::ClearAllTimers()
{
    m_timers.clear();
}