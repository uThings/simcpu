
; ABS function: R0 <- abs(R1)

ABS:       MV R1 R0          ; int abs (int a)
           JMPN ABS_NEG
           JMP END_ABS
ABS_NEG:   LDWI R0 0
           SUB R1 R0
END_ABS:   RET
