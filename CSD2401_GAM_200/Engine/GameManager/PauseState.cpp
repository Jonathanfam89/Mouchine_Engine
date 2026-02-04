/* Start Header************************************************************************ /
/*!
\file       PauseState.cpp
\author     Lee Hwee Min [100% - 23 lines]
\par        l.hweemin@digipen.edu
\date       November, 29, 2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "PauseState.h"
#include <iostream>

/**
 * @brief Load assets such as pause menu UI.
 */
void PauseState::Load() {
	std::cout << "[PAUSE] Load Pause UI Assets\n";
}

/**
 * @brief Initialize pause overlay state.
 */
void PauseState::Init() {
	std::cout << "[PAUSE] Init\n";
}

/**
 * @brief Pause logic update.
 */
void PauseState::Update(float) {
	std::cout << "[PAUSE] Frozen... press ESC to resume\n";
}

/**
 * @brief Draw pause overlay (UI only).
 */
void PauseState::Draw() {
	std::cout << "[PAUSE] Draw Pause Overlay\n";
}

/**
 * @brief Free objects
 */
void PauseState::Free() {
	std::cout << "[PAUSE] Free\n";
}

/**
 * @brief Unload textures/audios
 */
void PauseState::Unload() {
	std::cout << "[PAUSE] Unload\n";
}