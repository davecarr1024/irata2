; Test DEC instruction
LDA #$02
STA $10
DEC $10
LDA $10
CMP #$01
JEQ dec_zp_ok
CRS
dec_zp_ok:
LDA #$00
STA $0200
DEC $0200
LDA $0200
CMP #$FF
JEQ dec_abs_ok
CRS
dec_abs_ok:
HLT
