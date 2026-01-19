; Test PLA instruction (Pull Accumulator)
LDX #$FF
TXS
LDA #$00
PHA
LDA #$FF
PLA
BEQ pla_zero_ok
CRS
pla_zero_ok:
LDA #$80
PHA
LDA #$00
PLA
BMI pla_negative_ok
CRS
pla_negative_ok:
HLT
