LDA #$00
CMP #$01
LDA #$50
ADC #$50
BVS bvs_taken_ok
CRS
bvs_taken_ok:
LDA #$00
CMP #$01
LDA #$01
ADC #$01
BVS bvs_not_taken_fail
HLT
bvs_not_taken_fail:
CRS
