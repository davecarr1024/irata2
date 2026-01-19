; Test SEC instruction
LDA #$00
CMP #$01
SEC
BCS sec_ok
CRS
sec_ok:
HLT
