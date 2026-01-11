LDA #$00
JEQ jeq_taken_ok
CRS
jeq_taken_ok:
LDA #$01
JEQ jeq_not_taken_fail
LDA #$12
CMP #$12
JEQ jeq_cmp_ok
CRS
jeq_not_taken_fail:
CRS
jeq_cmp_ok:
HLT
