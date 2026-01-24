; ============================================================================
; Tables Module Unit Tests
; ============================================================================
; Tests lookup table values for direction, thrust, and bullet vectors.
; Verifies cardinal direction values and symmetry properties.
; ============================================================================

; Include module dependencies
.include "../hardware.asm"
.include "../variables.asm"

; Initialize stack
    LDX #$FF
    TXS

; ============================================================================
; Test 1: dir_table for North (angle 0)
; ============================================================================
test_dir_north:
    ; North should be (0, -2) = ($00, $FE)
    LDX #$00
    LDA dir_table_x, X
    CMP #$00
    JEQ test_dir_north_x_ok
    CRS
test_dir_north_x_ok:

    LDA dir_table_y, X
    CMP #$FE
    JEQ test_dir_north_y_ok
    CRS
test_dir_north_y_ok:

; ============================================================================
; Test 2: dir_table for East (angle 4)
; ============================================================================
test_dir_east:
    ; East should be (2, 0) = ($02, $00)
    LDX #$04
    LDA dir_table_x, X
    CMP #$02
    JEQ test_dir_east_x_ok
    CRS
test_dir_east_x_ok:

    LDA dir_table_y, X
    CMP #$00
    JEQ test_dir_east_y_ok
    CRS
test_dir_east_y_ok:

; ============================================================================
; Test 3: dir_table for South (angle 8)
; ============================================================================
test_dir_south:
    ; South should be (0, 2) = ($00, $02)
    LDX #$08
    LDA dir_table_x, X
    CMP #$00
    JEQ test_dir_south_x_ok
    CRS
test_dir_south_x_ok:

    LDA dir_table_y, X
    CMP #$02
    JEQ test_dir_south_y_ok
    CRS
test_dir_south_y_ok:

; ============================================================================
; Test 4: dir_table for West (angle 12)
; ============================================================================
test_dir_west:
    ; West should be (-2, 0) = ($FE, $00)
    LDX #$0C
    LDA dir_table_x, X
    CMP #$FE
    JEQ test_dir_west_x_ok
    CRS
test_dir_west_x_ok:

    LDA dir_table_y, X
    CMP #$00
    JEQ test_dir_west_y_ok
    CRS
test_dir_west_y_ok:

; ============================================================================
; Test 5: thrust_table for North (angle 0)
; ============================================================================
test_thrust_north:
    ; North thrust should be (0, -2) = ($00, $FE)
    LDX #$00
    LDA thrust_table_x, X
    CMP #$00
    JEQ test_thrust_north_x_ok
    CRS
test_thrust_north_x_ok:

    LDA thrust_table_y, X
    CMP #$FE
    JEQ test_thrust_north_y_ok
    CRS
test_thrust_north_y_ok:

; ============================================================================
; Test 6: thrust_table for East (angle 4)
; ============================================================================
test_thrust_east:
    ; East thrust should be (2, 0) = ($02, $00)
    LDX #$04
    LDA thrust_table_x, X
    CMP #$02
    JEQ test_thrust_east_x_ok
    CRS
test_thrust_east_x_ok:

    LDA thrust_table_y, X
    CMP #$00
    JEQ test_thrust_east_y_ok
    CRS
test_thrust_east_y_ok:

; ============================================================================
; Test 7: thrust_table for South (angle 8)
; ============================================================================
test_thrust_south:
    ; South thrust should be (0, 2) = ($00, $02)
    LDX #$08
    LDA thrust_table_x, X
    CMP #$00
    JEQ test_thrust_south_x_ok
    CRS
test_thrust_south_x_ok:

    LDA thrust_table_y, X
    CMP #$02
    JEQ test_thrust_south_y_ok
    CRS
test_thrust_south_y_ok:

; ============================================================================
; Test 8: thrust_table for West (angle 12)
; ============================================================================
test_thrust_west:
    ; West thrust should be (-2, 0) = ($FE, $00)
    LDX #$0C
    LDA thrust_table_x, X
    CMP #$FE
    JEQ test_thrust_west_x_ok
    CRS
test_thrust_west_x_ok:

    LDA thrust_table_y, X
    CMP #$00
    JEQ test_thrust_west_y_ok
    CRS
test_thrust_west_y_ok:

; ============================================================================
; Test 9: bullet_table for North (angle 0)
; ============================================================================
test_bullet_north:
    ; North bullet velocity should be (0, -4) = ($00, $FC)
    LDX #$00
    LDA bullet_table_vx, X
    CMP #$00
    JEQ test_bullet_north_vx_ok
    CRS
test_bullet_north_vx_ok:

    LDA bullet_table_vy, X
    CMP #$FC
    JEQ test_bullet_north_vy_ok
    CRS
test_bullet_north_vy_ok:

; ============================================================================
; Test 10: bullet_table for East (angle 4)
; ============================================================================
test_bullet_east:
    ; East bullet velocity should be (4, 0) = ($04, $00)
    LDX #$04
    LDA bullet_table_vx, X
    CMP #$04
    JEQ test_bullet_east_vx_ok
    CRS
test_bullet_east_vx_ok:

    LDA bullet_table_vy, X
    CMP #$00
    JEQ test_bullet_east_vy_ok
    CRS
test_bullet_east_vy_ok:

; ============================================================================
; Test 11: bullet_table for South (angle 8)
; ============================================================================
test_bullet_south:
    ; South bullet velocity should be (0, 4) = ($00, $04)
    LDX #$08
    LDA bullet_table_vx, X
    CMP #$00
    JEQ test_bullet_south_vx_ok
    CRS
test_bullet_south_vx_ok:

    LDA bullet_table_vy, X
    CMP #$04
    JEQ test_bullet_south_vy_ok
    CRS
test_bullet_south_vy_ok:

; ============================================================================
; Test 12: bullet_table for West (angle 12)
; ============================================================================
test_bullet_west:
    ; West bullet velocity should be (-4, 0) = ($FC, $00)
    LDX #$0C
    LDA bullet_table_vx, X
    CMP #$FC
    JEQ test_bullet_west_vx_ok
    CRS
test_bullet_west_vx_ok:

    LDA bullet_table_vy, X
    CMP #$00
    JEQ test_bullet_west_vy_ok
    CRS
test_bullet_west_vy_ok:

; ============================================================================
; Test 13: bullet_draw offsets for North (angle 0)
; ============================================================================
test_bullet_draw_north:
    ; North draw offset should be (0, -2) = ($00, $FE)
    LDX #$00
    LDA bullet_draw_x, X
    CMP #$00
    JEQ test_draw_north_x_ok
    CRS
test_draw_north_x_ok:

    LDA bullet_draw_y, X
    CMP #$FE
    JEQ test_draw_north_y_ok
    CRS
test_draw_north_y_ok:

; ============================================================================
; Test 14: bullet_draw offsets for East (angle 4)
; ============================================================================
test_bullet_draw_east:
    ; East draw offset should be (2, 0) = ($02, $00)
    LDX #$04
    LDA bullet_draw_x, X
    CMP #$02
    JEQ test_draw_east_x_ok
    CRS
test_draw_east_x_ok:

    LDA bullet_draw_y, X
    CMP #$00
    JEQ test_draw_east_y_ok
    CRS
test_draw_east_y_ok:

; ============================================================================
; Test 15: Verify diagonal direction (NE, angle 2)
; ============================================================================
test_dir_northeast:
    ; NE should be (2, -2) = ($02, $FE)
    LDX #$02
    LDA dir_table_x, X
    CMP #$02
    JEQ test_dir_ne_x_ok
    CRS
test_dir_ne_x_ok:

    LDA dir_table_y, X
    CMP #$FE
    JEQ test_dir_ne_y_ok
    CRS
test_dir_ne_y_ok:

; ============================================================================
; Test 16: Verify diagonal direction (SW, angle 10)
; ============================================================================
test_dir_southwest:
    ; SW should be (-2, 2) = ($FE, $02)
    LDX #$0A
    LDA dir_table_x, X
    CMP #$FE
    JEQ test_dir_sw_x_ok
    CRS
test_dir_sw_x_ok:

    LDA dir_table_y, X
    CMP #$02
    JEQ test_dir_sw_y_ok
    CRS
test_dir_sw_y_ok:

; ============================================================================
; All tests passed
; ============================================================================
    HLT

; Include tables module
.include "../tables.asm"
