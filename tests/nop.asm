; Test NOP instruction (no operation)
; NOP should not change zero flag or accumulator
LDA #$00
NOP
JEQ nop_zero_ok
CRS
nop_zero_ok:
; NOP should not set zero when A is non-zero
LDA #$01
NOP
JEQ nop_zero_fail
HLT
nop_zero_fail:
CRS
