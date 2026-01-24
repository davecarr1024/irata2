; ============================================================================
; Input Module
; ============================================================================
; Handles input polling and processing for the asteroids game.
;
; Interface:
;   input_poll      - Poll input, store key in cached_key, returns with Z=1 if no input
;   input_is_left   - Check if cached_key is left arrow, Z=1 if true
;   input_is_right  - Check if cached_key is right arrow, Z=1 if true
;   input_is_thrust - Check if cached_key is thrust (up), Z=1 if true
;   input_is_fire   - Check if cached_key is fire (space), Z=1 if true
;
; Uses: cached_key variable
; Modifies: A
; Preserves: X, Y
; ============================================================================

; ----------------------------------------------------------------------------
; input_poll - Poll for input
; ----------------------------------------------------------------------------
; Checks if input is available and reads the key code.
; Output: cached_key contains key code (or 0 if none)
;         Z flag: 1 = no input, 0 = input available
; Modifies: A
; Preserves: X, Y
; ----------------------------------------------------------------------------
input_poll:
    LDA INPUT_STATUS
    AND #$01
    BEQ input_poll_none
    ; Input available, read key
    LDA INPUT_KEY
    STA cached_key
    ; Clear Z flag (we have input)
    LDA #$01
    RTS

input_poll_none:
    LDA #$00
    STA cached_key
    ; Z flag is already set from AND #$01
    RTS

; ----------------------------------------------------------------------------
; input_is_left - Check if key is left arrow
; ----------------------------------------------------------------------------
; Output: Z=1 if cached_key == KEY_LEFT
; Modifies: A
; ----------------------------------------------------------------------------
input_is_left:
    LDA cached_key
    CMP #KEY_LEFT
    RTS

; ----------------------------------------------------------------------------
; input_is_right - Check if key is right arrow
; ----------------------------------------------------------------------------
; Output: Z=1 if cached_key == KEY_RIGHT
; Modifies: A
; ----------------------------------------------------------------------------
input_is_right:
    LDA cached_key
    CMP #KEY_RIGHT
    RTS

; ----------------------------------------------------------------------------
; input_is_thrust - Check if key is thrust (up arrow)
; ----------------------------------------------------------------------------
; Output: Z=1 if cached_key == KEY_UP
; Modifies: A
; ----------------------------------------------------------------------------
input_is_thrust:
    LDA cached_key
    CMP #KEY_UP
    RTS

; ----------------------------------------------------------------------------
; input_is_fire - Check if key is fire (space)
; ----------------------------------------------------------------------------
; Output: Z=1 if cached_key == KEY_SPACE
; Modifies: A
; ----------------------------------------------------------------------------
input_is_fire:
    LDA cached_key
    CMP #KEY_SPACE
    RTS
