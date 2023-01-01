
/*****************************************************************

 Copyright 2002   PIER LUCA MONTESSORO

 University of Udine
 ITALY

 montessoro@uniud.it
 www.montessoro.it

 This file is part of a freeware open source software package.
 It can be freely used (as it is or modified) as long as this
 copyright note is not removed.

 SIMCPU: common definitions module (simcpu.h)

******************************************************************/

/***********/
/* VERSION */
/***********/

#define SIM_VERSION "3.4"
#define ASM_VERSION "2.0"


/*******************/
/* FILE EXTENSIONS */
/*******************/

#define ASSEMBLY_SOURCE_EXT  ".axx"
#define EXECUTABLE_EXT       ".exx"
#define MAX_FNAME_LEN        256


/******************/
/* USER INTERFACE */
/******************/

#define TRACE_MODE_DISABLED     0
#define TRACE_MODE_INSTRUCTION  1
#define TRACE_MODE_EXTENDED     2


/********************/
/* CPU ARCHITECTURE */
/********************/

#define PARALLELISM  16UL
#define NREGISTERS   16UL
#define MEMSIZE      1UL << PARALLELISM

typedef unsigned short int  cpu_register;
typedef unsigned short int  word;
typedef unsigned char       byte;
typedef unsigned char       flag;
typedef struct TAG_instruction
        {
            byte opcode                              __attribute__ ((packed));
            struct TAG_instruction_fields
            {
               unsigned  group            : 2        __attribute__ ((packed));
               unsigned  addressing_mode  : 2        __attribute__ ((packed));
               unsigned  instruction_code : 4        __attribute__ ((packed));
               union TAG_operands
               {
                  struct TAG_registers
                  {
                     unsigned  first_register   : 4  __attribute__ ((packed));
                     unsigned  second_register  : 4  __attribute__ ((packed));
                  } registers                        __attribute__ ((packed));
                  byte offset                        __attribute__ ((packed));
               } operands                            __attribute__ ((packed));
            } instruction_fields                     __attribute__ ((packed));
         } instruction;


#define OPCODE(instruction_code) (((instruction_code) & 0xFF00) >> 8)
#define FIRST_REGISTER(instruction_code) (((instruction_code) & 0xF0) >> 4)
#define SECOND_REGISTER(instruction_code) ((instruction_code) & 0xF)
#define OFFSET(instruction_code) \
           ((signed int) ((char) ((instruction_code) & 0xFF)))


/******************************************/
/* ASSEMBLY LANGUAGE AND MACHINE LANGUAGE */
/******************************************/

#define MAXSYMLEN      64
#define MAXLINELEN    256
#define NINSTRUCTIONS  42
#define NDIRECTIVES     2
#define ENTRY_POINT  "START"


/* instructions formats */
#define OPCODE_ONLY    0
#define OPCODE_R_D16   1
#define OPCODE_R_D8    2
#define OPCODE_RR      3
#define OPCODE_R       4
#define OPCODE_D16     5
#define OPCODE_R_A16   6
#define OPCODE_A8      7
#define OPCODE_A16     8
#define DATA_ONLY_8    9
#define DATA_ONLY_16   10


/* codes */
#define GROUP_DATA_TRANSFER 0
#define GROUP_ARITM_LOGICAL 1
#define GROUP_INPUT_OUTPUT  2
#define GROUP_CONTROL       3

#define ADDR_IMMEDIATE      1
#define ADDR_ABSOLUTE       2
#define ADDR_REGISTER       3

#define LDWI  0x10
#define LDWA  0x20
#define LDWR  0x30
#define LDBI  0x11
#define LDBA  0x21
#define LDBR  0x31
#define STWA  0x22
#define STWR  0x32
#define STBA  0x23
#define STBR  0x33
#define MV    0x04
#define PUSH  0x08
#define POP   0x09
#define SPRD  0x0D
#define SPWR  0x0E
#define ADD   0x40
#define SUB   0x41
#define NOT   0x42
#define AND   0x43
#define OR    0x44
#define XOR   0x45
#define INC   0x48
#define DEC   0x49
#define LSH   0x4A
#define RSH   0x4B
#define INW   0x80
#define INB   0x81
#define OUTW  0x82
#define OUTB  0x83
#define TSTI  0x84
#define TSTO  0x85
#define BR    0xC0
#define JMP   0xC1
#define JMPZ  0xC2
#define JMPNZ 0xC3
#define JMPN  0xC4
#define JMPNN 0xC5
#define JMPC  0xC6
#define JMPV  0xC7
#define CALL  0xC8
#define RET   0xC9
#define HLT   0xCF


const struct TAG_machine_language
{
   char name[MAXSYMLEN];
   unsigned char format;
   byte opcode;
} machine_language[NINSTRUCTIONS] = 

{
   { "LDWI",  OPCODE_R_D16,  0x10 },
   { "LDWA",  OPCODE_R_A16,  0x20 },
   { "LDWR",  OPCODE_RR,     0x30 },
   { "LDBI",  OPCODE_R_D8,   0x11 },
   { "LDBA",  OPCODE_R_A16,  0x21 },
   { "LDBR",  OPCODE_RR,     0x31 },
   { "STWA",  OPCODE_R_A16,  0x22 },
   { "STWR",  OPCODE_RR,     0x32 },
   { "STBA",  OPCODE_R_A16,  0x23 },
   { "STBR",  OPCODE_RR,     0x33 },
   { "MV",    OPCODE_RR,     0x04 },
   { "PUSH",  OPCODE_R,      0x08 },
   { "POP",   OPCODE_R,      0x09 },
   { "SPRD",  OPCODE_R,      0x0D },
   { "SPWR",  OPCODE_R,      0x0E },
   { "ADD",   OPCODE_RR,     0x40 },
   { "SUB",   OPCODE_RR,     0x41 },
   { "NOT",   OPCODE_R,      0x42 },
   { "AND",   OPCODE_RR,     0x43 },
   { "OR",    OPCODE_RR,     0x44 },
   { "XOR",   OPCODE_RR,     0x45 },
   { "INC",   OPCODE_R,      0x48 },
   { "DEC",   OPCODE_R,      0x49 },
   { "LSH",   OPCODE_R,      0x4A },
   { "RSH",   OPCODE_R,      0x4B },
   { "INW",   OPCODE_R_D16,  0x80 },
   { "INB",   OPCODE_R_D16,  0x81 },
   { "OUTW",  OPCODE_R_D16,  0x82 },
   { "OUTB",  OPCODE_R_D16,  0x83 },
   { "TSTI",  OPCODE_D16,    0x84 },
   { "TSTO",  OPCODE_D16,    0x85 },
   { "BR",    OPCODE_A16,    0xC0 },
   { "JMP",   OPCODE_A8,     0xC1 },
   { "JMPZ",  OPCODE_A8,     0xC2 },
   { "JMPNZ", OPCODE_A8,     0xC3 },
   { "JMPN",  OPCODE_A8,     0xC4 },
   { "JMPNN", OPCODE_A8,     0xC5 },
   { "JMPC",  OPCODE_A8,     0xC6 },
   { "JMPV",  OPCODE_A8,     0xC7 },
   { "CALL",  OPCODE_A16,    0xC8 },
   { "RET",   OPCODE_ONLY,   0xC9 },
   { "HLT",   OPCODE_ONLY,   0xCF }
};


/* directives codes */
#define DIR_BYTE     0
#define DIR_WORD     1

const struct TAG_directives
{
   char name[MAXSYMLEN];
   unsigned short int directivecode;
} directives[NDIRECTIVES] = 

{
   { "BYTE",  DIR_BYTE },
   { "WORD",  DIR_WORD },
};
