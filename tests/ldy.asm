; Test LDY instruction (Load Y Register)
; Immediate load zero: reset A before TYA to avoid no-op pass
LDA #$55
LDY #$00
TYA
CMP #$01
JEQ ldy_imm_zero_fail
CMP #$00
JEQ ldy_imm_zero_ok
CRS
ldy_imm_zero_ok:
; Immediate load value
LDA #$00
LDY #$42
TYA
CMP #$41
JEQ ldy_imm_value_fail
CMP #$42
JEQ ldy_imm_value_ok
CRS
ldy_imm_value_ok:
; Zero page load
LDA #$11
STA $10
LDA #$00
LDY $10
LDA #$55
TYA
CMP #$10
JEQ ldy_zp_fail
CMP #$11
JEQ ldy_zp_ok
CRS
ldy_zp_ok:
; Absolute load
LDA #$22
STA $0200
LDA #$00
LDY $0200
LDA #$55
TYA
CMP #$21
JEQ ldy_abs_fail
CMP #$22
JEQ ldy_abs_ok
CRS
ldy_abs_ok:
; Zero page X-indexed load
LDX #$02
LDA #$33
STA $12
LDA #$00
LDY $10, X
LDA #$55
TYA
CMP #$32
JEQ ldy_zpx_fail
CMP #$33
JEQ ldy_zpx_ok
CRS
ldy_zpx_ok:
; Absolute X-indexed load
LDX #$03
LDA #$44
STA $0205
LDA #$00
LDY $0202, X
LDA #$55
TYA
CMP #$43
JEQ ldy_abx_fail
CMP #$44
JEQ ldy_abx_ok
CRS
ldy_abx_ok:
HLT
ldy_imm_zero_fail:
CRS
ldy_imm_value_fail:
CRS
ldy_zp_fail:
CRS
ldy_abs_fail:
CRS
ldy_zpx_fail:
CRS
ldy_abx_fail:
CRS
