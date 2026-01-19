; Test TSX instruction (Transfer SP to X)
LDX #$AB
TXS
LDX #$00
TSX
TXA
CMP #$AB
JEQ tsx_value_ok
CRS
tsx_value_ok:
LDX #$00
TXS
TSX
BEQ tsx_zero_ok
CRS
tsx_zero_ok:
HLT
