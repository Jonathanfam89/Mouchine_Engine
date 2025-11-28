
/* Start Header ************************************************************************/
/*!
\file       TestGameObject2.cs
\author     Jonathan Fam Qian Wei [100% - 38lines]
\par        Fam.q@digipen.edu
\date       Novenber, 7, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

using System;
using CSharpScript;

namespace GameScripts
{
    public class TestGameObjects2
    {
        private float rotationSpeed = 90.0f;

        public void OnStart()
        {
            // --- Moved Blue Square ---
            GameObject.CreateShape(
                "BlueSquare",
                (int)ShapeType.Square,
                300, 300,
                300, 300,
                0.0f, 0.0f, 1.0f, 1.0f, // Blue
                (int)CollisionLayer.Default,
                false, false, false
            );
            //Components.AddPhysicsComponent("BlueSquare");
        }

        public void OnUpdate(float deltaTime)
        {
            // Rotate Green Square every frame
            //Transform.AddRotation("GreenSquare", rotationSpeed * deltaTime);
        }

        public void OnDestroy()
        {

        }
    }
}