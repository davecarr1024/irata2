; Test DEX instruction (Decrement X)
; Basic decrement from 0x02 to 0x01
LDX #$02
DEX
TXA
CMP #$02
JEQ dex_basic_fail
CMP #$01
JEQ dex_ok
CRS
dex_ok:
; Wrap from 0x00 to 0xFF
LDX #$00
DEX
TXA
CMP #$00
JEQ dex_wrap_fail
CMP #$FF
JEQ dex_wrap_ok
CRS
dex_wrap_ok:
HLT
dex_basic_fail:
CRS
dex_wrap_fail:
CRS
