; ============================================================================
; Bullet Module Unit Tests
; ============================================================================
; Tests for bullet_init, bullet_fire, bullet_update, bullet_is_active
; ============================================================================

; Include module dependencies
.include "../hardware.asm"
.include "../variables.asm"

; Initialize stack
    LDX #$FF
    TXS

; ============================================================================
; Test 1: bullet_init sets bullet to inactive
; ============================================================================
test_bullet_init:
    ; Set bullet active to verify init clears it
    LDA #$01
    STA bullet_active
    LDA #$10
    STA bullet_life

    JSR bullet_init

    ; Check bullet_active == 0
    LDA bullet_active
    CMP #$00
    JEQ test_init_active_ok
    CRS
test_init_active_ok:

    ; Check bullet_life == 0
    LDA bullet_life
    CMP #$00
    JEQ test_init_life_ok
    CRS
test_init_life_ok:

; ============================================================================
; Test 2: bullet_is_active returns Z=1 when inactive
; ============================================================================
test_bullet_inactive:
    LDA #$00
    STA bullet_active

    JSR bullet_is_active

    ; A should be 0, Z flag should be set
    BEQ test_inactive_ok
    CRS
test_inactive_ok:

; ============================================================================
; Test 3: bullet_is_active returns Z=0 when active
; ============================================================================
test_bullet_active:
    LDA #$01
    STA bullet_active

    JSR bullet_is_active

    ; A should be 1, Z flag should be clear
    BNE test_active_ok
    CRS
test_active_ok:

; ============================================================================
; Test 4: bullet_fire activates bullet at ship position
; ============================================================================
test_bullet_fire:
    ; Initialize bullet as inactive
    JSR bullet_init

    ; Set ship position
    LDA #$50
    STA ship_x
    LDA #$60
    STA ship_y

    ; Set ship angle to 0 (north) and velocity
    LDA #$00
    STA ship_angle
    STA ship_vx
    STA ship_vy

    JSR bullet_fire

    ; Check bullet is active
    LDA bullet_active
    CMP #$01
    JEQ test_fire_active_ok
    CRS
test_fire_active_ok:

    ; Check bullet position == ship position
    LDA bullet_x
    CMP #$50
    JEQ test_fire_x_ok
    CRS
test_fire_x_ok:

    LDA bullet_y
    CMP #$60
    JEQ test_fire_y_ok
    CRS
test_fire_y_ok:

    ; Check bullet velocity from table (angle 0: vx=0, vy=$FC)
    LDA bullet_vx
    CMP #$00
    JEQ test_fire_vx_ok
    CRS
test_fire_vx_ok:

    LDA bullet_vy
    CMP #$FC
    JEQ test_fire_vy_ok
    CRS
test_fire_vy_ok:

    ; Check lifetime is set
    LDA bullet_life
    CMP #$30          ; BULLET_LIFETIME
    JEQ test_fire_life_ok
    CRS
test_fire_life_ok:

; ============================================================================
; Test 5: bullet_fire does not fire if already active
; ============================================================================
test_bullet_fire_blocked:
    ; Set bullet as active with known position
    LDA #$01
    STA bullet_active
    LDA #$20
    STA bullet_x
    LDA #$30
    STA bullet_y

    ; Set different ship position
    LDA #$80
    STA ship_x
    STA ship_y

    ; Try to fire - should be blocked
    JSR bullet_fire

    ; Bullet position should be unchanged
    LDA bullet_x
    CMP #$20
    JEQ test_fire_blocked_x_ok
    CRS
test_fire_blocked_x_ok:

    LDA bullet_y
    CMP #$30
    JEQ test_fire_blocked_y_ok
    CRS
test_fire_blocked_y_ok:

; ============================================================================
; Test 6: bullet_update moves bullet by velocity
; ============================================================================
test_bullet_update:
    ; Set up active bullet
    LDA #$01
    STA bullet_active
    LDA #$10
    STA bullet_life

    ; Set position
    LDA #$40
    STA bullet_x
    LDA #$50
    STA bullet_y

    ; Set velocity (+2, +3)
    LDA #$02
    STA bullet_vx
    LDA #$03
    STA bullet_vy

    JSR bullet_update

    ; Check position updated
    LDA bullet_x
    CMP #$42
    JEQ test_update_x_ok
    CRS
test_update_x_ok:

    LDA bullet_y
    CMP #$53
    JEQ test_update_y_ok
    CRS
test_update_y_ok:

    ; Check lifetime decremented
    LDA bullet_life
    CMP #$0F
    JEQ test_update_life_ok
    CRS
test_update_life_ok:

; ============================================================================
; Test 7: bullet_update deactivates when lifetime expires
; ============================================================================
test_bullet_expire:
    ; Set up active bullet with lifetime = 1
    LDA #$01
    STA bullet_active
    STA bullet_life

    ; Set position and velocity
    LDA #$40
    STA bullet_x
    STA bullet_y
    LDA #$01
    STA bullet_vx
    STA bullet_vy

    JSR bullet_update

    ; Life goes 1 -> 0, bullet still active this frame
    LDA bullet_life
    CMP #$00
    JEQ test_expire_life_ok
    CRS
test_expire_life_ok:

    ; Need one more update to deactivate
    JSR bullet_update

    ; Now bullet should be inactive
    LDA bullet_active
    CMP #$00
    JEQ test_expire_active_ok
    CRS
test_expire_active_ok:

; ============================================================================
; Test 8: bullet_update does nothing when inactive
; ============================================================================
test_bullet_update_inactive:
    ; Set up inactive bullet
    LDA #$00
    STA bullet_active

    ; Set position
    LDA #$40
    STA bullet_x
    LDA #$50
    STA bullet_y

    ; Set velocity
    LDA #$05
    STA bullet_vx
    STA bullet_vy

    JSR bullet_update

    ; Position should be unchanged
    LDA bullet_x
    CMP #$40
    JEQ test_update_inactive_x_ok
    CRS
test_update_inactive_x_ok:

    LDA bullet_y
    CMP #$50
    JEQ test_update_inactive_y_ok
    CRS
test_update_inactive_y_ok:

; ============================================================================
; Test 9: bullet_fire inherits ship velocity
; ============================================================================
test_bullet_fire_velocity:
    ; Initialize bullet as inactive
    JSR bullet_init

    ; Set ship position and velocity
    LDA #$80
    STA ship_x
    STA ship_y
    LDA #$02
    STA ship_vx
    LDA #$03
    STA ship_vy

    ; Set ship angle to 4 (east) - bullet_table gives vx=$04, vy=$00
    LDA #$04
    STA ship_angle

    JSR bullet_fire

    ; Bullet velocity = bullet_table + ship_velocity
    ; vx = $04 + $02 = $06
    LDA bullet_vx
    CMP #$06
    JEQ test_fire_vel_x_ok
    CRS
test_fire_vel_x_ok:

    ; vy = $00 + $03 = $03
    LDA bullet_vy
    CMP #$03
    JEQ test_fire_vel_y_ok
    CRS
test_fire_vel_y_ok:

; ============================================================================
; All tests passed
; ============================================================================
    HLT

; Include modules under test (graphics.asm must come before bullet.asm for draw_* aliases)
.include "../graphics.asm"
.include "../bullet.asm"
.include "../tables.asm"
