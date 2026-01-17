; Test ROR instruction (Rotate Right)
; With carry clear, ROR shifts right and puts 0 in bit 7
LDA #$02
ROR
CMP #$01
JEQ ror_ok
CRS
ror_ok:
; Test ROR with carry set (via ADC overflow)
LDA #$FF
ADC #$01   ; Sets carry
LDA #$00
ROR        ; Should put carry into bit 7 = 0x80
CMP #$80
JEQ ror_carry_ok
CRS
ror_carry_ok:
HLT
