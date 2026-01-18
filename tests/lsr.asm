; Test LSR instruction (Logical Shift Right)
LDA #$02
LSR
CMP #$01
JEQ lsr_ok
CRS
lsr_ok:
; Test LSR with multiple bits
LDA #$AA
LSR
CMP #$55
JEQ lsr_multi_ok
CRS
lsr_multi_ok:
; Zero page shift
LDA #$04
STA $10
LSR $10
LDA $10
CMP #$02
JEQ lsr_zp_ok
CRS
lsr_zp_ok:
; Absolute shift
LDA #$08
STA $0200
LSR $0200
LDA $0200
CMP #$04
JEQ lsr_abs_ok
CRS
lsr_abs_ok:
HLT
