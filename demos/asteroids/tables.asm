; ============================================================================
; Data Tables Module
; ============================================================================
; Lookup tables for direction vectors, thrust values, and bullet velocities.
; All tables are indexed by ship_angle (0-15 for 16 directions).
;
; Interface:
;   dir_table_x     - Unit direction X component (signed)
;   dir_table_y     - Unit direction Y component (signed)
;   thrust_table_x  - Thrust vector X component (signed)
;   thrust_table_y  - Thrust vector Y component (signed)
;   bullet_table_vx - Bullet velocity X (signed)
;   bullet_table_vy - Bullet velocity Y (signed)
;   bullet_draw_x   - Bullet draw offset X (signed)
;   bullet_draw_y   - Bullet draw offset Y (signed)
;
; Angle mapping (16 directions):
;   0 = North (up)
;   4 = East (right)
;   8 = South (down)
;   12 = West (left)
; ============================================================================

; ----------------------------------------------------------------------------
; Direction vectors (for ship orientation)
; ----------------------------------------------------------------------------
; These define the unit direction vector for each angle.
; Values are scaled: 2 = full step, 1 = diagonal, 0 = no movement
; Signed bytes: $FF = -1, $FE = -2
; ----------------------------------------------------------------------------
dir_table_x:
    .byte $00       ; 0: N   (0, -2)
    .byte $01       ; 1: NNE (1, -2)
    .byte $02       ; 2: NE  (2, -2)
    .byte $02       ; 3: ENE (2, -1)
    .byte $02       ; 4: E   (2, 0)
    .byte $02       ; 5: ESE (2, 1)
    .byte $02       ; 6: SE  (2, 2)
    .byte $01       ; 7: SSE (1, 2)
    .byte $00       ; 8: S   (0, 2)
    .byte $FF       ; 9: SSW (-1, 2)
    .byte $FE       ; 10: SW (-2, 2)
    .byte $FE       ; 11: WSW (-2, 1)
    .byte $FE       ; 12: W  (-2, 0)
    .byte $FE       ; 13: WNW (-2, -1)
    .byte $FE       ; 14: NW (-2, -2)
    .byte $FF       ; 15: NNW (-1, -2)

dir_table_y:
    .byte $FE       ; 0: N   (0, -2)
    .byte $FE       ; 1: NNE (1, -2)
    .byte $FE       ; 2: NE  (2, -2)
    .byte $FF       ; 3: ENE (2, -1)
    .byte $00       ; 4: E   (2, 0)
    .byte $01       ; 5: ESE (2, 1)
    .byte $02       ; 6: SE  (2, 2)
    .byte $02       ; 7: SSE (1, 2)
    .byte $02       ; 8: S   (0, 2)
    .byte $02       ; 9: SSW (-1, 2)
    .byte $02       ; 10: SW (-2, 2)
    .byte $01       ; 11: WSW (-2, 1)
    .byte $00       ; 12: W  (-2, 0)
    .byte $FF       ; 13: WNW (-2, -1)
    .byte $FE       ; 14: NW (-2, -2)
    .byte $FE       ; 15: NNW (-1, -2)

; ----------------------------------------------------------------------------
; Thrust vectors (match direction table for 16-way acceleration)
; ----------------------------------------------------------------------------
thrust_table_x:
    .byte $00       ; 0: N
    .byte $01       ; 1: NNE
    .byte $02       ; 2: NE
    .byte $02       ; 3: ENE
    .byte $02       ; 4: E
    .byte $02       ; 5: ESE
    .byte $02       ; 6: SE
    .byte $01       ; 7: SSE
    .byte $00       ; 8: S
    .byte $FF       ; 9: SSW
    .byte $FE       ; 10: SW
    .byte $FE       ; 11: WSW
    .byte $FE       ; 12: W
    .byte $FE       ; 13: WNW
    .byte $FE       ; 14: NW
    .byte $FF       ; 15: NNW

thrust_table_y:
    .byte $FE       ; 0: N
    .byte $FE       ; 1: NNE
    .byte $FE       ; 2: NE
    .byte $FF       ; 3: ENE
    .byte $00       ; 4: E
    .byte $01       ; 5: ESE
    .byte $02       ; 6: SE
    .byte $02       ; 7: SSE
    .byte $02       ; 8: S
    .byte $02       ; 9: SSW
    .byte $02       ; 10: SW
    .byte $01       ; 11: WSW
    .byte $00       ; 12: W
    .byte $FF       ; 13: WNW
    .byte $FE       ; 14: NW
    .byte $FE       ; 15: NNW

; ----------------------------------------------------------------------------
; Bullet velocities (faster than ship)
; ----------------------------------------------------------------------------
bullet_table_vx:
    .byte $00       ; 0: N
    .byte $02       ; 1: NNE
    .byte $04       ; 2: NE
    .byte $04       ; 3: ENE
    .byte $04       ; 4: E
    .byte $04       ; 5: ESE
    .byte $04       ; 6: SE
    .byte $02       ; 7: SSE
    .byte $00       ; 8: S
    .byte $FE       ; 9: SSW
    .byte $FC       ; 10: SW
    .byte $FC       ; 11: WSW
    .byte $FC       ; 12: W
    .byte $FC       ; 13: WNW
    .byte $FC       ; 14: NW
    .byte $FE       ; 15: NNW

bullet_table_vy:
    .byte $FC       ; 0: N
    .byte $FC       ; 1: NNE
    .byte $FC       ; 2: NE
    .byte $FE       ; 3: ENE
    .byte $00       ; 4: E
    .byte $02       ; 5: ESE
    .byte $04       ; 6: SE
    .byte $04       ; 7: SSE
    .byte $04       ; 8: S
    .byte $04       ; 9: SSW
    .byte $04       ; 10: SW
    .byte $02       ; 11: WSW
    .byte $00       ; 12: W
    .byte $FE       ; 13: WNW
    .byte $FC       ; 14: NW
    .byte $FC       ; 15: NNW

; ----------------------------------------------------------------------------
; Bullet draw offsets (for drawing bullet line)
; ----------------------------------------------------------------------------
bullet_draw_x:
    .byte $00       ; 0: N
    .byte $01       ; 1: NNE
    .byte $02       ; 2: NE
    .byte $02       ; 3: ENE
    .byte $02       ; 4: E
    .byte $02       ; 5: ESE
    .byte $02       ; 6: SE
    .byte $01       ; 7: SSE
    .byte $00       ; 8: S
    .byte $FF       ; 9: SSW
    .byte $FE       ; 10: SW
    .byte $FE       ; 11: WSW
    .byte $FE       ; 12: W
    .byte $FE       ; 13: WNW
    .byte $FE       ; 14: NW
    .byte $FF       ; 15: NNW

bullet_draw_y:
    .byte $FE       ; 0: N
    .byte $FE       ; 1: NNE
    .byte $FE       ; 2: NE
    .byte $FF       ; 3: ENE
    .byte $00       ; 4: E
    .byte $01       ; 5: ESE
    .byte $02       ; 6: SE
    .byte $02       ; 7: SSE
    .byte $02       ; 8: S
    .byte $02       ; 9: SSW
    .byte $02       ; 10: SW
    .byte $01       ; 11: WSW
    .byte $00       ; 12: W
    .byte $FF       ; 13: WNW
    .byte $FE       ; 14: NW
    .byte $FE       ; 15: NNW
