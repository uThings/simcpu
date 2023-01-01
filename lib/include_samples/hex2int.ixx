
; HEX2INT function: returns in R0 the value ot the hexadecimal number
;                   represented by the string whose address is passed in R1

HEX2INT:   LDBI R15 30       ; '0'
           LDBI R14 41       ; 'A'
           LDBI R13 0A       ; offset of values represented by the digits A-F
           LDWI R0 0

L_H2I:     LDBR R2 R1
           JMPZ END_H2I
           INC R1
           MV R2 R3          ; save the digit value
           SUB R14 R2        ; subtract the 'A' ASCII code value
           JMPN D_H2I        ; if negative, it'a decimal digit (0-9)
           ADD R13 R2        ; add the offset 
                             ; (now R2 contains the digit value)
           JMP BASE_H2I

D_H2I:     MV R3 R2          ; restore R2
           SUB R15 R2        ; subtract the '0' ASCII code value
                             ; (now R2 contains the digit value)

BASE_H2I:  LSH R0            ; myltiply R0 by 16
           LSH R0            ; ...
           LSH R0            ; ...
           LSH R0            ; ...
           ADD R2 R0
           JMP L_H2I

END_H2I:   RET
