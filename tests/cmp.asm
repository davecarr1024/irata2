; Test CMP instruction (Compare A with operand)
; Immediate equality
LDA #$12
CMP #$12
JEQ cmp_equal_ok
CRS
cmp_equal_ok:
; Immediate not-equal cases should not set zero
LDA #$12
CMP #$34
JEQ cmp_not_equal_fail
LDA #$00
CMP #$01
JEQ cmp_not_equal_fail
; Zero page equality
LDA #$42
STA $10
LDA #$42
CMP $10
JEQ cmp_zp_equal_ok
CRS
cmp_zp_equal_ok:
; Zero page not equal should not set zero
LDA #$41
CMP $10
JEQ cmp_zp_not_equal_fail
; Absolute equality
LDA #$55
STA $0200
LDA #$55
CMP $0200
JEQ cmp_abs_equal_ok
CRS
cmp_abs_equal_ok:
; Absolute not equal should not set zero
LDA #$54
CMP $0200
JEQ cmp_abs_not_equal_fail
; Zero page X-indexed equality
LDX #$02
LDA #$77
STA $12
LDA #$77
CMP $10, X
JEQ cmp_zpx_equal_ok
CRS
cmp_zpx_equal_ok:
; Zero page X-indexed not equal
LDA #$76
CMP $10, X
JEQ cmp_zpx_not_equal_fail
; Absolute X-indexed equality
LDX #$01
LDA #$88
STA $0301
LDA #$88
CMP $0300, X
JEQ cmp_abx_equal_ok
CRS
cmp_abx_equal_ok:
; Absolute X-indexed not equal
LDA #$87
CMP $0300, X
JEQ cmp_abx_not_equal_fail
; Absolute Y-indexed equality
LDY #$03
LDA #$99
STA $0403
LDA #$99
CMP $0400, Y
JEQ cmp_aby_equal_ok
CRS
cmp_aby_equal_ok:
; Absolute Y-indexed not equal
LDA #$98
CMP $0400, Y
JEQ cmp_aby_not_equal_fail
; Indexed indirect equality
LDX #$04
LDA #$0C
STA $24
LDA #$02
STA $25
LDA #$55
STA $020C
LDA #$55
CMP ($20, X)
JEQ cmp_izx_equal_ok
CRS
cmp_izx_equal_ok:
; Indirect indexed equality
LDY #$02
LDA #$00
STA $30
LDA #$02
STA $31
LDA #$66
STA $0202
LDA #$66
CMP ($30), Y
JEQ cmp_izy_equal_ok
CRS
cmp_izy_equal_ok:
HLT
cmp_not_equal_fail:
CRS
cmp_zp_not_equal_fail:
CRS
cmp_abs_not_equal_fail:
CRS
cmp_zpx_not_equal_fail:
CRS
cmp_abx_not_equal_fail:
CRS
cmp_aby_not_equal_fail:
CRS
