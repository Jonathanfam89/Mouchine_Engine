/* Start Header ************************************************************************/
/*!
\file       TimerSystem.h
\author     Jonathan Fam Qian Wei [100% - 38 lines]
\par        Fam.q@digipen.edu
\date       Novenber, 15, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/


#pragma once
#include "System.h"
#include <vector>
#include <functional>
#include <string>

// Define a callback type for easier usage
using TimerCallback = std::function<void()>;
using TimerID = unsigned int;

struct Timer {
    TimerID id;             // Unique numeric ID
    std::string name;       // Optional name for debugging
    float duration;         // Total time to wait (or interval for repeating)
    float elapsed;          // Time passed so far
    bool repeat;            // Should it restart after finishing?
    bool isFinished;        // Flag to mark for deletion
    bool isPaused;          // Is the timer currently paused?
    TimerCallback callback; // The function to execute
};

class TimerSystem : public ISystem
{
public:
    TimerSystem() = default;
    ~TimerSystem() = default;

    void Initialize() override;
    void Update(float dt) override;
    std::string GetName() override { return "TimerSystem"; }

    /**
     * @brief Schedules a function to be called after a delay.
     * @return A unique TimerID to control this timer later.
     */
    TimerID AddTimer(float duration, TimerCallback callback, bool repeat = false, const std::string& name = "");

    // --- Control Functions ---

    // Cancel a timer by its specific ID
    void CancelTimer(TimerID id);
    
    // Cancel all timers with a specific string name
    void CancelTimerByName(const std::string& name);

    // Pause a specific timer
    void PauseTimer(TimerID id);

    // Resume a specific timer
    void ResumeTimer(TimerID id);

    // Check if a timer exists and is running
    bool IsTimerActive(TimerID id);

    // Get time remaining on a specific timer (returns -1.0f if not found)
    float GetTimeRemaining(TimerID id);

    void ClearAllTimers();

private:
    std::vector<Timer> m_timers;
    TimerID m_nextId = 1; // Start IDs at 1 (0 is invalid)
};