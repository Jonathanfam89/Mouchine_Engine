
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
using System;
using System.Threading;

namespace GameScripts
{
    public class TestGameObject
    {
        private float redSquareX = -200.0f;
        private const float redSquareY = 300.0f;
        private const float moveSpeed = 100.0f;

        public void OnStart()
        {
            GameObject.CreateShape(
                "RedSquare",                     // name
                (int)ShapeType.Square,           // type: square (fixed from Circle)
                -200, 300,                        // x, y position
                200, 200,                        // width, height
                1.0f, 0.0f, 0.0f, 1.0f,          // r, g, b, a (red color)
                (int)CollisionLayer.Default,     // collision layer
                false,                           // hasPhysics = false (no physics)
                false,                           // isStatic (doesn't matter if no physics)
                false                            // hasGravity = false (no gravity)
            );  

            GameObject.CreateAnimatedImage("C#_AniImage",0,0,
                300, 300,
                "../Assets/spritesheet_R.png",
                13,10,//size of the spritesheet
                11, 4, 11, 5, // start row/col and end row/col
                8,(int)CollisionLayer.Default,
                false,false,
                false);

            GameObject.CreateAnimatedImage("Rolling", 0, -200, 200, 200, "../Assets/spritesheet_L.png", 13, 10, 8, 0, 8, 5, 12, (int)CollisionLayer.Default, false, false, false);
            Animation.RegisterAnimation("Rolling", "Roll", "../Assets/spritesheet_L.png", 13, 10, 9, 0, 9, 9, 10);
            Animation.RegisterAnimation("Rolling", "Idle", "../Assets/spritesheet_L.png", 13, 10, 0, 0, 0, 5, 10);
            Animation.RegisterAnimation("Rolling", "Walk", "../Assets/spritesheet_L.png", 13, 10, 1, 0, 1, 5, 10);

            Animation.AddToQueue("Rolling", "Roll");
            Animation.AddToQueue("Rolling", "Idle");
            Animation.AddToQueue("Rolling", "Walk");
            
        }

        

        public void OnUpdate(float deltaTime)
        {
            redSquareX -= moveSpeed * deltaTime;

            // 3. Apply the updated position to the game object
            Transform.SetPosition("RedSquare", redSquareX, redSquareY);

            if (redSquareX < -800)
            {
                redSquareX = 500;
            }
        }

        public void OnDestroy()
        {
            Debug.Log("TestGameObject: Cleaning up...");
        }
    }
}