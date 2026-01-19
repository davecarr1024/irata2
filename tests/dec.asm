; Test DEC instruction
LDA #$02
STA $10
DEC $10
LDA $10
CMP #$01
JEQ dec_zp_ok
CRS
dec_zp_ok:
LDA #$00
STA $0200
DEC $0200
LDA $0200
CMP #$FF
JEQ dec_abs_ok
CRS
dec_abs_ok:
; Zero page X decrement
LDX #$02
LDA #$09
STA $12
DEC $10, X
LDA $12
CMP #$08
JEQ dec_zpx_ok
CRS
dec_zpx_ok:
; Absolute X decrement
LDX #$01
LDA #$00
STA $0301
DEC $0300, X
LDA $0301
CMP #$FF
JEQ dec_abx_ok
CRS
dec_abx_ok:
HLT
