; Test INC instruction
LDA #$01
STA $10
INC $10
LDA $10
CMP #$02
JEQ inc_zp_ok
CRS
inc_zp_ok:
LDA #$FF
STA $0200
INC $0200
LDA $0200
CMP #$00
JEQ inc_abs_ok
CRS
inc_abs_ok:
; Zero page X increment
LDX #$02
LDA #$05
STA $12
INC $10, X
LDA $12
CMP #$06
JEQ inc_zpx_ok
CRS
inc_zpx_ok:
; Absolute X increment
LDX #$01
LDA #$09
STA $0301
INC $0300, X
LDA $0301
CMP #$0A
JEQ inc_abx_ok
CRS
inc_abx_ok:
HLT
