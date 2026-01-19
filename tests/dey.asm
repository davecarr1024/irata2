; Test DEY instruction (Decrement Y)
; Decrement to zero from 0x01
LDY #$01
DEY
TYA
CMP #$01
JEQ dey_zero_fail
CMP #$00
JEQ dey_zero_ok
CRS
dey_zero_ok:
; Decrement into positive range from 0x80
LDY #$80
DEY
TYA
CMP #$80
JEQ dey_decrement_fail
CMP #$7F
JEQ dey_decrement_ok
CRS
dey_decrement_ok:
; Wrap from 0x00 to 0xFF
LDY #$00
DEY
TYA
CMP #$00
JEQ dey_wrap_fail
CMP #$FF
JEQ dey_wrap_ok
CRS
dey_wrap_ok:
HLT
dey_zero_fail:
CRS
dey_decrement_fail:
CRS
dey_wrap_fail:
CRS
