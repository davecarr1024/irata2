; Test SBC instruction
; Note: SBC uses borrow = !carry, with carry initially 0 (borrow = 1)
; So 0x10 - 0x05 - 1 = 0x0A
LDA #$10
SBC #$05
CMP #$0A
JEQ sbc_ok
CRS
sbc_ok:
HLT
