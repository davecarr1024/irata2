; Test ORA instruction
LDA #$F0
ORA #$0F
CMP #$FF
JEQ ora_ok
CRS
ora_ok:
; Test ORA with zero identity
LDA #$42
ORA #$00
CMP #$42
JEQ ora_zero_ok
CRS
ora_zero_ok:
; Zero page ORA
LDA #$0F
STA $10
LDA #$F0
ORA $10
CMP #$FF
JEQ ora_zp_ok
CRS
ora_zp_ok:
; Absolute ORA
LDA #$0F
STA $0200
LDA #$F0
ORA $0200
CMP #$FF
JEQ ora_abs_ok
CRS
ora_abs_ok:
; Zero page X ORA
LDX #$02
LDA #$0C
STA $12
LDA #$30
ORA $10, X
CMP #$3C
JEQ ora_zpx_ok
CRS
ora_zpx_ok:
; Absolute X ORA
LDX #$01
LDA #$22
STA $0201
LDA #$11
ORA $0200, X
CMP #$33
JEQ ora_abx_ok
CRS
ora_abx_ok:
; Absolute Y ORA
LDY #$03
LDA #$40
STA $0303
LDA #$0F
ORA $0300, Y
CMP #$4F
JEQ ora_aby_ok
CRS
ora_aby_ok:
; Indexed indirect ORA
LDX #$04
LDA #$0C
STA $24
LDA #$02
STA $25
LDA #$0F
STA $020C
LDA #$F0
ORA ($20, X)
CMP #$FF
JEQ ora_izx_ok
CRS
ora_izx_ok:
; Indirect indexed ORA
LDY #$02
LDA #$00
STA $30
LDA #$02
STA $31
LDA #$40
STA $0202
LDA #$0F
ORA ($30), Y
CMP #$4F
JEQ ora_izy_ok
CRS
ora_izy_ok:
HLT
