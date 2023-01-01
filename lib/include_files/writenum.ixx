#include "div.ixx"

; WRITENUM function: writes on the output device the number 
;                    whose value is passed in R1
; NOTE: only non-negative numbers are supported

WRITENUM:  LDWI R10 1           ; void writenum (int n)
L1_WRNUM:  LDWI R11 09
           SUB R1 R11
           JMPNN L2_WRNUM
           LDWI R2 0A
           PUSH R10
           CALL DIV
           POP R10
           MV R0 R1
           PUSH R15
           INC R10
           JMP L1_WRNUM
L2_WRNUM:  LDWI R11 30
           ADD R1 R11
L3_WRNUM:  OUTB R11 0000
           TSTO 0000
           DEC R10
           JMPZ E_WRNUM
           POP R1               ; was R15, stored in the stack
           JMP L2_WRNUM
E_WRNUM:   RET
