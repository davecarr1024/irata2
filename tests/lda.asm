; Test LDA instruction (Load Accumulator)
; Immediate load zero: ensure A is overwritten
LDA #$55
LDA #$00
CMP #$01
JEQ lda_imm_zero_fail
CMP #$00
JEQ lda_zero_ok
CRS
lda_zero_ok:
; Immediate load non-zero
LDA #$01
CMP #$00
JEQ lda_imm_value_fail
CMP #$01
JEQ lda_imm_value_ok
CRS
lda_imm_value_ok:
; Immediate load negative
LDA #$80
CMP #$7F
JEQ lda_negative_fail
CMP #$80
JEQ lda_negative_ok
CRS
lda_negative_ok:
; Zero page load
LDA #$42
STA $10
LDA #$00
LDA $10
CMP #$41
JEQ lda_zp_fail
CMP #$42
JEQ lda_zp_ok
CRS
lda_zp_ok:
; Zero page load zero
LDA #$00
STA $11
LDA #$FF
LDA $11
CMP #$01
JEQ lda_zp_zero_fail
CMP #$00
JEQ lda_zp_zero_ok
CRS
lda_zp_zero_ok:
; Absolute load
LDA #$7F
STA $0200
LDA #$00
LDA $0200
CMP #$7E
JEQ lda_abs_fail
CMP #$7F
JEQ lda_abs_ok
CRS
lda_abs_ok:
; Zero page X-indexed load
LDA #$00
LDX #$02
LDA #$11
STA $12
LDA #$00
LDA $10, X
CMP #$10
JEQ lda_zpx_fail
CMP #$11
JEQ lda_zpx_ok
CRS
lda_zpx_ok:
; Absolute X-indexed load
LDA #$00
LDX #$01
LDA #$22
STA $0201
LDA #$00
LDA $0200, X
CMP #$21
JEQ lda_abx_fail
CMP #$22
JEQ lda_abx_ok
CRS
lda_abx_ok:
; Absolute Y-indexed load
LDA #$00
LDY #$03
LDA #$33
STA $0303
LDA #$00
LDA $0300, Y
CMP #$32
JEQ lda_aby_fail
CMP #$33
JEQ lda_aby_ok
CRS
lda_aby_ok:
; Indexed indirect (IZX) load
LDX #$04
LDA #$00
STA $24
LDA #$02
STA $25
LDA #$55
STA $0200
LDA #$00
LDA ($20, X)
CMP #$54
JEQ lda_izx_fail
CMP #$55
JEQ lda_izx_ok
CRS
lda_izx_ok:
; Indirect indexed (IZY) load
LDY #$03
LDA #$00
STA $30
LDA #$02
STA $31
LDA #$66
STA $0203
LDA #$00
LDA ($30), Y
CMP #$65
JEQ lda_izy_fail
CMP #$66
JEQ lda_izy_ok
CRS
lda_izy_ok:
HLT
lda_imm_zero_fail:
CRS
lda_imm_value_fail:
CRS
lda_negative_fail:
CRS
lda_zp_fail:
CRS
lda_zp_zero_fail:
CRS
lda_abs_fail:
CRS
lda_zpx_fail:
CRS
lda_abx_fail:
CRS
lda_aby_fail:
CRS
lda_izx_fail:
CRS
lda_izy_fail:
CRS
