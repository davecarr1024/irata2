; ============================================================================
; Ship Module Unit Tests
; ============================================================================
; Tests for ship_init, ship_rotate_ccw, ship_rotate_cw, ship_update
; ============================================================================

; Include module dependencies
.include "../hardware.asm"
.include "../variables.asm"

; Initialize stack
    LDX #$FF
    TXS

; ============================================================================
; Test 1: ship_init sets position to center
; ============================================================================
test_ship_init:
    ; Clear ship position first to verify init writes
    LDA #$00
    STA ship_x
    STA ship_y

    JSR ship_init

    ; Check ship_x == SCREEN_CENTER ($80)
    LDA ship_x
    CMP #SCREEN_CENTER
    JEQ test_init_x_ok
    CRS
test_init_x_ok:

    ; Check ship_y == $90 (slightly below center)
    LDA ship_y
    CMP #$90
    JEQ test_init_y_ok
    CRS
test_init_y_ok:

    ; Check ship_vx == 0
    LDA ship_vx
    CMP #$00
    JEQ test_init_vx_ok
    CRS
test_init_vx_ok:

    ; Check ship_vy == 0
    LDA ship_vy
    CMP #$00
    JEQ test_init_vy_ok
    CRS
test_init_vy_ok:

    ; Check ship_angle == 0 (facing north)
    LDA ship_angle
    CMP #$00
    JEQ test_init_angle_ok
    CRS
test_init_angle_ok:

    ; Check ship_thrust == 0
    LDA ship_thrust
    CMP #$00
    JEQ test_init_thrust_ok
    CRS
test_init_thrust_ok:

; ============================================================================
; Test 2: ship_rotate_cw increments angle
; ============================================================================
test_rotate_cw:
    ; Set angle to 0
    LDA #$00
    STA ship_angle

    JSR ship_rotate_cw

    ; Angle should be 1
    LDA ship_angle
    CMP #$01
    JEQ test_rotate_cw_ok
    CRS
test_rotate_cw_ok:

    ; Rotate again, should be 2
    JSR ship_rotate_cw
    LDA ship_angle
    CMP #$02
    JEQ test_rotate_cw2_ok
    CRS
test_rotate_cw2_ok:

; ============================================================================
; Test 3: ship_rotate_cw wraps from 15 to 0
; ============================================================================
test_rotate_cw_wrap:
    ; Set angle to 15
    LDA #$0F
    STA ship_angle

    JSR ship_rotate_cw

    ; Angle should wrap to 0
    LDA ship_angle
    CMP #$00
    JEQ test_rotate_cw_wrap_ok
    CRS
test_rotate_cw_wrap_ok:

; ============================================================================
; Test 4: ship_rotate_ccw decrements angle
; ============================================================================
test_rotate_ccw:
    ; Set angle to 5
    LDA #$05
    STA ship_angle

    JSR ship_rotate_ccw

    ; Angle should be 4
    LDA ship_angle
    CMP #$04
    JEQ test_rotate_ccw_ok
    CRS
test_rotate_ccw_ok:

; ============================================================================
; Test 5: ship_rotate_ccw wraps from 0 to 15
; ============================================================================
test_rotate_ccw_wrap:
    ; Set angle to 0
    LDA #$00
    STA ship_angle

    JSR ship_rotate_ccw

    ; Angle should wrap to 15
    LDA ship_angle
    CMP #$0F
    JEQ test_rotate_ccw_wrap_ok
    CRS
test_rotate_ccw_wrap_ok:

; ============================================================================
; Test 6: ship_update applies velocity to position
; ============================================================================
test_ship_update:
    ; Set position
    LDA #$40
    STA ship_x
    LDA #$50
    STA ship_y

    ; Set velocity (+2, +3)
    LDA #$02
    STA ship_vx
    LDA #$03
    STA ship_vy

    ; Set thrust flag to verify it gets cleared
    LDA #$01
    STA ship_thrust

    JSR ship_update

    ; Check position updated: x = $40 + $02 = $42
    LDA ship_x
    CMP #$42
    JEQ test_update_x_ok
    CRS
test_update_x_ok:

    ; y = $50 + $03 = $53
    LDA ship_y
    CMP #$53
    JEQ test_update_y_ok
    CRS
test_update_y_ok:

    ; Thrust flag should be cleared
    LDA ship_thrust
    CMP #$00
    JEQ test_update_thrust_ok
    CRS
test_update_thrust_ok:

; ============================================================================
; Test 7: ship_update with negative velocity
; ============================================================================
test_ship_update_negative:
    ; Set position
    LDA #$80
    STA ship_x
    LDA #$80
    STA ship_y

    ; Set velocity (-1, -2) in two's complement
    LDA #$FF
    STA ship_vx
    LDA #$FE
    STA ship_vy

    JSR ship_update

    ; Check position: x = $80 + $FF = $7F
    LDA ship_x
    CMP #$7F
    JEQ test_update_neg_x_ok
    CRS
test_update_neg_x_ok:

    ; y = $80 + $FE = $7E
    LDA ship_y
    CMP #$7E
    JEQ test_update_neg_y_ok
    CRS
test_update_neg_y_ok:

; ============================================================================
; Test 8: ship_apply_thrust adds to velocity
; ============================================================================
test_ship_apply_thrust:
    ; Set angle to 0 (north) - thrust should be (0, -1)
    LDA #$00
    STA ship_angle

    ; Set initial velocity to 0
    LDA #$00
    STA ship_vx
    STA ship_vy
    STA ship_thrust

    JSR ship_apply_thrust

    ; Check thrust flag is set
    LDA ship_thrust
    CMP #$01
    JEQ test_thrust_flag_ok
    CRS
test_thrust_flag_ok:

    ; Check vx = 0 + thrust_table_x[0] = 0 + 0 = 0
    LDA ship_vx
    CMP #$00
    JEQ test_thrust_vx_ok
    CRS
test_thrust_vx_ok:

    ; Check vy = 0 + thrust_table_y[0] = 0 + (-1) = $FF
    LDA ship_vy
    CMP #$FF
    JEQ test_thrust_vy_ok
    CRS
test_thrust_vy_ok:

; ============================================================================
; Test 9: ship_apply_thrust adds to existing velocity
; ============================================================================
test_ship_thrust_accumulates:
    ; Set angle to 4 (east) - thrust should be (+1, 0)
    LDA #$04
    STA ship_angle

    ; Set initial velocity to (5, 3)
    LDA #$05
    STA ship_vx
    LDA #$03
    STA ship_vy

    JSR ship_apply_thrust

    ; Check vx = 5 + 1 = 6
    LDA ship_vx
    CMP #$06
    JEQ test_thrust_accum_vx_ok
    CRS
test_thrust_accum_vx_ok:

    ; Check vy = 3 + 0 = 3
    LDA ship_vy
    CMP #$03
    JEQ test_thrust_accum_vy_ok
    CRS
test_thrust_accum_vy_ok:

; ============================================================================
; All tests passed
; ============================================================================
    HLT

; Include modules under test (graphics.asm must come before ship.asm for draw_* aliases)
.include "../graphics.asm"
.include "../ship.asm"
.include "../tables.asm"
