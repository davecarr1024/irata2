; Test BRK instruction
LDA #$00
STA $20
BRK
LDA #$01
STA $20
LDA $20
CMP #$01
JEQ brk_ok
CRS
brk_ok:
HLT
.org $8100
brk_handler:
LDA #$02
STA $20
RTI
.org $FFFE
.byte $00, $81
