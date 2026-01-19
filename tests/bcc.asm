; Test BCC instruction (Branch if carry clear)
; Carry clear after CMP when A < operand
LDA #$00
CMP #$01
BCC bcc_taken_ok
CRS
bcc_taken_ok:
; Carry set after CMP when A >= operand
LDA #$02
CMP #$01
BCC bcc_not_taken_fail
HLT
bcc_not_taken_fail:
CRS
