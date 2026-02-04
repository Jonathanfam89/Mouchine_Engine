/* Start Header************************************************************************ /
/*!
\file       PauseState.h
\author     Lee Hwee Min [100% - 12 lines]
\par        l.hweemin@digipen.edu
\date       November, 29, 2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/


#pragma once
#include "GameManager.h"

/*
 * @Brief Overlays on top of gameplay when paused.
 */
class PauseState : public GameState {
public:
	const char* GetName() const { return "PAUSE"; }

	void Load();
	void Init();
	void Update(float dt);
	void Draw();
	void Free();
	void Unload();
};