; Test RTS instruction (Return from Subroutine)
LDX #$FF
TXS
LDA #$00
JSR rts_sub
LDA #$55
CMP #$55
JEQ rts_ok
CRS
rts_ok:
HLT
rts_sub:
LDA #$42
RTS
