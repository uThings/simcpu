
; TOLOWER function (input character in R1)

LOWER_A:        byte 61              ; 61h = 97
UPPER_A:        byte 41              ; 41h = 65
LOWER_Z:        byte 7A              ; 7Ah = 122
UPPER_Z:        byte 5A              ; 5Ah = 90 (unused; see TOLOWER)

TOLOWER:        LDBA R2 UPPER_A      ; char tolower (char ch)
                LDBA R3 UPPER_Z
                LDBA R4 LOWER_A

                MV R1 R10
                SUB R2 R10
                JMPN TOLOWER_END   ; ch < 'A'?
                MV R3 R10
                SUB R1 R10
                JMPN TOLOWER_END   ; ch > 'Z'?
                SUB R2 R1
                ADD R4 R1
TOLOWER_END:    MV R1 R0
                RET
