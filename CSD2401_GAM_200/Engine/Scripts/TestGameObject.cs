using GameEngine;

namespace CSharpScript
{
    /// <summary>
    /// Simple script that creates two static square objects on start.
    /// Demonstrates basic GameObject creation functionality.
    /// </summary>
    public class TestGameObjects
    {
        /// <summary>
        /// Called when the script is first initialized.
        /// Creates two static squares (red and blue) for testing purposes.
        /// </summary>
        public void OnStart()
        {
            Debug.Log("TwoSquaresSpawner: Creating two squares...");

            // Create first square - Red, on the left
            GameObject.CreateShape(
                "RedSquare",                     // name
                (int)ShapeType.Square,           // type: square
                100, 300,                        // x, y position
                100, 100,                        // width, height
                1.0f, 0.0f, 0.0f, 1.0f,          // r, g, b, a (red color)
                (int)CollisionLayer.Default,     // collision layer
                false,                           // hasPhysics = false (no physics)
                false,                           // isStatic (doesn't matter if no physics)
                false                            // hasGravity = false (no gravity)
            );

            // Create second square - Blue, on the right
            GameObject.CreateShape(
                "BlueSquare",                    // name
                (int)ShapeType.Square,           // type: square
                400, 300,                        // x, y position
                100, 100,                        // width, height
                0.0f, 0.0f, 1.0f, 1.0f,          // r, g, b, a (blue color)
                (int)CollisionLayer.Default,     // collision layer
                false,                           // hasPhysics = false (no physics)
                false,                           // isStatic
                false                            // hasGravity = false (no gravity)
            );

            Debug.Log("TwoSquaresSpawner: Two squares created successfully!");
        }

        /// <summary>
        /// Called every frame to update game logic.
        /// This script has no update logic as the squares are static.
        /// </summary>
        /// <param name="deltaTime">The time elapsed since the last frame in seconds.</param>
        public void OnUpdate(float deltaTime)
        {
            // Nothing to update - squares are static
        }

        /// <summary>
        /// Called when the script is destroyed or the GameObject is removed.
        /// Performs cleanup operations and logs a message.
        /// </summary>
        public void OnDestroy()
        {
            Debug.Log("TwoSquaresSpawner: Cleaning up...");
        }
    }
}