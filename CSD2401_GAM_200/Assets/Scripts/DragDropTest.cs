using System;
using CSharpScript;

namespace CSharpScript
{
    public class DragDropTest : GameScript
    {
        public override void OnStart()
        {
            
        }

        public override void OnUpdate(float dt)
        {
            // You can add some logic here to use these variables
            // For example, if PublicBool is true, log the PublicFloat

        }
        public override void OnDestroy()
        {
            Debug.Log($"DragDropTest destroyed on {OwnerName}");
        }
    }
}