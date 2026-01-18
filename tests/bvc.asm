LDA #$00
CMP #$01
LDA #$01
ADC #$01
BVC bvc_taken_ok
CRS
bvc_taken_ok:
LDA #$00
CMP #$01
LDA #$50
ADC #$50
BVC bvc_not_taken_fail
HLT
bvc_not_taken_fail:
CRS
