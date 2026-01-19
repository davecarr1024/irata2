; Test RTI instruction
LDX #$FF
TXS
LDA #$80
PHA
LDA #$30
PHA
SEC
PHP
RTI
CRS
.org $8030
rti_target:
HLT
