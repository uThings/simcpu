
; PUTS function: sends to the output device the string 
;                whose address is passed in R1

PUTS:      LDBR R2 R1           ; void puts (char *str)
           JMPZ END_PUTS
L_PUTS:    OUTB R2 0000
           TSTO 0000     
           JMPNZ L_PUTS
           INC R1
           JMP PUTS
END_PUTS:  LDBI R2 0A
LEND_PUTS: OUTB R2 0000
           TSTO 0000     
           JMPNZ LEND_PUTS
           RET
