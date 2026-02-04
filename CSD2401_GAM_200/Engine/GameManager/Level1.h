/* Start Header************************************************************************ /
/*!
\file       Level1.cpp
\author     Lee Hwee Min [100% - 19 lines]
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
#include "PauseState.h"
#include "InputManager.h"
#include "core.h"
#include "gameObject.h"
#include "factory.h"

class Level1 : public GameState {
public:
	const char* GetName() const { return "LEVEL 1"; }

	void Load();
	void Init();
	void Update(float dt);
	void Draw();
	void Free();
	void Unload();

private:
	float timer = 0;
};