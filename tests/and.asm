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
; Absolute AND
LDA #$F0
STA $0200
LDA #$0F
AND $0200
CMP #$00
JEQ and_abs_ok
CRS
and_abs_ok:
HLT
