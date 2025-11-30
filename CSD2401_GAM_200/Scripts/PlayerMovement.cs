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

namespace GameScripts
{
    public class PlayerMovement
    {
        // basically assigns names to numbers that can be used to track states
        enum PlayerState
        {
            Idle = 0,
            Running,
            Jumping,
            Dashing,
            GroundPounding,
            Rolling,
            WallKicking,
            Ziplining,
            Gliding,
            WallSliding,
            WallJumping
        };

        private PlayerState currState = PlayerState.Idle;

        public PlayerMovementStats MoveStats;

        #region Variables
        // This property should be set by the C++ engine after the script is attached.
        public string gameObjectName = "Hero";

        // movement variables
        private vec2 _moveVelocity;
        private bool _isFacingRight;

        // public facing right for other scripts
        public bool IsFacingRight => _isFacingRight;

        // collision check variables
        private bool _isGrounded;
        private bool _bumpedHead;

        // jump variables
        public float VerticalVelocity { get; private set; }
        private bool _isJumping;
        private bool _isFastFalling;
        private bool _isFalling;
        private float _fastFallTime;
        private float _fastFallReleaseSpeed;
        private int _numberOfJumpsUsed;

        // wall Jump variables
        private bool _isWallJumping;
        private float _wallJumpTimer;
        private float _wallJumpDirection;

        // apex variables
        private float _apexPoint;
        private float _timePastApexThreshold;
        private bool _isPastApexThreshold;

        // jump buffer variables
        private float _jumpBufferTimer;
        private bool _jumpReleasedDuringBuffer;

        // coyote time variables
        private float _coyoteTimer;

        // dash variables
        private bool _isDashing;
        private float _dashTimer;
        private float _dashCooldownTimer;

        // wall kick variables
        private bool _isWallKicking = false;
        private int wallKickFrameCounter;           // [WIP] for cayote time 
        private bool canWallKick;                   // Checks if touching a wall, and can perform a wall kick
        private bool blockMovement = false;         // Prevents player from controlling character
        private float _wallKickTimer;
        private float _wallKickCooldownTimer;
        private bool _startedWallKickInAir = false;
        private float _wallKickTotalDuration;

        // wall slide variables
        private bool _isTouchingWall;
        private bool _isWallSliding;

        // ground pound variables
        private bool _isGroundPounding = false;     // checks if player is currently in groundpound state
        vec2 lastPos;                            // stores pos of when the player initiated groundpound (for conversion to horizontal dist)
        private float _groundPoundPauseTimer;

        // roll variables
        private bool _isRolling = false;            // checks if player is in rolling state
        private float _rollTimer;

        // glide variables
        private bool _isGliding = false;            // checks if player is in gliding state
        private float _glideCooldownTimer;
        private float _timeSinceLastGlideDeduction;  // timer for periodic deduction

        // zipline variables
        private bool _isZiplining = false;
        private bool _isBeingLaunched = false;
        private float _ziplineCoyoteTimer;
        private float _ziplineHeadOffset;

        // knockback
        // private KnockBack knockback; //
        #endregion


        public PlayerMovement()
        {
            // When the script is created, instantiate its stats
            MoveStats = new PlayerMovementStats();
            Debug.Log("PlayerMovement instantiated with new stats.");
        }

        public void OnStart()
        {
            // It's crucial that gameObjectName is set by the C++ engine before OnStart is called.
            if (gameObjectName == null)
            {
                Debug.Log("ERROR: gameObjectName is not set in PlayerMovement script!");
                return;
            }

            _isFacingRight = true;

            Debug.Log("PlayerMovement Script OnStart for object: " + gameObjectName);
            Physics.SetGravity(gameObjectName, true);

            vec2 scale = Transform.GetScale(gameObjectName);
            vec2 position = Transform.GetPosition(gameObjectName);
            //_ziplineHeadOffset = position.y + (scale.y / 2);
        }

        public void OnUpdate(float deltaTime)
        {
            if (gameObjectName == null) return;

            UpdatePlayerState();

            // Handle all timers
            _jumpBufferTimer -= deltaTime;
            _coyoteTimer -= deltaTime;
            _dashCooldownTimer -= deltaTime;
            _wallKickCooldownTimer -= deltaTime;
            _ziplineCoyoteTimer -= deltaTime;
            
            JumpChecks();
            // Handle Inputs
            HandleGeneralInput();
            //OnFixedUpdate(deltaTime);
        }

        public void OnFixedUpdate(float fixedDeltaTime)
        {
            Debug.Log("aaaa");
            if (gameObjectName == null) return;
            CollisionChecks();
                Debug.Log("jump done");

            if (!_isZiplining) //&& !knockback.IsBeingKnockedBack && !_isBeingLaunched)
            {
                Jump(fixedDeltaTime);
                Dash(fixedDeltaTime);
                WallKick(fixedDeltaTime);
                GroundPound(fixedDeltaTime);
                Roll(fixedDeltaTime);
                Glide(fixedDeltaTime);
                WallSlide(fixedDeltaTime);

                // only allow regular movement if not dashing
                if (!_isDashing && !_isWallKicking && !_isWallJumping && !_isGroundPounding)
                {
                    //Debug.Log(Input.Movement().x.ToString() + " " + Input.Movement().y.ToString());
                    if (_isGrounded)
                    {
                        Move(MoveStats.GroundAcceleration, MoveStats.GroundDeceleration, InputManager.Movement, fixedDeltaTime);
                    }
                    else
                    {
                        Move(MoveStats.AirAcceleration, MoveStats.AirDeceleration, InputManager.Movement, fixedDeltaTime);
                    }
                }
                else Debug.Log("aaaa");
            }
        }

        #region State & Input
        private void UpdatePlayerState()
        {
            // check if in air
            if (!_isGrounded)
            {
                if (_isWallJumping)
                {
                    currState = PlayerState.WallJumping;
                }
                else
                {
                    currState = PlayerState.Jumping;
                }
            }
            else
            {
                // horizontal input = moving - trigger run animation
                if (InputManager.Movement.x != 0)
                {
                    currState = PlayerState.Running;
                }
                else
                {
                    currState = PlayerState.Idle;
                }
            }

            // placed after jump and run, so it can be overriden 
            if (_isDashing) currState = PlayerState.Dashing;
            if (_isRolling) currState = PlayerState.Rolling;
            if (_isGroundPounding) currState = PlayerState.GroundPounding;
            if (_isGliding) currState = PlayerState.Gliding;
            if (_isWallKicking) currState = PlayerState.WallKicking;
            if (_isZiplining) currState = PlayerState.Ziplining;
            if (_isWallSliding) currState = PlayerState.WallSliding;
        }

        private void HandleGeneralInput()
        {
            // check for dash input
            if (InputManager.DashWasPressed && _dashCooldownTimer <= 0)
            {
                InitiateDash();
            }
            if (InputManager.WallKickWasPressed && _wallKickCooldownTimer <= 0 && canWallKick)
            {
                InitiateWallKick();
            }
            if (InputManager.GroundPoundWasPressed && !_isGrounded)
            {
                InitiateGroundPound();
            }
            if (InputManager.RollWasPressed)
            {
                InitiateRoll();
            }
            if (InputManager.GlideIsHeld && !_isGrounded)
            {
                InitiateGlide();
            }
            else if (_isGliding) // if key was let go but still in gliding state, cancel gliding action
            {
                StopGlide();
            }
        }

        #endregion

        #region Collison Checks

        private void CollisionChecks()
        {
            IsGrounded();
            BumpedHead();
            IsTouchingWall();

            if (_isTouchingWall && !_isGrounded)
            {
                // while on wall keep timer running
                _wallJumpTimer = MoveStats.WallCoyoteTime;

                // determine which way is away from the wall
                _wallJumpDirection = _isFacingRight ? -1f : 1f;
            }
            else
            {
                // if not on wall - start counting down for coyote time
                _wallJumpTimer -= Utils.GetDelta_Time(); // Assumes GetDeltaTime is accessible
            }

            canWallKick = _isTouchingWall;
        }

        private void IsGrounded()
        {
            _isGrounded = false;
            vec2 position = Transform.GetPosition(gameObjectName);
            vec2 scale = Transform.GetScale(gameObjectName);
            float halfWidth = Math.Abs(scale.x) / 2;
            float raycastLength = (scale.y / 2) + MoveStats.GroundDetectionRayLength;
            vec2 rayDirection = new vec2(0, -1);

            // Center raycast
            vec2 centerRayOrigin = new vec2(position.x, position.y - (scale.y / 2));
            if (RaycastForGround(centerRayOrigin, rayDirection, raycastLength))
            {
                _isGrounded = true;
                return;
            }

            // Left raycast
            vec2 leftRayOrigin = new vec2(position.x - halfWidth * 0.9f, position.y - (scale.y / 2));
            if (RaycastForGround(leftRayOrigin, rayDirection, raycastLength))
            {
                _isGrounded = true;
                return;
            }

            // Right raycast
            vec2 rightRayOrigin = new vec2(position.x + halfWidth * 0.9f, position.y - (scale.y / 2));
            if (RaycastForGround(rightRayOrigin, rayDirection, raycastLength))
            {
                _isGrounded = true;
                return;
            }
        }

        private bool RaycastForGround(vec2 origin, vec2 direction, float length)
        {
            if (Collision.Raycast(gameObjectName, origin, direction, length, out string hitObject, out float hitDistance))
            {
                if (Collision.GetCollisionLayer(hitObject) == (int)CollisionLayer.Ground)
                {
                    return true;
                }
            }
            return false;
        }

        private void BumpedHead()
        {
            // This needs to be reimplemented with multiple raycasts
            _bumpedHead = false;
        }

        private void IsTouchingWall()
        {
            _isTouchingWall = false;
            vec2 position = Transform.GetPosition(gameObjectName);
            vec2 scale = Transform.GetScale(gameObjectName);
            float halfHeight = scale.y / 2;
            float raycastLength = (Math.Abs(scale.x) / 2) + MoveStats.WallDetectionBoxLength;
            vec2 rayDirection = new vec2(_isFacingRight ? 1 : -1, 0);

            // Center raycast
            vec2 centerRayOrigin = new vec2(position.x, position.y);
            if (RaycastForWall(centerRayOrigin, rayDirection, raycastLength))
            {
                _isTouchingWall = true;
                return;
            }

            // Top raycast
            vec2 topRayOrigin = new vec2(position.x, position.y + halfHeight * 0.9f);
            if (RaycastForWall(topRayOrigin, rayDirection, raycastLength))
            {
                _isTouchingWall = true;
                return;
            }

            // Bottom raycast
            vec2 bottomRayOrigin = new vec2(position.x, position.y - halfHeight * 0.9f);
            if (RaycastForWall(bottomRayOrigin, rayDirection, raycastLength))
            {
                _isTouchingWall = true;
                return;
            }
        }

        private bool RaycastForWall(vec2 origin, vec2 direction, float length)
        {
            if (Collision.Raycast(gameObjectName, origin, direction, length, out string hitObject, out float hitDistance))
            {
                // Assuming walls are also on the Ground layer as per the C++ enum comment
                if (Collision.GetCollisionLayer(hitObject) == (int)CollisionLayer.Ground)
                {
                    return true;
                }
            }
            return false;
        }

        #endregion

        #region Movement
        public void Move(float acceleration, float deceleration, vec2 moveInput, float deltaTime)
        {
            if (blockMovement) moveInput = new vec2(0,0);

            if (moveInput.x != 0 || (_isRolling && _rollTimer >= 0))
            {
                TurnCheck(moveInput);

                vec2 targetVelocity = new vec2(moveInput.x * MoveStats.MaxWalkSpeed, 0f);

                if (!(_isRolling && _rollTimer >= 0))
                {
                    _moveVelocity = Lerp(_moveVelocity, targetVelocity, acceleration * deltaTime);
                }
                
                vec2 currentVelocity = Physics.GetVelocity(gameObjectName);
                Physics.SetVelocity(gameObjectName, _moveVelocity.x, currentVelocity.y);
            }
            else
            {
                _moveVelocity = Lerp(_moveVelocity, new vec2(0,0), deceleration * deltaTime);
                vec2 currentVelocity = Physics.GetVelocity(gameObjectName);
                Physics.SetVelocity(gameObjectName, _moveVelocity.x, currentVelocity.y);
            }
        }

        private void TurnCheck(vec2 moveInput)
        {
            if (_isFacingRight && moveInput.x < 0)
            {
                Turn(false);
            }
            else if (!_isFacingRight && moveInput.x > 0)
            {
                Turn(true);
            }
        }

        private void Turn(bool turnRight)
        {
            _isFacingRight = turnRight;
            vec2 currentScale = Transform.GetScale(gameObjectName);
            Transform.SetScale(gameObjectName, Math.Abs(currentScale.x) * (turnRight ? 1 : -1), currentScale.y);
        }

        #endregion

        #region Jump

        private void Jump(float deltaTime)
        {
            if (_isWallJumping && VerticalVelocity <= 0)
            {
                _isWallJumping = false;
                blockMovement = false; // give back control
            }

            // apply gravity while jumping
            if (_isJumping)
            {
                // check for head bump
                if (_bumpedHead)
                {
                    _isFastFalling = true;
                }

                // gravity on ascending 
                if (VerticalVelocity >= 0f)
                {
                    // apex controls
                    _apexPoint = Utils.MathfInverseLerp(MoveStats.InitialJumpVelocity, 0f, VerticalVelocity);

                    if (_apexPoint > MoveStats.ApexThreshold)
                    {
                        if (!_isPastApexThreshold)
                        {
                            _isPastApexThreshold = true;
                            _timePastApexThreshold = 0f;
                        }

                        if (_isPastApexThreshold)
                        {
                            _timePastApexThreshold += deltaTime;

                            if (_timePastApexThreshold < MoveStats.ApexHangTime)
                            {
                                VerticalVelocity = 0f;
                            }
                            else
                            {
                                VerticalVelocity = -0.01f;
                            }
                        }
                    }
                    // gravity on descending but not past apex threshold
                    else
                    {
                        VerticalVelocity += MoveStats.Gravity * deltaTime;
                        if (_isPastApexThreshold)
                        {
                            _isPastApexThreshold = false;
                        }
                    }
                }
                // gravity on descending
                else if (!_isFastFalling)
                {
                    VerticalVelocity += MoveStats.Gravity * MoveStats.GravityOnReleaseMultiplier * deltaTime;
                }
                else if (VerticalVelocity < 0f)
                {
                    if (!_isFalling)
                    {
                        _isFalling = true;
                    }
                }
            }

            // jump cut
            if (_isFastFalling)
            {
                if (_fastFallTime >= MoveStats.TimeForUpwardsCancel)
                {
                    VerticalVelocity += MoveStats.Gravity * MoveStats.GravityOnReleaseMultiplier * deltaTime;
                }
                else if (_fastFallTime < MoveStats.TimeForUpwardsCancel)
                {
                    VerticalVelocity = Utils.MathfLerp(_fastFallReleaseSpeed, 0f, (_fastFallTime / MoveStats.TimeForUpwardsCancel));
                }

                _fastFallTime += deltaTime;
            }

            // normal gravity while falling
            if (!_isGrounded && !_isJumping)
            {
                if (!_isFalling)
                {
                    _isFalling = true;
                }

                VerticalVelocity += MoveStats.Gravity * deltaTime;
            }

            // clamp fall speed
            VerticalVelocity = Utils.MathfClamp(VerticalVelocity, -MoveStats.MaxFallSpeed, 80f);
            
            vec2 currentVelocity = Physics.GetVelocity(gameObjectName);
            Physics.SetVelocity(gameObjectName, currentVelocity.x, VerticalVelocity);
        }

        private void JumpChecks()
        {
            // when jump is pressed
            if (InputManager.JumpWasPressed)
            {
                _jumpBufferTimer = MoveStats.JumpBufferTime;
                _jumpReleasedDuringBuffer = false;
            }

            // when jump is released - with hold to jump enabled
            if (MoveStats.EnableVariableJumpHeight)
            {
                if (InputManager.JumpWasReleased)
                {
                    if (_jumpBufferTimer > 0f)
                    {
                        _jumpReleasedDuringBuffer = true;
                    }

                    if (_isJumping && VerticalVelocity > 0f)
                    {
                        if (_isPastApexThreshold)
                        {
                            _isPastApexThreshold = false;
                            _isFastFalling = true;
                            _fastFallTime = MoveStats.TimeForUpwardsCancel;
                            VerticalVelocity = 0f;
                        }
                        else
                        {
                            _isFastFalling = true;
                            _fastFallReleaseSpeed = VerticalVelocity;
                        }
                    }
                }
            }

            // wall jump
            if (_jumpBufferTimer > 0f && _wallJumpTimer > 0f)
            {
                InitiateWallJump();
                _jumpBufferTimer = 0f; // consume buffer to prevent other jumps
            }
            // initiate jump with jump buffering and coyote time
            else if (_jumpBufferTimer > 0f && !_isJumping && (_isGrounded || _coyoteTimer > 0f))
            {
                InitiateJump(1);

                if (_jumpReleasedDuringBuffer)
                {
                    _isFastFalling = true;
                    _fastFallReleaseSpeed = VerticalVelocity;
                }
            }
            // double jump
            else if (_jumpBufferTimer > 0f && _isJumping && _numberOfJumpsUsed < MoveStats.NumberOfJumpsAllowed)
            {
                _isFastFalling = false;
                InitiateJump(1);
            }
            // handle air jump after the coyote time has lapsed (take off an extra jump so the player does not get a bonus jump)
            else if (_jumpBufferTimer > 0f && _isFalling && _numberOfJumpsUsed < MoveStats.NumberOfJumpsAllowed - 1)
            {
                InitiateJump(2);
                _isFastFalling = false;
            }

            // landing
            if ((_isJumping || _isFalling) && _isGrounded && VerticalVelocity <= 0f)
            {
                if (_isWallKicking)
                {
                    StopWallKick();
                }

                // consume buffered jump upon landing
                _jumpBufferTimer = 0f;

                _isJumping = false;
                _isFalling = false;
                _isFastFalling = false;
                _fastFallTime = 0f;
                _isPastApexThreshold = false;
                _numberOfJumpsUsed = 0;

                _isWallJumping = false;
                blockMovement = false;

                _ziplineCoyoteTimer = 0f;

                VerticalVelocity = -0.5f; // small negative value to keep grounded
            }
        }

        private void InitiateJump(int numberOfJumpsUsed)
        {
            if (_isRolling)
            {
                StopRoll();
            }

            if (!_isJumping)
            {
                _isJumping = true;
            }

            _jumpBufferTimer = 0f;
            _numberOfJumpsUsed += numberOfJumpsUsed;
            VerticalVelocity = MoveStats.InitialJumpVelocity;
        }

        private void InitiateWallJump()
        {
            // state change
            _isWallJumping = true;
            blockMovement = true;

            // stop previous state
            _isWallSliding = false;
            _isJumping = true;

            // apply forces for wall jump
            VerticalVelocity = MoveStats.WallJumpForce.y;
            float horizontalDirection = _isFacingRight ? -1f : 1f;
            _moveVelocity.x = MoveStats.WallJumpForce.x * horizontalDirection;

            vec2 currentVelocity = Physics.GetVelocity(gameObjectName);
            Physics.SetVelocity(gameObjectName, _moveVelocity.x, currentVelocity.y);

            // turn the player
            Turn(!_isFacingRight);
        }
        #endregion

        #region Dash

        private void InitiateDash()
        {
            if (_isDashing) return;

            _isWallSliding = false;
            _isWallJumping = false;
            blockMovement = false;

            _isDashing = true;
            _dashTimer = MoveStats.DashDuration;
            _dashCooldownTimer = MoveStats.DashCooldown;

            // determine dash direction
            float dashDirection = _isFacingRight ? 1f : -1f;

            // set horizontal velocity
            _moveVelocity.x = dashDirection * MoveStats.DashSpeed;

            // cancel vertical movement
            VerticalVelocity = 0f;

            // ensure jump velocity does not interfere
            Physics.SetVelocity(gameObjectName, _moveVelocity.x, 0);
        }

        private void Dash(float deltaTime)
        {
            if (_isDashing)
            {
                // maintain dash speed and zero vertical velocity while dashing
                VerticalVelocity = 0f;
                float direction = _isFacingRight ? 1 : -1;
                float move = MoveStats.DashSpeed * deltaTime;

                // Move manually instead of overriding physics velocity
                Vector2 newVec = new Vector2(Transform.GetPosition(gameObjectName).x, Transform.GetPosition(gameObjectName).y);
                newVec += new Vector2(move * direction, 0);
                Transform.SetPosition(gameObjectName, newVec.X, newVec.Y);

                _dashTimer -= deltaTime;

                // check if dash has finished
                if (_dashTimer <= 0)
                {
                    StopDash();
                }
            }
        }

        private void StopDash()
        {
            _isDashing = false;
            vec2 currentVelocity = Physics.GetVelocity(gameObjectName);
            Physics.SetVelocity(gameObjectName, currentVelocity.x, 0f);
        }

        #endregion

        #region Wall Kick
        private void InitiateWallKick() { }
        private void WallKick(float deltaTime) { }
        private void StopWallKick() { }
        #endregion

        #region Wall Slide
        private void WallSlide(float deltaTime)
        {
            bool isPushingTowardsWall = (_isFacingRight && InputManager.Movement.x > 0) ||
                                        (!_isFacingRight && InputManager.Movement.x < 0);

            vec2 currentVelocity = Physics.GetVelocity(gameObjectName);

            if (_isTouchingWall && !_isGrounded && isPushingTowardsWall)
            {
                if (currentVelocity.y < 0)
                {
                    _isWallSliding = true;
                    float newYVelocity = Utils.MathfClamp(currentVelocity.y, -MoveStats.WallSlidingSpeed, 999f);
                    Physics.SetVelocity(gameObjectName, currentVelocity.x, newYVelocity);
                }
            }
            else
            {
                if (_isWallSliding)
                {
                    _isWallSliding = false;
                }
            }
        }
        #endregion

        #region Ground Pound
        void InitiateGroundPound() { }
        void GroundPound(float deltaTime) { }
        void StopGroundPound() { }
        #endregion

        #region Roll
        void InitiateRoll() { }
        void Roll(float deltaTime) { }
        void StopRoll() { }
        #endregion

        #region Air Glide
        void InitiateGlide() { }
        void Glide(float deltaTime) { }
        void StopGlide() { }
        #endregion

        #region Zipline
        // Zipline logic is complex and will be ported later
        #endregion

        public void OnDestroy()
        {
            Debug.Log("PlayerMovement Script Destroyed for object: " + gameObjectName);
        }

        private static vec2 Lerp(vec2 a, vec2 b, float t)
        {
            t = Utils.MathfClamp(t, 0, 1);
            return new vec2(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t);
        }
    }
}
