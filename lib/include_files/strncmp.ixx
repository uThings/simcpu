
; STRNCMP function: compares up to R3 characters the strings addressed by
;                   R1 and R2 (returns 0 if equal)


STRNCMP:   MV R1 R11   ; save R1 in R11
           MV R2 R12   ; save R2 in R12
           XOR R0 R0
           MV R3 R15
           JMPZ E_STRNCMP
L_STRNCMP: LDBR R13 R11
           LDBR R14 R12
           MV R13 R0
           SUB R14 R0
           JMPNZ E_STRNCMP
           MV R13 R5    ; end of string?
           JMPZ E_STRNCMP
           MV R14 R5
           JMPZ E_STRNCMP
           DEC R3
           JMPZ E_STRNCMP
           INC R11
           INC R12
           JMP L_STRNCMP
E_STRNCMP: RET



