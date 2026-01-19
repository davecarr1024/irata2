; Test PHA instruction (Push Accumulator)
LDX #$FF
TXS
LDA #$42
PHA
LDA #$00
PLA
CMP #$42
JEQ pha_value_ok
CRS
pha_value_ok:
TSX
TXA
CMP #$FF
JEQ pha_sp_ok
CRS
pha_sp_ok:
HLT
