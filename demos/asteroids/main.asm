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
; Input Handler
; ============================================================================
; Polls input and dispatches to appropriate handlers.
; ============================================================================
handle_input:
    JSR input_poll
    BEQ handle_input_done   ; No input

    ; Check for left arrow (rotate CCW)
    JSR input_is_left
    BNE check_right
    JSR ship_rotate_ccw
    JMP handle_input_done

check_right:
    ; Check for right arrow (rotate CW)
    JSR input_is_right
    BNE check_thrust
    JSR ship_rotate_cw
    JMP handle_input_done

check_thrust:
    ; Check for up arrow (thrust)
    JSR input_is_thrust
    BNE check_fire
    JSR ship_apply_thrust
    JMP handle_input_done

check_fire:
    ; Check for space (fire)
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
