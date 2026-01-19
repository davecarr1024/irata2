; Test TAY instruction (Transfer A to Y)
; Zero transfer: clear A before reading back through TYA
LDA #$00
TAY
LDA #$55
TYA
CMP #$01
JEQ tay_zero_fail
CMP #$00
JEQ tay_zero_ok
CRS
tay_zero_ok:
; Value transfer: reset A so TYA must restore 0x42
LDA #$42
TAY
LDA #$00
TYA
CMP #$41
JEQ tay_value_fail
CMP #$42
JEQ tay_value_ok
CRS
tay_value_ok:
; Negative transfer: reset A so TYA must restore 0x80
LDA #$80
TAY
LDA #$00
TYA
CMP #$7F
JEQ tay_negative_fail
CMP #$80
JEQ tay_negative_ok
CRS
tay_negative_ok:
HLT
tay_zero_fail:
CRS
tay_value_fail:
CRS
tay_negative_fail:
CRS
