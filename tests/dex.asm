; Test DEX instruction
LDX #$02
DEX
TXA
CMP #$01
JEQ dex_ok
CRS
dex_ok:
LDX #$00
DEX
TXA
CMP #$FF
JEQ dex_wrap_ok
CRS
dex_wrap_ok:
HLT
