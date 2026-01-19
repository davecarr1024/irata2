; Test BNE instruction (Branch if zero clear)
; Zero clear after CMP when not equal
LDA #$12
CMP #$34
BNE bne_taken_ok
CRS
bne_taken_ok:
; Zero set after CMP when equal
LDA #$12
CMP #$12
BNE bne_not_taken_fail
HLT
bne_not_taken_fail:
CRS
