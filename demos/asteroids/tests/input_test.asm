; ============================================================================
; Input Module Unit Tests
; ============================================================================
; Tests for input_is_left, input_is_right, input_is_thrust, input_is_fire,
; input_is_esc
; Note: We test the key checking functions by setting cached_key directly,
; since KEY_STATE is memory-mapped I/O that may not be writable in tests.
; ============================================================================

; Include module dependencies
.include "../hardware.asm"
.include "../variables.asm"

; Initialize stack
    LDX #$FF
    TXS

; ============================================================================
; Test 1: input_is_left returns Z=1 when left is held
; ============================================================================
test_is_left_held:
    ; Set cached_key with STATE_LEFT bit set
    LDA #STATE_LEFT
    STA cached_key

    JSR input_is_left

    ; Z flag should be set (key is held)
    BEQ test_left_held_ok
    CRS
test_left_held_ok:

; ============================================================================
; Test 2: input_is_left returns Z=0 when left is not held
; ============================================================================
test_is_left_not_held:
    ; Set cached_key without STATE_LEFT
    LDA #STATE_RIGHT
    STA cached_key

    JSR input_is_left

    ; Z flag should be clear (key not held)
    BNE test_left_not_held_ok
    CRS
test_left_not_held_ok:

; ============================================================================
; Test 3: input_is_right returns Z=1 when right is held
; ============================================================================
test_is_right_held:
    ; Set cached_key with STATE_RIGHT bit set
    LDA #STATE_RIGHT
    STA cached_key

    JSR input_is_right

    ; Z flag should be set (key is held)
    BEQ test_right_held_ok
    CRS
test_right_held_ok:

; ============================================================================
; Test 4: input_is_right returns Z=0 when right is not held
; ============================================================================
test_is_right_not_held:
    ; Set cached_key without STATE_RIGHT
    LDA #STATE_LEFT
    STA cached_key

    JSR input_is_right

    ; Z flag should be clear (key not held)
    BNE test_right_not_held_ok
    CRS
test_right_not_held_ok:

; ============================================================================
; Test 5: input_is_thrust returns Z=1 when up is held
; ============================================================================
test_is_thrust_held:
    ; Set cached_key with STATE_UP bit set
    LDA #STATE_UP
    STA cached_key

    JSR input_is_thrust

    ; Z flag should be set (key is held)
    BEQ test_thrust_held_ok
    CRS
test_thrust_held_ok:

; ============================================================================
; Test 6: input_is_thrust returns Z=0 when up is not held
; ============================================================================
test_is_thrust_not_held:
    ; Set cached_key without STATE_UP
    LDA #STATE_DOWN
    STA cached_key

    JSR input_is_thrust

    ; Z flag should be clear (key not held)
    BNE test_thrust_not_held_ok
    CRS
test_thrust_not_held_ok:

; ============================================================================
; Test 7: input_is_fire returns Z=1 when space is held
; ============================================================================
test_is_fire_held:
    ; Set cached_key with STATE_SPACE bit set
    LDA #STATE_SPACE
    STA cached_key

    JSR input_is_fire

    ; Z flag should be set (key is held)
    BEQ test_fire_held_ok
    CRS
test_fire_held_ok:

; ============================================================================
; Test 8: input_is_fire returns Z=0 when space is not held
; ============================================================================
test_is_fire_not_held:
    ; Set cached_key without STATE_SPACE
    LDA #STATE_UP
    STA cached_key

    JSR input_is_fire

    ; Z flag should be clear (key not held)
    BNE test_fire_not_held_ok
    CRS
test_fire_not_held_ok:

; ============================================================================
; Test 9: Multiple keys can be held simultaneously
; ============================================================================
test_multiple_keys:
    ; Set cached_key with LEFT and UP held
    LDA #STATE_LEFT
    ORA #STATE_UP
    STA cached_key

    ; Check left is held
    JSR input_is_left
    BEQ test_multi_left_ok
    CRS
test_multi_left_ok:

    ; Check thrust is held
    JSR input_is_thrust
    BEQ test_multi_thrust_ok
    CRS
test_multi_thrust_ok:

    ; Check right is NOT held
    JSR input_is_right
    BNE test_multi_right_ok
    CRS
test_multi_right_ok:

    ; Check fire is NOT held
    JSR input_is_fire
    BNE test_multi_fire_ok
    CRS
test_multi_fire_ok:

; ============================================================================
; Test 10: No keys held
; ============================================================================
test_no_keys:
    ; Set cached_key to 0 (no keys)
    LDA #$00
    STA cached_key

    ; All checks should return Z=0 (not held)
    JSR input_is_left
    BNE test_no_left_ok
    CRS
test_no_left_ok:

    JSR input_is_right
    BNE test_no_right_ok
    CRS
test_no_right_ok:

    JSR input_is_thrust
    BNE test_no_thrust_ok
    CRS
test_no_thrust_ok:

    JSR input_is_fire
    BNE test_no_fire_ok
    CRS
test_no_fire_ok:

; ============================================================================
; Test 11: All keys held
; ============================================================================
test_all_keys:
    ; Set cached_key with all keys
    LDA #STATE_UP
    ORA #STATE_DOWN
    ORA #STATE_LEFT
    ORA #STATE_RIGHT
    ORA #STATE_SPACE
    STA cached_key

    ; All relevant checks should return Z=1 (held)
    JSR input_is_left
    BEQ test_all_left_ok
    CRS
test_all_left_ok:

    JSR input_is_right
    BEQ test_all_right_ok
    CRS
test_all_right_ok:

    JSR input_is_thrust
    BEQ test_all_thrust_ok
    CRS
test_all_thrust_ok:

    JSR input_is_fire
    BEQ test_all_fire_ok
    CRS
test_all_fire_ok:

; ============================================================================
; Test 12: input_is_esc returns Z=1 when escape is held
; ============================================================================
test_is_esc_held:
    ; Set cached_key with STATE_ESC bit set
    LDA #STATE_ESC
    STA cached_key

    JSR input_is_esc

    ; Z flag should be set (key is held)
    BEQ test_esc_held_ok
    CRS
test_esc_held_ok:

; ============================================================================
; Test 13: input_is_esc returns Z=0 when escape is not held
; ============================================================================
test_is_esc_not_held:
    ; Set cached_key without STATE_ESC
    LDA #STATE_UP
    STA cached_key

    JSR input_is_esc

    ; Z flag should be clear (key not held)
    BNE test_esc_not_held_ok
    CRS
test_esc_not_held_ok:

; ============================================================================
; Test 14: ESC can be held with other keys
; ============================================================================
test_esc_with_others:
    ; Set cached_key with ESC and LEFT
    LDA #STATE_ESC
    ORA #STATE_LEFT
    STA cached_key

    ; Check ESC is held
    JSR input_is_esc
    BEQ test_esc_combo_ok
    CRS
test_esc_combo_ok:

    ; Check LEFT is also held
    JSR input_is_left
    BEQ test_esc_combo_left_ok
    CRS
test_esc_combo_left_ok:

; ============================================================================
; All tests passed
; ============================================================================
    HLT

; Include module under test
.include "../input.asm"
