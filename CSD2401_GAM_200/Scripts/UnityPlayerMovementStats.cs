
///* Start Header ************************************************************************/
///*!
//\file       PlayerMovement.cs
//\author     Ashfer [100% - ????????????????????????????lines]
//\par        m.binafzalkhan@digipen.edu
//\date       Novenber, 26, 2025

//Copyright (C) 2025 DigiPen Institute of Technology.
//Reproduction or disclosure of this file or its contents
//without the prior written consent of DigiPen Institute of
//Technology is prohibited.
//*/
///* End Header **************************************************************************/

//using CSharpScript;
//using System;
//using System.Numerics;
//using System.Threading;
//using static System.Math;

//// This file contains values that will be able to be adjusted via the editor.
//namespace GameScripts
//{
//    public class PlayerMovementStats
//    {
//        //[Header("Walk")]
//        //[Tooltip("Maximum speed player can reach while moving")]
//        //[Range(1f, 100f)] 
//        public float MaxWalkSpeed = 20f;
//        //[Tooltip("How quickly player reaches max speed on the ground")]
//        //[Range(0.25f, 50f)] 
//        public float GroundAcceleration = 7.5f;
//        //[Tooltip("How quickly player stops when no input is given on the ground")]
//        //[Range(0.25f, 50f)] 
//        public float GroundDeceleration = 30f;
//        //[Tooltip("How quickly player can change direction in the air")]
//        //[Range(0.25f, 50f)] 
//        public float AirAcceleration = 20f;
//        //[Tooltip("How quickly horizontal movement slows in the air with no input")]
//        //[Range(0.25f, 50f)] 
//        public float AirDeceleration = 5f;

//        //[Header("Grounded/Collision Checks")]
//        //[Tooltip("Which layers are considered 'ground' for collision checks")]
//        //public LayerMask GroundLayer;
//        //[Tooltip("Length of the raycast sent downwards from feet to detect the ground")]
//        public float GroundDetectionRayLength = 0.02f;
//        //[Tooltip("Length of the raycast sent upwards from head to detect ceilings")]
//        public float HeadDetectionRayLength = 0.02f;
//        //[Tooltip("Adjusts the width of the boxcast used for head collision checks")]
//        //[Range(0f, 1f)] 
//        public float HeadWidth = 0.75f;

//        //[Header("Wall Checks")]
//        //[Tooltip("Which layers are considered 'walls' for collision checks")]
//        //public LayerMask WallLayer;
//        //[Tooltip("Length of the boxcast sent sideways to detect walls")]
//        public float WallDetectionBoxLength = 0.05f;
//        //[Tooltip("Speed player slides down a wall")]
//        public float WallSlidingSpeed = 2f;

//        //[Header("Jump")]
//        //[Tooltip("Turn ON or OFF Hold to Jump")]
//        public bool EnableVariableJumpHeight = false;
//        //[Tooltip("Height in Unity units for a full jump")]
//        public float JumpHeight = 5.7f;
//        //[Tooltip("Multiplier to fine-tune jump physics without changing other gravity/time values")]
//        //[Range(1f, 1.1f)] 
//        public float JumpHeightCompensationFactor = 1.054f;
//        //[Tooltip("Time in seconds it takes to reach the peak of a jump")]
//        public float TimeTillJumpApex = 0.35f;
//        //[Tooltip("Multiplies gravity when jump button is released early - causing a 'jump cut' and a shorter hop")]
//        //[Range(0.01f, 5f)] 
//        public float GravityOnReleaseMultiplier = 3f;
//        //[Tooltip("Maximum speed player can fall at (terminal velocity)")]
//        public float MaxFallSpeed = 40f;
//        //[Tooltip("How many times player can jump before touching the ground (2 = Double Jump - 1 = Single Jump)")]
//        //[Range(1, 5)] 
//        public int NumberOfJumpsAllowed = 1;

//        //[Header("Jump Cut")]
//        //[Tooltip("Controls how quickly upward momentum is cancelled when jump button is released")]
//        //[Range(0.02f, 0.3f)] 
//        public float TimeForUpwardsCancel = 0.02f;

//        //[Header("Jump Apex")]
//        //[Tooltip("Point in the jump's ascent (0-1) where apex 'hang time' can begin")]
//        //[Range(0.5f, 1f)] 
//        public float ApexThreshold = 0.97f;
//        //[Tooltip("How long player 'hangs' at the top of jump - allows for air control")]
//        //[Range(0.01f, 1f)] 
//        public float ApexHangTime = 0.04f;

//        //[Header("Jump Buffer")]
//        //[Tooltip("Allows a jump input to be registered shortly *before* landing")]
//        //[Range(0f, 1f)] 
//        public float JumpBufferTime = 0.1f;

//        //[Header("Jump Coyote Time")]
//        //[Tooltip("Allows player to jump for a short time *after* walking off a ledge")]
//        //[Range(0f, 1f)] 
//        public float JumpCoyoteTime = 0.1f;

//        //[Header("Dash")]
//        //[Tooltip("Speed during a dash")]
//        public float DashSpeed = 35f;
//        //[Tooltip("How long the dash lasts in seconds")]
//        public float DashDuration = 0.22f;
//        //[Tooltip("Time in seconds player must wait before dashing again")]
//        public float DashCooldown = 0.5f;

//        //Header("Roll")]
//        //[Tooltip("Base duration of roll in seconds")]
//        public float BaseRollDuration = 0.3f;
//        //[Tooltip("Speed of player during the roll")]
//        public float RollSpeed = 30f;
//        //[Tooltip("How much the roll duration is extended based on the height of the ground pound - Higher value = less extension")]
//        public float RollHeightToDurationDivisor = 30f;

//        //[Header("Wall Kick")]
//        //[Tooltip("Multiplier for how fast the player kicks off the wall")]
//        public float WallKickSpeedModifier = 30f;
//        //[Tooltip("Multiplier for how long the wall kick lasts")]
//        public float WallKickVerticalModifier = 5f;
//        //[Tooltip("Controls how much height is gained per wallkick")]
//        public float WallKickDurationMultiplier = 0.75f;
//        //[Tooltip("Controls the speed over the duration of the kick. Time (X-axis) goes from 0 to 1. Value (Y-axis) is a speed multiplier.")]
//        //public AnimationCurve WallKickEaseCurve;

//        //[Header("Wall Jump")]
//        //[Tooltip("Force applied when wall jumping. X = away from wall, Y = up.")]
//        public vec2 WallJumpForce = new vec2(20f, 25f);
//        //[Tooltip("Time after leaving a wall that you can still wall jump (Coyote Time).")]
//        public float WallCoyoteTime = 0.1f;

//        //[Header("Ground Pound")]
//        //[Tooltip("How long player pauses in the air before pounding downwards")]
//        public float GroundPoundPauseTime = 0.3f;
//        //[Tooltip("The downward speed of the ground pound")]
//        public float GroundPoundFallSpeed = 20f;

//        //[Header("Air Glide")]
//        //[Tooltip("How much to reduce player's fall speed by - 0.5 = 50% speed")]
//        //[Range(0.1f, 1f)] 
//        public float GlideFallSpeedMultiplier = 0.6f;
//        //[Tooltip("Time in seconds player must wait before gliding again")]
//        public float GlideCooldown = 0.2f;

//        //[Header("Zipline")]
//        //[Tooltip("Speed at which player travels along the zipline")]
//        public float ZiplineSpeed = 30f;
//        //[Tooltip("How long the smooth transition onto the zipline should take in seconds.")]
//        public float ZiplineAttachDuration = 0.15f;

//        //[Header("Zipline Launch")]
//        //[Tooltip("How long the launch boost lasts after leaving the zipline.")]
//        public float ZiplineLaunchDuration = 0.25f;
//        //[Tooltip("The initial force applied when launching. X is forward, Y is upward.")]
//        public vec2 ZiplineLaunchForce = new vec2(40f, 16f);
//        //[Tooltip("Controls the launch force over its duration. A curve that starts at 1 and ends at 0 is recommended.")]
//        //public AnimationCurve ZiplineLaunchCurve;
//        //[Tooltip("How long player can still activate a zipline after leaving its trigger (coyote time)")]
//        public float ZiplineCoyoteTime = 0.2f;

//        //[Header("Time Deduction Penalties")]
//        //[Tooltip("Time deducted for performing a jump")]
//        public float JumpTimeDeduction = 1.0f;
//        //[Tooltip("Time deducted for performing a dash")]
//        public float DashTimeDeduction = 1.0f;
//        //[Tooltip("Time deducted periodically while on a zipline")]
//        public float ZiplineTimeDeduction = 3.0f;
//        //[Tooltip("Interval in seconds at which zipline time deduction is applied")]
//        public float ZiplineDeductionInterval = 0.2f;
//        //[Tooltip("Time deducted for ground pound")]
//        public float GroundPoundTimeDeduction = 3.0f;
//        //[Tooltip("Time deducted for roll")]
//        public float RollTimeDeduction = 2.0f;
//        //[Tooltip("Time deducted periodically while gliding")]
//        public float GlideTimeDeduction = 1.0f;
//        //[Tooltip("Interval in seconds at which glide time deduction is applied")]
//        public float GlideDeductionInterval = 0.5f;
//        //[Tooltip("Time deducted for a wall kick")]
//        public float WallKickTimeDeduction = 2.5f;

//        //[Header("Jump Visualization Tool")]
//        public bool ShowWalkJumpArc = false;
//        public bool StopOnCollision = true;
//        public bool DrawRight = true;
//        //[Range(5, 100)]
//        public int ArcResolution = 20;
//        //[Range(0, 100)]
//        public int VisualizationSteps = 90;

//        //[Header("Debug")]
//        public bool DebugShowIsGroundedBox;
//        public bool DebugShowHeadBumpBox;


//        public float Gravity { get; private set; }
//        public float InitialJumpVelocity { get; private set; }

//        public float AdjustedJumpHeight { get; private set; }

//        public float KnockbackInitialVerticalVelocity { get; private set; }

//        private void OnValidate()
//        {
//            CalculateValues();
//        }
//        private void OnEnable()
//        {
//            CalculateValues();
//        }

//        private void CalculateValues()
//        {
//            AdjustedJumpHeight = JumpHeight * JumpHeightCompensationFactor;
//            Gravity = -(2f * AdjustedJumpHeight) / (float)(Pow(TimeTillJumpApex, 2f));
//            InitialJumpVelocity = Abs(Gravity) * TimeTillJumpApex;
//        }
//    }
//}