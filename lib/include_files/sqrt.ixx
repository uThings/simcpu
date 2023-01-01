#include "mul.ixx"
#include "div.ixx"

; SQRT function: returns in R0 the square root of the number passed in R1
; NOTE: SQRT returns zero if the number in R1 is not a perfect square
;
; Newton algorithm is used:
;
; int sqrt(int n)
; {
;    int n1, n2;
; 
;    n1 = n;
;    n2 = 1;
;    while (n1 != n2 && n1 - n2 != 1 && n2 - n1 != 1)
;    {
;       n1 = (n1 + n2) / 2;
;       n2 = n / n1;
;    }
; 
;    if (n1*n1 != n)
;       return 0;
;    else
;       return n1;
; }
;

SQRT:      MV R1 R10            ; n1 is R11, n is R10
           MV R1 R11
           XOR R12 R12
           INC R12              ; n2 is R12   (n2 = 1)

                                ; while (n1 != n2 &&
                                ;        n1 - n2 != 1 && n2 - n1 != 1)
SQRT_L1:   MV R11 R3
           SUB R12 R3
           JMPZ SQRT_L2
           INC R3
           JMPZ SQRT_L2
           DEC R3
           DEC R3
           JMPZ SQRT_L2
                                ;    {
           ADD R12 R11          ;       n1 = (n1 + n2) / 2;
           RSH R11
           PUSH R11             ;       n2 = n / n1;
           PUSH R10
           MV R10 R1
           MV R11 R2
           CALL DIV
           POP R10
           POP R11
           MV R0 R12

           JMP SQRT_L1          ;    }

SQRT_L2:   MV R12 R1             ;    if (n1*n1 != n)
           MV R12 R2
           PUSH R10
           PUSH R12
           CALL MUL
           POP R12
           POP R10
           MV R0 R2
           MV R12 R0
           SUB R10 R2
           JMPZ SQRT_L3
           XOR R0 R0            ;       return 0;
                                ;    else
SQRT_L3:   RET                  ;       return n2;     
