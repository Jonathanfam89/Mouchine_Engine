/* Start Header ************************************************************************/
/*!
\file       RaycastTest.cs
\author     Jonathan Fam Qian Wei [100% - 10 lines]
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
    public class RaycastTest
    {
        public string name = "RaycastTest";
        private vec2 initialPosition;
        private float speed = 100.0f;

        public void OnStart()
        {
            // Store the initial position of the red square
            initialPosition = CSharpScript.Transform.GetPosition("RedSquare");
        }

        public void OnUpdate(float deltaTime)
        {
            // Move the red square from right to left
            vec2 currentPosition = CSharpScript.Transform.GetPosition("RedSquare");
            currentPosition.x -= speed * deltaTime;
            CSharpScript.Transform.SetPosition("RedSquare", currentPosition.x, currentPosition.y);

            // Perform a raycast to the left
            vec2 direction = new vec2(-1, 0);
            if (CSharpScript.Collision.Raycast("RedSquare", currentPosition, direction, 200.0f, out string hitObjectName, out float hitDistance))
            {
                CSharpScript.Debug.Log($"Raycast hit: {hitObjectName} at distance: {hitDistance}");

                // Check if the hit object is the green square and if the distance is less than 100
                if (hitObjectName == "GreenSquare" && hitDistance < 100.0f)
                {
                    // Teleport back to the initial position
                    CSharpScript.Transform.SetPosition("RedSquare",500, 300);
                    CSharpScript.Debug.Log("Red square teleported back to initial position.");
                }
            }
        }
    }
}
