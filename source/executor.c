
/*****************************************************************

 Copyright 2002   PIER LUCA MONTESSORO

 University of Udine
 ITALY

 montessoro@uniud.it
 www.montessoro.it

 This file is part of a freeware open source software package.
 It can be freely used (as it is or modified) as long as this
 copyright note is not removed.

 SIMCPU: simulation module (executor.c)

******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "simcpu.h"

#define INPUTBUFFERSIZE 256
#define MAXMSGLEN 256

/* CPU */
cpu_register  program_counter;
cpu_register  instruction_register;
cpu_register  stack_pointer;
flag          zero;
flag          negative;
flag          carry;
flag          overflow;
cpu_register  r[NREGISTERS];

/* memory */
byte memory[MEMSIZE];
char comments[MEMSIZE][MAXLINELEN+1];

/* I/O */
#define IO_DEVICES 2
#define MONITOR_ADDR  0
#define KEYBOARD_ADDR 1
char input_buffer[INPUTBUFFERSIZE] = "";
int input_index = 0;
int input_completion_flag = 0;
int output_completion_flag = 0;

/* execution control */
unsigned char breakpoints[MEMSIZE];
unsigned char single_step = 0;
unsigned char trace_mode = TRACE_MODE_DISABLED;


/*****************************************************************/

/* utility functions prototypes */

word loader (FILE *fp);
word initialize_executor (FILE *fp);
word executor (void);
void show_memory (word starting_address, int size);
void show_cpu (void);
void show_input_buffer (void);
const char *instruction_name (byte opcode);
int instruction_length (byte opcode);
void message (char *msg);
void error (char *err_msg);
const char *sim_version (void);

/* bus and cpu activity functions prototypes */

byte memory_read_byte (word address);
word memory_read_word (word address);
void memory_write_byte (word address, byte data);
void memory_write_word (word address, word data);
byte input_byte (word address);
word input_word (word address);
void output_byte (word address, byte data);
void output_word (word address, word data);
int test_input (word address);
int test_output (word address);
void set_program_counter (word value);
void set_instruction_register (word value);
void set_stack_pointer (word value);
word read_program_counter (void);
word read_instruction_register (void);
word read_stack_pointer (void);
void set_flag_zero (byte value);
void set_flag_negative (byte value);
void set_flag_carry (byte value);
void set_flag_overflow (byte value);
byte read_flag_zero (void);
byte read_flag_negative (void);
byte read_flag_carry (void);
byte read_flag_overflow (void);
void set_register (int index, word value);
word read_register (int index);
void halt (void);

/* execution control */
void user_control (word program_counter);
extern void set_break_on_next_instruction (int d);
void int_setup (void);


/*****************************************************************/

word loader (FILE *fp)
{
   int starting_address;
   int i, j, k, mem_content;
   char line[MAXLINELEN];

   /* starting address */
   if (fgets (line, MAXLINELEN, fp) == NULL)
      error ("Wrong input file format");
   sscanf (line, "%x", &starting_address);
   
   /* memory contents and comments */
   i = 0;
   while (i < (int) MEMSIZE && fgets (line, MAXLINELEN, fp) != NULL)
   {
      sscanf (line, "%x", &mem_content);
      memory[i] = (byte) mem_content;

      j = 2;
      k = 0;

      while (line[j] != '\n' && line[j] != '|')
         j++;

      if (line[j] == '|')
      {
         j++;
         while (line[j] != '\n')
            comments[i][k++] = line[j++];
      }
      comments[i][k] = '\0';
      breakpoints[i] = 0;

      i++;
   }

   /* set to zero the remaining memory cells */
   while (i < (int) MEMSIZE)
   {
      memory[i] = 0;
      comments[i][0] = '\0';
      breakpoints[i] = 0;
      i++;
   }

   return (word) starting_address;
}

/*****************************************************************/

byte memory_read_byte (word address)
{
   if (trace_mode == TRACE_MODE_EXTENDED)
   {
      char s[MAXMSGLEN];

      sprintf (s, "TRACE: %02X read from MEM[%04X]",
               (unsigned int) memory[address], (unsigned int) address);
      message (s);
   }

   return (memory[address]);
}

word memory_read_word (word address)
{
   if (trace_mode == TRACE_MODE_EXTENDED)
   {
      char s[MAXMSGLEN];

      sprintf (s, "TRACE: %04X read from MEM[%04X]",
               (unsigned int) (memory[address] | (memory [address+1] << 8)), 
               (unsigned int) address);
      message (s);
   }

   return (memory[address] | (memory [address+1] << 8));
}

void memory_write_byte (word address, byte data)
{
   if (trace_mode == TRACE_MODE_EXTENDED)
   {
      char s[MAXMSGLEN];

      sprintf (s, "TRACE: %02X written in MEM[%04X]",
               (unsigned int) data, (unsigned int) address);
      message (s);
   }

   memory [address] = data;
   return;
}

void memory_write_word (word address, word data)
{
   if (trace_mode == TRACE_MODE_EXTENDED)
   {
      char s[MAXMSGLEN];

      sprintf (s, "TRACE: %04X written in MEM[%04X]",
               (unsigned int) data, (unsigned int) address);
      message (s);
   }

   memory [address] = data & 0xFF;
   memory [address+1] = data >> 8;
   return;
}

byte input_byte (word address)
{
   if (address != KEYBOARD_ADDR)
      error ("wrong input device address");

   if (trace_mode == TRACE_MODE_EXTENDED)
   {
      char s[MAXMSGLEN];

      if (input_buffer [input_index] != '\0')
         sprintf (s, "TRACE: %02X read from input buffer",
                   (unsigned int) input_buffer[input_index]);
      else
         sprintf (s, "TRACE: read from input buffer failed (buffer empty)");

      message (s);
   }

   if (input_buffer [input_index] != '\0')
   {
      /* set the flag */
      input_completion_flag = 1;

      return input_buffer [input_index++];
   }
   else
   {
      input_completion_flag = 0;

      return 0;
   }
}

word input_word (word address)
{
   if (trace_mode == TRACE_MODE_EXTENDED)
   {
      char s[MAXMSGLEN];

      sprintf (s,
         "TRACE: attempting to read a 16-bits wide word from input buffer");

      message (s);
   }

   if (address != KEYBOARD_ADDR)
      error ("wrong input device address");
   else
      error ("16-bit binary input is not supported for keyboard");

   return 0; /* never reached */
}

void output_byte (word address, byte data)
{
   char outbuf[MAXMSGLEN];

   if (address != MONITOR_ADDR)
      error ("wrong output device address");

   if (trace_mode == TRACE_MODE_EXTENDED)
   {
      char s[MAXMSGLEN];

      sprintf (s, "TRACE: output request for data %02X", (unsigned int) data);

      message (s);
   }

   if (data >= 32 && data <= 127)
      sprintf (outbuf, "OUTPUT: %02X (ASCII %c)",
               (unsigned int) data, (char) data);
   else
      sprintf (outbuf, "OUTPUT: %02X", (unsigned int) data);

   message (outbuf);

   /* set the flag */
   output_completion_flag = 1;

   return;
}

void output_word (word address, word data)
{
   char outbuf[MAXMSGLEN];

   if (address != MONITOR_ADDR)
      error ("wrong output device address");

   if (trace_mode == TRACE_MODE_EXTENDED)
   {
      char s[MAXMSGLEN];

      sprintf (s, "TRACE: output request for data %04X", (unsigned int) data);

      message (s);
   }

   sprintf (outbuf, "OUTPUT: %04X", (unsigned int) data);

   message (outbuf);

   /* set the flag */
   output_completion_flag = 1;

   return;
}

int test_input (word address)
{
   if (address != KEYBOARD_ADDR)
      error ("wrong input device address");

   if (trace_mode == TRACE_MODE_EXTENDED)
   {
      char s[MAXMSGLEN];

      sprintf (s, "TRACE: testing input status at address %04X",
               (unsigned int) address);

      message (s);
   }

   return input_completion_flag;
}

int test_output (word address)
{
   if (address != MONITOR_ADDR)
      error ("wrong output device address");

   if (trace_mode == TRACE_MODE_EXTENDED)
   {
      char s[MAXMSGLEN];

      sprintf (s, "TRACE: testing output completion at address %04X",
               (unsigned int) address);

      message (s);
   }

   return output_completion_flag;
}

void set_program_counter (word value)
{
   if (trace_mode == TRACE_MODE_EXTENDED)
   {
      char s[MAXMSGLEN];

      sprintf (s, "TRACE: program counter set from %04X to %04X",
               (unsigned int) program_counter, (unsigned int) value);

      message (s);
   }

   program_counter = value;
   return;
}

void set_instruction_register (word value)
{
   if (trace_mode == TRACE_MODE_EXTENDED)
   {
      char s[MAXMSGLEN];

      sprintf (s, "TRACE: instruction_register set from %04X to %04X",
               (unsigned int) instruction_register, (unsigned int) value);

      message (s);
   }

   instruction_register = value;
   return;
}

void set_stack_pointer (word value)
{
   if (trace_mode == TRACE_MODE_EXTENDED)
   {
      char s[MAXMSGLEN];

      sprintf (s, "TRACE: stack pointer set from %04X to %04X",
               (unsigned int) stack_pointer, (unsigned int) value);

      message (s);
   }

   stack_pointer = value;
   return;
}

word read_program_counter (void)
{
   if (trace_mode == TRACE_MODE_EXTENDED)
   {
      char s[MAXMSGLEN];

      sprintf (s, "TRACE: %04X read from program counter",
               (unsigned int) program_counter);

      message (s);
   }

   return program_counter;
}

word read_instruction_register (void)
{
   if (trace_mode == TRACE_MODE_EXTENDED)
   {
      char s[MAXMSGLEN];

      sprintf (s, "TRACE: %04X read from instruction_register (opcode: %s)",
               (unsigned int) instruction_register, 
               instruction_name (OPCODE (instruction_register)));

      message (s);
   }

   return instruction_register;
}

word read_stack_pointer (void)
{
   if (trace_mode == TRACE_MODE_EXTENDED)
   {
      char s[MAXMSGLEN];

      sprintf (s, "TRACE: %04X read from stack_pointer",
               (unsigned int) stack_pointer);

      message (s);
   }

   return stack_pointer;
}

void set_flag_zero (byte value)
{
   if (trace_mode == TRACE_MODE_EXTENDED)
   {
      char s[MAXMSGLEN];

      sprintf (s, "TRACE: flag zero set to %1X", (unsigned int) value);

      message (s);
   }

   zero = value;
   return;
}

void set_flag_negative (byte value)
{
   if (trace_mode == TRACE_MODE_EXTENDED)
   {
      char s[MAXMSGLEN];

      sprintf (s, "TRACE: flag negative set to %1X", (unsigned int) value);

      message (s);
   }

   negative = value;
   return;
}

void set_flag_carry (byte value)
{
   if (trace_mode == TRACE_MODE_EXTENDED)
   {
      char s[MAXMSGLEN];

      sprintf (s, "TRACE: flag carry set to %1X", (unsigned int) value);

      message (s);
   }

   carry = value;
   return;
}

void set_flag_overflow (byte value)
{
   if (trace_mode == TRACE_MODE_EXTENDED)
   {
      char s[MAXMSGLEN];

      sprintf (s, "TRACE: flag overflow set to %1X", (unsigned int) value);

      message (s);
   }

   overflow = value;
   return;
}

byte read_flag_zero (void)
{
   if (trace_mode == TRACE_MODE_EXTENDED)
   {
      char s[MAXMSGLEN];

      sprintf (s, "TRACE: %1X read from flag zero", (unsigned int) zero);

      message (s);
   }

   return zero;
}

byte read_flag_negative (void)
{
   if (trace_mode == TRACE_MODE_EXTENDED)
   {
      char s[MAXMSGLEN];

      sprintf (s, "TRACE: %1X read from flag negative",
               (unsigned int) negative);

      message (s);
   }

   return negative;
}

byte read_flag_carry (void)
{
   if (trace_mode == TRACE_MODE_EXTENDED)
   {
      char s[MAXMSGLEN];

      sprintf (s, "TRACE: %1X read from flag carry",
               (unsigned int) carry);

      message (s);
   }

   return carry;
}

byte read_flag_overflow (void)
{
   if (trace_mode == TRACE_MODE_EXTENDED)
   {
      char s[MAXMSGLEN];

      sprintf (s, "TRACE: %1X read from flag overflow",
               (unsigned int) overflow);

      message (s);
   }

   return overflow;
}

void set_register (int index, word value)
{
   if (trace_mode == TRACE_MODE_EXTENDED)
   {
      char s[MAXMSGLEN];

      sprintf (s, "TRACE: register R%1d set to %04X",
               index, (unsigned int) value);

      message (s);
   }

   r[index] = value;
   return;
}

word read_register (int index)
{
   if (trace_mode == TRACE_MODE_EXTENDED)
   {
      char s[MAXMSGLEN];

      sprintf (s, "TRACE: %04X read from register R%1d",
               (unsigned int) r[index], index);

      message (s);
   }

   return r[index];
}

void halt()
{
   if (trace_mode == TRACE_MODE_EXTENDED)
   {
      char s[MAXMSGLEN];

      sprintf (s, "TRACE: halt instruction");

      message (s);
   }

   return;
}

/*****************************************************************/

word initialize_executor (FILE *fp)
{
   int i;

   /* load the program and initialize the program counter */
   program_counter = loader (fp);

   /* CPU initialization */
   instruction_register = 0;
   stack_pointer = 0;
   zero = 0;
   negative = 0;
   carry = 0;
   overflow = 0;

   for (i = 0; i < (int) NREGISTERS; i++)
      r[i] = 0;

   return program_counter;
}

/*****************************************************************/

word executor (void)
{
   long int t, x, y;

   while (1)
   {
      if (trace_mode == TRACE_MODE_EXTENDED)
      {
         char s[MAXMSGLEN];

         sprintf (s,
            "\nTRACE: starting new instruction, program counter = %04X",
            (unsigned int) program_counter);

         message (s);
      }

      /* instruction fetch */
      set_instruction_register (memory_read_word (program_counter));

      if (trace_mode == TRACE_MODE_INSTRUCTION)
      {
         char s[MAXMSGLEN];

         sprintf (s, "Executing instruction at address %04X:",
                  (unsigned int) program_counter);
         message (s);
         show_memory (program_counter,
                      instruction_length (OPCODE (instruction_register)));
      }

      /* program_counter update */
      set_program_counter (program_counter + 2);

      /* instruction decode */
      switch (OPCODE (read_instruction_register()))
      {
         case LDWI:
            set_register (FIRST_REGISTER(instruction_register), 
               t = memory_read_word (read_program_counter()));
            if ((t & 0xFFFF) == 0)
               set_flag_zero (1);
            else
               set_flag_zero (0);
            if (t & 0x8000)
               set_flag_negative (1);
            else
               set_flag_negative (0);
            set_program_counter (program_counter + 2);
            break;

         case LDWA:
            set_register (FIRST_REGISTER(instruction_register),
               t = memory_read_word (memory_read_word
                                        (read_program_counter())));
            if ((t & 0xFFFF) == 0)
               set_flag_zero (1);
            else
               set_flag_zero (0);
            if (t & 0x8000)
               set_flag_negative (1);
            else
               set_flag_negative (0);
            set_program_counter (program_counter + 2);
            break;

         case LDWR:
            set_register (FIRST_REGISTER(instruction_register), 
               t = memory_read_word
                      (read_register
                          (SECOND_REGISTER(instruction_register))));
            if ((t & 0xFFFF) == 0)
               set_flag_zero (1);
            else
               set_flag_zero (0);
            if (t & 0x8000)
               set_flag_negative (1);
            else
               set_flag_negative (0);
            break;

         case LDBI:
            set_register (FIRST_REGISTER(instruction_register), 
               t = memory_read_byte (read_program_counter()));
            if ((t & 0xFFFF) == 0)
               set_flag_zero (1);
            else
               set_flag_zero (0);
            set_program_counter (program_counter + 1);
            break;

         case LDBA:
            set_register (FIRST_REGISTER(instruction_register), 
               t = memory_read_byte
                      (memory_read_word (read_program_counter())));
            if ((t & 0xFFFF) == 0)
               set_flag_zero (1);
            else
               set_flag_zero (0);
            set_program_counter (program_counter + 2);
            break;

         case LDBR:
            set_register (FIRST_REGISTER(instruction_register), 
               t = memory_read_byte
                      (read_register
                          (SECOND_REGISTER(instruction_register))));
            if ((t & 0xFFFF) == 0)
               set_flag_zero (1);
            else
               set_flag_zero (0);
            break;

         case STWA:
            memory_write_word (memory_read_word (read_program_counter()),
                               read_register
                                  (FIRST_REGISTER(instruction_register)));
            set_program_counter (program_counter + 2);
            break;

         case STWR:
            memory_write_word (read_register
                                 (SECOND_REGISTER(instruction_register)),
                               read_register
                                 (FIRST_REGISTER(instruction_register)));
            break;

         case STBA:
            memory_write_byte (memory_read_word (read_program_counter()),
                               (byte) read_register
                                      (FIRST_REGISTER(instruction_register)));
            set_program_counter (program_counter + 2);
            break;

         case STBR:
            memory_write_byte (read_register
                                  (SECOND_REGISTER(instruction_register)),
                               (byte) read_register
                                      (FIRST_REGISTER(instruction_register)));
            break;

         case MV:
            set_register (SECOND_REGISTER(instruction_register) , 
               t = read_register (FIRST_REGISTER(instruction_register)));
            if ((t & 0xFFFF) == 0)
               set_flag_zero (1);
            else
               set_flag_zero (0);
            if (t & 0x8000)
               set_flag_negative (1);
            else
               set_flag_negative (0);
            break;

         case PUSH:
            memory_write_word (read_stack_pointer(),
                               read_register
                                  (FIRST_REGISTER(instruction_register)));
            set_stack_pointer (stack_pointer + 2);
            break;

         case POP:
            set_stack_pointer (stack_pointer - 2);
            set_register (FIRST_REGISTER(instruction_register), 
                          t = memory_read_word (read_stack_pointer()));
            if ((t & 0xFFFF) == 0)
               set_flag_zero (1);
            else
               set_flag_zero (0);
            if (t & 0x8000)
               set_flag_negative (1);
            else
               set_flag_negative (0);
            break;

         case SPRD:
            set_register (FIRST_REGISTER(instruction_register),
                          t = read_stack_pointer());
            if ((t & 0xFFFF) == 0)
               set_flag_zero (1);
            else
               set_flag_zero (0);
            if (t & 0x8000)
               set_flag_negative (1);
            else
               set_flag_negative (0);
            break;

         case SPWR:
            set_stack_pointer (read_register
                                  (FIRST_REGISTER(instruction_register)));
            break;

         case ADD:
            x = r[FIRST_REGISTER(instruction_register)];
            y = r[SECOND_REGISTER(instruction_register)];
            set_register (SECOND_REGISTER(instruction_register),
               t = r[SECOND_REGISTER(instruction_register)] + 
                   read_register (FIRST_REGISTER(instruction_register)));
            if ((t & 0xFFFF) == 0)
               set_flag_zero (1);
            else
               set_flag_zero (0);
            if (t & 0x8000)
               set_flag_negative (1);
            else
               set_flag_negative (0);
            if (t & 0x10000)
               set_flag_carry (1);
            else
               set_flag_carry (0);
            if (((x & 0x8000) == 0 && (y & 0x8000) != 0) || 
                ((x & 0x8000) != 0 && (y & 0x8000) == 0))
               set_flag_overflow (0); /* different signs: no overflow */
            else
               if (((x & 0x8000) == 0 && (t & 0x8000) != 0) ||
                   ((x & 0x8000) != 0 && (t & 0x8000) == 0))
                  set_flag_overflow (1); /* sign changed: overflow */
               else
                  set_flag_overflow (0); /* sign not changed: no overflow */
            break;

         case SUB:
            x = r[FIRST_REGISTER(instruction_register)];
            y = r[SECOND_REGISTER(instruction_register)];
            set_register (SECOND_REGISTER(instruction_register), 
               t = r[SECOND_REGISTER(instruction_register)] - 
                   read_register (FIRST_REGISTER(instruction_register)));
            if ((t & 0xFFFF) == 0)
               set_flag_zero (1);
            else
               set_flag_zero (0);
            if (t & 0x8000)
               set_flag_negative (1);
            else
               set_flag_negative (0);
            if (t & 0x10000)
               set_flag_carry (1);
            else
               set_flag_carry (0);
            if (((x & 0x8000) == 0 && (y & 0x8000) == 0) || 
                ((x & 0x8000) != 0 && (y & 0x8000) != 0))
               set_flag_overflow (0); /* same sign: no overflow */
            else
               if (((x & 0x8000) == 0 && (t & 0x8000) != 0) ||
                   ((x & 0x8000) != 0 && (t & 0x8000) == 0))
                  set_flag_overflow (1); /* t's sign != first operand's */
               else
                  set_flag_overflow (0); /* t's sign == first operand's */
            break;

         case NOT:
            set_register (FIRST_REGISTER(instruction_register), 
               t = ~read_register (FIRST_REGISTER(instruction_register)));
            if ((t & 0xFFFF) == 0)
               set_flag_zero (1);
            else
               set_flag_zero (0);
            if (t & 0x8000)
               set_flag_negative (1);
            else
               set_flag_negative (0);
            set_flag_carry (0);
            set_flag_overflow (0);
            break;

         case AND:
            set_register (SECOND_REGISTER(instruction_register),
               t = r[SECOND_REGISTER(instruction_register)] &
                   read_register (FIRST_REGISTER(instruction_register)));
            if ((t & 0xFFFF) == 0)
               set_flag_zero (1);
            else
               set_flag_zero (0);
            if (t & 0x8000)
               set_flag_negative (1);
            else
               set_flag_negative (0);
            set_flag_carry (0);
            set_flag_overflow (0);
            break;

         case OR:
            set_register (SECOND_REGISTER(instruction_register), 
               t = r[SECOND_REGISTER(instruction_register)] |
                   read_register (FIRST_REGISTER(instruction_register)));
            if ((t & 0xFFFF) == 0)
               set_flag_zero (1);
            else
               set_flag_zero (0);
            if (t & 0x8000)
               set_flag_negative (1);
            else
               set_flag_negative (0);
            set_flag_carry (0);
            set_flag_overflow (0);
            break;

         case XOR:
           set_register (SECOND_REGISTER(instruction_register), 
               t = r[SECOND_REGISTER(instruction_register)] ^
                   read_register (FIRST_REGISTER(instruction_register)));
            if ((t & 0xFFFF) == 0)
               set_flag_zero (1);
            else
               set_flag_zero (0);
            if (t & 0x8000)
               set_flag_negative (1);
            else
               set_flag_negative (0);
            set_flag_carry (0);
            set_flag_overflow (0);
            break;

         case INC:
            x = r[FIRST_REGISTER(instruction_register)];
            set_register (FIRST_REGISTER(instruction_register), 
                          t = r[FIRST_REGISTER(instruction_register)] + 1);
            if ((t & 0xFFFF) == 0)
               set_flag_zero (1);
            else
               set_flag_zero (0);
            if (t & 0x8000)
               set_flag_negative (1);
            else
               set_flag_negative (0);
            if (t & 0x10000)
               set_flag_carry (1);
            else
               set_flag_carry (0);
            if ((x & 0x8000) == 0 && (t & 0x8000) != 0)
               set_flag_overflow (1); /* sign changed: overflow */
            else
               set_flag_overflow (0); /* sign not changed: no overflow */
            break;

         case DEC:
            x = r[FIRST_REGISTER(instruction_register)];
            set_register (FIRST_REGISTER(instruction_register),
                          t = r[FIRST_REGISTER(instruction_register)] - 1);
            if ((t & 0xFFFF) == 0)
               set_flag_zero (1);
            else
               set_flag_zero (0);
            if (t & 0x8000)
               set_flag_negative (1);
            else
               set_flag_negative (0);
            if (t & 0x10000)
               set_flag_carry (1);
            else
               set_flag_carry (0);
            if ((x & 0x8000) != 0 && (t & 0x8000) == 0)
               set_flag_overflow (1); /* sign changed: overflow */
            else
               set_flag_overflow (0); /* sign not changed: no overflow */
            break;

         case LSH:
            set_register (FIRST_REGISTER(instruction_register), 
                          t = r[FIRST_REGISTER(instruction_register)] << 1);
            if ((t & 0xFFFF) == 0)
               set_flag_zero (1);
            else
               set_flag_zero (0);
            if (t & 0x8000)
               set_flag_negative (1);
            else
               set_flag_negative (0);
            if (t & 0x10000)
               set_flag_carry (1);
            else
               set_flag_carry (0);
            set_flag_overflow (0);
            break;

         case RSH:
            set_register (FIRST_REGISTER(instruction_register), 
                          t = r[FIRST_REGISTER(instruction_register)] >> 1);
            if ((t & 0xFFFF) == 0)
               set_flag_zero (1);
            else
               set_flag_zero (0);
            if (t & 0x8000)
               set_flag_negative (1);
            else
               set_flag_negative (0);
            if (t & 0x10000)
               set_flag_carry (1);
            else
               set_flag_carry (0);
            set_flag_overflow (0);
            break;

         case INW:
            set_register (FIRST_REGISTER(instruction_register), 
                          t = input_word (memory_read_word
                                             (read_program_counter())));
            set_program_counter (program_counter + 2);
            if ((t & 0xFFFF) == 0)
               set_flag_zero (1);
            else
               set_flag_zero (0);
            if (t & 0x8000)
               set_flag_negative (1);
            else
               set_flag_negative (0);
            break;

         case INB:
            set_register (FIRST_REGISTER(instruction_register), 
                          t = input_byte (memory_read_word
                                             (read_program_counter())));
            if ((t & 0xFFFF) == 0)
               set_flag_zero (1);
            else
               set_flag_zero (0);
            if (t & 0x8000)
               set_flag_negative (1);
            else
               set_flag_negative (0);
            set_program_counter (program_counter + 2);
            break;

         case OUTW:
            output_word (memory_read_word (read_program_counter()),
                         read_register
                            (FIRST_REGISTER(instruction_register)));
            set_program_counter (program_counter + 2);
            break;

         case OUTB:
            output_byte (memory_read_word (read_program_counter()),
                         (byte) read_register
                                   (FIRST_REGISTER(instruction_register)));
            set_program_counter (program_counter + 2);
            break;

         case TSTI:
            if (test_input (memory_read_word (read_program_counter())) == 1)
            {
               set_flag_zero (1);
               input_completion_flag = 0;
            }
            else
               set_flag_zero (0);
            set_program_counter (program_counter + 2);
            break;

         case TSTO:
            if (test_output (memory_read_word (read_program_counter())) == 1)
            {
               set_flag_zero (1);
               output_completion_flag = 0;
            }
            else
               set_flag_zero (0);
            set_program_counter (program_counter + 2);
            break;

         case BR:
            set_program_counter (memory_read_word (read_program_counter()));
            break;

         case JMP:
            set_program_counter (program_counter +
                                 OFFSET(instruction_register));
            break;

         case JMPZ:
            if (read_flag_zero())
               set_program_counter (program_counter +
                                    OFFSET(instruction_register));
            break;

         case JMPNZ:
            if (!read_flag_zero())
               set_program_counter (program_counter +
                                    OFFSET(instruction_register));
            break;

         case JMPN:
            if (read_flag_negative())
               set_program_counter (program_counter +
                                    OFFSET(instruction_register));
            break;

         case JMPNN:
            if (!read_flag_negative())
               set_program_counter (program_counter +
                                    OFFSET(instruction_register));
            break;

         case JMPC:
            if (read_flag_carry())
               set_program_counter (program_counter +
                                    OFFSET(instruction_register));
            break;

         case JMPV:
            if (read_flag_overflow())
               set_program_counter (program_counter +
                                    OFFSET(instruction_register));
            break;

         case CALL:
            t = memory_read_word (read_program_counter());
            set_program_counter (program_counter + 2);
            memory_write_word (read_stack_pointer(),
                               read_program_counter());
            set_stack_pointer (stack_pointer + 2);
            set_program_counter (t);
            break;

         case RET:
            set_stack_pointer (stack_pointer - 2);
            set_program_counter (memory_read_word (read_stack_pointer()));
            break;

         case HLT:
            halt();
            return program_counter;

         default:
            error ("invalid instruction opcode");
      }

      if (single_step || breakpoints[program_counter])
      {
         single_step = 0;
         return program_counter;
      }
   }
}

/*****************************************************************/

void show_memory (word starting_address, int size)
{
   int i;
   char s[MAXMSGLEN];

   if (size <= 0)
      size = 1;
   for (i = starting_address;
        (i < starting_address + size) && (i < (int) MEMSIZE); i++)
   {
      sprintf (s, "MEM[%04X]: %02X | %.224s",
               i, (unsigned int) memory[i], comments[i]);
      message (s);
   }
   return;
}

/*****************************************************************/

void show_cpu (void)
{
   int i;
   char s[MAXMSGLEN];

   sprintf (s, "CPU status: PC = %04X   IR = %04X   SP = %04X",
            (unsigned int) program_counter,
            (unsigned int) instruction_register,
            (unsigned int) stack_pointer);
   message (s);

   sprintf (s, "CPU status: zero = %1X   negative = %1X"
            "   carry = %1X   overflow = %1X",
            (unsigned int) zero,
            (unsigned int) negative,
            (unsigned int) carry,
            (unsigned int) overflow);
   message (s);

   for (i = 0; i < (int) NREGISTERS; i += 4)
   {
      sprintf (s, "CPU status: "
                  "R%02d = %04X   R%02d = %04X   R%02d = %04X   R%02d = %04X",
                   i, (unsigned int) r[i],
                   i + 1, (unsigned int) r[i + 1],
                   i + 2, (unsigned int) r[i + 2],
                   i + 3, (unsigned int) r[i + 3]);
      message (s);
   }

   return;
}

/*****************************************************************/

void show_input_buffer (void)
{
   int i;
   char s[MAXMSGLEN];

   if (input_buffer[input_index] == '\0')
   {
      sprintf (s, "(input buffer is empty)");
      message (s);

      return;
   }

   for (i = input_index; input_buffer[i] != '\0'; i++)
   {
      if (input_buffer[i] >= 32 && input_buffer[i] < 127)
      {
         sprintf (s, "%02X (ASCII '%c')", input_buffer[i], input_buffer[i]);
         message (s);
      }
      else if (input_buffer[i] == '\n')
      {
         sprintf (s, "%02X (ASCII '\\n')", input_buffer[i]);
         message (s);
      }
      else
      {
         sprintf (s, "%02X", input_buffer[i]);
         message (s);
      }
   }

   return;
}

/*****************************************************************/

void error (char *err_msg)
{
   char s[MAXMSGLEN];

   sprintf (s, "FATAL ERROR: %s", err_msg);
   message (s);
   sprintf (s, "Current program counter: %04X",
            (unsigned int) program_counter);
   message (s);
   message ("");

   show_cpu();
   message ("");

   if (program_counter < 4)
      show_memory (0, 12);
   else
      show_memory (program_counter - 4, 12);

   exit (EXIT_FAILURE);
}

/*****************************************************************/

const char *sim_version (void)
{
   return SIM_VERSION;
}

/*****************************************************************/

const char *instruction_name (byte opcode)
{
   int i;

   i = 0;
   while (i < NINSTRUCTIONS)
   {
      if (machine_language[i].opcode == opcode)
         return machine_language[i].name;
      i++;
   }

   return "unknown";
}

/*****************************************************************/

int instruction_length (byte opcode)
{
   int i;

   i = 0;
   while (i < NINSTRUCTIONS)
   {
      if (machine_language[i].opcode == opcode)
      {
         switch (machine_language[i].format)
         {
            case OPCODE_ONLY:
            case OPCODE_RR:
            case OPCODE_R:
            case OPCODE_A8:
               return 2;

            case OPCODE_R_D8:
               return 3;

            case OPCODE_R_D16:
            case OPCODE_D16:
            case OPCODE_R_A16:
            case OPCODE_A16:
               return 4;

            default:
               return 4;
         }
      }
      i++;
   }

   return 4;
}

