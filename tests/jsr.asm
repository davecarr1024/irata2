; Test JSR instruction (Jump to Subroutine)
LDX #$FF
TXS
LDA #$00
STA $0200
JSR jsr_sub
LDA $0200
CMP #$42
JEQ jsr_ok
CRS
jsr_ok:
HLT
jsr_sub:
LDA #$42
STA $0200
RTS
