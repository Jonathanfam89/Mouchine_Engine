using System;
using System.Runtime.CompilerServices;

namespace CSharpScript // Or your namespace
{
    public class GameScript
    {
        // 1. The C++ Engine will set this variable automatically!
        // We use string ID for now because your C++ uses string maps
        public string OwnerName;

        // 2. Helper to make it feel like Unity
        // Instead of Transform.SetPosition("Name", x, y), we can just do transform.Position
        public TransformWrapper transform
        {
            get { return new TransformWrapper(OwnerName); }
        }

        public class TransformWrapper
        {
            private string id;
            public TransformWrapper(string name) { id = name; }

            public vec2 position
            {
                get { return Transform.GetPosition(id); }
                set { Transform.SetPosition(id, value.x, value.y); }
            }

            // Add Rotation/Scale helpers here...
        }

        // Virtual methods that user scripts override
        public virtual void OnStart() { }
        public virtual void OnUpdate(float dt) { }
        public virtual void OnDestroy() { }
    }
}