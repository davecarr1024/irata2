LDA #$00
JEQ lda_zero_ok
CRS
lda_zero_ok:
LDA #$01
JEQ lda_zero_fail
LDA #$80
JEQ lda_negative_fail
LDA #$42
STA $10
LDA #$00
LDA $10
CMP #$42
JEQ lda_zp_ok
CRS
lda_zp_ok:
LDA #$00
STA $11
LDA #$FF
LDA $11
JEQ lda_zp_zero_ok
CRS
lda_zp_zero_ok:
LDA #$7F
STA $0200
LDA #$00
LDA $0200
CMP #$7F
JEQ lda_abs_ok
CRS
lda_abs_ok:
HLT
lda_zero_fail:
CRS
lda_negative_fail:
CRS
