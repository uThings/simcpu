
; SWAP function: *R1 <-> *R2

SWAP:      LDWR R10 R1            ; void swap (int *pa, int *pb)
           LDWR R11 R2
           STWR R10 R2
           STWR R11 R1
           RET
