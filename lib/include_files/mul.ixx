#include "overflow.ixx"

; MUL function: R0 <- R1 * R2
; NOTE: only non-negative numbers are supported

MUL:       LDWI R0 0            ; int mul (int a, int b)
           MV R1 R15
L_MUL:     JMPZ END_MUL
           ADD R2 R0
           JMPC MUL_OVFL
           DEC R15
           JMP L_MUL
END_MUL:   RET
MUL_OVFL:  CALL OVERFLOW
           HLT
