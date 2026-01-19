; Test STY instruction (Store Y)
; Zero page store
LDY #$55
STY $10
LDA #$00
LDA $10
CMP #$54
JEQ sty_zp_fail
CMP #$55
JEQ sty_zp_ok
CRS
sty_zp_ok:
; Absolute store
LDY #$66
STY $0200
LDA #$00
LDA $0200
CMP #$65
JEQ sty_abs_fail
CMP #$66
JEQ sty_abs_ok
CRS
sty_abs_ok:
; Zero page X-indexed store
LDX #$02
LDY #$77
STY $10, X
LDA #$00
LDA $12
CMP #$76
JEQ sty_zpx_fail
CMP #$77
JEQ sty_zpx_ok
CRS
sty_zpx_ok:
; Absolute X-indexed store
LDX #$03
LDY #$88
STY $0202, X
LDA #$00
LDA $0205
CMP #$87
JEQ sty_abx_fail
CMP #$88
JEQ sty_abx_ok
CRS
sty_abx_ok:
HLT
sty_zp_fail:
CRS
sty_abs_fail:
CRS
sty_zpx_fail:
CRS
sty_abx_fail:
CRS
