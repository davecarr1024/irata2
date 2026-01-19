; Test PHP instruction (Push Status)
LDX #$FF
TXS
LDA #$80
CMP #$00
PHP
LDA #$00
CMP #$00
PLP
BMI php_neg_ok
CRS
php_neg_ok:
BCS php_carry_ok
CRS
php_carry_ok:
BNE php_zero_ok
CRS
php_zero_ok:
HLT
