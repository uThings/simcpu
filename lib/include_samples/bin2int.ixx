
; BIN2INT function: returns in R0 the value of the binary number
;                   represented by the string whose address is passed in R1

BIN2INT:   LDBI R15 30       ; '0'
           LDWI R0 0

L_B2I:     LDBR R2 R1
           JMPZ END_B2I
           INC R1
           LSH R0            ; R0 <-- R0 * 2
           SUB R15 R2        ; subtract the '0' ASCII code value
           JMPZ ADD0
ADD1:      INC R0
ADD0:      JMP L_B2I
END_B2I:   RET
