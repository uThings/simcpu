
#include "tolower.ixx"

; TOLOWER_STR function: replaces lowecase character in the string with uppercase characters; string address is passed in R1

TOLOWER_STR:    LDBR R5 R1      ; void tolower_str (char *s)
                JMPZ TOLWRSTR_END
                PUSH R1
                MV R5 R1
                CALL TOLOWER
                POP R1
                STBR R0 R1
                INC R1
                JMP TOLOWER_STR

TOLWRSTR_END:   RET
