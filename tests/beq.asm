LDA #$00
CMP #$00
BEQ beq_taken_ok
CRS
beq_taken_ok:
LDA #$01
CMP #$02
BEQ beq_not_taken_fail
HLT
beq_not_taken_fail:
CRS
