LDY #$01
DEY
TYA
JEQ dey_zero_ok
CRS
dey_zero_ok:
LDY #$80
DEY
TYA
CMP #$7F
JEQ dey_decrement_ok
CRS
dey_decrement_ok:
LDY #$00
DEY
TYA
CMP #$FF
JEQ dey_wrap_ok
CRS
dey_wrap_ok:
HLT
