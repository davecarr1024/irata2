; Test BVC instruction (Branch if overflow clear)
; Overflow clear after simple add that does not overflow
LDA #$01
ADC #$01
BVC bvc_taken_ok
CRS
bvc_taken_ok:
; Overflow set after adding 0x7F + 0x01
LDA #$7F
ADC #$01
BVC bvc_not_taken_fail
HLT
bvc_not_taken_fail:
CRS
