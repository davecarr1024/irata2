; Test ROL instruction (Rotate Left)
; With carry clear, ROL shifts left and puts 0 in bit 0
LDA #$01
ROL
CMP #$02
JEQ rol_ok
CRS
rol_ok:
; Test ROL with carry set (via ADC overflow)
LDA #$FF
ADC #$01   ; Sets carry
LDA #$00
ROL        ; Should put carry into bit 0
CMP #$01
JEQ rol_carry_ok
CRS
rol_carry_ok:
; Zero page rotate
LDA #$00
CMP #$01
LDA #$01
STA $10
ROL $10
LDA $10
CMP #$02
JEQ rol_zp_ok
CRS
rol_zp_ok:
HLT
