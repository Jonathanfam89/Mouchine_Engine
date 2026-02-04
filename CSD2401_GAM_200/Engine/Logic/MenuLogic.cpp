/* Start Header ************************************************************************/
/*!
\file       MenuLogic.cpp
\author     Jonathan Fam Qian Wei [100% - 83 lines]
\par        Fam.q@digipen.edu
\date       October, 2, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "MenuLogic.h"
#include <iostream>

std::string introPath = "../Assets/Cutscene_M3.mpeg";

// abit overkill but if we want more screens in the menu this is better
enum MenuScreens {
    Home = 0,       // Normal landing page
    Settings
};
MenuScreens currScreen = MenuScreens::Home;

void MenuLogicSystem::Update(float dt)
{
    // Update audio fade
    AudioManager* audioManager = CORE->GetAudioManager();
    if (audioManager)
    {
        audioManager->UpdateFade(dt);
    }

    // if a scene change is pending, count down timer first so the click sound can be played
    if (pendingSceneChange)
    {
        pendingTimer -= dt;
        if (pendingTimer <= 0.0f)                       // timer delay to play the sound effect before loading into scene
        {
            if (currScreen == MenuScreens::Home)
            {
                if (hoveredButtonName == "PlayButton")
                {
                    std::cout << "PlayButton Clicked!" << std::endl;

                    CutsceneSystem* cutscene = CORE->GetCutsceneSystem();
                    if (!cutscene->HasFinished(introPath)) {    // FIRST CLICK: Play Cutscene
                        if (!cutscene->IsPlaying()) {
                            cutscene->PlayVideo(introPath);
                            // Don't fade back in - let the fade out complete
                            std::cout << "[MenuLogic] Playing cutscene, music remains faded out" << std::endl;
                        }
                    }
                    else {                                      // SECOND CLICK: Video has finished, load the game
                        std::cout << "[MenuLogic] Requesting scene load with AUTO-PLAY = TRUE" << std::endl;
                        std::cout << "[MenuLogic] Music should already be fading out from earlier" << std::endl;
                        CORE->RequestSceneLoad("../Assets/Scenes/Scene1.json", true);
                        std::cout << "[MenuLogic] RequestSceneLoad called" << std::endl;
                    }

                }
                else if (hoveredButtonName == "SettingsButton")
                {
                    std::cout << "SettingsButton Clicked!" << std::endl;

                    currScreen = MenuScreens::Settings;
                    //CORE->RequestSceneLoad("../Assets/Scenes/Scene2.json", true);
                }
                else if (hoveredButtonName == "CreditsButton")
                {
                    std::cout << "CreditsButton Clicked!" << std::endl;
                    // to add credits screen here
                    //CORE->RequestSceneLoad("../Assets/Scenes/Credits.json", true);
                }
                else if (hoveredButtonName == "QuitButton")
                {
                    std::cout << "QuitButton Clicked!" << std::endl;
                    glfwSetWindowShouldClose(CORE->GetGraphicsSystem()->getWindow(), GLFW_TRUE);
                    #ifdef USE_IMGUI
                    CORE->GetLevelEditor()->StopPlayMode();
                    #endif
                    //CORE->RequestSceneLoad("../Assets/Scenes/Scene3.json", true);
                }
            }

            // clear
            pendingSceneChange = false;
            hoveredButtonName.clear();
        }

        return;
    }

    if (currScreen == MenuScreens::Home)
    {
        if (GameObjectFactory::Instance().FindGameObject("Screen_Settings"))
        {
            GameObjectFactory::Instance().FindGameObject("Screen_Settings")->SetActive(false);
        }
        if (GameObjectFactory::Instance().FindGameObject("Screen_TitleButtons"))
        {
            GameObjectFactory::Instance().FindGameObject("Screen_TitleButtons")->SetActive(true);
        }
    }
    else if (currScreen == MenuScreens::Settings)
    {
        //hoveredButtonName.clear();
        if (GameObjectFactory::Instance().FindGameObject("Screen_Settings"))
        {
            GameObjectFactory::Instance().FindGameObject("Screen_Settings")->SetActive(true);
        }
        if (GameObjectFactory::Instance().FindGameObject("Screen_TitleButtons"))
        {
            GameObjectFactory::Instance().FindGameObject("Screen_TitleButtons")->SetActive(false);
        }
        //return;
    }

    double mouseX = InputManager::GetMouseWorldX();
    double mouseY = InputManager::GetMouseWorldY();
    bool isClicked = InputManager::IsMouseLeftClicked();

    auto& factory = GameObjectFactory::Instance();
    std::string currentHover = "";

    // Depending on scene, check for which buttons have been pressed
    switch (currScreen)
    {
    case MenuScreens::Home:
        if (IsMouseOver(factory.FindGameObject("PlayButton"), mouseX, mouseY))
        {
            currentHover = "PlayButton";
        }
        else if (IsMouseOver(factory.FindGameObject("SettingsButton"), mouseX, mouseY))
        {
            currentHover = "SettingsButton";
        }
        else if (IsMouseOver(factory.FindGameObject("CreditsButton"), mouseX, mouseY))
        {
            currentHover = "CreditsButton";
        }
        else if (IsMouseOver(factory.FindGameObject("QuitButton"), mouseX, mouseY))
        {
            currentHover = "QuitButton";
        }
        break;
    case MenuScreens::Settings:
        // do settings stuff here
        if (IsMouseOver(factory.FindGameObject("MuteButton"), mouseX, mouseY))
        {
            currentHover = "MuteButton";
        }
        else if (IsMouseOver(factory.FindGameObject("BackButton"), mouseX, mouseY))
        {
            currentHover = "BackButton";
        }
        break;
    default:
        break;
    }

   
    // If the hovered button has changed since last frame
    if (currentHover != hoveredButtonName)
    {
        // Revert the PREVIOUS button texture to normal 
        if (!hoveredButtonName.empty())
        {
            SetButtonTexture(hoveredButtonName, false);
        }

        if (!currentHover.empty())
        {
            SetButtonTexture(currentHover, true);
            // Play hover sound once when entering the button

            AudioManager* audioManager = CORE->GetAudioManager();
            if (audioManager)
            {
                audioManager->TriggerEvent("UI_Hover");
            }
        }

        // Update tracked state
        hoveredButtonName = currentHover;
    }

    // Only process clicks if we are actually hovering over a button
    if (isClicked && !hoveredButtonName.empty())
    {
        // Play Click Sound
        AudioManager* audioManager = CORE->GetAudioManager();
        if (audioManager)
        {
            audioManager->TriggerEvent("UI_Click");
            std::cout << "menu button clicking sound is triggered" << std::endl;
        }

        // Start small delay before loading scene
        if (hoveredButtonName != "SettingsButton" && hoveredButtonName != "MuteButton" && hoveredButtonName != "BackButton" && hoveredButtonName != "CreditsButton")
        {
            // Start audio fade out for scene transitions
            if (hoveredButtonName == "PlayButton" && audioManager)
            {
                audioManager->FadeOutCategory("music", 1.0f);  // Fade out over 1.0 sec
                std::cout << "[MenuLogic] Started music fade out" << std::endl;
            }

            pendingSceneChange = true;
            pendingTimer = 2.0f;                       // Wait 3.0s so 2.5s fade completes before scene loads
        }
        else
        {
            if (hoveredButtonName == "MuteButton")
            {
                //AudioManager* audioManager = CORE->GetAudioManager();
                if (audioManager)
                {
                    audioManager->ToggleMasterMute();


                }
                SetButtonTexture("MuteButton", true); // Update texture after toggling
            }
            else if (hoveredButtonName == "BackButton")
            {
                currScreen = MenuScreens::Home;
            }
            else if (hoveredButtonName == "SettingsButton")
            {
                currScreen = MenuScreens::Settings;
            }
            else if (hoveredButtonName == "CreditsButton")
            {
                std::cout << "CreditsButton Clicked!" << std::endl;
                // TODO: Add credits screen logic here
                //CORE->RequestSceneLoad("../Assets/Scenes/Credits.json", true);
            }
        }
    }
}

bool MenuLogicSystem::IsMouseOver(GameObject* button, double mouseX, double mouseY)
{
    if (!button) return false;
    auto* tx = button->GetComponent<TransformComponent>();
    if (!tx) return false;

    return IsAreaClicked(tx->posX, tx->posY, tx->scaleX, tx->scaleY, static_cast<float>(mouseX), static_cast<float>(mouseY));
}

//  swap textures
void MenuLogicSystem::SetButtonTexture(std::string buttonName, bool isHovered)
{
    auto& factory = GameObjectFactory::Instance();
    GameObject* btn = factory.FindGameObject(buttonName);
    if (!btn) return;

    auto* rc = btn->GetComponent<RenderComponent>();
    if (!rc) return;

    std::string texturePath;

    if (buttonName == "PlayButton") {
        texturePath = isHovered ? "../Assets/Sprites/Icons/Menu_play_NEW_HOVER.png" : "../Assets/Sprites/Icons/Menu_Play.png";
    }
    else if (buttonName == "SettingsButton") {
        texturePath = isHovered ? "../Assets/Sprites/Icons/Menu_settings_NEW_HOVER.png" : "../Assets/Sprites/Icons/Menu_settings.png";
    }
    else if (buttonName == "QuitButton") {
        texturePath = isHovered ? "../Assets/Sprites/Icons/Menu_quit_NEW_HOVER.png" : "../Assets/Sprites/Icons/Menu_Quit.png";
    }
    else if (buttonName == "MuteButton") {
        bool isMuted = false;
        AudioManager* audioManager = CORE->GetAudioManager();
        if (audioManager)
        {
            isMuted = audioManager->IsMuted();
        }
        texturePath = isMuted ? "../Assets/Sprites/Icons/Volume_Mute.png" : "../Assets/Sprites/Icons/Volume_Unmute.png";
    }
    else if (buttonName == "BackButton") {
        texturePath = isHovered ? "../Assets/Sprites/Icons/Back_button_dark.png" : "../Assets/Sprites/Icons/Back_button.png";
    }
    else if (buttonName == "CreditsButton") {
        texturePath = isHovered ? "../Assets/Sprites/Icons/Menu_credits_NEW_HOVER.png" : "../Assets/Sprites/Icons/Menu_credits_NEW.png";
    }
  
    unsigned int newtex = GraphicsSystem::LoadTexture(texturePath.c_str());
    rc->SetTextureID(newtex);
}

void MenuLogicSystem::Reset()
{
    currScreen = MenuScreens::Home;
}
