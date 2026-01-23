  .org $8000

  ; Zero-page layout
  ; $00 ship_x
  ; $01 ship_y
  ; $02 vel_x
  ; $03 vel_y
  ; $04 angle (0-7)
  ; $05 bullet_x
  ; $06 bullet_y
  ; $07 bullet_vx
  ; $08 bullet_vy
  ; $09 bullet_active
  ; $0A bullet_life
  ; $0B temp_dx
  ; $0C temp_dy
  ; $0D temp2
  ; $0E nose_x
  ; $0F nose_y
  ; $10 input

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

loop:
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
  AND #$07
  STA $04
  JMP update

check_right:
  CMP #$04
  BNE check_thrust
  LDA $04
  CLC
  ADC #$01
  AND #$07
  STA $04
  JMP update

check_thrust:
  CMP #$01
  BNE check_fire
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
  LDA thrust_dx, X
  STA $0B
  LDA thrust_dy, X
  STA $0C

  LDA $0B
  ASL
  STA $0D
  ASL
  CLC
  ADC $0D
  CLC
  ADC $00
  STA $0E

  LDA $0C
  ASL
  STA $0D
  ASL
  CLC
  ADC $0D
  CLC
  ADC $01
  STA $0F

  ; Ship: facing line
  LDA #$03
  STA $4100
  LDA $00
  STA $4101
  LDA $01
  STA $4102
  LDA $0E
  STA $4103
  LDA $0F
  STA $4104
  LDA #$03
  STA $4105
  LDA #$01
  STA $4106

  ; Bullet: point
  LDA $09
  BEQ asteroid
  LDA #$02
  STA $4100
  LDA $05
  STA $4101
  LDA $06
  STA $4102
  LDA #$02
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

thrust_dx:
  .byte $00, $01, $01, $01, $00, $FF, $FF, $FF
thrust_dy:
  .byte $FF, $FF, $00, $01, $01, $01, $00, $FF
bullet_dx:
  .byte $00, $02, $03, $02, $00, $FE, $FD, $FE
bullet_dy:
  .byte $FD, $FE, $00, $02, $03, $02, $00, $FE
