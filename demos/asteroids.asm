  .org $8000

  LDA #$80
  STA $00
  LDA #$90
  STA $01

loop:
  LDA $4000
  AND #$01
  BEQ draw

  LDA $4002
  CMP #$01
  BNE check_down
  DEC $01
  JMP draw

check_down:
  CMP #$02
  BNE check_left
  INC $01
  JMP draw

check_left:
  CMP #$03
  BNE check_right
  DEC $00
  JMP draw

check_right:
  CMP #$04
  BNE draw
  INC $00

draw:
  LDA #$01
  STA $4100
  LDA #$00
  STA $4105
  LDA #$01
  STA $4106

  ; Ship: triangle
  LDA #$03
  STA $4100
  LDA $00
  STA $4101
  LDA $01
  SEC
  SBC #$06
  STA $4102
  LDA $00
  SEC
  SBC #$04
  STA $4103
  LDA $01
  CLC
  ADC #$04
  STA $4104
  LDA #$03
  STA $4105
  LDA #$01
  STA $4106

  LDA #$03
  STA $4100
  LDA $00
  SEC
  SBC #$04
  STA $4101
  LDA $01
  CLC
  ADC #$04
  STA $4102
  LDA $00
  CLC
  ADC #$04
  STA $4103
  LDA $01
  CLC
  ADC #$04
  STA $4104
  LDA #$03
  STA $4105
  LDA #$01
  STA $4106

  LDA #$03
  STA $4100
  LDA $00
  CLC
  ADC #$04
  STA $4101
  LDA $01
  CLC
  ADC #$04
  STA $4102
  LDA $00
  STA $4103
  LDA $01
  SEC
  SBC #$06
  STA $4104
  LDA #$03
  STA $4105
  LDA #$01
  STA $4106

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
