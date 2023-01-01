
; STRCMP function: compares the strings addressed by R1 and R2
;                  (returns 0 if equal)

STRCMP:    MV R1 R11   ; save R1 in R11
           MV R2 R12   ; save R2 in R12
L_STRCMP:  LDBR R3 R11
           LDBR R4 R12
           MV R3 R0
           SUB R4 R0
           JMPNZ E_STRCMP
           MV R3 R5    ; end of string?
           JMPZ E_STRCMP
           MV R4 R5
           JMPZ E_STRCMP
           INC R11
           INC R12
           JMP L_STRCMP
E_STRCMP:  RET



