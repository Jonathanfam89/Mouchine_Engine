
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
                114,115,//start and end frames
                8,(int)CollisionLayer.Default,
                false,false,
                false);

            // Create second square - Blue, on the right
            GameObject.CreateShape(
                "Ground",                    // name
                (int)ShapeType.Square,           // type: square
                0, -500,                        // x, y position
                3000, 500,                        // width, height
                0.0f, 0.0f, 0.0f, 1.0f,          // r, g, b, a (blue color)
                (int)CollisionLayer.Ground,     // collision layer
                true,                           // hasPhysics = false (no physics)
                true,                           // isStatic
                false                            // hasGravity = false (no gravity)
            );

            
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
            Console.WriteLine(Utils.GetDelta_Time());
         }

        public void OnDestroy()
        {
            Debug.Log("TestGameObject: Cleaning up...");
        }
    }
}