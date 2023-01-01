
; STRTRIM function: shorten the string whose address is passed in R1
;                   to the lenght passed in R2, if longer

STRTRIM:   LDBR R10 R1           ; int strtrim (char *str, int len)
           JMPZ E_STRTRIM
           MV R2 R3
           JMPNZ CONTINUE
           LDBI R10 0
           STBR R10 R1
           JMP E_STRTRIM
CONTINUE:  INC R1
           DEC R2
           JMP STRTRIM
E_STRTRIM: RET
