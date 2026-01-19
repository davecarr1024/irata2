; Test BEQ instruction (Branch if zero set)
; Zero set after CMP when equal
LDA #$12
CMP #$12
BEQ beq_taken_ok
CRS
beq_taken_ok:
; Zero clear after CMP when not equal
LDA #$12
CMP #$34
BEQ beq_not_taken_fail
HLT
beq_not_taken_fail:
CRS
