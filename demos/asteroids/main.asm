; ============================================================================
; Asteroids - Main Entry Point
; ============================================================================
; A simple Asteroids-style game demonstrating modular assembly programming.
;
; Controls:
;   Left Arrow  - Rotate counter-clockwise
;   Right Arrow - Rotate clockwise
;   Up Arrow    - Thrust
;   Space       - Fire
;
; Modules:
;   hardware.asm  - Hardware register constants
;   variables.asm - Zero-page variable definitions
;   graphics.asm  - Graphics subroutines
;   tables.asm    - Direction and velocity lookup tables
;   input.asm     - Input handling
;   ship.asm      - Ship logic and rendering
;   bullet.asm    - Bullet logic and rendering
;   asteroid.asm  - Asteroid logic and rendering
; ============================================================================

; Include all module files
.include "hardware.asm"
.include "variables.asm"

.org $8000

; ============================================================================
; Entry Point
; ============================================================================
start:
    ; Initialize stack pointer
    LDX #$FF
    TXS

    ; Initialize all game objects
    JSR ship_init
    JSR bullet_init
    JSR asteroid_init

; ============================================================================
; Main Game Loop
; ============================================================================
main_loop:
    ; --- Input Phase ---
    JSR handle_input

    ; --- Update Phase ---
    JSR ship_update
    JSR bullet_update

    ; --- Render Phase ---
    JSR gfx_clear
    JSR ship_draw
    JSR bullet_draw
    JSR asteroid_draw
    JSR gfx_frame

    ; Loop forever
    JMP main_loop

; ============================================================================
; Rate Limiting Constants
; ============================================================================
.equ ROTATE_DELAY,  $04     ; Frames between rotations (4 = ~15 rotations/sec)
.equ THRUST_DELAY,  $02     ; Frames between thrust applications

; ============================================================================
; Input Handler
; ============================================================================
; Polls input and handles continuous key holding with rate limiting.
; Processes all held keys each frame (rotation, thrust, fire).
; ============================================================================
handle_input:
    ; --- Update timers ---
    ; Decrement rotate_timer if > 0
    LDA rotate_timer
    BEQ rotate_timer_done
    SEC
    SBC #$01
    STA rotate_timer
rotate_timer_done:

    ; Decrement thrust_timer if > 0
    LDA thrust_timer
    BEQ thrust_timer_done
    SEC
    SBC #$01
    STA thrust_timer
thrust_timer_done:

    ; --- Poll input ---
    JSR input_poll
    BEQ handle_input_done   ; No input available

    ; --- Check rotation (left/right) ---
    ; Only rotate if timer is 0
    LDA rotate_timer
    BNE check_thrust        ; Timer not ready, skip rotation

    ; Check for left arrow (rotate CCW)
    JSR input_is_left
    BNE check_right_rot
    JSR ship_rotate_ccw
    LDA #ROTATE_DELAY
    STA rotate_timer        ; Reset timer
    JMP check_thrust

check_right_rot:
    ; Check for right arrow (rotate CW)
    JSR input_is_right
    BNE check_thrust
    JSR ship_rotate_cw
    LDA #ROTATE_DELAY
    STA rotate_timer        ; Reset timer

check_thrust:
    ; --- Check thrust (up arrow) ---
    ; Only thrust if timer is 0
    LDA thrust_timer
    BNE check_fire          ; Timer not ready, skip thrust

    JSR input_is_thrust
    BNE check_fire
    JSR ship_apply_thrust
    LDA #THRUST_DELAY
    STA thrust_timer        ; Reset timer

check_fire:
    ; --- Check fire (space) ---
    ; Fire has its own cooldown (bullet_active), no extra timer needed
    JSR input_is_fire
    BNE handle_input_done
    JSR bullet_fire

handle_input_done:
    RTS

; ============================================================================
; Include Module Code (must come after main code to avoid address conflicts)
; ============================================================================
.include "graphics.asm"
.include "input.asm"
.include "ship.asm"
.include "bullet.asm"
.include "asteroid.asm"

; ============================================================================
; Data Tables (at end of ROM)
; ============================================================================
.include "tables.asm"
