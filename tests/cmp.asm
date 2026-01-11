LDA #$12
CMP #$12
JEQ cmp_equal_ok
CRS
cmp_equal_ok:
LDA #$12
CMP #$34
JEQ cmp_not_equal_fail
LDA #$00
CMP #$01
JEQ cmp_not_equal_fail
HLT
cmp_not_equal_fail:
CRS
