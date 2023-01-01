
; DIV function: R0 <- R1 / R2 and  R15 <- R1 % R2 
; NOTE: only non-negative numbers are supported

DIV:       LDWI R0 0            ; int div (int a, int b)
           MV R1 R15
L_DIV:     MV R15 R3
           SUB R2 R3
           JMPN END_DIV
           SUB R2 R15
           INC R0
           JMP L_DIV
END_DIV:   RET
