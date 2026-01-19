; Test BCS instruction (Branch if carry set)
; Carry set after CMP when A >= operand
LDA #$02
CMP #$01
BCS bcs_taken_ok
CRS
bcs_taken_ok:
; Carry clear after CMP when A < operand
LDA #$00
CMP #$01
BCS bcs_not_taken_fail
HLT
bcs_not_taken_fail:
CRS
