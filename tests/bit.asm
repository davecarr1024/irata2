; Test BIT instruction
LDA #$F0
STA $10
LDA #$0F
BIT $10
JEQ bit_zp_zero_ok
CRS
bit_zp_zero_ok:
LDA #$FF
STA $11
LDA #$80
BIT $11
JEQ bit_zp_nonzero_fail
BMI bit_zp_negative_ok
CRS
bit_zp_negative_ok:
LDA #$00
STA $0200
LDA #$FF
BIT $0200
JEQ bit_abs_zero_ok
CRS
bit_abs_zero_ok:
LDA #$80
STA $0201
LDA #$80
BIT $0201
BMI bit_abs_negative_ok
CRS
bit_abs_negative_ok:
HLT
bit_zp_nonzero_fail:
CRS
