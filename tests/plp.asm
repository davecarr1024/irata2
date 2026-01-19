; Test PLP instruction (Pull Status)
LDX #$FF
TXS
LDA #$00
CMP #$00
PHP
LDA #$01
CMP #$00
PLP
BEQ plp_zero_ok
CRS
plp_zero_ok:
HLT
