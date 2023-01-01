
; TOUPPER function (input character in R1)

LOWER_A:        byte 61              ; 61h = 97
UPPER_A:        byte 41              ; 41h = 65
LOWER_Z:        byte 7A              ; 7Ah = 122
UPPER_Z:        byte 5A              ; 5Ah = 90 (unused; see TOLOWER)

TOUPPER:        LDBA R2 LOWER_A      ; char toupper (char ch)
                LDBA R3 LOWER_Z
                LDBA R4 UPPER_A

                MV R1 R10
                SUB R2 R10
                JMPN TOUPPER_END   ; ch < 'a'?
                MV R3 R10
                SUB R1 R10
                JMPN TOUPPER_END   ; ch > 'z'?
                SUB R2 R1
                ADD R4 R1
TOUPPER_END:    MV R1 R0
                RET
