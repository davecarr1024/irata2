; Test INX instruction (Increment X)
; Basic increment from 0x00 to 0x01
LDX #$00
INX
TXA
CMP #$00
JEQ inx_basic_fail
CMP #$01
JEQ inx_ok
CRS
inx_ok:
; Wrap from 0xFF to 0x00
LDX #$FF
INX
TXA
CMP #$01
JEQ inx_zero_fail
CMP #$00
JEQ inx_zero_ok
CRS
inx_zero_ok:
HLT
inx_basic_fail:
CRS
inx_zero_fail:
CRS
