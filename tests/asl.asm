; Test ASL instruction (Arithmetic Shift Left)
LDA #$01
ASL
CMP #$02
JEQ asl_ok
CRS
asl_ok:
; Test ASL with multiple bits
LDA #$55
ASL
CMP #$AA
JEQ asl_multi_ok
CRS
asl_multi_ok:
HLT
