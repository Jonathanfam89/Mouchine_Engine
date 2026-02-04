/* Start Header************************************************************************ /
/*!
\file       GameManager.h
\author     Lee Hwee Min [100% - 32 lines]
\par        l.hweemin@digipen.edu
\date       November, 29, 2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include <stack>
#include <memory>
#include <iostream>

extern bool g_UseGameStates;

/*
 * @Brief Base class for all game states
 */
class GameState {
public:
	virtual ~GameState() {}

	virtual const char* GetName() const = 0;

	virtual void Load() {}
	virtual void Init() {}
	virtual void Update(float dt) { (void)dt; }
	virtual void Draw() {}
	virtual void Free() {}
	virtual void Unload() {}
};


/*
 * @Brief A stack - based state machine ( Push/pop state system)
 */
class GameManager {
public:
	static GameManager& Instance();

	void ChangeState(std::unique_ptr<GameState> state); // Remove current, add new
	void PushState(std::unique_ptr<GameState> state);	// Add on top of current
	void PopState();									// Remove top state
	
	void Update(float dt); // Call update on active state only
	void Draw();		   // Call Draw on active state only

	void GameStateLoop(float dt);

	const char* GetActiveStateName() const;
	unsigned int GetStackSize() const;

	static bool g_UseGameStates;
private:
	GameManager() {}
	std::stack<std::unique_ptr<GameState>> m_stateStack;
};