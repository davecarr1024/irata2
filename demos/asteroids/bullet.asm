; ============================================================================
; Bullet Module
; ============================================================================
; Handles bullet state, firing, physics, and rendering.
;
; Interface:
;   bullet_init     - Initialize bullet as inactive
;   bullet_fire     - Fire bullet from ship position in ship direction
;   bullet_update   - Update bullet position, handle lifetime
;   bullet_draw     - Draw bullet if active
;   bullet_is_active - Check if bullet is active (Z=0 if active)
;
; Uses: bullet_x, bullet_y, bullet_vx, bullet_vy, bullet_active, bullet_life
; Modifies: A, X (some routines)
; ============================================================================

; Bullet lifetime (frames)
.equ BULLET_LIFETIME, $30   ; 48 frames

; ----------------------------------------------------------------------------
; bullet_init - Initialize bullet state
; ----------------------------------------------------------------------------
; Sets bullet to inactive.
; Modifies: A
; ----------------------------------------------------------------------------
bullet_init:
    LDA #$00
    STA bullet_active
    STA bullet_life
    RTS

; ----------------------------------------------------------------------------
; bullet_is_active - Check if bullet is active
; ----------------------------------------------------------------------------
; Output: Z=0 if active, Z=1 if inactive
; Modifies: A
; ----------------------------------------------------------------------------
bullet_is_active:
    LDA bullet_active
    RTS

; ----------------------------------------------------------------------------
; bullet_fire - Fire bullet from ship
; ----------------------------------------------------------------------------
; Spawns bullet at ship position with velocity based on ship angle + ship velocity.
; Only fires if bullet is not already active.
; Modifies: A, X
; ----------------------------------------------------------------------------
bullet_fire:
    ; Check if already active
    LDA bullet_active
    BNE bullet_fire_done    ; Can't fire if already active

    ; Set active
    LDA #$01
    STA bullet_active

    ; Position at ship
    LDA ship_x
    STA bullet_x
    LDA ship_y
    STA bullet_y

    ; Velocity from table + ship velocity
    LDX ship_angle

    LDA bullet_table_vx, X
    CLC
    ADC ship_vx
    STA bullet_vx

    LDA bullet_table_vy, X
    CLC
    ADC ship_vy
    STA bullet_vy

    ; Store draw offsets
    LDA bullet_draw_x, X
    STA bullet_dx
    LDA bullet_draw_y, X
    STA bullet_dy

    ; Set lifetime
    LDA #BULLET_LIFETIME
    STA bullet_life

bullet_fire_done:
    RTS

; ----------------------------------------------------------------------------
; bullet_update - Update bullet state
; ----------------------------------------------------------------------------
; Updates position from velocity, decrements lifetime.
; Deactivates bullet when lifetime reaches zero.
; Modifies: A
; ----------------------------------------------------------------------------
bullet_update:
    ; Skip if not active
    LDA bullet_active
    BEQ bullet_update_done

    ; Update position
    LDA bullet_x
    CLC
    ADC bullet_vx
    STA bullet_x

    LDA bullet_y
    CLC
    ADC bullet_vy
    STA bullet_y

    ; Decrement lifetime
    LDA bullet_life
    BEQ bullet_deactivate
    SEC
    SBC #$01
    STA bullet_life
    BNE bullet_update_done

bullet_deactivate:
    LDA #$00
    STA bullet_active

bullet_update_done:
    RTS

; ----------------------------------------------------------------------------
; bullet_draw - Draw bullet if active
; ----------------------------------------------------------------------------
; Draws bullet as a short line in direction of travel.
; Modifies: A
; ----------------------------------------------------------------------------
bullet_draw:
    ; Skip if not active
    LDA bullet_active
    BEQ bullet_draw_done

    ; Set color
    LDA #COLOR_MED
    STA draw_color

    ; Line from bullet position to position + draw offset
    LDA bullet_x
    STA draw_x0
    LDA bullet_y
    STA draw_y0

    LDA bullet_x
    CLC
    ADC bullet_dx
    STA draw_x1

    LDA bullet_y
    CLC
    ADC bullet_dy
    STA draw_y1

    JSR gfx_line

bullet_draw_done:
    RTS
