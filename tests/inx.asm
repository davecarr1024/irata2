; Test INX instruction
LDX #$00
INX
TXA
CMP #$01
JEQ inx_ok
CRS
inx_ok:
LDX #$FF
INX
TXA
CMP #$00
JEQ inx_zero_ok
CRS
inx_zero_ok:
HLT
