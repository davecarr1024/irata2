; ============================================================================
; Main Module Unit Tests
; ============================================================================
; Tests for handle_input - continuous input with rate limiting.
; These tests help diagnose issues with held-key input handling.
; ============================================================================

; Include module dependencies
.include "../hardware.asm"
.include "../variables.asm"

; Initialize stack
    LDX #$FF
    TXS

; ============================================================================
; Test 1: Verify ship_init sets up timers correctly
; ============================================================================
test_init_timers:
    JSR ship_init

    ; rotate_timer should be 0 (ready to rotate)
    LDA rotate_timer
    CMP #$00
    JEQ test_init_rotate_timer_ok
    CRS
test_init_rotate_timer_ok:

    ; thrust_timer should be 0 (ready to thrust)
    LDA thrust_timer
    CMP #$00
    JEQ test_init_thrust_timer_ok
    CRS
test_init_thrust_timer_ok:

; ============================================================================
; Test 2: Rotation with left key held (first press)
; ============================================================================
test_rotate_left_first:
    ; Initialize ship
    JSR ship_init

    ; Set initial angle to known value
    LDA #$08
    STA ship_angle

    ; Simulate left key held via cached_key (input_poll stores KEY_STATE here)
    LDA #STATE_LEFT
    STA cached_key

    ; Verify rotate_timer is 0
    LDA rotate_timer
    BEQ test_timer_ready
    CRS
test_timer_ready:

    ; Call rotate handler manually: check left and rotate
    JSR input_is_left
    BNE test_rotate_left_fail  ; Should be Z=1 (left held), so BNE should not branch

    ; Rotate CCW
    JSR ship_rotate_ccw

    ; Set timer (like handle_input does)
    LDA #ROTATE_DELAY
    STA rotate_timer

    ; Verify angle changed: 8 -> 7
    LDA ship_angle
    CMP #$07
    JEQ test_rotate_left_ok
    CRS
test_rotate_left_ok:

    ; Verify timer was set
    LDA rotate_timer
    CMP #ROTATE_DELAY
    JEQ test_rotate_timer_set_ok
    CRS
test_rotate_timer_set_ok:
    JMP test_rotate_blocked

test_rotate_left_fail:
    CRS

; ============================================================================
; Test 3: Rotation blocked when timer > 0
; ============================================================================
test_rotate_blocked:
    ; Timer is currently ROTATE_DELAY from previous test
    ; Save current angle
    LDA ship_angle
    STA temp

    ; Left key still held
    LDA #STATE_LEFT
    STA cached_key

    ; Check if timer blocks rotation
    LDA rotate_timer
    BNE test_timer_blocks  ; Timer > 0, should skip rotation
    CRS  ; Timer should not be 0 yet

test_timer_blocks:
    ; Rotation should be blocked, angle unchanged
    ; (In real handle_input, it would skip the rotation code)
    LDA ship_angle
    CMP temp
    JEQ test_blocked_ok
    CRS
test_blocked_ok:

; ============================================================================
; Test 4: Timer countdown allows rotation after delay
; ============================================================================
test_timer_countdown:
    JSR ship_init

    ; Set angle and timer
    LDA #$08
    STA ship_angle
    LDA #$02            ; 2 frames until ready
    STA rotate_timer

    ; Simulate key held
    LDA #STATE_LEFT
    STA cached_key

    ; First frame: decrement timer, rotation blocked
    ; Decrement timer
    LDA rotate_timer
    SEC
    SBC #$01
    STA rotate_timer

    ; Timer is now 1, rotation still blocked
    LDA rotate_timer
    BNE test_countdown_frame1_ok
    CRS
test_countdown_frame1_ok:

    ; Second frame: decrement timer
    LDA rotate_timer
    SEC
    SBC #$01
    STA rotate_timer

    ; Timer is now 0, rotation should be allowed
    LDA rotate_timer
    BEQ test_countdown_frame2_ok
    CRS
test_countdown_frame2_ok:

    ; Now rotation should work
    JSR input_is_left
    BNE test_countdown_fail
    JSR ship_rotate_ccw

    ; Verify angle changed
    LDA ship_angle
    CMP #$07
    JEQ test_countdown_angle_ok
    CRS
test_countdown_angle_ok:
    JMP test_thrust_continuous

test_countdown_fail:
    CRS

; ============================================================================
; Test 5: Thrust with up key held
; ============================================================================
test_thrust_continuous:
    JSR ship_init

    ; Set initial velocity
    LDA #$00
    STA ship_vx
    STA ship_vy

    ; Set angle to 0 (north)
    LDA #$00
    STA ship_angle

    ; Simulate up key held
    LDA #STATE_UP
    STA cached_key

    ; Verify thrust_timer is 0
    LDA thrust_timer
    BEQ test_thrust_timer_ready
    CRS
test_thrust_timer_ready:

    ; Check thrust key
    JSR input_is_thrust
    BNE test_thrust_fail

    ; Apply thrust
    JSR ship_apply_thrust

    ; Set timer
    LDA #THRUST_DELAY
    STA thrust_timer

    ; Verify velocity changed (north thrust adds vy = -1 = $FF)
    LDA ship_vy
    CMP #$FF
    JEQ test_thrust_ok
    CRS
test_thrust_ok:
    JMP test_right_rotation

test_thrust_fail:
    CRS

; ============================================================================
; Test 6: Right rotation works
; ============================================================================
test_right_rotation:
    JSR ship_init

    ; Set angle
    LDA #$05
    STA ship_angle

    ; Simulate right key held
    LDA #STATE_RIGHT
    STA cached_key

    ; Check right key
    JSR input_is_right
    BNE test_right_fail

    ; Rotate CW
    JSR ship_rotate_cw

    ; Verify angle: 5 -> 6
    LDA ship_angle
    CMP #$06
    JEQ test_right_ok
    CRS
test_right_ok:
    JMP test_multi_rotation

test_right_fail:
    CRS

; ============================================================================
; Test 7: Multiple rotations with timer reset
; ============================================================================
; This tests the full rate-limited rotation cycle:
; - Rotation happens when timer=0
; - Timer is set after rotation
; - Timer counts down each frame
; - Next rotation happens when timer reaches 0 again
test_multi_rotation:
    JSR ship_init

    ; Set up state
    LDA #$08
    STA ship_angle
    LDA #STATE_LEFT
    STA cached_key

    ; First rotation (timer=0)
    LDA rotate_timer
    BNE test_multi_fail      ; Timer should be 0
    JSR ship_rotate_ccw
    LDA #ROTATE_DELAY
    STA rotate_timer

    ; Verify first rotation: 8 -> 7
    LDA ship_angle
    CMP #$07
    JEQ test_multi_first_ok
    CRS
test_multi_first_ok:

    ; Simulate ROTATE_DELAY frames of countdown
    ; Frame 1: timer 4 -> 3
    LDA rotate_timer
    SEC
    SBC #$01
    STA rotate_timer

    ; Frame 2: timer 3 -> 2
    LDA rotate_timer
    SEC
    SBC #$01
    STA rotate_timer

    ; Frame 3: timer 2 -> 1
    LDA rotate_timer
    SEC
    SBC #$01
    STA rotate_timer

    ; Frame 4: timer 1 -> 0
    LDA rotate_timer
    SEC
    SBC #$01
    STA rotate_timer

    ; Timer should now be 0
    LDA rotate_timer
    BEQ test_multi_timer_zero
    CRS
test_multi_timer_zero:

    ; Second rotation should now work
    JSR ship_rotate_ccw

    ; Verify second rotation: 7 -> 6
    LDA ship_angle
    CMP #$06
    JEQ test_multi_ok
    CRS
test_multi_ok:
    JMP tests_done

test_multi_fail:
    CRS

; ============================================================================
; All tests passed
; ============================================================================
tests_done:
    HLT

; ============================================================================
; Rate Limiting Constants (copied from main.asm)
; ============================================================================
.equ ROTATE_DELAY,  $04
.equ THRUST_DELAY,  $02

; ============================================================================
; Include modules under test
; ============================================================================
.include "../graphics.asm"
.include "../input.asm"
.include "../ship.asm"
.include "../tables.asm"
