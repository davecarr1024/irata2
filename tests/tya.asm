; Test TYA instruction (Transfer Y to A)
; Zero transfer: overwrite A before TYA
LDY #$00
LDA #$55
TYA
CMP #$01
JEQ tya_zero_fail
CMP #$00
JEQ tya_zero_ok
CRS
tya_zero_ok:
; Value transfer: ensure 0x55 arrives in A
LDY #$55
LDA #$00
TYA
CMP #$54
JEQ tya_value_fail
CMP #$55
JEQ tya_value_ok
CRS
tya_value_ok:
; Negative transfer: ensure 0x80 arrives in A
LDY #$80
LDA #$00
TYA
CMP #$7F
JEQ tya_negative_fail
CMP #$80
JEQ tya_negative_ok
CRS
tya_negative_ok:
HLT
tya_zero_fail:
CRS
tya_value_fail:
CRS
tya_negative_fail:
CRS
