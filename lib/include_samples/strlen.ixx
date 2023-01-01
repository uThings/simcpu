
; STRLEN function: returns in R0 the lenght of the string 
;                  whose address is passed i R1

STRLEN:    LDWI R0 0            ; int strlen (char *str)
L_STRLEN:  LDBR R2 R1
           JMPZ E_STRLEN
           INC R1
           INC R0
           JMP L_STRLEN
E_STRLEN:  RET
