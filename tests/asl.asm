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
; Zero page shift
LDA #$01
STA $10
ASL $10
LDA $10
CMP #$02
JEQ asl_zp_ok
CRS
asl_zp_ok:
; Absolute shift
LDA #$02
STA $0200
ASL $0200
LDA $0200
CMP #$04
JEQ asl_abs_ok
CRS
asl_abs_ok:
; Zero page X shift
LDX #$02
LDA #$01
STA $12
ASL $10, X
LDA $12
CMP #$02
JEQ asl_zpx_ok
CRS
asl_zpx_ok:
; Absolute X shift
LDX #$01
LDA #$02
STA $0301
ASL $0300, X
LDA $0301
CMP #$04
JEQ asl_abx_ok
CRS
asl_abx_ok:
HLT
