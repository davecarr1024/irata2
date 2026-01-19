; Test CLV instruction
LDA #$00
CMP #$01
LDA #$7F
ADC #$01
BVS overflow_set
CRS
overflow_set:
CLV
BVC clv_ok
CRS
clv_ok:
HLT
