; Test LSR instruction (Logical Shift Right)
LDA #$02
LSR
CMP #$01
JEQ lsr_ok
CRS
lsr_ok:
; Test LSR with multiple bits
LDA #$AA
LSR
CMP #$55
JEQ lsr_multi_ok
CRS
lsr_multi_ok:
HLT
