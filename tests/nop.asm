LDA #$00
NOP
JEQ nop_zero_ok
CRS
nop_zero_ok:
LDA #$01
NOP
JEQ nop_zero_fail
HLT
nop_zero_fail:
CRS
