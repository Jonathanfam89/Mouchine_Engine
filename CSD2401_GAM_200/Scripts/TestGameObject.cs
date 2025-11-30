
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