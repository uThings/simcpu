#include "overflow.ixx"
#include "mul.ixx"

; EXP function: R0 <- R1 ^ R2
; NOTE: only non-negative numbers are supported

EXP:       LDWI R0 1            ; int exp (int base, int exp)
           MV R2 R12
L_EXP:     JMPZ END_EXP
           PUSH R1
           PUSH R2
           MV R0 R2
           CALL MUL
           POP R2
           POP R1
           DEC R2
           JMP L_EXP
END_EXP:   RET
