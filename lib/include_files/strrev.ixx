
; STRREV function: string reverse (string address in R1)

STRREV:    MV R1 R10            ; make a copy of the string address
           LDWI R0 0
           PUSH R0              ; place a terminator in the stack
SREV_L1:   LDBR R2 R10
           JMPZ SREV_L2
           PUSH R2
           INC R10
           JMP SREV_L1
SREV_L2:   MV R1 R10            ; restore the pointer
SREV_L3:   POP R2
           JMPZ SREV_END
           STBR R2 R10
           INC R10
           JMP SREV_L3
SREV_END:  RET
