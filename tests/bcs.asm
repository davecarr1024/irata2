LDA #$02
CMP #$01
BCS bcs_taken_ok
CRS
bcs_taken_ok:
LDA #$00
CMP #$01
BCS bcs_not_taken_fail
HLT
bcs_not_taken_fail:
CRS
