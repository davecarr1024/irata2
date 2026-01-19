; Test STA instruction (Store A)
; Zero page store
LDA #$42
STA $10
LDA #$00
LDA $10
CMP #$41
JEQ sta_basic_fail
CMP #$42
JEQ sta_basic_ok
CRS
sta_basic_ok:
; Zero page store zero
LDA #$00
STA $11
LDA #$FF
LDA $11
CMP #$01
JEQ sta_zero_fail
CMP #$00
JEQ sta_zero_ok
CRS
sta_zero_ok:
; Absolute store
LDA #$7E
STA $0200
LDA #$00
LDA $0200
CMP #$7D
JEQ sta_abs_fail
CMP #$7E
JEQ sta_abs_ok
CRS
sta_abs_ok:
; Zero page X-indexed store
LDX #$02
LDA #$55
STA $10, X
LDA #$00
LDA $12
CMP #$54
JEQ sta_zpx_fail
CMP #$55
JEQ sta_zpx_ok
CRS
sta_zpx_ok:
; Absolute X-indexed store
LDX #$01
LDA #$66
STA $0200, X
LDA #$00
LDA $0201
CMP #$65
JEQ sta_abx_fail
CMP #$66
JEQ sta_abx_ok
CRS
sta_abx_ok:
; Absolute Y-indexed store
LDY #$03
LDA #$77
STA $0300, Y
LDA #$00
LDA $0303
CMP #$76
JEQ sta_aby_fail
CMP #$77
JEQ sta_aby_ok
CRS
sta_aby_ok:
HLT
sta_basic_fail:
CRS
sta_zero_fail:
CRS
sta_abs_fail:
CRS
sta_zpx_fail:
CRS
sta_abx_fail:
CRS
sta_aby_fail:
CRS
