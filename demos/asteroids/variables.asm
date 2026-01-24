; ============================================================================
; Zero Page Variable Definitions
; ============================================================================
; Defines all zero-page variable locations used by asteroids modules.
;
; Interface:
;   Constants only - no code or data
;
; Memory Map:
;   $00-$0F: Ship state
;   $10-$1F: Bullet state
;   $20-$2F: Scratch/temp variables
;   $30-$3F: Drawing computed values
; ============================================================================

; ----------------------------------------------------------------------------
; Ship State ($00-$0F)
; ----------------------------------------------------------------------------
.equ ship_x,        $00     ; Ship X position
.equ ship_y,        $01     ; Ship Y position
.equ ship_vx,       $02     ; Ship X velocity
.equ ship_vy,       $03     ; Ship Y velocity
.equ ship_angle,    $04     ; Ship angle (0-15, 16 directions)
.equ ship_thrust,   $05     ; Thrusting flag (0 or 1)

; ----------------------------------------------------------------------------
; Bullet State ($10-$1F)
; ----------------------------------------------------------------------------
.equ bullet_x,      $10     ; Bullet X position
.equ bullet_y,      $11     ; Bullet Y position
.equ bullet_vx,     $12     ; Bullet X velocity
.equ bullet_vy,     $13     ; Bullet Y velocity
.equ bullet_active, $14     ; Bullet active flag
.equ bullet_life,   $15     ; Bullet lifetime counter
.equ bullet_dx,     $16     ; Bullet draw X offset
.equ bullet_dy,     $17     ; Bullet draw Y offset

; ----------------------------------------------------------------------------
; Asteroid State ($18-$1F)
; ----------------------------------------------------------------------------
.equ asteroid_x,    $18     ; Asteroid X position
.equ asteroid_y,    $19     ; Asteroid Y position
.equ asteroid_sz,   $1A     ; Asteroid size

; ----------------------------------------------------------------------------
; Scratch/Temp Variables ($20-$2F)
; ----------------------------------------------------------------------------
.equ temp,          $20     ; General purpose temp
.equ temp2,         $21     ; General purpose temp 2
.equ cached_key,    $22     ; Cached input key code
.equ dir_x,         $23     ; Direction vector X
.equ dir_y,         $24     ; Direction vector Y
.equ perp_x,        $25     ; Perpendicular vector X
.equ perp_y,        $26     ; Perpendicular vector Y

; ----------------------------------------------------------------------------
; Ship Drawing Variables ($30-$3F)
; ----------------------------------------------------------------------------
.equ nose_x,        $30     ; Ship nose X
.equ nose_y,        $31     ; Ship nose Y
.equ left_x,        $32     ; Ship left wing X
.equ left_y,        $33     ; Ship left wing Y
.equ right_x,       $34     ; Ship right wing X
.equ right_y,       $35     ; Ship right wing Y
.equ tail_x,        $36     ; Ship tail X
.equ tail_y,        $37     ; Ship tail Y
.equ flame_x,       $38     ; Flame tip X
.equ flame_y,       $39     ; Flame tip Y
