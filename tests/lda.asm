LDA #$00
JEQ lda_zero_ok
CRS
lda_zero_ok:
LDA #$01
JEQ lda_zero_fail
LDA #$80
JEQ lda_negative_fail
HLT
lda_zero_fail:
CRS
lda_negative_fail:
CRS
