
; STRCAT function: appends the string addressed by R2 
;                  at the end of the one addressed by R1

STRCAT:    LDBR R10 R1
           JMPZ SCAT_L1
           INC R1
           JMP STRCAT
SCAT_L1:   LDBR R10 R2
           JMPZ SCAT_L2
           STBR R10 R1
           INC R1
           INC R2
           JMP SCAT_L1
SCAT_L2:   STBR R10 R1           ; copy the '\0'
           RET
