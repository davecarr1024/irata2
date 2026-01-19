LDA #$00
TAY
TYA
JEQ tay_zero_ok
CRS
tay_zero_ok:
LDA #$42
TAY
TYA
CMP #$42
JEQ tay_value_ok
CRS
tay_value_ok:
LDA #$80
TAY
TYA
CMP #$80
JEQ tay_negative_ok
CRS
tay_negative_ok:
HLT
