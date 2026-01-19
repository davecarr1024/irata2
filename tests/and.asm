; Test AND instruction
LDA #$FF
AND #$0F
CMP #$0F
JEQ and_ok
CRS
and_ok:
; Test AND with zero
LDA #$F0
AND #$0F
JEQ and_zero_ok
CRS
and_zero_ok:
; Zero page AND
LDA #$0F
STA $10
LDA #$FF
AND $10
CMP #$0F
JEQ and_zp_ok
CRS
and_zp_ok:
; Absolute AND
LDA #$F0
STA $0200
LDA #$0F
AND $0200
CMP #$00
JEQ and_abs_ok
CRS
and_abs_ok:
; Zero page X AND
LDX #$02
LDA #$33
STA $12
LDA #$3F
AND $10, X
CMP #$33
JEQ and_zpx_ok
CRS
and_zpx_ok:
; Absolute X AND
LDX #$01
LDA #$0C
STA $0201
LDA #$3C
AND $0200, X
CMP #$0C
JEQ and_abx_ok
CRS
and_abx_ok:
; Absolute Y AND
LDY #$03
LDA #$55
STA $0303
LDA #$F5
AND $0300, Y
CMP #$55
JEQ and_aby_ok
CRS
and_aby_ok:
; Indexed indirect AND
LDX #$04
LDA #$0A
STA $24
LDA #$02
STA $25
LDA #$0F
STA $020A
LDA #$F0
AND ($20, X)
CMP #$00
JEQ and_izx_ok
CRS
and_izx_ok:
; Indirect indexed AND
LDY #$03
LDA #$0B
STA $30
LDA #$02
STA $31
LDA #$0F
STA $020E
LDA #$FF
AND ($30), Y
CMP #$0F
JEQ and_izy_ok
CRS
and_izy_ok:
HLT
