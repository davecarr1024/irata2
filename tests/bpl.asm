; Test BPL instruction (Branch if negative clear)
; Negative clear after loading 0x01
LDA #$01
BPL bpl_taken_ok
CRS
bpl_taken_ok:
; Negative set after loading 0x80
LDA #$80
BPL bpl_not_taken_fail
HLT
bpl_not_taken_fail:
CRS
