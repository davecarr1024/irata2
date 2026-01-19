; Test EOR instruction
LDA #$FF
EOR #$0F
CMP #$F0
JEQ eor_ok
CRS
eor_ok:
; Test EOR with same value gives zero
LDA #$42
EOR #$42
JEQ eor_zero_ok
CRS
eor_zero_ok:
; Zero page EOR
LDA #$F0
STA $10
LDA #$FF
EOR $10
CMP #$0F
JEQ eor_zp_ok
CRS
eor_zp_ok:
; Absolute EOR
LDA #$0F
STA $0200
LDA #$F0
EOR $0200
CMP #$FF
JEQ eor_abs_ok
CRS
eor_abs_ok:
; Zero page X EOR
LDX #$02
LDA #$55
STA $12
LDA #$FF
EOR $10, X
CMP #$AA
JEQ eor_zpx_ok
CRS
eor_zpx_ok:
; Absolute X EOR
LDX #$01
LDA #$0F
STA $0201
LDA #$F0
EOR $0200, X
CMP #$FF
JEQ eor_abx_ok
CRS
eor_abx_ok:
; Absolute Y EOR
LDY #$03
LDA #$3C
STA $0303
LDA #$F0
EOR $0300, Y
CMP #$CC
JEQ eor_aby_ok
CRS
eor_aby_ok:
; Indexed indirect EOR
LDX #$04
LDA #$0C
STA $24
LDA #$02
STA $25
LDA #$0F
STA $020C
LDA #$FF
EOR ($20, X)
CMP #$F0
JEQ eor_izx_ok
CRS
eor_izx_ok:
; Indirect indexed EOR
LDY #$02
LDA #$00
STA $30
LDA #$02
STA $31
LDA #$3C
STA $0202
LDA #$F0
EOR ($30), Y
CMP #$CC
JEQ eor_izy_ok
CRS
eor_izy_ok:
HLT
