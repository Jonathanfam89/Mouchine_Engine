using CSharpScript;

namespace GameScripts
{
    public static class InputManager
    {
        public static vec2 Movement => Input.Movement();
        public static bool DashWasPressed => Input.GetKeyPressed(Input.KeyCode.J);
        public static bool WallKickWasPressed => Input.GetKeyPressed(Input.KeyCode.L);
        public static bool GroundPoundWasPressed => Input.GetKeyPressed(Input.KeyCode.S);
        public static bool RollWasPressed => Input.GetKeyPressed(Input.KeyCode.K);
        public static bool JumpWasPressed => Input.GetKeyPressed(Input.KeyCode.Space);
        public static bool JumpWasReleased => Input.GetKeyReleased(Input.KeyCode.Space);
        public static bool ZiplineWasPressed => Input.GetKeyPressed(Input.KeyCode.W);
        public static bool ZiplineIsHeld => Input.GetKeyHeld(Input.KeyCode.W);
        public static bool GlideIsHeld => Input.GetKeyHeld(Input.KeyCode.W);
    }
}
