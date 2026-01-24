; ============================================================================
; Input Module
; ============================================================================
; Handles input polling and processing for the asteroids game.
; Uses KEY_STATE register for continuous input detection.
;
; Interface:
;   input_poll      - Read key state, returns with Z=1 if no keys held
;   input_is_left   - Check if left arrow is held, Z=1 if true
;   input_is_right  - Check if right arrow is held, Z=1 if true
;   input_is_thrust - Check if up arrow is held, Z=1 if true
;   input_is_fire   - Check if space is held, Z=1 if true
;
; Uses: cached_key variable (stores KEY_STATE bitmask)
; Modifies: A
; Preserves: X, Y
; ============================================================================

; ----------------------------------------------------------------------------
; input_poll - Poll for input
; ----------------------------------------------------------------------------
; Reads the KEY_STATE register to check currently held keys.
; Output: cached_key contains key state bitmask (or 0 if none)
;         Z flag: 1 = no keys held, 0 = at least one key held
; Modifies: A
; Preserves: X, Y
; ----------------------------------------------------------------------------
input_poll:
    LDA KEY_STATE
    STA cached_key
    ; Z flag is set if A == 0 (no keys held)
    RTS

; ----------------------------------------------------------------------------
; input_is_left - Check if left arrow is held
; ----------------------------------------------------------------------------
; Output: Z=1 if left arrow is currently held
; Modifies: A
; ----------------------------------------------------------------------------
input_is_left:
    LDA cached_key
    AND #STATE_LEFT
    ; If left is held, A != 0, so Z=0 (not equal)
    ; We want Z=1 when held, so invert
    EOR #STATE_LEFT
    RTS

; ----------------------------------------------------------------------------
; input_is_right - Check if right arrow is held
; ----------------------------------------------------------------------------
; Output: Z=1 if right arrow is currently held
; Modifies: A
; ----------------------------------------------------------------------------
input_is_right:
    LDA cached_key
    AND #STATE_RIGHT
    EOR #STATE_RIGHT
    RTS

; ----------------------------------------------------------------------------
; input_is_thrust - Check if up arrow is held
; ----------------------------------------------------------------------------
; Output: Z=1 if up arrow is currently held
; Modifies: A
; ----------------------------------------------------------------------------
input_is_thrust:
    LDA cached_key
    AND #STATE_UP
    EOR #STATE_UP
    RTS

; ----------------------------------------------------------------------------
; input_is_fire - Check if space is held
; ----------------------------------------------------------------------------
; Output: Z=1 if space is currently held
; Modifies: A
; ----------------------------------------------------------------------------
input_is_fire:
    LDA cached_key
    AND #STATE_SPACE
    EOR #STATE_SPACE
    RTS
