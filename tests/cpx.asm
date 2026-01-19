; Test CPX instruction (Compare X with operand)
LDX #$12
CPX #$12
JEQ cpx_imm_ok
CRS
cpx_imm_ok:
LDX #$12
CPX #$34
JEQ cpx_imm_fail
LDX #$42
STX $10
LDX #$42
CPX $10
JEQ cpx_zp_ok
CRS
cpx_zp_ok:
LDX #$41
CPX $10
JEQ cpx_zp_fail
LDX #$55
STX $0200
LDX #$55
CPX $0200
JEQ cpx_abs_ok
CRS
cpx_abs_ok:
LDX #$54
CPX $0200
JEQ cpx_abs_fail
HLT
cpx_imm_fail:
CRS
cpx_zp_fail:
CRS
cpx_abs_fail:
CRS
