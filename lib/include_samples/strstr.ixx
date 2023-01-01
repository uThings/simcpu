#include "strlen.ixx"
#include "strncmp.ixx"

; STRSTR function:  char *strstr(const char *R1, const char *R2);
;                   Locates  the  first  occurence  in  the  string pointed
;                   to by R1 of the sequence of characters in the string
;                   pointed to by R2 (excluding  the terminating null
;                   character).
;                   Returns  a  pointer to the located string segment, or
;                   a null pointer if the string R2 is not found.


STRSTR:        PUSH R1
               PUSH R2
               MV R2 R1
               CALL STRLEN
               MV R0 R3
               POP R2
               POP R1
L_STRSTR:      CALL STRNCMP
               MV R0 R10
               JMPZ FOUND_STRSTR
               INC R1
               LDBR R12 R1
               JMPZ NFOUND_STRSTR
               JMP L_STRSTR

NFOUND_STRSTR: XOR R0 R0
               RET

FOUND_STRSTR:  MV R1 R0
               RET




