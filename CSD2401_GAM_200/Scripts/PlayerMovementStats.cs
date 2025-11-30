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
using static System.Math;

// This file contains values that will be able to be adjusted via the editor.
namespace GameScripts
{
    public class PlayerMovementStats
    {
        // Walk
        public float MaxWalkSpeed = 20f;
        public float GroundAcceleration = 7.5f;
        public float GroundDeceleration = 30f;
        public float AirAcceleration = 20f;
        public float AirDeceleration = 5f;

        // Grounded/Collision Checks
        public float GroundDetectionRayLength = 0.02f;
        public float HeadDetectionRayLength = 0.02f;
        public float HeadWidth = 0.75f;

        // Wall Checks
        public float WallDetectionBoxLength = 0.05f;
        public float WallSlidingSpeed = 2f;

        // Jump
        public bool EnableVariableJumpHeight = false;
        public float JumpHeight = 5.7f;
        public float JumpHeightCompensationFactor = 1.054f;
        public float TimeTillJumpApex = 0.35f;
        public float GravityOnReleaseMultiplier = 3f;
        public float MaxFallSpeed = 40f;
        public int NumberOfJumpsAllowed = 1;

        // Jump Cut
        public float TimeForUpwardsCancel = 0.02f;

        // Jump Apex
        public float ApexThreshold = 0.97f;
        public float ApexHangTime = 0.04f;

        // Jump Buffer
        public float JumpBufferTime = 0.1f;

        // Jump Coyote Time
        public float JumpCoyoteTime = 0.1f;

        // Dash
        public float DashSpeed = 35f;
        public float DashDuration = 0.22f;
        public float DashCooldown = 0.5f;

        // Roll
        public float BaseRollDuration = 0.3f;
        public float RollSpeed = 30f;
        public float RollHeightToDurationDivisor = 30f;

        // Wall Kick
        public float WallKickSpeedModifier = 30f;
        public float WallKickVerticalModifier = 5f;
        public float WallKickDurationMultiplier = 0.75f;

        // Wall Jump
        public vec2 WallJumpForce = new vec2(20f, 25f);
        public float WallCoyoteTime = 0.1f;

        // Ground Pound
        public float GroundPoundPauseTime = 0.3f;
        public float GroundPoundFallSpeed = 20f;

        // Air Glide
        public float GlideFallSpeedMultiplier = 0.6f;
        public float GlideCooldown = 0.2f;

        // Zipline
        public float ZiplineSpeed = 30f;
        public float ZiplineAttachDuration = 0.15f;

        // Zipline Launch
        public float ZiplineLaunchDuration = 0.25f;
        public vec2 ZiplineLaunchForce = new vec2(40f, 16f);
        public float ZiplineCoyoteTime = 0.2f;
        
        // Time Deduction Penalties
        public float JumpTimeDeduction = 1.0f;
        public float DashTimeDeduction = 1.0f;
        public float ZiplineTimeDeduction = 3.0f;
        public float ZiplineDeductionInterval = 0.2f;
        public float GroundPoundTimeDeduction = 3.0f;
        public float RollTimeDeduction = 2.0f;
        public float GlideTimeDeduction = 1.0f;
        public float GlideDeductionInterval = 0.5f;
        public float WallKickTimeDeduction = 2.5f;

        // Jump Visualization Tool
        public bool ShowWalkJumpArc = false;
        public bool StopOnCollision = true;
        public bool DrawRight = true;
        public int ArcResolution = 20;
        public int VisualizationSteps = 90;

        // Debug
        public bool DebugShowIsGroundedBox;
        public bool DebugShowHeadBumpBox;
        
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