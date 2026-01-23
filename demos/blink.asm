  .org $8000

  LDA #$80
  STA $00
  STA $01
  LDA #$00
  STA $02

loop:
  LDA $02
  EOR #$03
  STA $02

  LDA #$01
  STA $4100
  LDA $02
  STA $4105
  LDA #$01
  STA $4106

  LDA #$02
  STA $4100
  LDA $00
  STA $4101
  LDA $01
  STA $4102
  LDA $02
  STA $4105
  LDA #$01
  STA $4106

  LDA #$02
  STA $4107

  LDX #$FF
delay_outer:
  LDY #$FF
delay_inner:
  DEY
  BNE delay_inner
  DEX
  BNE delay_outer

  JMP loop
