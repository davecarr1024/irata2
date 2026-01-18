LDA #$00
CMP #$01
BMI bmi_taken_ok
CRS
bmi_taken_ok:
LDA #$01
CMP #$01
BMI bmi_not_taken_fail
HLT
bmi_not_taken_fail:
CRS
