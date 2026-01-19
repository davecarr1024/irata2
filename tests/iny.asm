; Test INY instruction (Increment Y)
; Basic increment from 0x00 to 0x01
LDY #$00
INY
TYA
CMP #$00
JEQ iny_increment_fail
CMP #$01
JEQ iny_increment_ok
CRS
iny_increment_ok:
; Increment into negative range
LDY #$7F
INY
TYA
CMP #$7F
JEQ iny_negative_fail
CMP #$80
JEQ iny_negative_ok
CRS
iny_negative_ok:
; Wrap from 0xFF to 0x00
LDY #$FF
INY
TYA
CMP #$01
JEQ iny_wrap_fail
CMP #$00
JEQ iny_wrap_ok
CRS
iny_wrap_ok:
HLT
iny_increment_fail:
CRS
iny_negative_fail:
CRS
iny_wrap_fail:
CRS
