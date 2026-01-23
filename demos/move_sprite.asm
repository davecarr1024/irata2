  .org $8000

  LDA #$80
  STA $00
  STA $01

loop:
  LDA $4000
  AND #$01
  BEQ render

  LDA $4002
  CMP #$01
  BNE check_down
  DEC $01
  JMP render

check_down:
  CMP #$02
  BNE check_left
  INC $01
  JMP render

check_left:
  CMP #$03
  BNE check_right
  DEC $00
  JMP render

check_right:
  CMP #$04
  BNE render
  INC $00

render:
  LDA #$01
  STA $4100
  LDA #$00
  STA $4105
  LDA #$01
  STA $4106

  LDA #$02
  STA $4100
  LDA $00
  STA $4101
  LDA $01
  STA $4102
  LDA #$03
  STA $4105
  LDA #$01
  STA $4106

  LDA #$02
  STA $4107

  JMP loop
