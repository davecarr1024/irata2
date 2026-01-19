; Test BMI instruction (Branch if negative set)
; Negative set after loading 0x80
LDA #$80
BMI bmi_taken_ok
CRS
bmi_taken_ok:
; Negative clear after loading 0x01
LDA #$01
BMI bmi_not_taken_fail
HLT
bmi_not_taken_fail:
CRS
