; Test JEQ instruction (jump if equal)
; JEQ should jump when zero flag is set
LDA #$00
JEQ jeq_taken_ok
CRS
jeq_taken_ok:
; JEQ should not jump when zero flag is clear
LDA #$01
JEQ jeq_not_taken_fail
; JEQ should respect zero set by CMP
LDA #$12
CMP #$12
JEQ jeq_cmp_ok
CRS
jeq_not_taken_fail:
CRS
jeq_cmp_ok:
HLT
