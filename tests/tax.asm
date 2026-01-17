; Test TAX instruction (Transfer A to X)
; Basic transfer
LDA #$42
TAX
TXA
CMP #$42
JEQ tax_basic_ok
CRS
tax_basic_ok:
; Transfer zero
LDA #$00
TAX
TXA
CMP #$00
JEQ tax_zero_ok
CRS
tax_zero_ok:
; Transfer max value
LDA #$FF
TAX
TXA
CMP #$FF
JEQ tax_max_ok
CRS
tax_max_ok:
HLT
