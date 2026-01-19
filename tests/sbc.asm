; Test SBC instruction
; Note: SBC uses borrow = !carry, with carry initially 0 (borrow = 1)
; So 0x10 - 0x05 - 1 = 0x0A
LDA #$10
SBC #$05
CMP #$0A
JEQ sbc_ok
CRS
sbc_ok:
; Zero page subtract with carry clear
LDA #$00
CMP #$01
LDA #$05
STA $10
LDA #$10
SBC $10
CMP #$0A
JEQ sbc_zp_ok
CRS
sbc_zp_ok:
; Absolute subtract with carry clear
LDA #$00
CMP #$01
LDA #$08
STA $0200
LDA #$20
SBC $0200
CMP #$17
JEQ sbc_abs_ok
CRS
sbc_abs_ok:
; Zero page X subtract with carry clear
LDA #$00
CMP #$01
LDX #$02
LDA #$04
STA $12
LDA #$12
SBC $10, X
CMP #$0D
JEQ sbc_zpx_ok
CRS
sbc_zpx_ok:
; Absolute X subtract with carry clear
LDA #$00
CMP #$01
LDX #$01
LDA #$06
STA $0201
LDA #$20
SBC $0200, X
CMP #$19
JEQ sbc_abx_ok
CRS
sbc_abx_ok:
; Absolute Y subtract with carry clear
LDA #$00
CMP #$01
LDY #$03
LDA #$07
STA $0303
LDA #$30
SBC $0300, Y
CMP #$28
JEQ sbc_aby_ok
CRS
sbc_aby_ok:
HLT
