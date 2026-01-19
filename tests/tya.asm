LDY #$00
TYA
JEQ tya_zero_ok
CRS
tya_zero_ok:
LDY #$55
TYA
CMP #$55
JEQ tya_value_ok
CRS
tya_value_ok:
LDY #$80
TYA
CMP #$80
JEQ tya_negative_ok
CRS
tya_negative_ok:
HLT
