; Test BVS instruction (Branch if overflow set)
; Overflow set after adding 0x7F + 0x01
LDA #$7F
ADC #$01
BVS bvs_taken_ok
CRS
bvs_taken_ok:
; Overflow clear after adding 0x01 + 0x01
LDA #$01
ADC #$01
BVS bvs_not_taken_fail
HLT
bvs_not_taken_fail:
CRS
