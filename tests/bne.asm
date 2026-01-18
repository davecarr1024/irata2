LDA #$00
CMP #$01
BNE bne_taken_ok
CRS
bne_taken_ok:
LDA #$02
CMP #$02
BNE bne_not_taken_fail
HLT
bne_not_taken_fail:
CRS
