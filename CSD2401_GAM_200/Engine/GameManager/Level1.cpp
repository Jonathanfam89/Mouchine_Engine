/* Start Header************************************************************************ /
/*!
\file       Level1.cpp
\author     Lee Hwee Min [100% - 32 lines]
\par        l.hweemin@digipen.edu
\date       November, 29, 2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "Level1.h"
#include "PauseState.h"
//#include "Level2.h"
#include <iostream>
#include <Windows.h> // Just a helper for now

void Level1::Load() {
	std::cout << "[LEVEL 1] Load Assets\n";
    
    //Serialization::LoadScene("Assets/Scenes/Level1.json");
}

void Level1::Init() {
    std::cout << "[LEVEL 1] Initialize\n";
}

void Level1::Update(float dt) {
	timer += dt;
	std::cout << "[LEVEL 1] Updating, timer = " << timer << "\n";

    // Press P to enter pause -- for testing, will have to be place in inputmanager
    if (GetAsyncKeyState('P') & 1)
    {
        GameManager::Instance().PushState(
            std::unique_ptr<GameState>(new PauseState()));
    }

    // Press ESC to remove THIS LEVEL -- for testing, will have to be place in inputmanager
    if (GetAsyncKeyState(VK_ESCAPE) & 1)
    {
        GameManager::Instance().PopState();
    }

    // Proceed to level 2th
    //if (true) {
    //    GameManager::Instance().ChangeState(std::make_unique<Level2>());
    //}

}

void Level1::Draw() {
    std::cout << "[LEVEL 1] Drawing Level\n";
}

void Level1::Free() {
    std::cout << "[LEVEL 1] Free Objects\n";
    //GameObjectFactory::Instance().DestroyAllGameObjects();
}

void Level1::Unload() {
    std::cout << "[LEVEL 1] Unload Assets\n";
  
}