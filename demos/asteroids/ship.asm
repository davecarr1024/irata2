; ============================================================================
; Ship Module
; ============================================================================
; Handles ship state, physics, and rendering.
;
; Interface:
;   ship_init       - Initialize ship at center of screen
;   ship_rotate_ccw - Rotate ship counter-clockwise
;   ship_rotate_cw  - Rotate ship clockwise
;   ship_thrust     - Apply thrust in current direction
;   ship_update     - Update ship position from velocity
;   ship_draw       - Draw ship triangle and optional flame
;
; Uses: ship_x, ship_y, ship_vx, ship_vy, ship_angle, ship_thrust
; Modifies: A, X (some routines)
; ============================================================================

; ----------------------------------------------------------------------------
; ship_init - Initialize ship state
; ----------------------------------------------------------------------------
; Sets ship to center of screen, zero velocity, facing up.
; Modifies: A
; ----------------------------------------------------------------------------
ship_init:
    LDA #SCREEN_CENTER
    STA ship_x
    LDA #$90                ; Slightly below center
    STA ship_y
    LDA #$00
    STA ship_vx
    STA ship_vy
    STA ship_angle          ; Facing up (north)
    STA ship_thrust
    STA rotate_timer        ; Ready to rotate immediately
    STA thrust_timer        ; Ready to thrust immediately
    RTS

; ----------------------------------------------------------------------------
; ship_rotate_ccw - Rotate counter-clockwise
; ----------------------------------------------------------------------------
; Decrements angle, wrapping from 0 to 15.
; Modifies: A
; ----------------------------------------------------------------------------
ship_rotate_ccw:
    LDA ship_angle
    SEC
    SBC #$01
    AND #$0F                ; Wrap to 0-15
    STA ship_angle
    RTS

; ----------------------------------------------------------------------------
; ship_rotate_cw - Rotate clockwise
; ----------------------------------------------------------------------------
; Increments angle, wrapping from 15 to 0.
; Modifies: A
; ----------------------------------------------------------------------------
ship_rotate_cw:
    LDA ship_angle
    CLC
    ADC #$01
    AND #$0F                ; Wrap to 0-15
    STA ship_angle
    RTS

; ----------------------------------------------------------------------------
; ship_thrust - Apply thrust
; ----------------------------------------------------------------------------
; Adds thrust vector to velocity based on current angle.
; Modifies: A, X
; ----------------------------------------------------------------------------
ship_apply_thrust:
    LDA #$01
    STA ship_thrust         ; Set thrusting flag for flame

    LDX ship_angle
    LDA thrust_table_x, X
    CLC
    ADC ship_vx
    STA ship_vx

    LDA thrust_table_y, X
    CLC
    ADC ship_vy
    STA ship_vy
    RTS

; ----------------------------------------------------------------------------
; ship_update - Update position from velocity
; ----------------------------------------------------------------------------
; Applies velocity to position. Position wraps at screen edges.
; Modifies: A
; ----------------------------------------------------------------------------
ship_update:
    ; Reset thrust flag (will be set if thrusting this frame)
    LDA #$00
    STA ship_thrust

    ; Update X position
    LDA ship_x
    CLC
    ADC ship_vx
    STA ship_x

    ; Update Y position
    LDA ship_y
    CLC
    ADC ship_vy
    STA ship_y

    RTS

; ----------------------------------------------------------------------------
; ship_draw - Draw the ship triangle and optional flame
; ----------------------------------------------------------------------------
; Computes triangle vertices from position and angle, draws 3 lines.
; Uses: dir_x, dir_y, perp_x, perp_y, nose_*, left_*, right_*, tail_*
; Modifies: A, X
; ----------------------------------------------------------------------------
ship_draw:
    ; Load direction vector for current angle
    LDX ship_angle
    LDA dir_table_x, X
    STA dir_x
    LDA dir_table_y, X
    STA dir_y

    ; Compute perpendicular vector (for wing spread)
    ; perp = (-dir_y, dir_x)
    LDA dir_y
    EOR #$FF
    CLC
    ADC #$01                ; Negate: perp_x = -dir_y
    STA perp_x
    LDA dir_x
    STA perp_y              ; perp_y = dir_x

    ; Compute nose position: ship + dir * 3
    ; nose_x = ship_x + dir_x * 3
    LDA dir_x
    ASL                     ; * 2
    STA temp
    CLC
    ADC dir_x               ; * 3
    CLC
    ADC ship_x
    STA nose_x

    LDA dir_y
    ASL                     ; * 2
    STA temp
    CLC
    ADC dir_y               ; * 3
    CLC
    ADC ship_y
    STA nose_y

    ; Compute tail position: ship - dir * 2
    LDA dir_x
    ASL                     ; * 2
    STA temp
    LDA ship_x
    SEC
    SBC temp
    STA tail_x

    LDA dir_y
    ASL                     ; * 2
    STA temp
    LDA ship_y
    SEC
    SBC temp
    STA tail_y

    ; Compute left wing: tail + perp * 2
    LDA perp_x
    ASL                     ; * 2
    STA temp
    LDA tail_x
    CLC
    ADC temp
    STA left_x

    LDA perp_y
    ASL                     ; * 2
    STA temp
    LDA tail_y
    CLC
    ADC temp
    STA left_y

    ; Compute right wing: tail - perp * 2
    LDA perp_x
    ASL                     ; * 2
    STA temp
    LDA tail_x
    SEC
    SBC temp
    STA right_x

    LDA perp_y
    ASL                     ; * 2
    STA temp
    LDA tail_y
    SEC
    SBC temp
    STA right_y

    ; Draw triangle: nose -> left -> right -> nose
    LDA #COLOR_WHITE
    STA draw_color

    ; Line: nose to left
    LDA nose_x
    STA draw_x0
    LDA nose_y
    STA draw_y0
    LDA left_x
    STA draw_x1
    LDA left_y
    STA draw_y1
    JSR gfx_line

    ; Line: left to right
    LDA left_x
    STA draw_x0
    LDA left_y
    STA draw_y0
    LDA right_x
    STA draw_x1
    LDA right_y
    STA draw_y1
    JSR gfx_line

    ; Line: right to nose
    LDA right_x
    STA draw_x0
    LDA right_y
    STA draw_y0
    LDA nose_x
    STA draw_x1
    LDA nose_y
    STA draw_y1
    JSR gfx_line

    ; Draw flame if thrusting
    LDA ship_thrust
    BEQ ship_draw_done

    ; Compute flame tip: tail - dir * 2
    LDA dir_x
    ASL                     ; * 2
    STA temp
    LDA tail_x
    SEC
    SBC temp
    STA flame_x

    LDA dir_y
    ASL                     ; * 2
    STA temp
    LDA tail_y
    SEC
    SBC temp
    STA flame_y

    ; Draw flame line
    LDA #COLOR_DIM
    STA draw_color
    LDA tail_x
    STA draw_x0
    LDA tail_y
    STA draw_y0
    LDA flame_x
    STA draw_x1
    LDA flame_y
    STA draw_y1
    JSR gfx_line

ship_draw_done:
    RTS
