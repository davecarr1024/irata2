LDY #$55
STY $10
LDA $10
CMP #$55
JEQ sty_zp_ok
CRS
sty_zp_ok:
LDY #$66
STY $0200
LDA $0200
CMP #$66
JEQ sty_abs_ok
CRS
sty_abs_ok:
LDX #$02
LDY #$77
STY $10, X
LDA $12
CMP #$77
JEQ sty_zpx_ok
CRS
sty_zpx_ok:
LDX #$03
LDY #$88
STY $0202, X
LDA $0205
CMP #$88
JEQ sty_abx_ok
CRS
sty_abx_ok:
HLT
