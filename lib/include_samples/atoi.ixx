#include "overflow.ixx"

; ATOI function: returns in R0 the numerical value represented by
;                the string whose address is passed in R1

ATOI:      LDBI R15 30          ; '0'
           LDBI R14 2D          ; '-'
           LDWI R0 0
L_ATOI:    LDBR R2 R1
           SUB R2 R14           ; negative?
           JMPNZ LABS_ATOI
           INC R1
LABS_ATOI: LDBR R2 R1
           JMPZ SIGN_ATOI
           SUB R15 R2
           MV R0 R4
           LSH R0               ; R0 * 10 = R0 + R0 + R0 << 3
           JMPC ATOI_OVFL
           LSH R0
           JMPC ATOI_OVFL
           LSH R0
           JMPC ATOI_OVFL
           ADD R4 R0
           JMPC ATOI_OVFL
           ADD R4 R0
           JMPC ATOI_OVFL
           ADD R2 R0
           JMPC ATOI_OVFL
           INC R1
           JMP LABS_ATOI
SIGN_ATOI: MV R14 R13
           JMPNZ END_ATOI
           SUB R0 R14           ; R14 here contains zero
           MV R14 R0 
END_ATOI:  RET
ATOI_OVFL: CALL OVERFLOW
           HLT
