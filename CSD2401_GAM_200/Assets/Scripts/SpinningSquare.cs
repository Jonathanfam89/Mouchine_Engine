using System;
using CSharpScript;

namespace CSharpScript
{
    public class SpinningSquare : GameScript
    {
        public float RotationSpeed = 2.0f;
        public bool ShouldRotate = true;
        public string Message = "Hello, Editor!";
        public int LoopCount = 5;
        public float MoveSpeed = 50.0f;
        public bool TestPhysics = true;

        private bool physicsSetup = false;

        public override void OnStart()
        {
            Debug.Log($"SpinningSquare started on {OwnerName}");

            if (TestPhysics)
            {
                // Add physics component
                Components.AddPhysicsComponent(OwnerName);

                // Add collider component
                Components.AddCollisionComponent(OwnerName);

                // Set physics properties
                Physics.SetMass(OwnerName, 1.0f);
                Physics.SetFriction(OwnerName, 0.5f);
                Physics.SetGravity(OwnerName, false);
                Physics.SetStatic(OwnerName, false);

                // Set initial velocity to make it move
                Physics.SetVelocity(OwnerName, MoveSpeed, 0.0f);

                physicsSetup = true;
            }
        }

        public override void OnUpdate(float dt)
        {
            if (ShouldRotate)
            {
                Transform.AddRotation(OwnerName, RotationSpeed * dt);
                Graphics.SetColor(OwnerName, 0.0f, 0.0f, 0.0f, 1.0f);
            }
        }

        public override void OnDestroy()
        {
            Debug.Log($"SpinningSquare destroyed on {OwnerName}");
        }
    }
}