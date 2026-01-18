LDA #$02
CMP #$01
BPL bpl_taken_ok
CRS
bpl_taken_ok:
LDA #$00
CMP #$01
BPL bpl_not_taken_fail
HLT
bpl_not_taken_fail:
CRS
