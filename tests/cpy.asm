; Test CPY instruction (Compare Y with operand)
LDY #$12
CPY #$12
JEQ cpy_imm_ok
CRS
cpy_imm_ok:
LDY #$12
CPY #$34
JEQ cpy_imm_fail
LDY #$42
STY $10
LDY #$42
CPY $10
JEQ cpy_zp_ok
CRS
cpy_zp_ok:
LDY #$41
CPY $10
JEQ cpy_zp_fail
LDY #$55
STY $0200
LDY #$55
CPY $0200
JEQ cpy_abs_ok
CRS
cpy_abs_ok:
LDY #$54
CPY $0200
JEQ cpy_abs_fail
HLT
cpy_imm_fail:
CRS
cpy_zp_fail:
CRS
cpy_abs_fail:
CRS
