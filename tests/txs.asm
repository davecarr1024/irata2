; Test TXS instruction (Transfer X to SP)
LDX #$AA
TXS
TSX
TXA
CMP #$AA
JEQ txs_ok
CRS
txs_ok:
HLT
