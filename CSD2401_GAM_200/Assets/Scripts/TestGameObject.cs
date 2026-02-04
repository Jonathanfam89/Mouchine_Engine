
/* Start Header ************************************************************************/
/*!
\file       TestGameObject.cs
\author     Jonathan Fam Qian Wei [100% - 55lines]
\par        Fam.q@digipen.edu
\date       Novenber, 7, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

using CSharpScript;
using System.Runtime.CompilerServices;

namespace GameScripts
{
    public class TestGameObject
    {
        public void OnStart()
        {
            // Create the animated player object
            CSharpScript.GameObject.CreateAnimatedImage(
                "Hero",
                0, 0,
                100, 100,
                "../Assets/Mouch_idle.png",
                1, 2,
                0, 0, 0, 1,
                12.0f,
                (int)CSharpScript.CollisionLayer.Player,
                true,
                false,
                true
            );
            // Add sounds
            Audio.AddSound("jump_sfx", "../Assets/Audio/Player/Jump.wav", false, 0.8f, 1.0f, "sfx");
            Audio.AddSound("footstep_sfx", "../Assets/audio/Player/Footstep.wav", false, 0.8f, 1.0f, "sfx");
            Audio.AddSound("groundpound_sfx", "../Assets/audio/Player/Groundpound.wav", false, 0.8f, 1.0f, "sfx");
            Audio.AddSound("roll_sfx", "../Assets/audio/Player/Roll.wav", false, 0.2f, 1.0f, "sfx");
            Audio.AddSound("dash_sfx", "../Assets/audio/Player/Dash.wav", false, 0.8f, 1.0f, "sfx");
            Audio.AddSound("wallkick_sfx", "../Assets/audio/Player/Wallkick.wav", false, 0.8f, 1.0f, "sfx");
            Audio.AddSound("land_sfx", "../Assets/audio/Player/Land.wav", false, 0.3f, 1.0f, "sfx");

            Animation.RegisterAnimation("Hero", "Idle", "../Assets/spritesheet_R.png"       , 13, 10, 7, 0, 7, 2, 4.0f);
            Animation.RegisterAnimation("Hero", "Run", "../Assets/spritesheet_R.png"        , 13, 10, 12, 0, 12, 7, 16.0f);
            Animation.RegisterAnimation("Hero", "Jump", "../Assets/spritesheet_R.png"       , 13, 10, 11, 4, 11, 5, 12.0f);
            Animation.RegisterAnimation("Hero", "Glide", "../Assets/spritesheet_R.png"      , 13, 10, 1, 0, 1, 2, 8.0f);
            Animation.RegisterAnimation("Hero", "Roll", "../Assets/spritesheet_R.png"       , 13, 10, 8, 0, 8, 5, 12.0f);
            Animation.RegisterAnimation("Hero", "Groundpound", "../Assets/spritesheet_R.png", 13, 10, 9, 3, 9, 4, 10.0f);
            Animation.RegisterAnimation("Hero", "Dash", "../Assets/spritesheet_R.png"       , 13, 10, 3, 0, 3, 2, 4.0f);
            Animation.RegisterAnimation("Hero", "Wallkick", "../Assets/spritesheet_R.png"   , 13, 10, 4, 3, 4, 4, 4.0f);
            Animation.RegisterAnimation("Hero", "Wallslide", "../Assets/spritesheet_L.png"  , 13, 10, 4, 0, 4, 1, 4.0f);
            Animation.RegisterAnimation("Hero", "Falling", "../Assets/spritesheet_L.png"    , 13, 10, 10, 0, 10, 1, 4.0f);
            Animation.RegisterAnimation("Hero", "Land", "../Assets/spritesheet_L.png"       , 13, 10, 10, 3, 10, 4, 4.0f);

            // Attach the player movement script to the Hero
            CSharpScript.Scripting.AttachScript("Hero", "PlayerMovement");

            // Create the red square
            CSharpScript.GameObject.CreateShape(
                "RedSquare",
                (int)CSharpScript.ShapeType.Square,
                500, 300,
                50, 50,
                1.0f, 0.0f, 0.0f, 1.0f,
                (int)CSharpScript.CollisionLayer.Default,
                false, false, false
            );

            // Create the green square
            CSharpScript.GameObject.CreateShape(
                "GreenSquare",
                (int)CSharpScript.ShapeType.Square,
                -300, 300,
                50, 50,
                0.0f, 1.0f, 0.0f, 1.0f,
                (int)CSharpScript.CollisionLayer.Default,
                false, false, false
            );

            // Create the ground object
            CSharpScript.GameObject.CreateShape(
                "Ground",
                (int)CSharpScript.ShapeType.Square,
                0, -200,
                1000, 50,
                0.5f, 0.5f, 0.5f, 1.0f,
                (int)CSharpScript.CollisionLayer.Ground,
                true, true, false
            );
            Components.AddCollisionComponent("Hero");

            // Attach the RaycastTest script to the RedSquare
            CSharpScript.Scripting.AttachScript("RedSquare", "RaycastTest");
        }

        public void OnUpdate(float deltaTime)
        {
 
        }

        public void OnDestroy()
        {
            CSharpScript.Debug.Log("TestGameObject: Cleaning up...");
        }
    }
}