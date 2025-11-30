//using System;
//using System.Collections;
//using System.Runtime.CompilerServices;
//using Unity.Mathematics;
//using UnityEngine;

//public class PlayerMovement : MonoBehaviour
//{
//    // basically assigns names to numbers that can be used to track states
//    enum PlayerState
//    {
//        Idle = 0,
//        Running,
//        Jumping,
//        Dashing,
//        GroundPounding,
//        Rolling,
//        WallKicking,
//        Ziplining,
//        Gliding,
//        WallSliding,
//        WallJumping
//    };

//    private PlayerState currState = PlayerState.Idle;

//    [Header("References")]
//    public PlayerMovementStats MoveStats;
//    //[SerializeField] private Collider2D _feetColl;
//    //[SerializeField] private Collider2D _bodyColl;
//    //[SerializeField] private BoxCollider2D _feetColl;
//    //[SerializeField] private CapsuleCollider2D _bodyColl;
//    [SerializeField] private BoxCollider2D _bodyColl;
//    [SerializeField] private BoxCollider2D _feetColl;
//    [SerializeField] private Collider2D _WallCheckColl;

//    [Header("Animation")]
//    [SerializeField] private Animator _animator;

//    [Header("Audio")]
//    public PlayerSoundData sounds;
//    private Coroutine _wallSoundCoroutine;

//    // f = flags - for time deductions
//    // Leaving idle alone for now - would prolly need a way to detect if the player has been idle for 1-2 seconds, rather than each frame theyre stood still
//    public bool fDash, fJump, fIdle, fZipline, fGroundPound, fGlide, fRoll, fWallKick;

//    private Rigidbody2D _rb;

//    #region Variables
//    // movement variables
//    private Vector2 _moveVelocity;
//    private bool _isFacingRight;

//    // public facing right for other scripts
//    public bool IsFacingRight => _isFacingRight;

//    // collision check variables
//    private RaycastHit2D _groundHit;
//    private RaycastHit2D _wallHit;
//    private RaycastHit2D _headHit;
//    private bool _isGrounded;
//    private bool _bumpedHead;

//    // jump variables
//    public float VerticalVelocity { get; private set; }
//    private bool _isJumping;
//    private bool _isFastFalling;
//    private bool _isFalling;
//    private float _fastFallTime;
//    private float _fastFallReleaseSpeed;
//    private int _numberOfJumpsUsed;

//    // wall Jump variables
//    private bool _isWallJumping;
//    private float _wallJumpTimer;
//    private float _wallJumpDirection;

//    // apex variables
//    private float _apexPoint;
//    private float _timePastApexThreshold;
//    private bool _isPastApexThreshold;

//    // jump buffer variables
//    private float _jumpBufferTimer;
//    private bool _jumpReleasedDuringBuffer;

//    // coyote time variables
//    private float _coyoteTimer;

//    // dash variables
//    private bool _isDashing;
//    private float _dashTimer;
//    private float _dashCooldownTimer;

//    // wall kick variables
//    private bool _isWallKicking = false;
//    private int wallKickFrameCounter;           // [WIP] for cayote time 
//    private bool canWallKick;                   // Checks if touching a wall, and can perform a wall kick
//    private bool blockMovement = false;         // Prevents player from controlling character
//    private float _wallKickTimer;
//    private float _wallKickCooldownTimer;
//    private bool _startedWallKickInAir = false;
//    private float _wallKickTotalDuration;

//    // wall slide variables
//    private bool _isTouchingWall;
//    private bool _isWallSliding;

//    // ground pound variables
//    private bool _isGroundPounding = false;     // checks if player is currently in groundpound state
//    Vector2 lastPos;                            // stores pos of when the player initiated groundpound (for conversion to horizontal dist)
//    private float _groundPoundPauseTimer;

//    // roll variables
//    private bool _isRolling = false;            // checks if player is in rolling state
//    private float _rollTimer;

//    // glide variables
//    private bool _isGliding = false;            // checks if player is in gliding state
//    private float _glideCooldownTimer;
//    private float _timeSinceLastGlideDeduction;  // timer for periodic deduction

//    // zipline variables
//    private Zipline _availableZipline;
//    private bool _isZiplining = false;
//    private Coroutine _ziplineCoroutine;
//    private bool _isBeingLaunched = false;
//    private Zipline _lastAvailableZipline;
//    private float _ziplineCoyoteTimer;
//    private float _ziplineHeadOffset;

//    // camera 
//    private float _fallSpeedYDampingChangeThreshold;

//    // pickups
//    private Coroutine _speedBoostCoroutine;
//    public float SpeedMultiplier { get; private set; } = 1f;

//    // knockback
//    private KnockBack knockback;

//    #endregion


//    private void Start()
//    {
//        _fallSpeedYDampingChangeThreshold = CameraManager.instance._fallSpeedYDampingChangeThreshold;
//        knockback = GetComponent<KnockBack>();
//    }

//    private void Awake()
//    {
//        _isFacingRight = true;

//        _rb = GetComponent<Rigidbody2D>();

//        if (_bodyColl != null)
//        {
//            _ziplineHeadOffset = _bodyColl.offset.y + (_bodyColl.size.y / 2);
//        }

//        if (_animator != null)
//        {
//            _animator.SetFloat("Direction", 1f); // Start facing right.
//        }

//    }

//    private void Update()
//    {
//        // if the game is not in the "Playing" state = do nothing.
//        if (GameManager.instance.CurrentState != GameState.Playing)
//        {
//            return;
//        }

//        UpdateAnimatorParameters();

//        // prints velocity, for fine tuning
//        //Debug.Log(_moveVelocity);

//        HandleZiplineInput();
//        JumpChecks();
//        UpdatePlayerState();

//        // dash cooldown
//        if (_dashCooldownTimer > 0)
//        {
//            _dashCooldownTimer -= Time.deltaTime;
//        }

//        if (_wallKickCooldownTimer > 0)
//        {
//            _wallKickCooldownTimer -= Time.deltaTime;
//        }

//        // check for dash input
//        if (InputManager.DashWasPressed && _dashCooldownTimer <= 0)
//        {
//            InitiateDash();
//        }
//        if (InputManager.WallKickWasPressed && _wallKickCooldownTimer <= 0 && canWallKick)
//        {
//            InitiateWallKick();
//        }
//        if (InputManager.GroundPoundWasPressed && !_isGrounded)
//        {
//            InitiateGroundPound();
//        }
//        if (InputManager.RollWasPressed)
//        {
//            InitiateRoll();
//        }
//        if (InputManager.GlideIsHeld && !_isGrounded)
//        {
//            InitiateGlide();
//        }
//        else if (_isGliding)                 // if key was let go but still in gliding state, cancel gliding action
//        {
//            StopGlide();
//        }

//        if (_ziplineCoyoteTimer > 0)
//        {
//            _ziplineCoyoteTimer -= Time.deltaTime;
//        }

//        // if falling past a certain speed threshold
//        if (_rb.linearVelocity.y < _fallSpeedYDampingChangeThreshold && !CameraManager.instance.IsLerpingYDamping && !CameraManager.instance.LerpedFromPlayerFalling)
//        {
//            CameraManager.instance.LerpYDamping(true);
//        }

//        // if standing still or moving up
//        if (_rb.linearVelocity.y >= 0f && !CameraManager.instance.IsLerpingYDamping && CameraManager.instance.LerpedFromPlayerFalling)
//        {
//            // reset it so it can be called again
//            CameraManager.instance.LerpedFromPlayerFalling = false;

//            CameraManager.instance.LerpYDamping(false);
//        }
//    }

//    private void FixedUpdate()
//    {
//        // for physics-based actions
//        if (GameManager.instance.CurrentState != GameState.Playing) return;

//        CollisionChecks();

//        if (!_isZiplining && !knockback.IsBeingKnockedBack && !_isBeingLaunched)
//        {
//            Jump();
//            Dash();
//            WallKick();
//            GroundPound();
//            Roll();
//            Glide();
//            WallSlide();

//            // only allow regular movement if not dashing
//            if (!_isDashing && !_isWallKicking && !_isWallJumping && !_isGroundPounding)
//            {
//                if (_isGrounded)
//                {
//                    Move(MoveStats.GroundAcceleration, MoveStats.GroundDeceleration, InputManager.Movement);
//                }
//                else
//                {
//                    Move(MoveStats.AirAcceleration, MoveStats.AirDeceleration, InputManager.Movement);
//                }
//            }
//        }

//    }

//    #region SFX & VFX

//    private void UpdateAnimatorParameters()
//    {
//        if (_animator == null) return;

//        // Ground movement
//        float horizontalSpeed = Mathf.Abs(_rb.linearVelocity.x);
//        _animator.SetFloat("xVelocity", horizontalSpeed);

//        // In-Air Logic
//        _animator.SetBool("isGrounded", _isGrounded);
//        _animator.SetFloat("yVelocity", _rb.linearVelocity.y);

//        // Abilities
//        _animator.SetBool("isDashing", _isDashing);
//        _animator.SetBool("isRolling", _isRolling);
//        _animator.SetBool("isZiplining", _isZiplining);
//        _animator.SetBool("isWallKicking", _isWallKicking);
//        _animator.SetBool("isWallSliding", _isWallSliding);
//        _animator.SetBool("isDiving", _isGroundPounding);
//    }

//    public void PlayFootstepSound()
//    {
//        if (sounds != null && sounds.footsteps != null)
//        {
//            AudioManager.instance.PlayRandomPlayerSFX(sounds.footsteps);
//        }
//    }

//    private IEnumerator PlayWallSlideAudioSequence(Sound wallLandSound, Sound wallSlideSound)
//    {
//        // 1. Play the initial "impact" sound one time.
//        if (wallLandSound != null)
//        {
//            AudioManager.instance.PlayPlayerSFX(wallLandSound);
//        }

//        // 2. Wait for the duration of the impact sound clip before proceeding.
//        //    We add a tiny buffer (0.05s) to ensure it finishes cleanly.
//        yield return new WaitForSeconds(wallLandSound.clip.length + 0.05f);

//        // 3. After the wait, if we are STILL wall sliding, start the looping sound.
//        //    This check prevents the loop from starting if the player has already let go.
//        if (_isWallSliding && wallSlideSound != null)
//        {
//            AudioManager.instance.StartLoopingPlayerSFX(wallSlideSound);
//        }
//    }

//    #endregion


//    #region Collison Checks

//    private void CollisionChecks()
//    {
//        IsGrounded();
//        BumpedHead();
//        IsTouchingWall();

//        if (_isTouchingWall && !_isGrounded)
//        {
//            // while on wall keep timer running
//            _wallJumpTimer = MoveStats.WallCoyoteTime;

//            // determine which way is away from the wall
//            _wallJumpDirection = _isFacingRight ? -1f : 1f;
//        }
//        else
//        {
//            // if not on wall - start counting down for coyote time
//            _wallJumpTimer -= Time.deltaTime;
//        }

//        //canWallKick = _isTouchingWall && !_isGrounded;
//        canWallKick = _isTouchingWall;
//    }


//    private void IsGrounded()
//    {
//        Vector2 boxCastOrigin = new Vector2(_feetColl.bounds.center.x, _feetColl.bounds.min.y);
//        Vector2 boxCastSize = new Vector2(_feetColl.bounds.size.x, MoveStats.GroundDetectionRayLength);

//        _groundHit = Physics2D.BoxCast(boxCastOrigin, boxCastSize, 0f, Vector2.down, MoveStats.GroundDetectionRayLength, MoveStats.GroundLayer);

//        if (_groundHit.collider != null)
//        {
//            _isGrounded = true;
//        }
//        else
//        {
//            _isGrounded = false;
//        }

//        // MOVED TO ISTOUCHINGWALL()
//        //Vector2 boxMiddleOrigin = new Vector2(_WallCheckColl.bounds.center.x, _WallCheckColl.bounds.min.y);
//        //Vector2 boxMiddleSize = new Vector2(_WallCheckColl.bounds.size.x, MoveStats.GroundDetectionRayLength);      // Prolly adjust for cayote time
//        //// checks for if touching a wall
//        //if (_isFacingRight)
//        //{
//        //    _wallHit = Physics2D.BoxCast(boxMiddleOrigin, boxMiddleSize, 0f, Vector2.right, MoveStats.GroundDetectionRayLength, MoveStats.GroundLayer);
//        //}
//        //else
//        //{
//        //    _wallHit = Physics2D.BoxCast(boxMiddleOrigin, boxMiddleSize, 0f, Vector2.left, MoveStats.GroundDetectionRayLength, MoveStats.GroundLayer);
//        //}

//        // MOVED TO COLLISIONCHECKS()
//        //canWallKick = (_wallHit.collider != null && !_isGrounded) ? true : false;

//        #region Debug Visualizaton
//        if (MoveStats.DebugShowIsGroundedBox)
//        {
//            Color rayColor;
//            if (_isGrounded)
//            {
//                rayColor = Color.green;
//            }
//            else
//            {
//                rayColor = Color.red;
//            }

//            Debug.DrawRay(new Vector2(boxCastOrigin.x - boxCastSize.x / 2, boxCastOrigin.y), Vector2.down * MoveStats.GroundDetectionRayLength, rayColor);
//            Debug.DrawRay(new Vector2(boxCastOrigin.x + boxCastSize.x / 2, boxCastOrigin.y), Vector2.down * MoveStats.GroundDetectionRayLength, rayColor);
//            Debug.DrawRay(new Vector2(boxCastOrigin.x - boxCastSize.x / 2, boxCastOrigin.y - MoveStats.GroundDetectionRayLength), Vector2.right * boxCastSize.x, rayColor);
//        }
//        #endregion
//    }


//    private void BumpedHead()
//    {
//        Vector2 boxCastOrigin = new Vector2(_feetColl.bounds.center.x, _bodyColl.bounds.max.y);
//        Vector2 boxCastSize = new Vector2(_feetColl.bounds.size.x * MoveStats.HeadWidth, MoveStats.HeadDetectionRayLength);

//        _headHit = Physics2D.BoxCast(boxCastOrigin, boxCastSize, 0f, Vector2.up, MoveStats.HeadDetectionRayLength, MoveStats.GroundLayer);

//        if (_headHit.collider != null)
//        {
//            _bumpedHead = true;
//        }
//        else
//        {
//            _bumpedHead = false;
//        }

//        #region Debug Visualizaton 
//        if (MoveStats.DebugShowHeadBumpBox)
//        {
//            float headWidth = MoveStats.HeadWidth;

//            Color rayColor;
//            if (_bumpedHead)
//            {
//                rayColor = Color.green;
//            }
//            else
//            {
//                rayColor = Color.red;
//            }

//            Debug.DrawRay(new Vector2(boxCastOrigin.x - boxCastSize.x / 2 * headWidth, boxCastOrigin.y), Vector2.up * MoveStats.HeadDetectionRayLength, rayColor);
//            Debug.DrawRay(new Vector2(boxCastOrigin.x + (boxCastSize.x / 2) * headWidth, boxCastOrigin.y), Vector2.up * MoveStats.HeadDetectionRayLength, rayColor);
//            Debug.DrawRay(new Vector2(boxCastOrigin.x - boxCastSize.x / 2 * headWidth, boxCastOrigin.y + MoveStats.HeadDetectionRayLength), Vector2.right * boxCastSize.x * headWidth, rayColor);
//        }

//        #endregion 
//    }

//    private void IsTouchingWall()
//    {
//        Vector2 boxCastOrigin = new Vector2(_WallCheckColl.bounds.center.x, _WallCheckColl.bounds.min.y);
//        Vector2 boxCastSize = new Vector2(_WallCheckColl.bounds.size.x, 0.1f);

//        float checkDistance = MoveStats.WallDetectionBoxLength;
//        LayerMask layerToHit = MoveStats.WallLayer;           

//        RaycastHit2D wallHit;
//        Vector2 direction = _isFacingRight ? Vector2.right : Vector2.left;

//        wallHit = Physics2D.BoxCast(boxCastOrigin, boxCastSize, 0f, direction, checkDistance, layerToHit);

//        _isTouchingWall = wallHit.collider != null;
//    }

//    #endregion

//    #region Movement
//    private void UpdatePlayerState()
//    {
//        // check if in air
//        if (!_isGrounded)
//        {
//            //currState = PlayerState.Jumping;
//            if (_isWallJumping)
//            {
//                currState = PlayerState.WallJumping;
//            }
//            else
//            {
//                currState = PlayerState.Jumping;
//            }
//        }
//        else
//        {
//            // horizontal input = moving - trigger run animation
//            if (InputManager.Movement.x != 0)
//            {
//                currState = PlayerState.Running;
//            }
//            else
//            {
//                currState = PlayerState.Idle;
//            }
//        }
//        // placed after jump and run, so it can be overriden 
//        if (_isDashing) currState = PlayerState.Dashing;
//        if (_isRolling) currState = PlayerState.Rolling;
//        if (_isGroundPounding) currState = PlayerState.GroundPounding;
//        if (_isGliding) currState = PlayerState.Gliding;
//        if (_isWallKicking) currState = PlayerState.WallKicking;
//        if (_isZiplining) currState = PlayerState.Ziplining;
//        if (_isWallSliding) currState = PlayerState.WallSliding;
//    }

//    public void Move(float acceleration, float deceleration, Vector2 moveInput)
//    {
//        if (blockMovement) moveInput = Vector2.zero;            // gotta do this funky business bc if we need the accel/ decel

//        if (moveInput != Vector2.zero || (_isRolling && _rollTimer >= 0))
//        {
//            TurnCheck(moveInput);

//            Vector2 targetVelocity = Vector2.zero;

//            // apply walk speed with speed multiplier if valid - for pickup effects
//            targetVelocity = new Vector2(moveInput.x, 0f) * MoveStats.MaxWalkSpeed * SpeedMultiplier;

//            if (!(_isRolling && _rollTimer >= 0))               // did this so it would continously move forward in roll state
//            {
//                _moveVelocity = Vector2.Lerp(_moveVelocity, targetVelocity, acceleration * Time.fixedDeltaTime);
//            }

//            _rb.linearVelocity = new Vector2(_moveVelocity.x, _rb.linearVelocity.y);
//        }
//        else
//        {
//            _moveVelocity = Vector2.Lerp(_moveVelocity, Vector2.zero, deceleration * Time.fixedDeltaTime);
//            _rb.linearVelocity = new Vector2(_moveVelocity.x, _rb.linearVelocity.y);
//        }

//    }

//    private void TurnCheck(Vector2 moveInput)
//    {

//        if (_isFacingRight && moveInput.x < 0)
//        {
//            Turn(false);
//        }
//        else if (!_isFacingRight && moveInput.x > 0)
//        {
//            Turn(true);
//        }
//    }

//    private void Turn(bool turnRight)
//    {
//        if (turnRight)
//        {
//            _isFacingRight = true;
//            transform.Rotate(0f, 180f, 0f);
//        }
//        else
//        {
//            _isFacingRight = false;
//            transform.Rotate(0f, -180f, 0f);
//        }
//    }

//    #endregion

//    #region Jump

//    private void Jump()
//    {
//        if (_isWallJumping && VerticalVelocity <= 0)
//        {
//            _isWallJumping = false;
//            blockMovement = false; // give back control
//        }

//        // apply gravity while jumping
//        if (_isJumping)
//        {
//            // check for head bump
//            if (_bumpedHead)
//            {
//                _isFastFalling = true;
//            }

//            // gravity on ascending 
//            if (VerticalVelocity >= 0f)
//            {
//                // apex controls
//                _apexPoint = Mathf.InverseLerp(MoveStats.InitialJumpVelocity, 0f, VerticalVelocity);

//                if (_apexPoint > MoveStats.ApexThreshold)
//                {
//                    if (!_isPastApexThreshold)
//                    {
//                        _isPastApexThreshold = true;
//                        _timePastApexThreshold = 0f;
//                    }

//                    if (_isPastApexThreshold)
//                    {
//                        _timePastApexThreshold += Time.fixedDeltaTime;

//                        if (_timePastApexThreshold < MoveStats.ApexHangTime)
//                        {
//                            VerticalVelocity = 0f;
//                        }
//                        else
//                        {
//                            VerticalVelocity = -0.01f;
//                        }
//                    }
//                }

//                // gravity on descending but not past apex threshold
//                else
//                {
//                    VerticalVelocity += MoveStats.Gravity * Time.fixedDeltaTime;
//                    if (_isPastApexThreshold)
//                    {
//                        _isPastApexThreshold = false;
//                    }
//                }
//            }

//            // gravity on descending
//            else if (!_isFastFalling)
//            {
//                VerticalVelocity += MoveStats.Gravity * MoveStats.GravityOnReleaseMultiplier * Time.fixedDeltaTime;
//            }

//            else if (VerticalVelocity < 0f)
//            {
//                if (!_isFalling)
//                {
//                    _isFalling = true;
//                }
//            }
//        }

//        // jump cut
//        if (_isFastFalling)
//        {
//            if (_fastFallTime >= MoveStats.TimeForUpwardsCancel)
//            {
//                VerticalVelocity += MoveStats.Gravity * MoveStats.GravityOnReleaseMultiplier * Time.fixedDeltaTime;
//            }
//            else if (_fastFallTime < MoveStats.TimeForUpwardsCancel)
//            {
//                VerticalVelocity = Mathf.Lerp(_fastFallReleaseSpeed, 0f, (_fastFallTime / MoveStats.TimeForUpwardsCancel));
//            }

//            _fastFallTime += Time.fixedDeltaTime;
//        }

//        // normal gravity while falling
//        if (!_isGrounded && !_isJumping)
//        {
//            if (!_isFalling)
//            {
//                _isFalling = true;
//            }

//            VerticalVelocity += MoveStats.Gravity * Time.fixedDeltaTime;
//        }

//        // clamp fall speed
//        VerticalVelocity = Mathf.Clamp(VerticalVelocity, -MoveStats.MaxFallSpeed, 80f);

//        _rb.linearVelocity = new Vector2(_rb.linearVelocity.x, VerticalVelocity);
//    }

//    private void JumpChecks()
//    {
//        // when jump is pressed
//        if (InputManager.JumpWasPressed)
//        {
//            _jumpBufferTimer = MoveStats.JumpBufferTime;
//            _jumpReleasedDuringBuffer = false;
//        }

//        // when jump is released - with hold to jump enabled
//        if (MoveStats.EnableVariableJumpHeight)
//        {
//            if (InputManager.JumpWasReleased)
//            {
//                if (_jumpBufferTimer > 0f)
//                {
//                    _jumpReleasedDuringBuffer = true;
//                }

//                if (_isJumping && VerticalVelocity > 0f)
//                {
//                    if (_isPastApexThreshold)
//                    {
//                        _isPastApexThreshold = false;
//                        _isFastFalling = true;
//                        _fastFallTime = MoveStats.TimeForUpwardsCancel;
//                        VerticalVelocity = 0f;
//                    }
//                    else
//                    {
//                        _isFastFalling = true;
//                        _fastFallReleaseSpeed = VerticalVelocity;
//                    }
//                }
//            }
//        }

//        // wall jump
//        if (_jumpBufferTimer > 0f && _wallJumpTimer > 0f)
//        {
//            InitiateWallJump();
//            _jumpBufferTimer = 0f; // consume buffer to prevent other jumps
//        }

//        // initiate jump with jump buffering and coyote time
//        else if (_jumpBufferTimer > 0f && !_isJumping && (_isGrounded || _coyoteTimer > 0f))
//        {
//            InitiateJump(1);

//            if (_jumpReleasedDuringBuffer)
//            {
//                _isFastFalling = true;
//                _fastFallReleaseSpeed = VerticalVelocity;
//            }
//        }

//        // double jump
//        else if (_jumpBufferTimer > 0f && _isJumping && _numberOfJumpsUsed < MoveStats.NumberOfJumpsAllowed)
//        {
//            _isFastFalling = false;
//            InitiateJump(1);
//        }

//        // handle air jump after the coyote time has lapsed (take off an extra jump so the player does not get a bonus jump)
//        else if (_jumpBufferTimer > 0f && _isFalling && _numberOfJumpsUsed < MoveStats.NumberOfJumpsAllowed - 1)
//        {
//            InitiateJump(2);
//            _isFastFalling = false;
//        }

//        // landing
//        if ((_isJumping || _isFalling) && _isGrounded && VerticalVelocity <= 0f)
//        {
//            // Check that the sound profile and the new array exist and are not empty.
//            if (sounds != null && sounds.landSounds != null && sounds.landSounds.Length > 0)
//            {
//                // Call the random SFX player with our new array of land sounds.
//                AudioManager.instance.PlayRandomPlayerSFX(sounds.landSounds);
//            }

//            // Fire the 'Land' trigger the moment we hit the ground.
//            if (_animator != null)
//            {
//                _animator.SetTrigger("Land");
//            }

//            // WallKick() grounded check
//            if (_isWallKicking)
//            {
//                StopWallKick();
//            }

//            // consume buffered jump upon landing
//            _jumpBufferTimer = 0f;

//            _isJumping = false;
//            _isFalling = false;
//            _isFastFalling = false;
//            _fastFallTime = 0f;
//            _isPastApexThreshold = false;
//            _numberOfJumpsUsed = 0;

//            _isWallJumping = false;
//            blockMovement = false;

//            _ziplineCoyoteTimer = 0f;

//            VerticalVelocity = Physics2D.gravity.y;
//        }
//    }

//    private void InitiateJump(int numberOfJumpsUsed)
//    {
//        // Fire the 'Jump' trigger in the Animator the moment a jump starts.
//        if (_animator != null)
//        {
//            _animator.SetTrigger("Jump");
//        }

//        // play jump audio
//        if (sounds != null && sounds.jump != null) AudioManager.instance.PlayPlayerSFX(sounds.jump);

//        if (_isRolling)
//        {
//            StopRoll();
//        }

//        if (!_isJumping)
//        {
//            fJump = true;
//            _isJumping = true;
//        }

//        _jumpBufferTimer = 0f;
//        _numberOfJumpsUsed += numberOfJumpsUsed;
//        VerticalVelocity = MoveStats.InitialJumpVelocity;
//    }

//    private void InitiateWallJump()
//    {
//        // play wall jump audio
//        if (sounds != null && sounds.wallJump != null) AudioManager.instance.PlayPlayerSFX(sounds.wallJump);

//        // state change
//        _isWallJumping = true;
//        blockMovement = true; 

//        // stop previous state
//        _isWallSliding = false;
//        _isJumping = true;

//        // apply forces for wall jump
//        VerticalVelocity = MoveStats.WallJumpForce.y;
//        float horizontalDirection = _isFacingRight ? -1f : 1f;
//        _moveVelocity.x = MoveStats.WallJumpForce.x * horizontalDirection;
//        _rb.linearVelocity = new Vector2(_moveVelocity.x, _rb.linearVelocity.y);

//        // turn the player
//        Turn(!_isFacingRight);
//    }

//    #region Jump Debug Visualization


//    //private void OnDrawGizmos()
//    //{
//    //    if (MoveStats.ShowWalkJumpArc)
//    //    {
//    //        DrawJumpArc(MoveStats.MaxWalkSpeed, Color.white);
//    //    }
//    //}

//    //private void DrawJumpArc(float moveSpeed, Color gizmoColor)
//    //{
//    //    Vector2 startPosition = new Vector2(_feetColl.bounds.center.x, _feetColl.bounds.center.y);
//    //    Vector2 previousPosition = startPosition;
//    //    float speed = 0f;

//    //    if (MoveStats.DrawRight)
//    //    {
//    //        speed = moveSpeed;
//    //    }
//    //    else
//    //    {
//    //        speed = -moveSpeed;
//    //    }

//    //    Vector2 velocity = new Vector2(speed, MoveStats.InitialJumpVelocity);

//    //    Gizmos.color = gizmoColor;

//    //    float timeStep = 2 * MoveStats.TimeTillJumpApex / MoveStats.ArcResolution;

//    //    for (int i = 0; i < MoveStats.VisualizationSteps; i++)
//    //    {
//    //        float simulationTime = i * timeStep;
//    //        Vector2 displacement;
//    //        Vector2 drawPoint;

//    //        // ascending
//    //        if (simulationTime < MoveStats.TimeTillJumpApex)
//    //        {
//    //            displacement = velocity * simulationTime * 0.5f * new Vector2(0, MoveStats.Gravity) * simulationTime * simulationTime;
//    //        }
//    //        else if (simulationTime < MoveStats.TimeTillJumpApex + MoveStats.ApexHangTime)
//    //        {
//    //            float apexTime = simulationTime - MoveStats.TimeTillJumpApex;
//    //            displacement = velocity * MoveStats.TimeTillJumpApex + 0.5f * new Vector2(0, MoveStats.TimeTillJumpApex * MoveStats.TimeTillJumpApex);
//    //            displacement += new Vector2(speed, 0) * apexTime; // no vertical movement during hang time
//    //        }
//    //        else // descending
//    //        {
//    //            float descendTime = simulationTime - (MoveStats.TimeTillJumpApex + MoveStats.ApexHangTime);
//    //            displacement = velocity * MoveStats.TimeTillJumpApex + 0.5f * new Vector2(0, MoveStats.Gravity) * MoveStats.TimeTillJumpApex * MoveStats.TimeTillJumpApex;
//    //            displacement += new Vector2(speed, 0) * MoveStats.ApexHangTime; // horizontal movement during hang time
//    //            displacement += new Vector2(speed, 0) * descendTime + 0.5f * new Vector2(0, MoveStats.Gravity) * descendTime * descendTime;
//    //        }

//    //        drawPoint = startPosition + displacement;

//    //        if (MoveStats.StopOnCollision)
//    //        {
//    //            RaycastHit2D hit = Physics2D.Raycast(previousPosition, drawPoint - previousPosition, Vector2.Distance(previousPosition, drawPoint), MoveStats.GroundLayer);
//    //            if (hit.collider != null)
//    //            {
//    //                // if hit is detected, stop drawing the arc at the hit point
//    //                Gizmos.DrawLine(previousPosition, hit.point);
//    //                break;
//    //            }
//    //        }

//    //        Gizmos.DrawLine(previousPosition, drawPoint);
//    //        previousPosition = drawPoint;
//    //    }
//    //}
//    #endregion

//    #endregion

//    #region Dash

//    private void InitiateDash()
//    {
//        // First, check if we are already dashing.
//        if (_isDashing) return;

//        // If not, THEN play the sound and start the dash.
//        if (sounds != null && sounds.dash != null) AudioManager.instance.PlayPlayerSFX(sounds.dash);

//        // cancel out of wall slide with dash
//        _isWallSliding = false;

//        _isWallJumping = false;
//        blockMovement = false;

//        fDash = true;

//        _isDashing = true;
//        _dashTimer = MoveStats.DashDuration;
//        _dashCooldownTimer = MoveStats.DashCooldown;

//        // determine dash direction
//        float dashDirection = _isFacingRight ? 1f : -1f;

//        // set horizontal velocity
//        _moveVelocity.x = dashDirection * MoveStats.DashSpeed;

//        // cancel vertical movement
//        VerticalVelocity = 0f;

//        // ensure jump velocity does not interfere
//        _rb.linearVelocity = new Vector2(_rb.linearVelocity.x, 0);
//    }

//    private void Dash()
//    {
//        if (_isDashing)
//        {
//            // maintain dash speed and zero vertical velocity while dashing
//            VerticalVelocity = 0f;
//            _rb.linearVelocity = new Vector2(_moveVelocity.x, 0f);

//            _dashTimer -= Time.fixedDeltaTime;

//            // check if dash has finished
//            if (_dashTimer <= 0)
//            {
//                StopDash();
//            }
//        }
//    }

//    private void StopDash()
//    {
//        _isDashing = false;
//        _rb.linearVelocity = new Vector2(_moveVelocity.x, 0f);

//        // removed this so player comes to a stop more naturally
//        //_moveVelocity.x = 0;
//        //_rb.linearVelocity = new Vector2(0, _rb.linearVelocity.y); // stop horizontal movement immediately
//    }

//    #endregion

//    #region Wall Kick

//    // Possibly, wall kick can just be
//    // - Detect wall kick inputs and checks
//    // - Apply dash force with inputted direction, takes reference from player current speed, maybe min result speed quite high also
//    // - Pass player the new kick speed, and let physics handle the rest
//    // [WIP] when we have a good momentum building movement style, wallkicks MUST convert the horizontal momentum to the opposite direction

//    private void InitiateWallKick()
//    {
//        _isWallKicking = true;

//        // play wall kick audio
//        if (sounds != null && sounds.wallKick != null) AudioManager.instance.PlayPlayerSFX(sounds.wallKick);

//        fWallKick = true;         // time deduction
//        blockMovement = true;                                   // stops player control until landed on ground
//        _wallKickTimer = MoveStats.DashDuration * MoveStats.WallKickDurationMultiplier; // 0.15f
//        _wallKickCooldownTimer = MoveStats.DashCooldown;        // 0.5f
//        _wallKickTotalDuration = MoveStats.DashDuration * MoveStats.WallKickDurationMultiplier;
//        _wallKickTimer = _wallKickTotalDuration;

//        // --- USING STATS.CS CHANGE ---
//        //_wallKickTimer = MoveStats.DashDuration * 3;            // 0.15f
//        //_wallKickCooldownTimer = MoveStats.DashCooldown;        // 0.5f

//        // determine dash direction
//        float wallKickDir = _isFacingRight ? -1f : 1f;    // Check direction player is facing, will likely need inverse this (reverse of dash bc we're yknow kicking the wall)

//        // flip to face new direction
//        Turn(!_isFacingRight);

//        // set horizontal velocity
//        _moveVelocity.x = wallKickDir * MoveStats.WallKickSpeedModifier;
//        _moveVelocity.y = 0;

//        // --- USINGS STATS.CS CHANGE ---
//        //// set horizontal velocity
//        //_moveVelocity.x = wallKickDir * MoveStats.DashSpeed * 1.5f;
//        //_moveVelocity.y = 0;

//        _rb.linearVelocity = new Vector2(_rb.linearVelocity.x, 0);
//    }

//    private void WallKick()
//    {
//        if (_isWallKicking)
//        {
//            Debug.Log("y veloc " + _rb.linearVelocityY);

//            blockMovement = true;
//            _isJumping = false;
//            _wallKickTimer -= Time.fixedDeltaTime;

//            // calculate progress through the kick (from 0.0 @ start to 1.0 @ end)
//            float progress = 1f - (_wallKickTimer / _wallKickTotalDuration);

//            // get speed multiplier from curve @ current progress point
//            float curveMultiplier = MoveStats.WallKickEaseCurve.Evaluate(progress);

//            // actual speed is peak speed multiplied by curve's value
//            float currentSpeed = _moveVelocity.x * curveMultiplier;
//            float verticalGain = MoveStats.WallKickVerticalModifier * curveMultiplier;

//            // apply new eased speed
//            // to fix the jank vert momentum, we just ignore gravity for now, and give back control to the grav after wall kicks
//            // can change this to a var!
//            _rb.linearVelocity = new Vector2(currentSpeed, verticalGain);

//            // check if the kick's duration has finished
//            if (_wallKickTimer <= 0)
//            {
//                StopWallKick();
//            }
//        }
//    }

//    private void StopWallKick()
//    {
//        // end of wall kick - restore control to player here
//        _isWallKicking = false;
//        blockMovement = false;

//        // reset horizontal velocity so player can stop / move normally
//        //_rb.linearVelocity += new Vector2(0, _rb.linearVelocity.y); // stop horizontal movement immediately
//        _rb.linearVelocity = new Vector2(0, _rb.linearVelocity.y);

//        //_moveVelocity = _rb.linearVelocity;
//        wallKickFrameCounter = 0;                       // [WIP] for cayote time upon leaving wall
//    }
//    #endregion

//    #region Wall Slide

//    //private void WallSlide()
//    //{
//    //    // check if pushing towards wall 
//    //    bool isPushingTowardsWall = (_isFacingRight && InputManager.Movement.x > 0) ||
//    //                                (!_isFacingRight && InputManager.Movement.x < 0);

//    //    Debug.Log($"isGrounded: {_isGrounded} | isTouchingWall: {_isTouchingWall} | isPushing: {isPushingTowardsWall} | Y Velocity: {_rb.linearVelocity.y}");

//    //    if (_isTouchingWall && !_isGrounded && isPushingTowardsWall)
//    //    {
//    //        if (_rb.linearVelocity.y < 0)
//    //        {
//    //            _isWallSliding = true;

//    //            // play looping wall slide audio
//    //            if (sounds != null && sounds.wallSlide != null)
//    //            {
//    //                AudioManager.instance.StartLoopingPlayerSFX(sounds.wallSlide);
//    //            }

//    //            _rb.linearVelocity = new Vector2(_rb.linearVelocity.x, Mathf.Max(_rb.linearVelocity.y, -MoveStats.WallSlidingSpeed));
//    //        }
//    //    }
//    //    else
//    //    {
//    //        if (_isWallSliding)
//    //        {
//    //            _isWallSliding = false; // Set the state first

//    //            // STOP the looping sound
//    //            AudioManager.instance.StopLoopingPlayerSFX();
//    //        }
//    //    }
//    //}

//    private void WallSlide()
//    {
//        // Guard clauses to ensure we are not grounded or being knocked back.
//        if (_isGrounded || (knockback != null && knockback.IsBeingKnockedBack))
//        {
//            // If we were sliding, make sure to stop all related sounds.
//            if (_isWallSliding)
//            {
//                _isWallSliding = false;
//                AudioManager.instance.StopLoopingPlayerSFX();
//                if (_wallSoundCoroutine != null)
//                {
//                    StopCoroutine(_wallSoundCoroutine);
//                }
//            }
//            return;
//        }

//        // --- The rest of the logic ---
//        bool isPushingTowardsWall = (_isFacingRight && InputManager.Movement.x > 0) ||
//                                    (!_isFacingRight && InputManager.Movement.x < 0);

//        // Condition to be in the wall slide state
//        if (_isTouchingWall && isPushingTowardsWall && _rb.linearVelocity.y < 0)
//        {
//            // This block only runs on the VERY FIRST FRAME of the wall slide.
//            if (!_isWallSliding)
//            {
//                _isWallSliding = true; // Set the state to true immediately

//                // --- NEW: Start the audio sequence coroutine ---
//                if (_wallSoundCoroutine != null)
//                {
//                    StopCoroutine(_wallSoundCoroutine); // Stop any previous routine just in case
//                }
//                _wallSoundCoroutine = StartCoroutine(PlayWallSlideAudioSequence(sounds.wallLand, sounds.wallSlide));
//            }

//            // Apply the physics effect every frame we are sliding
//            _rb.linearVelocity = new Vector2(_rb.linearVelocity.x, Mathf.Max(_rb.linearVelocity.y, -MoveStats.WallSlidingSpeed));
//        }
//        else // If we are no longer meeting the conditions to slide
//        {
//            if (_isWallSliding)
//            {
//                _isWallSliding = false;

//                // --- NEW: Stop the audio sequence ---
//                // Stop the looping sound immediately.
//                AudioManager.instance.StopLoopingPlayerSFX();
//                // Stop the coroutine so the looping sound doesn't start after an impact.
//                if (_wallSoundCoroutine != null)
//                {
//                    StopCoroutine(_wallSoundCoroutine);
//                }
//            }
//        }
//    }
//    #endregion

//    #region Ground Pound

//    void InitiateGroundPound()
//    {

//        if (_isGroundPounding) return;

//        // If we pass the check, THEN start the action and play the sound.
//        if (sounds != null && sounds.groundPound != null) AudioManager.instance.PlayPlayerSFX(sounds.groundPound);

//        fGroundPound = true; // time deduction
//        _isGroundPounding = true;

//        _groundPoundPauseTimer = MoveStats.GroundPoundPauseTime;

//        // can maybe do a -= until <= 0, so smoother stop to midair, but for demo we just instant stop and pause in midair before groundpound
//        _moveVelocity.x = 0;
//        _moveVelocity.y = -MoveStats.GroundPoundFallSpeed;

//        // --- USING STATS.CS CHANGE ---
//        //_moveVelocity.y = -20f;             // [WIP] was just demoing here, if you happy with the feel of this value then can move to movestats!

//        _rb.linearVelocity = new Vector2(0, 0);
//        lastPos = _rb.position;             // store position where groundpound initiated - used for calculating horizontal distance to convert
//    }

//    void GroundPound()
//    {
//        if (_isGroundPounding)
//        {
//            if (_groundPoundPauseTimer > 0)                         // Timer for how long the player stays in air for before moving downwards
//            {
//                _groundPoundPauseTimer -= Time.deltaTime;
//                _rb.linearVelocity = new Vector2(0f, 0f);
//                _rb.position = lastPos;
//            }
//            else
//            {
//                // Disable player control until hit ground or dashing
//                if (_isGrounded || _isDashing)
//                {
//                    blockMovement = false;
//                    StopGroundPound();
//                }
//                else
//                {
//                    _rb.linearVelocity += new Vector2(0, _moveVelocity.y);          // zoom down!
//                }
//            }
//        }
//    }

//    void StopGroundPound()
//    {
//        // --- USING STATS.CS CHANGE --- 
//        //_groundPoundPauseTimer = _groundPoundPauseInitTimer;                // Reset timers, restore control to player

//        // moved to InitiateGroundPound()
//        //_groundPoundPauseTimer = MoveStats.GroundPoundPauseTime;            // Reset timers, restore control to player

//        // moving this to init roll so can check if was GP-ing
//        // _isGroundPounding = false;
//        // blockMovement = false;

//        // might not be necessary
//        _rb.linearVelocity += new Vector2(0, _rb.linearVelocity.y);         // stop horizontal movement immediately
//        _rb.linearVelocity *= 0;                                            // stop horizontal movement immediately

//        // gotta check again bc dash can cancel ground pounds
//        if (_isGrounded)                                                    // if groundpound hits the...ground... then convert into roll! 
//        {
//            InitiateRoll();
//            return;
//        }
//    }

//    #endregion

//    #region Roll

//    void InitiateRoll()
//    {
//        if (_isRolling || !_isGrounded) return;

//        // If the check passes, THEN play the sound.
//        if (sounds != null && sounds.roll != null) AudioManager.instance.PlayPlayerSFX(sounds.roll);

//        fRoll = true;                                   // time deduction
//        _isRolling = true;

//        _rollTimer = (!_isGroundPounding) ? MoveStats.BaseRollDuration : MoveStats.BaseRollDuration + (lastPos.y - _rb.position.y) / MoveStats.RollHeightToDurationDivisor;
//        Mathf.Clamp(_rollTimer, 0f, 2.0f);
//        _isGroundPounding = false;
//        blockMovement = false;

//        _moveVelocity.x = (_isFacingRight) ? MoveStats.RollSpeed : -MoveStats.RollSpeed;
//        _moveVelocity.y = 0;

//        // --- USING STATS.CS CHANGE ---
//        //// Rolltime = height travelled / a given amount (ALGORITHM TBD)
//        //_rollTimer = _rollInitTimer + (lastPos.y - _rb.position.y) / 50f;

//        //_moveVelocity.x = (_isFacingRight) ? 50 : -50;                      // Depending on direction facing, move to specified direction for XX amount of seconds!
//        //_moveVelocity.y = 0;

//        _rb.linearVelocity = new Vector2(0, 0);
//    }

//    void Roll()
//    {
//        if (_isRolling)
//        {
//            // [WIP] change hitbox (deactivate curr one and activate smoller one/ resize hitbox via code)
//            // thinking height should affect how long you have the speed vs how fast you go, for consistency
//            // once roll timer ends, player goes back to running at full speed
//            // cancellable via jump/ dash

//            // !!!  need change this to upon wall contact, canWallKick applies for only if !grounded  !!!
//            // can possibly just change to returning out of roll state if !grounded, playtest required!
//            if (canWallKick)
//            {
//                StopRoll();
//                return; // Exit the function immediately
//            }

//            // Disable player control until jump or dash (maybe we can let players roll in air but idk tbh)
//            if (_rollTimer <= 0 || _isDashing)
//            {
//                blockMovement = false;
//                StopRoll();
//            }
//            else
//            {
//                // bonus speed and smaller hitbox here!
//                _rollTimer -= Time.deltaTime;
//                //Vector2 moveVec = new Vector2(_moveVelocity.x, 0);
//            }
//        }
//    }

//    void StopRoll()
//    {
//        if (!_isRolling) return;

//        // --- USING STATS.CS CHANGE ---
//        //_rollTimer = MoveStats.BaseRollDuration;                    // Reset timers, restore control to players

//        _isRolling = false;
//        blockMovement = false;

//        // [WIP] check if player can run at full speed (no need to re-accelerate) after roll (possibly bonus speed?)

//        //_rb.linearVelocity += new Vector2(0, _rb.linearVelocity.y);         // stop horizontal movement immediately
//    }

//    #endregion

//    #region Air Glide

//    void InitiateGlide()
//    {
//        // play air glide audio
//        if (sounds != null && sounds.airGlide != null) AudioManager.instance.PlayPlayerSFX(sounds.airGlide);

//        if (_isGliding || _glideCooldownTimer > 0) return;              // If cooldown not ended

//        _isGliding = true;

//        // player slowly descends downwards
//        //_moveVelocity.y = 10f;                                          // Honestly idt this rly makes a difference? since we scaling the verticalvelocity alr
//        //_rb.linearVelocity = new Vector2(0, 0);
//    }

//    void Glide()
//    {
//        // If player is in air and holds SPACE(but we have double jump now so change to 1 jump first!), enter glide state
//        // slowly descends, hovering in the air, can still control horizontal direction
//        // on spacebar release/ dash/ grounded, player goes back into normal movement state

//        // updates cooldown (since this runs in update())
//        if (_glideCooldownTimer > 0)
//        {
//            _glideCooldownTimer -= Time.deltaTime;
//        }

//        // if cooldown ready + input pressed
//        if (_isGliding)
//        {
//            // cooldown ready
//            // cancel if hit ground or dash
//            if (_isGrounded || _isDashing)
//            {
//                StopGlide();
//            }
//            //else
//            //{
//            //    // only apply the glide effect when falling
//            //    if (VerticalVelocity < 0)
//            //    {
//            //        VerticalVelocity *= MoveStats.GlideFallSpeedMultiplier;
//            //    }

//            //    // --- USING STATS.CS CHANGE ---
//            //    //// apply gliding movement here
//            //    //VerticalVelocity *= .5f;
//            //    //_rb.linearVelocity += new Vector2(0, _moveVelocity.y);
//            //}

//            else
//            {
//                // Timer for periodic time deduction
//                _timeSinceLastGlideDeduction += Time.deltaTime;
//                if (_timeSinceLastGlideDeduction >= MoveStats.GlideDeductionInterval)
//                {
//                    fGlide = true; // Set the flag for the TimerManager
//                    _timeSinceLastGlideDeduction = 0f; // Reset the timer
//                }

//                // only apply the glide effect when falling
//                if (VerticalVelocity < 0)
//                {
//                    VerticalVelocity *= MoveStats.GlideFallSpeedMultiplier;
//                }
//            }
//        }

//    }

//    void StopGlide()
//    {
//        // --- USING STATS.CS CHANGE ---
//        //_glideCooldownTimer = _glideInitTimer;

//        // reset timers - adjust flags - return control to player
//        _glideCooldownTimer = MoveStats.GlideCooldown;
//        _timeSinceLastGlideDeduction = 0f;
//        _isGliding = false;
//        blockMovement = false;
//    }


//    #endregion

//    #region Zipline

//    private void HandleZiplineInput()
//    {
//        // condition for starting is the same inside trigger or in coyote time
//        if (InputManager.ZiplineWasPressed && !_isZiplining && (_availableZipline != null || _ziplineCoyoteTimer > 0))
//        {
//            Zipline targetZipline = _availableZipline != null ? _availableZipline : _lastAvailableZipline;
//            if (targetZipline == null) return;

//            _ziplineCoyoteTimer = 0f;

//            Vector2 startPos = FindClosestPointOnLineSegment(targetZipline.WorldStartPoint, targetZipline.WorldEndPoint, transform.position);
//            Vector2 endPos;

//            // determine which of the zipline's ends is physically on the right side in world space
//            Vector2 rightMostPoint = (targetZipline.WorldEndPoint.x > targetZipline.WorldStartPoint.x)
//                ? targetZipline.WorldEndPoint
//                : targetZipline.WorldStartPoint;

//            Vector2 leftMostPoint = (targetZipline.WorldEndPoint.x > targetZipline.WorldStartPoint.x)
//                ? targetZipline.WorldStartPoint
//                : targetZipline.WorldEndPoint;

//            // choose the destination based on the direction player is currently facing
//            if (IsFacingRight)
//            {
//                // if facing right - go to rightmost point
//                endPos = rightMostPoint;
//            }
//            else
//            {
//                // if facing left - go to leftmost point
//                endPos = leftMostPoint;
//            }

//            if (_ziplineCoroutine != null)
//            {
//                StopCoroutine(_ziplineCoroutine);
//            }
//            //_ziplineCoroutine = StartCoroutine(ZiplineCoroutine(startPos, endPos));
//            _ziplineCoroutine = StartCoroutine(ZiplineCoroutine(targetZipline, startPos, endPos));
//        }
//    }

//    private Vector2 FindClosestPointOnLineSegment(Vector2 lineStart, Vector2 lineEnd, Vector2 point)
//    {
//        Vector2 lineDirection = lineEnd - lineStart;
//        float lineLengthSquared = lineDirection.sqrMagnitude;

//        if (lineLengthSquared == 0.0f)
//        {
//            return lineStart;
//        }

//        // project point onto the line and clamp it to the segment
//        float t = Vector2.Dot(point - lineStart, lineDirection) / lineLengthSquared;
//        t = Mathf.Clamp01(t);

//        return lineStart + t * lineDirection;
//    }

//    private void OnTriggerEnter2D(Collider2D other)
//    {
//        // check if the other collider has a Zipline component
//        if (other.TryGetComponent<Zipline>(out Zipline zipline))
//        {
//            _availableZipline = zipline;

//            // store has most recent zipline touched
//            _lastAvailableZipline = zipline;
//        }
//    }

//    private void OnTriggerExit2D(Collider2D other)
//    {
//        // check if exiting the trigger of the currently available zipline
//        if (other.TryGetComponent<Zipline>(out Zipline zipline) && zipline == _availableZipline)
//        {
//            _availableZipline = null;

//            // start coyote timer
//            _ziplineCoyoteTimer = MoveStats.ZiplineCoyoteTime;
//        }
//    }

//    private IEnumerator ZiplineCoroutine(Zipline currentZipline, Vector2 startPos, Vector2 endPos)
//    {
//        _isZiplining = true;

//        // play zipline audio
//        if (sounds != null && sounds.zipline != null) AudioManager.instance.PlayPlayerSFX(sounds.zipline);

//        currentZipline.PlayDebrisEffect();

//        VerticalVelocity = 0f;
//        _rb.linearVelocity = Vector2.zero;
//        _rb.bodyType = RigidbodyType2D.Kinematic;
//        blockMovement = true;

//        Vector3 initialPlayerPosition = transform.position;
//        Vector3 targetPositionOnZipline = new Vector3(startPos.x, startPos.y - _ziplineHeadOffset, transform.position.z);

//        float transitionElapsedTime = 0f;

//        // CHANGED: Use MoveStats.ZiplineAttachDuration
//        while (transitionElapsedTime < MoveStats.ZiplineAttachDuration)
//        {
//            // CHANGED: Use MoveStats.ZiplineAttachDuration
//            transform.position = Vector3.Lerp(initialPlayerPosition, targetPositionOnZipline, transitionElapsedTime / MoveStats.ZiplineAttachDuration);
//            transitionElapsedTime += Time.deltaTime;
//            yield return null;
//        }

//        transform.position = targetPositionOnZipline;

//        float travelDirectionX = endPos.x - startPos.x;
//        if (travelDirectionX > 0 && !IsFacingRight) Turn(true);
//        else if (travelDirectionX < 0 && IsFacingRight) Turn(false);

//        float originalZ = transform.position.z;
//        float journeyLength = Vector2.Distance(startPos, endPos);
//        if (journeyLength <= 0)
//        {
//            _isZiplining = false;
//            blockMovement = false;
//            _rb.bodyType = RigidbodyType2D.Dynamic;
//            currentZipline.StopDebrisEffect();
//            yield break;
//        }

//        float travelTime = journeyLength / MoveStats.ZiplineSpeed;
//        float elapsedTime = 0f;
//        float timeSinceLastDeduction = 0f;

//        while (elapsedTime < travelTime && InputManager.ZiplineIsHeld)
//        {
//            elapsedTime += Time.deltaTime;
//            timeSinceLastDeduction += Time.deltaTime;

//            if (timeSinceLastDeduction >= MoveStats.ZiplineDeductionInterval)
//            {
//                fZipline = true;
//                timeSinceLastDeduction = 0f;
//            }

//            Vector2 newPosition = Vector2.Lerp(startPos, endPos, elapsedTime / travelTime);
//            currentZipline.UpdateDebrisEffectPosition(newPosition);
//            transform.position = new Vector3(newPosition.x, newPosition.y - _ziplineHeadOffset, originalZ);

//            yield return null;
//        }

//        currentZipline.StopDebrisEffect();
//        _rb.bodyType = RigidbodyType2D.Dynamic;

//        float horizontalDirection = Mathf.Sign(endPos.x - startPos.x);
//        StartCoroutine(LaunchFromZiplineCoroutine(horizontalDirection));

//        _isZiplining = false;
//        blockMovement = false;
//    }


//    private IEnumerator LaunchFromZiplineCoroutine(float horizontalDirection)
//    {
//        _isBeingLaunched = true;
//        float elapsedTime = 0f;

//        while (elapsedTime < MoveStats.ZiplineLaunchDuration)
//        {
//            elapsedTime += Time.deltaTime;

//            // use AnimationCurve to get force multipler for the current time - normalise elapsedTime (from 0 to 1) to properly evaluate the curve
//            float curveMultiplier = MoveStats.ZiplineLaunchCurve.Evaluate(elapsedTime / MoveStats.ZiplineLaunchDuration);

//            // calculate force for this frame
//            Vector2 launchForce = new Vector2(
//                MoveStats.ZiplineLaunchForce.x * horizontalDirection,
//                MoveStats.ZiplineLaunchForce.y
//            );

//            // apply force - scaled by curve's value for this point in time
//            _rb.linearVelocity = launchForce * curveMultiplier;

//            // sync custom VerticalVelocity so Jump() does not conflict after launch ends
//            VerticalVelocity = _rb.linearVelocity.y;

//            yield return null;
//        }

//        _isBeingLaunched = false;
//    }

//    #endregion

//    #region Pickups - Speed Boost

//    public void ApplySpeedBoost(float multiplier, float duration)
//    {
//        // if boost already active - stop it before starting new one
//        if (_speedBoostCoroutine != null)
//        {
//            StopCoroutine(_speedBoostCoroutine);
//        }

//        _speedBoostCoroutine = StartCoroutine(SpeedBoostCoroutine(multiplier, duration));
//    }

//    private IEnumerator SpeedBoostCoroutine(float multiplier, float duration)
//    {
//        // apply speed multiplier
//        SpeedMultiplier = multiplier;

//        // wait for specified duration
//        yield return new WaitForSeconds(duration);

//        // reset multiplier back to default value
//        SpeedMultiplier = 1f;

//        // clear coroutine reference
//        _speedBoostCoroutine = null;
//    }

//    #endregion
//}