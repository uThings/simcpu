#include "gets.ixx"
#include "atoi.ixx"

INBUFADDR: word 0E000           ; beginning of input buffer area

; READNUM function: reads an integer from the input device
;                   and returns its value in R0

READNUM:   LDWA R1 INBUFADDR    ; int readnum (void)
           PUSH R1              
           CALL GETS            
           POP R1
           CALL ATOI
           RET
