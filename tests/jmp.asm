; Test JMP instruction (absolute and indirect)
JMP jmp_abs_target
CRS
jmp_abs_target:
LDA #$20
STA $10
LDA #$80
STA $11
JMP ($10)
CRS
.org $8020
jmp_ind_target:
HLT
