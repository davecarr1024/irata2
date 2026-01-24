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
;   Escape      - Exit game
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
    JSR debug_draw_input
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

    ; --- Check escape (exit game) ---
    JSR input_is_esc
    BNE check_rotation
    HLT                     ; Exit game

check_rotation:
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
; Debug Input Overlay
; ============================================================================
; Draws a small set of points that light up when keys are held.
; Positions are fixed near the top-left corner.
; Uses: cached_key
; Modifies: A
; ============================================================================
debug_draw_input:
    ; Control line (always visible)
    LDA #COLOR_WHITE
    STA draw_color
    LDA #$04
    STA draw_x0
    LDA #$06
    STA draw_y0
    LDA #$2C
    STA draw_x1
    LDA #$06
    STA draw_y1
    JSR gfx_line

    ; Left
    LDA cached_key
    AND #STATE_LEFT
    BEQ debug_right
    LDA #COLOR_WHITE
    STA draw_color
    LDA #$08
    STA draw_x0
    STA draw_x1
    LDA #$06
    STA draw_y0
    LDA #$0E
    STA draw_y1
    JSR gfx_line

debug_right:
    LDA cached_key
    AND #STATE_RIGHT
    BEQ debug_up
    LDA #COLOR_WHITE
    STA draw_color
    LDA #$10
    STA draw_x0
    STA draw_x1
    LDA #$06
    STA draw_y0
    LDA #$0E
    STA draw_y1
    JSR gfx_line

debug_up:
    LDA cached_key
    AND #STATE_UP
    BEQ debug_space
    LDA #COLOR_WHITE
    STA draw_color
    LDA #$18
    STA draw_x0
    STA draw_x1
    LDA #$06
    STA draw_y0
    LDA #$0E
    STA draw_y1
    JSR gfx_line

debug_space:
    LDA cached_key
    AND #STATE_SPACE
    BEQ debug_esc
    LDA #COLOR_WHITE
    STA draw_color
    LDA #$20
    STA draw_x0
    STA draw_x1
    LDA #$06
    STA draw_y0
    LDA #$0E
    STA draw_y1
    JSR gfx_line

debug_esc:
    LDA cached_key
    AND #STATE_ESC
    BEQ debug_done
    LDA #COLOR_WHITE
    STA draw_color
    LDA #$28
    STA draw_x0
    STA draw_x1
    LDA #$06
    STA draw_y0
    LDA #$0E
    STA draw_y1
    JSR gfx_line

debug_done:
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
