#include "puts.ixx"

; OVERFLOW function: writes the "overflow" error message 
;                    and halts the program

OVFLMSG:   byte 4F              ; 'Overflow' (error message)
           byte 76
           byte 65
           byte 72
           byte 66
           byte 6C
           byte 6F
           byte 77
           byte 00

OVERFLOW:  LDWI R1 OVFLMSG
           CALL PUTS
           HLT
