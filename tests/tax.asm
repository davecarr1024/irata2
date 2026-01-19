; Test TAX instruction (Transfer A to X)
; Basic transfer: reset A before reading back from X to avoid no-op pass
LDA #$42
TAX
LDA #$00
TXA
CMP #$41
JEQ tax_basic_fail
CMP #$42
JEQ tax_basic_ok
CRS
tax_basic_ok:
; Transfer zero: overwrite A so TXA must restore zero
LDA #$00
TAX
LDA #$55
TXA
CMP #$01
JEQ tax_zero_fail
CMP #$00
JEQ tax_zero_ok
CRS
tax_zero_ok:
; Transfer max value: ensure TXA restores 0xFF
LDA #$FF
TAX
LDA #$00
TXA
CMP #$FE
JEQ tax_max_fail
CMP #$FF
JEQ tax_max_ok
CRS
tax_max_ok:
HLT
tax_basic_fail:
CRS
tax_zero_fail:
CRS
tax_max_fail:
CRS
