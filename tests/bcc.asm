LDA #$00
CMP #$01
BCC bcc_taken_ok
CRS
bcc_taken_ok:
LDA #$02
CMP #$01
BCC bcc_not_taken_fail
HLT
bcc_not_taken_fail:
CRS
