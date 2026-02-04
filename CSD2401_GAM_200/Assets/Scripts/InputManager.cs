using CSharpScript;

namespace CSharpScript
{
    public static class InputManager
    {
        public static vec2 Movement => Input.Movement();
        public static bool JumpWasPressed => Input.GetKeyPressed(Input.KeyCode.Space);
        public static bool JumpWasReleased => Input.GetKeyReleased(Input.KeyCode.Space);
        public static bool RunWasPressed => Input.GetKeyPressed(Input.KeyCode.J);
        public static bool RunIsHeld => Input.GetKeyHeld(Input.KeyCode.J);
        public static bool WallKickWasPressed => Input.GetKeyPressed(Input.KeyCode.L);
        public static bool GroundPoundWasPressed => Input.GetKeyPressed(Input.KeyCode.K);
        public static bool RollWasPressed => Input.GetKeyPressed(Input.KeyCode.K);
        public static bool GlideIsHeld => Input.GetKeyHeld(Input.KeyCode.W) || Input.GetKeyHeld(Input.KeyCode.Space);
        public static bool ZiplineWasPressed => Input.GetKeyPressed(Input.KeyCode.L);
        public static bool ZiplineIsHeld => Input.GetKeyHeld(Input.KeyCode.L);
        //public static bool DashWasPressed => Input.GetKeyPressed(Input.KeyCode.J);
    }
}
