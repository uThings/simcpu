
#include "toupper.ixx"

; TOUPPER_STR function: replaces lowecase character in the string with uppercase characters; string address is passed in R1

TOUPPER_STR:    LDBR R5 R1           ; void toupper_str (char *s)
                JMPZ TOUPSTR_END
                PUSH R1
                MV R5 R1
                CALL TOUPPER
                POP R1
                STBR R0 R1
                INC R1
                JMP TOUPPER_STR

TOUPSTR_END:    RET
