; Test INC instruction
LDA #$01
STA $10
INC $10
LDA $10
CMP #$02
JEQ inc_zp_ok
CRS
inc_zp_ok:
LDA #$FF
STA $0200
INC $0200
LDA $0200
CMP #$00
JEQ inc_abs_ok
CRS
inc_abs_ok:
HLT
