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
LDA #$42
STA $10
LDA #$42
CMP $10
JEQ cmp_zp_equal_ok
CRS
cmp_zp_equal_ok:
LDA #$41
CMP $10
JEQ cmp_zp_not_equal_fail
LDA #$55
STA $0200
LDA #$55
CMP $0200
JEQ cmp_abs_equal_ok
CRS
cmp_abs_equal_ok:
LDA #$54
CMP $0200
JEQ cmp_abs_not_equal_fail
HLT
cmp_not_equal_fail:
CRS
cmp_zp_not_equal_fail:
CRS
cmp_abs_not_equal_fail:
CRS
