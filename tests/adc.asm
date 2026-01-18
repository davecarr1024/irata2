; Test ADC instruction
; Note: CMP sets carry when A >= operand, affecting subsequent ADC
LDA #$10
ADC #$05
CMP #$15
JEQ adc_ok
CRS
adc_ok:
; After CMP #$15 (equal), carry is set
; So 0xFF + 0x02 + 1(carry) = 0x102 = 0x02
LDA #$FF
ADC #$02
CMP #$02
JEQ adc_carry_ok
CRS
adc_carry_ok:
; Zero page add without carry
LDA #$00
CMP #$01
LDA #$05
STA $10
LDA #$10
ADC $10
CMP #$15
JEQ adc_zp_ok
CRS
adc_zp_ok:
; Zero page add with carry
LDA #$01
CMP #$00
LDA #$01
STA $11
LDA #$FF
ADC $11
CMP #$01
JEQ adc_zp_carry_ok
CRS
adc_zp_carry_ok:
HLT
