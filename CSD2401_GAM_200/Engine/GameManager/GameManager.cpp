/* Start Header************************************************************************ /
/*!
\file       GameManager.cpp
\author     Lee Hwee Min [100% - 53 lines]
\par        l.hweemin@digipen.edu
\date       November, 29, 2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "GameManager.h"

GameManager& GameManager::Instance() {
	static GameManager instance;
	return instance;
}

/**
 * @brief Replace the current active state with a new one.
 */
void GameManager::ChangeState(std::unique_ptr<GameState> state) {
	if (!m_stateStack.empty()) {
		std::cout << "[GAME MANAGER] CLEANUP: " << m_stateStack.top()->GetName() << std::endl;
		m_stateStack.top()->Free();
		m_stateStack.top()->Unload();
		m_stateStack.pop();
	}

	std::cout << "[GAME MANAGER] CHANGE TO " << state->GetName() << "\n";
	state->Load();
	state->Init();
	m_stateStack.push(std::move(state));
}

/**
 * @brief Overlay a new state without removing the existing one. (eg PauseState)
 */
void GameManager::PushState(std::unique_ptr<GameState> state) {
	std::cout << "[GAME MANAGER] PUSH - " << state->GetName() << "\n";
	state->Load();
	state->Init();
	m_stateStack.push(std::move(state));
}

/**
 * @brief Pop the active state and return to the previous one.
 */
void GameManager::PopState() {
	if (!m_stateStack.empty()) {
		std::cout << "[GAME MANAGER] POP - " << m_stateStack.top()->GetName() << "\n";
		m_stateStack.top()->Free();
		m_stateStack.top()->Unload();
		m_stateStack.pop();
	}
}

void GameManager::Update(float dt) {
	if (!m_stateStack.empty()) {
		m_stateStack.top()->Update(dt);
	}
}

void GameManager::Draw() {
	if (!m_stateStack.empty()) {
		m_stateStack.top()->Draw();
	}
}

const char* GameManager::GetActiveStateName() const {
	if (m_stateStack.empty()) return "[NO ACTIVE STATE]";
	return m_stateStack.top()->GetName();
}

unsigned int GameManager::GetStackSize() const {
	return (unsigned int)m_stateStack.size();
}

/**
 * @brief Convenience wrapper for Main Loop
 */
void GameManager::GameStateLoop(float dt)
{
	GameManager::Instance().Update(dt);
	GameManager::Instance().Draw();
}