; Test AND instruction
LDA #$FF
AND #$0F
CMP #$0F
JEQ and_ok
CRS
and_ok:
; Test AND with zero
LDA #$F0
AND #$0F
JEQ and_zero_ok
CRS
and_zero_ok:
HLT
