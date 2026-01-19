LDY #$00
INY
TYA
CMP #$01
JEQ iny_increment_ok
CRS
iny_increment_ok:
LDY #$7F
INY
TYA
CMP #$80
JEQ iny_negative_ok
CRS
iny_negative_ok:
LDY #$FF
INY
TYA
JEQ iny_wrap_ok
CRS
iny_wrap_ok:
HLT
