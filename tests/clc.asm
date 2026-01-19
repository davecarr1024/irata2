; Test CLC instruction
LDA #$01
CMP #$00
CLC
BCC clc_ok
CRS
clc_ok:
HLT
