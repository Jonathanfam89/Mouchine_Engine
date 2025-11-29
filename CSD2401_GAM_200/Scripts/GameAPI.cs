/* Start Header ************************************************************************/
/*!
\file       GameAPI.cs
\author     Jonathan Fam Qian Wei [100% - 125 lines]
\par        Fam.q@digipen.edu
\date       Novenber, 7, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using static System.Runtime.CompilerServices.RuntimeHelpers;

namespace CSharpScript
{
    // --- Structs ---
    [StructLayout(LayoutKind.Sequential)]
    public struct vec2
    {
        public float x;
        public float y;

        public vec2(float x, float y)
        {
            this.x = x;
            this.y = y;
        }

        public override string ToString()
        {
            return $"({x}, {y})";
        }
    }
    [StructLayout(LayoutKind.Sequential)]
    public struct vec4
    {
        public float x, y, z, w;

        public vec4(float x, float y, float z, float w)
        {
            this.x = x;
            this.y = y;
            this.z = z;
            this.w = w;
        }

        public override string ToString()
        {
            return $"({x}, {y}, {z}, {w})";
        }
    }

    // --- Enums ---
    // These must match your C++ enum values exactly

    /// <summary>
    /// Enumeration of shape types for rendering.
    /// Must match the C++ ShapeType enum values exactly.
    /// </summary>
    public enum ShapeType
    {
        /// <summary>Square shape.</summary>
        Square = 0,
        /// <summary>Circle shape.</summary>
        Circle = 1,
        /// <summary>Point shape.</summary>
        Point = 2,
        /// <summary>Line shape.</summary>
        Line = 3
    }

    /// <summary>
    /// Enumeration of collision layers for collision detection.
    /// Must match the C++ CollisionLayer enum values exactly.
    /// </summary>
    public enum CollisionLayer
    {
        /// <summary>Default collision layer.</summary>
        Default = 0,
        /// <summary>Player collision layer.</summary>
        Player = 1,
        /// <summary>Enemy collision layer.</summary>
        Enemy = 2,
        /// <summary>Ground collision layer.</summary>
        Ground = 3
        // Add more layers as defined in your C++ Collision.h
    }

    /// <summary>
    /// Enumeration of collider types for collision shapes.
    /// Must match the C++ ColliderType enum values exactly.
    /// </summary>
    public enum ColliderType
    {
        /// <summary>Box collider type.</summary>
        Box = 0,
        /// <summary>Circle collider type.</summary>
        Circle = 1
        // Add more if you have them in C++
    }


    /// <summary>
    /// Debug utilities for logging from C# to C++ console.
    /// </summary>
    public static class Debug
    {
        /// <summary>
        /// Logs a message to the C++ engine console.
        /// </summary>
        /// <param name="message">The message string to log.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Log(string message);
    }

    /// <summary>
    /// GameObject creation and management functions.
    /// </summary>
    public static class GameObject
    {
        /// <summary>
        /// Creates a shape-based game object with specified properties.
        /// </summary>
        /// <param name="name">The unique name identifier for the GameObject.</param>
        /// <param name="type">The shape type (use ShapeType enum values).</param>
        /// <param name="x">The initial X position.</param>
        /// <param name="y">The initial Y position.</param>
        /// <param name="width">The width of the shape.</param>
        /// <param name="height">The height of the shape.</param>
        /// <param name="r">Red component of the color (0.0 to 1.0).</param>
        /// <param name="g">Green component of the color (0.0 to 1.0).</param>
        /// <param name="b">Blue component of the color (0.0 to 1.0).</param>
        /// <param name="a">Alpha component of the color (0.0 to 1.0).</param>
        /// <param name="layer">The collision layer (use CollisionLayer enum values).</param>
        /// <param name="hasPhysics">Whether the object should have physics simulation.</param>
        /// <param name="isStatic">Whether the object is static (immovable).</param>
        /// <param name="hasGravity">Whether the object is affected by gravity.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void CreateShape(
            string name,
            int type,
            float x, float y,
            float width, float height,
            float r, float g, float b, float a,
            int layer,
            bool hasPhysics,
            bool isStatic,
            bool hasGravity);

        /// <summary>
        /// Creates an image-based game object with a texture.
        /// </summary>
        /// <param name="name">The unique name identifier for the GameObject.</param>
        /// <param name="type">The shape type for the image (use ShapeType enum values).</param>
        /// <param name="x">The initial X position.</param>
        /// <param name="y">The initial Y position.</param>
        /// <param name="width">The width of the image.</param>
        /// <param name="height">The height of the image.</param>
        /// <param name="texturePath">The file path to the texture image.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void CreateImage(
            string name,
            int type,
            float x, float y,
            float width, float height,
            string texturePath);

        /// <summary>
        /// Creates an animated sprite game object with sprite sheet animation.
        /// </summary>
        /// <param name="name">The unique name identifier for the GameObject.</param>
        /// <param name="x">The initial X position.</param>
        /// <param name="y">The initial Y position.</param>
        /// <param name="width">The width of the image.</param>
        /// <param name="height">The height of the image.</param>
        /// <param name="texturePath">The file path to the sprite sheet texture.</param>
        /// <param name="rows">The number of rows in the sprite sheet.</param>
        /// <param name="columns">The number of columns in the sprite sheet.</param>
        /// <param name="startFrame">The starting frame index for the animation.</param>
        /// <param name="endFrame">The ending frame index for the animation.</param>
        /// <param name="fps">The frames per second for the animation playback.</param>
        /// <param name="layer">The collision layer (use CollisionLayer enum values).</param>
        /// <param name="hasPhysics">Whether the object should have physics simulation.</param>
        /// <param name="isStatic">Whether the object is static (immovable).</param>
        /// <param name="hasGravity">Whether the object is affected by gravity.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void CreateAnimatedImage(
            string name,
            float x, float y,
            float width, float height,
            string texturePath,
            int rows, int columns,
            int startRow, int startCol, int endRow, int endCol,
            float fps,
            int layer,
            bool hasPhysics,
            bool isStatic,
            bool hasGravity);
    }

    /// <summary>
    /// Transform component functions (position, rotation, scale).
    /// </summary>
    public static class Transform
    {
        /// <summary>
        /// Sets the position of a GameObject.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        /// <param name="x">The X coordinate.</param>
        /// <param name="y">The Y coordinate.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetPosition(string objectName, float x, float y);

        /// <summary>
        /// Sets the rotation of a GameObject.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        /// <param name="rotation">The rotation angle in radians.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetRotation(string objectName, float rotation);

        /// <summary>
        /// Adds to the current rotation of a GameObject.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        /// <param name="rotation">The rotation angle to add (in radians).</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void AddRotation(string objectName, float rotation);

        /// <summary>
        /// Sets uniform scale (same X and Y) - internal call.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        /// <param name="scale">The scale value for both X and Y axes.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetScale_Single(string objectName, float scale);

        /// <summary>
        /// Sets non-uniform scale (different X and Y) - internal call.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        /// <param name="x">The scale value for the X axis.</param>
        /// <param name="y">The scale value for the Y axis.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetScale_XY(string objectName, float x, float y);

        /// <summary>
        /// Helper overload for uniform scale (same for X and Y).
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        /// <param name="scale">The scale value for both X and Y axes.</param>
        public static void SetScale(string objectName, float scale)
        {
            SetScale_Single(objectName, scale);
        }

        /// <summary>
        /// Helper overload for non-uniform scale (different X and Y).
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        /// <param name="x">The scale value for the X axis.</param>
        /// <param name="y">The scale value for the Y axis.</param>
        public static void SetScale(string objectName, float x, float y)
        {
            SetScale_XY(objectName, x, y);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void GetPosition_Internal(string objectName, out float x, out float y);

        public static vec2 GetPosition(string objectName)
        {
            GetPosition_Internal(objectName, out float x, out float y);
            return new vec2(x, y);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float GetRotation(string objectName);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void GetScale_Internal(string objectName, out float x, out float y);

        public static vec2 GetScale(string objectName)
        {
            GetScale_Internal(objectName, out float x, out float y);
            return new vec2(x, y);
        }
    }

    /// <summary>
    /// Graphics/rendering functions.
    /// </summary>
    public static class Graphics
    {
        /// <summary>
        /// Sets the shape type for a GameObject's render component.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        /// <param name="shapeType">The shape type (use ShapeType enum values).</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetShapeType(string objectName, int shapeType);

        /// <summary>
        /// Sets the texture for a GameObject.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        /// <param name="filename">The file path to the texture image.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetTexture(string objectName, string filename);

        /// <summary>
        /// Sets the color of a GameObject.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        /// <param name="r">Red component (0.0 to 1.0).</param>
        /// <param name="g">Green component (0.0 to 1.0).</param>
        /// <param name="b">Blue component (0.0 to 1.0).</param>
        /// <param name="a">Alpha component (0.0 to 1.0).</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetColor(string objectName, float r, float g, float b, float a);

        /// <summary>
        /// Sets the alpha (transparency) value for a GameObject.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        /// <param name="alpha">The alpha value (0.0 to 1.0, where 0.0 is fully transparent and 1.0 is fully opaque).</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetAlpha(string objectName, float alpha);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern vec4 GetColor(string objectName);
    }

    /// <summary>
    /// Physics component functions.
    /// </summary>
    public static class Physics
    {
        /// <summary>
        /// Sets the velocity of a GameObject.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        /// <param name="vx">The X component of the velocity.</param>
        /// <param name="vy">The Y component of the velocity.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetVelocity(string objectName, float vx, float vy);

        /// <summary>
        /// Sets the jump velocity value for a GameObject.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        /// <param name="jumpForce">The jump force/velocity value.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetJumpValue(string objectName, float jumpForce);

        /// <summary>
        /// Sets whether a GameObject is affected by gravity.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        /// <param name="enabled">True if the object should be affected by gravity, false otherwise.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetGravity(string objectName, bool enabled);

        /// <summary>
        /// Sets the gravity vector for a GameObject.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        /// <param name="x">The X component of the gravity vector.</param>
        /// <param name="y">The Y component of the gravity vector.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetGravityValue(string objectName, float x, float y);

        /// <summary>
        /// Sets whether a GameObject is static (immovable).
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        /// <param name="isStatic">True if the object should be static, false otherwise.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetStatic(string objectName, bool isStatic);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern vec2 GetVelocity(string objectName);
    }

    /// <summary>
    /// Collision detection functions.
    /// </summary>
    public static class Collision
    {
        /// <summary>
        /// Sets the collision shape type for a GameObject.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        /// <param name="shape">The collision shape type (use ColliderType enum values).</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetCollisionShape(string objectName, int shape);

        /// <summary>
        /// Sets the collision layer for a GameObject.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        /// <param name="layer">The collision layer (use CollisionLayer enum values).</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetCollisionLayer(string objectName, int layer);

        /// <summary>
        /// Casts a ray into the scene and returns information about what it hits.
        /// </summary>
        /// <param name="origin">The starting point of the ray in world coordinates.</param>
        /// <param name="direction">The direction of the ray (should be a normalized vector).</param>
        /// <param name="distance">The maximum distance the ray should travel.</param>
        /// <param name="hitObjectName">Output: The name of the game object that was hit.</param>
        /// <param name="hitDistance">Output: The distance from the origin to the hit point.</param>
        /// <returns>True if the ray hit something, false otherwise.</returns>
        public static bool Raycast(string selfName, vec2 origin, vec2 direction, float distance, out string hitObjectName, out float hitDistance)
        {
            return Raycast_Internal(selfName, origin.x, origin.y, direction.x, direction.y, distance, out hitObjectName, out hitDistance);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Raycast_Internal(string selfName, float x, float y, float dx, float dy, float distance, out string hitObjectName, out float hitDistance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool OnTriggerEnter(string obj1, string obj2);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool OnTriggerExit(string obj1, string obj2);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool OnTriggerStay(string obj1, string obj2);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern int GetCollisionLayer(string objectName);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern int GetCollisionShape(string objectName);
    }

    /// <summary>
    /// Component management functions.
    /// </summary>
    public static class Scripting
    {
        /// <summary>
        /// Attaches a script to a GameObject.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        /// <param name="scriptClassName">The name of the script class to attach.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool AttachScript(string objectName, string scriptClassName);
    }

    public static class Components
    {
        /// <summary>
        /// Sets the active state of a component on a GameObject.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        /// <param name="componentName">The name of the component.</param>
        /// <param name="active">The new active state.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetComponentActive(string objectName, string componentName, bool active);

        /// <summary>
        /// Removes a component from a GameObject.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        /// <param name="componentName">The name of the component to remove.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void RemoveComponent(string objectName, string componentName);

        /// <summary>
        /// Adds a Transform component to a GameObject.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void AddTransformComponent(string objectName);

        /// <summary>
        /// Adds a Render component to a GameObject.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void AddRenderComponent(string objectName);

        /// <summary>
        /// Adds a Collision component to a GameObject.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void AddCollisionComponent(string objectName);

        /// <summary>
        /// Adds a Physics component to a GameObject.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void AddPhysicsComponent(string objectName);
    }


    /// <summary>
    /// Audio system functions.
    /// </summary>
    public static class Audio
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void AddSound(string name, string filepath, bool loop, float volume,float pitch, string category);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void PlaySound(string soundName, string channel);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void StopSound(string soundName);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void StopAllSounds();

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void RemoveSound(string soundName);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void TriggerEvent(string eventName);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void AddEventSound(string eventName, string soundName);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetVolume(string soundName, float volume);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetCategoryVolume(string categoryName, float volume);
    }

    /// <summary>
    /// Camera management functions.
    /// </summary>
    public static class Camera
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetPosition(float x, float y);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetZoom(float zoom);

        /// <summary>
        /// Sets camera mode. Ensure enum matches C++ CameraComponent::CameraMode.
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetMode(int mode);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern vec2 GetPosition();

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float GetZoom();

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern int GetMode();
    }

    public static class Animation
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void PlayAnimation(string objectName, string animationName);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void AddToQueue(string objectName, string animationName);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void RegisterAnimation(string objectName, string animationName, string texturePath, int rows, int cols, int startRow, int startCol, int endRow, int endCol, float fps);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetAnimation(string objectName, int startFrame, int endFrame, float fps);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern string GetCurrentAnimationName(string objectName);
    }

    /// <summary>
    /// Utility functions.
    /// </summary>
    public static class Utils
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern int RandomRange(int min, int max);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float GetDelta_Time();
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float GetFixed_Delta_Time();
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float GetFramesPerSecond();
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float MathfLerp(float a, float b, float t);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float MathfInverseLerp(float a, float b, float value);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float MathfClamp(float value, float min, float max);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern int MathiClamp(int value, int min, int max);
    }

    public static class Input
    {
        /// Key codes mapping to standard GLFW/ASCII constants.
        /// </summary>
        public enum KeyCode
        {
            // Printable keys
            Space = 32,
            Apostrophe = 39,  /* ' */
            Comma = 44,       /* , */
            Minus = 45,       /* - */
            Period = 46,      /* . */
            Slash = 47,       /* / */

            // Alphabet
            A = 65, B = 66, C = 67, D = 68, E = 69, F = 70, G = 71, H = 72,
            I = 73, J = 74, K = 75, L = 76, M = 77, N = 78, O = 79, P = 80,
            Q = 81, R = 82, S = 83, T = 84, U = 85, V = 86, W = 87, X = 88,
            Y = 89, Z = 90,

            // Function Keys & Modifiers
            Escape = 256,
            Enter = 257,
            Right = 262,
            Left = 263,
            Down = 264,
            Up = 265,
            Pause = 284,
            LeftShift = 340,
            LeftControl = 341,
            LeftAlt = 342,
            RightShift = 344,
            RightControl = 345,
            RightAlt = 346,
            Menu = 348
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool IsKeyPressed(int key);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool IsKeyReleased(int key);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool IsKeyHeld(int key);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool IsMouseButtonPressed(int button);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool IsMouseButtonReleased(int button);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool IsMouseButtonHeld(int button);


        /// <summary>
        /// Gets the mouse X position in screen coordinates.
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float GetMouseX();

        /// <summary>
        /// Gets the mouse Y position in screen coordinates.
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float GetMouseY();

        /// <summary>
        /// Gets the mouse X position in world coordinates (accounting for camera).
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float GetMouseWorldX();

        /// <summary>
        /// Gets the mouse Y position in world coordinates (accounting for camera).
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float GetMouseWorldY();


        /// <summary>
        /// Checks if a specific key was pressed in the current frame.
        /// </summary>
        /// <param name="key">The KeyCode to check.</param>
        /// <returns>True if the key was pressed in the current frame.</returns>
        public static bool GetKeyPressed(KeyCode key)
        {
            return IsKeyPressed((int)key);
        }

        /// <summary>
        /// Checks if a specific key was released in the current frame.
        /// </summary>
        /// <param name="key">The KeyCode to check.</param>
        /// <returns>True if the key was released in the current frame.</returns>
        public static bool GetKeyReleased(KeyCode key)
        {
            return IsKeyReleased((int)key);
        }

        /// <summary>
        /// Checks if a specific key is currently held down.
        /// </summary>
        /// <param name="key">The KeyCode to check.</param>
        /// <returns>True if the key is pressed.</returns>
        public static bool GetKeyHeld(KeyCode key)
        {
            return IsKeyHeld((int)key);
        }

        /// <summary>
        /// Checks if a mouse button was pressed in the current frame.
        /// 0 = Left, 1 = Right, 2 = Middle.
        /// </summary>
        public static bool GetMouseButtonPressed(int button)
        {
            return IsMouseButtonPressed(button);
        }

        /// <summary>
        /// Checks if a mouse button was released in the current frame.
        /// 0 = Left, 1 = Right, 2 = Middle.
        /// </summary>
        public static bool GetMouseButtonReleased(int button)
        {
            return IsMouseButtonReleased(button);
        }

        /// <summary>
        /// Checks if a mouse button is held down.
        /// 0 = Left, 1 = Right, 2 = Middle.
        /// </summary>
        public static bool GetMouseButtonHeld(int button)
        {
            return IsMouseButtonHeld(button);
        }
    }


}