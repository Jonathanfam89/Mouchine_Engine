/* Start Header ************************************************************************/
/*!
\file       PlayerMovement.cs
\author     Ashfer [100% - ????????????????????????????lines]
\par        m.binafzalkhan@digipen.edu
\date       Novenber, 26, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

using CSharpScript;
using System;
using System.Numerics;
using static System.Math;

// This file contains values that will be able to be adjusted via the editor.
namespace CSharpScript
{
    public class PlayerMovementStats : GameScript
    {
        private static float mod = 12.5f;
        // CLAMPS - Max values 
        public float Clamp_MaxRunSpeed = 50f * mod;
        public float Clamp_MaxJumpHeight = 80f * mod;

        // walking tiers
        public float T0_BaseSpeed = 1f * mod;       //2
        public float T0_MaxSpeed = 12f * mod;       //12

        // running tiers
        public float T1_Threshold = 18f * mod;       //18
        public float T2_Threshold = 24f * mod;       //24
        public float T3_Threshold = 30f * mod;       //30
        public float T3_MaxSpeed = 32f * mod;        //40

        // Running
        public bool UseToggleRun = false;
        public float RunBaseSpeed = 15f * mod;
        public float RunCoastDeceleration = 15f * mod;

        // Accel/ Decel
        public float GroundAcceleration = 20f * mod;
        public float GroundDeceleration = 50f * mod;
        //[Range(0.25f, 50f)] 
        public float AirAcceleration = 20f * mod;
        //[Range(0.25f, 50f)] 
        public float AirDeceleration = 5f * mod;
        public float TurnResetSpeed = 5f * mod;
        public float TentAcceleration = 10f * mod;

        // Turn Skid
        //[Range(0.1f, 1f)] 
        public float SkidSpeedRetained = 0.25f;
        public float T1_SkidDuration = 0.5f;
        public float T2_SkidDuration = 0.8f;
        public float T3_SkidDuration = 1.0f;

        // Wall Bonk
        public float T3_BonkDuration = 0.8f;
        public Vector2 T3_BonkForce = new Vector2(12f * mod, 10f * mod);

        // Grounded/Collision Checks
        public float GroundDetectionRayLength = 10.0f;
        public float HeadDetectionRayLength = 0.02f;
        public float HeadWidth = 0.75f;

        // Wall Checks
        public float WallDetectionBoxLength = 50.0f;            // unity is 0.05 - 0.02 (since our raycast is v diff)

        // Jump
        public bool EnableVariableJumpHeight = false;
        public float JumpHeight = 100f;         // unity is 5.7 * mod???                        // unity is 5.7f * mod
        public float T1_JumpMultiplier = 1.05f;
        public float T2_JumpMultiplier = 1.08f;
        public float T3_JumpMultiplier = 1.15f;
        public float JumpHeightCompensationFactor = 1.054f;
        public float TimeTillJumpApex = 0.35f;
        public float GravityOnReleaseMultiplier = 3.0f;         // was 1.4
        public float MaxFallSpeed = 40f * mod;
        public int NumberOfJumpsAllowed = 1;

        // Jump Cut
        public float TimeForUpwardsCancel = 0.02f;

        // land timer
        public float LandAnimationLength = 0.2f;

        // Jump Apex
        public float ApexThreshold = 0.9f;                      // was 0.97
        public float ApexHangTime = 0.04f;

        // Jump Buffer
        public float JumpBufferTime = 0.1f;

        // Jump Coyote Time
        public float JumpCoyoteTime = 0.1f;

        // Wall Jump
        public vec2 WallJumpForce = new vec2(20f * mod, 25f * mod);
        public float WallCoyoteTime = 0.1f;

        // Wall Kick
        public float WallSlidingSpeed = 4f * mod;                     // was 2
        public float WKBaseDuration = 0.25f;
        public float WKCooldown = 0.5f;
        public float WKSpeedModifier = 30f * mod;
        public float WKVerticalModifier = 5f * mod;
        public float WKDurationMultiplier = 0.75f;
        // dont have tht WKEase graph thing in unity

        // Roll
        public float BaseRollDuration = 0.5f;       // minimum time
        public float MidRollDuration = 0.65f;
        public float MaxRollDuration = 0.8f;
        public float BaseRollSpeed = 15f * mod;     // needs to be faster than walk speed to feel like a burst forward
        public float MidRollSpeed = 30f * mod;
        public float MaxRollSpeed = 40f * mod;


        // Ground Pound
        public float GPPauseTime = 0.3f;
        public float GPFallSpeed = 50f * mod;
        public float RollHeightToDurationDivisor = 100f * mod;
        public float GroundPoundMaxSpeedHeightThreshold = 50f;

        // Air Glide
        public float GlideFallSpeedMultiplier = 0.5f;
        public float GlideCooldown = 0.2f;

        // Zipline
        public float ZiplineSpeed = 30f * mod;
        public float ZiplineAttachDuration = 0.15f;

        // Zipline Launch
        public float ZiplineLaunchDuration = 0.25f;
        public vec2 ZiplineLaunchForce = new vec2(40f * mod, 16f * mod);
        public float ZiplineCoyoteTime = 0.2f;
        
        // Time Deduction Penalties
        public float JumpTimeDeduction = 1.0f;
        public float RunTimeDeduction = 0.1f;
        public float ZiplineTimeDeduction = 3.0f;
        public float ZiplineDeductionInterval = 0.2f;
        public float GroundPoundTimeDeduction = 3.0f;
        public float RollTimeDeduction = 2.0f;
        public float GlideTimeDeduction = 1.0f;
        public float GlideDeductionInterval = 0.5f;
        public float WallKickTimeDeduction = 2.5f;

        // Calculated values
        public float Gravity { get; private set; }
        public float InitialJumpVelocity { get; private set; }
        public float AdjustedJumpHeight { get; private set; }
        public float KnockbackInitialVerticalVelocity { get; private set; }

        public PlayerMovementStats()
        {
            CalculateValues();
        }

        private void CalculateValues()
        {
            AdjustedJumpHeight = JumpHeight * JumpHeightCompensationFactor;
            Gravity = -(2f * AdjustedJumpHeight) / (float)(Pow(TimeTillJumpApex, 2f));
            InitialJumpVelocity = Abs(Gravity) * TimeTillJumpApex;
        }
    }
}