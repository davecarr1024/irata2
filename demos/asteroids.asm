  .org $8000

  ; Zero-page layout
  ; $00 ship_x
  ; $01 ship_y
  ; $02 vel_x
  ; $03 vel_y
  ; $04 angle (0-15)
  ; $05 bullet_x
  ; $06 bullet_y
  ; $07 bullet_vx
  ; $08 bullet_vy
  ; $09 bullet_active
  ; $0A bullet_life
  ; $0B bullet_draw_dx
  ; $0C bullet_draw_dy
  ; $0D temp
  ; $0E dir_x
  ; $0F dir_y
  ; $10 input
  ; $11 thrusting
  ; $12 perp_x
  ; $13 perp_y
  ; $14 nose_x
  ; $15 nose_y
  ; $16 left_x
  ; $17 left_y
  ; $18 right_x
  ; $19 right_y
  ; $1A tail_x
  ; $1B tail_y

  LDA #$80
  STA $00
  LDA #$90
  STA $01
  LDA #$00
  STA $02
  STA $03
  STA $04
  STA $09
  STA $0A
  STA $11

loop:
  LDA #$00
  STA $11
  LDA $4000
  AND #$01
  BEQ update

  LDA $4002
  STA $10

  CMP #$03
  BNE check_right
  LDA $04
  SEC
  SBC #$01
  AND #$0F
  STA $04
  JMP update

check_right:
  CMP #$04
  BNE check_thrust
  LDA $04
  CLC
  ADC #$01
  AND #$0F
  STA $04
  JMP update

check_thrust:
  CMP #$01
  BNE check_fire
  LDA #$01
  STA $11
  LDX $04
  LDA thrust_dx, X
  CLC
  ADC $02
  STA $02
  LDA thrust_dy, X
  CLC
  ADC $03
  STA $03
  JMP update

check_fire:
  CMP #$20
  BNE update
  LDA $09
  BNE update
  LDA #$01
  STA $09
  LDA $00
  STA $05
  LDA $01
  STA $06
  LDX $04
  LDA bullet_dx, X
  CLC
  ADC $02
  STA $07
  LDA bullet_dy, X
  CLC
  ADC $03
  STA $08
  LDA bullet_draw_dx, X
  STA $0B
  LDA bullet_draw_dy, X
  STA $0C
  LDA #$30
  STA $0A

update:
  LDA $00
  CLC
  ADC $02
  STA $00
  LDA $01
  CLC
  ADC $03
  STA $01

  LDA $09
  BEQ draw
  LDA $05
  CLC
  ADC $07
  STA $05
  LDA $06
  CLC
  ADC $08
  STA $06
  LDA $0A
  BEQ deactivate
  SEC
  SBC #$01
  STA $0A
  BNE draw

deactivate:
  LDA #$00
  STA $09

draw:
  LDA #$01
  STA $4100
  LDA #$00
  STA $4105
  LDA #$01
  STA $4106

  LDX $04
  LDA dir_dx, X
  STA $0E
  LDA dir_dy, X
  STA $0F

  LDA $0F
  EOR #$FF
  CLC
  ADC #$01
  STA $12
  LDA $0E
  STA $13

  LDA $0E
  ASL
  STA $0D
  CLC
  ADC $0E
  CLC
  ADC $00
  STA $14

  LDA $0F
  ASL
  STA $0D
  CLC
  ADC $0F
  CLC
  ADC $01
  STA $15

  LDA $0E
  ASL
  STA $0D
  LDA $00
  SEC
  SBC $0D
  STA $1A

  LDA $0F
  ASL
  STA $0D
  LDA $01
  SEC
  SBC $0D
  STA $1B

  LDA $12
  ASL
  STA $0D
  LDA $1A
  CLC
  ADC $0D
  STA $16

  LDA $13
  ASL
  STA $0D
  LDA $1B
  CLC
  ADC $0D
  STA $17

  LDA $12
  ASL
  STA $0D
  LDA $1A
  SEC
  SBC $0D
  STA $18

  LDA $13
  ASL
  STA $0D
  LDA $1B
  SEC
  SBC $0D
  STA $19

  ; Ship: triangle
  LDA #$03
  STA $4100
  LDA $14
  STA $4101
  LDA $15
  STA $4102
  LDA $16
  STA $4103
  LDA $17
  STA $4104
  LDA #$03
  STA $4105
  LDA #$01
  STA $4106

  LDA #$03
  STA $4100
  LDA $16
  STA $4101
  LDA $17
  STA $4102
  LDA $18
  STA $4103
  LDA $19
  STA $4104
  LDA #$03
  STA $4105
  LDA #$01
  STA $4106

  LDA #$03
  STA $4100
  LDA $18
  STA $4101
  LDA $19
  STA $4102
  LDA $14
  STA $4103
  LDA $15
  STA $4104
  LDA #$03
  STA $4105
  LDA #$01
  STA $4106

  ; Bullet: line
  LDA $09
  BEQ flame
  LDA #$03
  STA $4100
  LDA $05
  STA $4101
  LDA $06
  STA $4102
  LDA $05
  CLC
  ADC $0B
  STA $4103
  LDA $06
  CLC
  ADC $0C
  STA $4104
  LDA #$02
  STA $4105
  LDA #$01
  STA $4106

flame:
  LDA $11
  BEQ asteroid
  LDA $0E
  ASL
  STA $0D
  LDA $1A
  SEC
  SBC $0D
  STA $16
  LDA $0F
  ASL
  STA $0D
  LDA $1B
  SEC
  SBC $0D
  STA $17

  LDA #$03
  STA $4100
  LDA $1A
  STA $4101
  LDA $1B
  STA $4102
  LDA $16
  STA $4103
  LDA $17
  STA $4104
  LDA #$01
  STA $4105
  LDA #$01
  STA $4106

asteroid:

  ; Asteroid: square at fixed position
  LDA #$03
  STA $4100
  LDA #$30
  STA $4101
  LDA #$30
  STA $4102
  LDA #$40
  STA $4103
  LDA #$30
  STA $4104
  LDA #$02
  STA $4105
  LDA #$01
  STA $4106

  LDA #$03
  STA $4100
  LDA #$40
  STA $4101
  LDA #$30
  STA $4102
  LDA #$40
  STA $4103
  LDA #$40
  STA $4104
  LDA #$02
  STA $4105
  LDA #$01
  STA $4106

  LDA #$03
  STA $4100
  LDA #$40
  STA $4101
  LDA #$40
  STA $4102
  LDA #$30
  STA $4103
  LDA #$40
  STA $4104
  LDA #$02
  STA $4105
  LDA #$01
  STA $4106

  LDA #$03
  STA $4100
  LDA #$30
  STA $4101
  LDA #$40
  STA $4102
  LDA #$30
  STA $4103
  LDA #$30
  STA $4104
  LDA #$02
  STA $4105
  LDA #$01
  STA $4106

  LDA #$02
  STA $4107

  JMP loop

dir_dx:
  .byte $00, $01, $02, $02, $02, $02, $02, $01
  .byte $00, $FF, $FE, $FE, $FE, $FE, $FE, $FF
dir_dy:
  .byte $FE, $FE, $FE, $FF, $00, $01, $02, $02
  .byte $02, $02, $02, $01, $00, $FF, $FE, $FE
thrust_dx:
  .byte $00, $01, $01, $01, $01, $01, $01, $01
  .byte $00, $FF, $FF, $FF, $FF, $FF, $FF, $FF
thrust_dy:
  .byte $FF, $FF, $FF, $FF, $00, $01, $01, $01
  .byte $01, $01, $01, $01, $00, $FF, $FF, $FF
bullet_dx:
  .byte $00, $02, $04, $04, $04, $04, $04, $02
  .byte $00, $FE, $FC, $FC, $FC, $FC, $FC, $FE
bullet_dy:
  .byte $FC, $FC, $FC, $FE, $00, $02, $04, $04
  .byte $04, $04, $04, $02, $00, $FE, $FC, $FC
bullet_draw_dx:
  .byte $00, $01, $02, $02, $02, $02, $02, $01
  .byte $00, $FF, $FE, $FE, $FE, $FE, $FE, $FF
bullet_draw_dy:
  .byte $FE, $FE, $FE, $FF, $00, $01, $02, $02
  .byte $02, $02, $02, $01, $00, $FF, $FE, $FE
