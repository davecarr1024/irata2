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
; Zero page AND
LDA #$0F
STA $10
LDA #$FF
AND $10
CMP #$0F
JEQ and_zp_ok
CRS
and_zp_ok:
HLT
