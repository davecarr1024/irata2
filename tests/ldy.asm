LDY #$00
TYA
JEQ ldy_imm_zero_ok
CRS
ldy_imm_zero_ok:
LDY #$42
TYA
CMP #$42
JEQ ldy_imm_value_ok
CRS
ldy_imm_value_ok:
LDA #$11
STA $10
LDY $10
TYA
CMP #$11
JEQ ldy_zp_ok
CRS
ldy_zp_ok:
LDA #$22
STA $0200
LDY $0200
TYA
CMP #$22
JEQ ldy_abs_ok
CRS
ldy_abs_ok:
LDX #$02
LDA #$33
STA $12
LDY $10, X
TYA
CMP #$33
JEQ ldy_zpx_ok
CRS
ldy_zpx_ok:
LDX #$03
LDA #$44
STA $0205
LDY $0202, X
TYA
CMP #$44
JEQ ldy_abx_ok
CRS
ldy_abx_ok:
HLT
