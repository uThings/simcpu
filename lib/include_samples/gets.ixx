
; GETS function: reads a line and stores the string
;                at memory address passed in R1

GETS:      LDBI R0 0A           ; void gets (char *str)
L_GETS:    INB R2 0001
           TSTI 0001     
           JMPNZ L_GETS
           STBR R2 R1    
           SUB R0 R2
           JMPZ END_GETS   
           INC R1
           JMP L_GETS
END_GETS:  LDBI R2 0
           STBR R2 R1    
           RET
