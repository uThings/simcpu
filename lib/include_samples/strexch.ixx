
; STREXCH function: exchanges the strings addressed by R1 and R2

TEMPSTRADDR:  word 0F800    

STREXCH:   PUSH R1               ; save address of string 1
           PUSH R2               ; save address of string 2
           LDWA R2 TEMPSTRADDR
           CALL STRCPY           ; copy string 1 into temp
           POP R1                ; exchange the addresses
           POP R2
           PUSH R1               ; save address of string 2 (now in R1)
           CALL STRCPY           ; copy string 2 into string 1
           POP R2
           LDWA R1 TEMPSTRADDR
           CALL STRCPY           ; copy temp into string 2
           RET

STRCPY:    LDBR R10 R1
           STBR R10 R2
           JMPZ END_SCPY
           INC R1
           INC R2
           JMP STRCPY
END_SCPY:  RET


