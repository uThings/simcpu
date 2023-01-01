
/*****************************************************************

 Copyright 2002   PIER LUCA MONTESSORO

 University of Udine
 ITALY

 montessoro@uniud.it
 www.montessoro.it

 This file is part of a freeware open source software package.
 It can be freely used (as it is or modified) as long as this
 copyright note is not removed.

 SIMCPU: assembler module (assembler.c)

******************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "simcpu.h"

#define MAXMSGLEN 256

word entry_point_address;
word current_address;
int labels_counter;
int current_line;

struct TAG_memory
{
   byte cell;
   char source_line[MAXLINELEN];
   unsigned char instruction_format;
   unsigned char to_be_finalized;
   char target_label[MAXSYMLEN];
} memory_map[MEMSIZE];

struct TAG_source_labels
{
   char name[MAXSYMLEN];
   word address;
} source_labels[MEMSIZE];


void init (void);
void compile (FILE *fsource);
int get_token (char buffer[], char token[], int start);
int find_instruction (char s[]);
int find_register (char s[]);
void memory_write_word (word address, word data);
void memory_write_byte (word address, byte data);
void add_label (char *label, word address);
void finalize_labels (void);
word find_address (char *label);
int  is_label (char s[]);
int  is_hex (char s[]);
int  htoi (char s[]);
int  hex_digit_value (char d);
void write_exe (FILE *fp);
void error (char *msg);

/*****************************************************************/

int main (int argc, char *argv[])
{
   char sourcefname[MAX_FNAME_LEN],
        exefname[MAX_FNAME_LEN];
   FILE *fsource, *fexe;

   if (argc != 2)
   {
      printf ("SimCPU assembly language source file (without .axx extension): ");
      scanf ("%s", sourcefname);
      while (getchar() != '\n');
      strcpy (exefname, sourcefname);
   }
   else
   {
      strcpy (sourcefname, argv[1]);
      strcpy (exefname, argv[1]);
   }

   strcat (sourcefname, ASSEMBLY_SOURCE_EXT);
   strcat (exefname, EXECUTABLE_EXT);

   if ((fsource = fopen (sourcefname, "r")) == NULL)
   {
      printf ("Cannot open file %s\n", sourcefname);
      exit (EXIT_FAILURE);
   }

   if ((fexe = fopen (exefname, "w")) == NULL)
   {
      printf ("Cannot open file %s\n", exefname);
      exit (EXIT_FAILURE);
   }

   printf ("SIMCPU assembler module (rel. " ASM_VERSION ")\n");

   init ();
   compile (fsource);
   entry_point_address = find_address (ENTRY_POINT);
   write_exe (fexe);

   fclose (fsource);
   fclose (fexe);

   return EXIT_SUCCESS;
}

/*****************************************************************/

void init (void)
{
   unsigned int i;

   for (i = 0; i < MEMSIZE; i++)
   {
      memory_map[i].cell = 0;
      memory_map[i].source_line[0] = '\0';
      memory_map[i].instruction_format = 0;
      memory_map[i].to_be_finalized = 0;
      memory_map[i].target_label[0] = '\0';
      source_labels[i].name[0] = '\0';
   }

   current_address = 0;
   current_line = 0;
   labels_counter = 0;

   return;
}

/*****************************************************************/

void compile (FILE *fsource)
{
   char error_msg [MAXMSGLEN];
   char line [MAXLINELEN];
   char token [MAXSYMLEN];
   int i, iparser, instruction_index;
   int first_register, second_register, data;

   while (fgets (line, MAXLINELEN, fsource) != NULL)
   {
      current_line++;
      iparser = 0;

      /* remove the newline */
      i = strlen (line) - 1;
      if (line [i] == '\n')
         line [i] = '\0';

      /* get the first token: a comment, a label or an opcode */

      if ((iparser = get_token (line, token, iparser)) == -1)
         continue;   /* a blank line or a comment */


      /* save the source line */
      strcpy (memory_map[current_address].source_line, line);


      /* label? */
      if (is_label (token))
      {
         i = strlen (token) - 1;
         token [i] = '\0';
         add_label (token, current_address);

         if ((iparser = get_token (line, token, iparser)) == -1)
         {
            sprintf (error_msg, "empty line after the label");
            error (error_msg);
         }
      }

      /* identify instruction */

      if ((instruction_index = find_instruction (token)) == -1)
      {
         sprintf (error_msg, "unknown instruction or directive %s", token);
         error (error_msg);
      }

      if (instruction_index >= NINSTRUCTIONS)
      {
         /* directive */
         switch (directives[instruction_index - NINSTRUCTIONS].directivecode)
         {
            case DIR_BYTE:
               if ((iparser = get_token (line, token, iparser)) == -1)
               {
                  sprintf (error_msg, "byte initializer not found");
                  error (error_msg);
               }
               if (is_hex (token))
               {
                  data = htoi (token);
               }
               else
               {
                  sprintf (error_msg, "a byte cannot be initialized "
                                      "with a word-wide address");
                  error (error_msg);
               }

               memory_map[current_address].instruction_format = DATA_ONLY_8;

               memory_write_byte (current_address, (byte) data);
               current_address += 1;
               break;

            case DIR_WORD:
               if ((iparser = get_token (line, token, iparser)) == -1)
               {
                  sprintf (error_msg, "word initializer not found");
                  error (error_msg);
               }
               if (is_hex (token))
               {
                  data = htoi (token);
               }
               else
               {
                  strcpy (memory_map[current_address].target_label, token);
                  memory_map[current_address].to_be_finalized = 1;
                  data = 0;  /* dummy value */
               }

               memory_map[current_address].instruction_format = DATA_ONLY_16;

               memory_write_word (current_address, (word) data);
               current_address += 2;
               break;

            default:       /* not a directive */
               sprintf (error_msg,
                        "wrong code returned by find_instruction");
               error (error_msg);
         }   
      }
      else
      {
         memory_map[current_address].instruction_format =
                                machine_language[instruction_index].format;

         switch (machine_language[instruction_index].format)
         {
            case OPCODE_ONLY:
               memory_write_byte (current_address, (byte) 0);
               current_address += 1;
               memory_write_byte
                  (current_address,
                   machine_language[instruction_index].opcode);
               current_address += 1;
               break;


            case OPCODE_R_D16:
               if ((iparser = get_token (line, token, iparser)) == -1)
               {
                  sprintf (error_msg, "register expected");
                  error (error_msg);
               }
               if ((first_register = find_register (token)) == -1)
               {
                  sprintf (error_msg, "invalid register %s", token);
                  error (error_msg);
               }

               if ((iparser = get_token (line, token, iparser)) == -1)
               {
                  sprintf (error_msg, "data item expected");
                  error (error_msg);
               }
               if (is_hex (token))
               {
                  data = htoi (token);
               }
               else
               {
                  strcpy (memory_map[current_address].target_label, token);
                  memory_map[current_address].to_be_finalized = 1;
                  data = 0;  /* dummy value */
               }

               memory_write_byte
                  (current_address, (byte) (first_register << 4));
               current_address += 1;
               memory_write_byte
                  (current_address,
                   machine_language[instruction_index].opcode);
               current_address += 1;
               memory_write_word (current_address, (word) data);
               current_address += 2;
               break;


            case OPCODE_R_D8:
               if ((iparser = get_token (line, token, iparser)) == -1)
               {
                  sprintf (error_msg, "register expected");
                  error (error_msg);
               }
               if ((first_register = find_register (token)) == -1)
               {
                  sprintf (error_msg, "invalid register %s", token);
                  error (error_msg);
               }

               if ((iparser = get_token (line, token, iparser)) == -1)
               {
                  sprintf (error_msg, "data item expected");
                  error (error_msg);
               }
               if (is_hex (token))
               {
                  data = htoi (token);
               }
               else
               {
                  strcpy (memory_map[current_address].target_label, token);
                  memory_map[current_address].to_be_finalized = 1;
                  data = 0;  /* dummy value */
               }

               memory_write_byte
                  (current_address, (byte) (first_register << 4));
               current_address += 1;
               memory_write_byte
                  (current_address,
                   machine_language[instruction_index].opcode);
               current_address += 1;
               memory_write_byte (current_address, (byte) data);
               current_address += 1;
               break;


            case OPCODE_RR:
               if ((iparser = get_token (line, token, iparser)) == -1)
               {
                  sprintf (error_msg, "first register expected");
                  error (error_msg);
               }
               if ((first_register = find_register (token)) == -1)
               {
                  sprintf (error_msg, "invalid first register %s", token);
                  error (error_msg);
               }

               if ((iparser = get_token (line, token, iparser)) == -1)
               {
                  sprintf (error_msg, "second register expected");
                  error (error_msg);
               }
               if ((second_register = find_register (token)) == -1)
               {
                  sprintf (error_msg, "invalid second register %s", token);
                  error (error_msg);
               }

               memory_write_byte
                  (current_address,
                   (byte) (first_register << 4) | second_register);
               current_address += 1;
               memory_write_byte
                  (current_address,
                   machine_language[instruction_index].opcode);
               current_address += 1;
               break;


            case OPCODE_R:
               if ((iparser = get_token (line, token, iparser)) == -1)
               {
                  sprintf (error_msg, "register expected");
                  error (error_msg);
               }
               if ((first_register = find_register (token)) == -1)
               {
                  sprintf (error_msg, "invalid register %s", token);
                  error (error_msg);
               }

               memory_write_byte
                  (current_address, (byte) (first_register << 4));
               current_address += 1;
               memory_write_byte
                  (current_address,
                   machine_language[instruction_index].opcode);
               current_address += 1;
               break;


            case OPCODE_D16:
               if ((iparser = get_token (line, token, iparser)) == -1)
               {
                  sprintf (error_msg, "data item expected");
                  error (error_msg);
               }
               if (is_hex (token))
               {
                  data = htoi (token);
               }
               else
               {
                  strcpy (memory_map[current_address].target_label, token);
                  memory_map[current_address].to_be_finalized = 1;
                  data = 0;  /* dummy value */
               }

               memory_write_byte (current_address, (byte) 0);
               current_address += 1;
               memory_write_byte
                  (current_address,
                   machine_language[instruction_index].opcode);
               current_address += 1;
               memory_write_word (current_address, (word) data);
               current_address += 2;
               break;


            case OPCODE_R_A16:
               if ((iparser = get_token (line, token, iparser)) == -1)
               {
                  sprintf (error_msg, "register expected");
                  error (error_msg);
               }
               if ((first_register = find_register (token)) == -1)
               {
                  sprintf (error_msg, "invalid register %s", token);
                  error (error_msg);
               }

               if ((iparser = get_token (line, token, iparser)) == -1)
               {
                  sprintf (error_msg, "label expected");
                  error (error_msg);
               }
               strcpy (memory_map[current_address].target_label, token);
               memory_map[current_address].to_be_finalized = 1;

               memory_write_byte
                  (current_address, (byte) (first_register << 4));
               current_address += 1;
               memory_write_byte
                  (current_address,
                   machine_language[instruction_index].opcode);
               current_address += 1;
               current_address += 2;   /* address to be finalized */
               break;


            case OPCODE_A8:
               if ((iparser = get_token (line, token, iparser)) == -1)
               {
                  sprintf (error_msg, "address expected");
                  error (error_msg);
               }
               strcpy (memory_map[current_address].target_label, token);
               memory_map[current_address].to_be_finalized = 1;

               current_address += 1;   /* address to be finalized */
               memory_write_byte
                  (current_address,
                   machine_language[instruction_index].opcode);
               current_address += 1;
               break;


            case OPCODE_A16:
               if ((iparser = get_token (line, token, iparser)) == -1)
               {
                  sprintf (error_msg, "address expected");
                  error (error_msg);
               }
               strcpy (memory_map[current_address].target_label, token);
               memory_map[current_address].to_be_finalized = 1;

               memory_write_byte
                  (current_address, (byte) 0);
               current_address += 1;
               memory_write_byte
                  (current_address,
                   machine_language[instruction_index].opcode);
               current_address += 1;
               current_address += 2;   /* address to be finalized */
               break;


            default:
               sprintf (error_msg,
                        "wrong format code in structure machine_language");
               error (error_msg);

         }
      }
   }

   finalize_labels ();

   return;
}

/*****************************************************************/

/* get the token after the current index position and returs the 
   new index position at the end of the token */

int get_token (char buffer[], char token[], int start)
{
   int i, j;

#ifdef DEBUG
   printf ("DEBUG: get_token started, buffer=\"%s\", start=%d\n",
           buffer, start);
#endif

   i = start;
   j = 0;
   while (buffer [i] != '\0' && (buffer [i] == ' ' || buffer [i] == '\t' || 
                                 buffer [i] == '\n' || buffer [i] == 0x0D || buffer [i] == 0x0A))
      i++;

   if (buffer[i] == '\0' || buffer[i] == ';')
      return -1;

   while (buffer [i] != '\0' && !(buffer [i] == ' ' || buffer [i] == '\t' || 
                                  buffer [i] == '\n' || buffer [i] == 0x0D || buffer [i] == 0x0A))
   {
      token [j] = toupper (buffer [i]);
      j++;
      i++;
   }

   token [j] = '\0';

#ifdef DEBUG
   printf ("DEBUG: get_token found \"%s\"\n", token);
#endif

   return i;
}


int find_instruction (char s[])
{
   int i;

   /* Note: find_instruction returns instruction index, if instruction, or
            NINSTRUCTIONS + directive index, if directive */

   /* check for instructions */
   for (i = 0; i < NINSTRUCTIONS; i++)
   {
      if (!strcmp (machine_language[i].name, s))
      {
          return i;
      }
   }

   /* check for directives */
   for (i = 0; i < NDIRECTIVES; i++)
   {
      if (!strcmp (directives[i].name, s))
      {
          return NINSTRUCTIONS + i;
      }
   }

   return -1;
}


int find_register (char s[])
{
   int n;

   if (s[0] != 'R')
      return -1;

   if (s[1] < '0' || s[1] > '9')
      return -1;

   if (s[2] != '\0' && (s[2] < '0' || s[2] > '9'))
      return -1;

   n = atoi (s+1);

   if (n < 0 || n >= NREGISTERS)
      return -1;

   return atoi (s+1);
}

/*****************************************************************/

void memory_write_word (word address, word data)
{
   memory_map[address].cell = data & 0xFF;
   memory_map[address+1].cell = data >> 8;
   return;
}


void memory_write_byte (word address, byte data)
{
   memory_map[address].cell = data;
   return;
}

/*****************************************************************/

void add_label (char *label, word address)
{
   char msg[MAXMSGLEN];
   int i = 0;

   while (i < labels_counter)
   {
      if (!strcmp (label, source_labels[i].name))
      {
         sprintf (msg, "duplicate label '%s'", label);
         error (msg);
      }
      i++;
   }

   strcpy (source_labels[labels_counter].name, label);
   source_labels[labels_counter].address = address;
   labels_counter++;

   return;
}


void finalize_labels (void)
{
   unsigned int i;

   for (i = 0; i < MEMSIZE; i++)
   {
      if (memory_map[i].to_be_finalized)
      {

#ifdef DEBUG
         printf ("DEBUG: finalization of address %d, "
                 "label \"%s\", source \"%s\"\n",
                 i, memory_map[i].target_label, memory_map[i].source_line);
#endif

         switch (memory_map[i].instruction_format)
         {
            case OPCODE_R_D16:
            case OPCODE_R_A16:
            case OPCODE_A16:
               memory_write_word ((word) (i + 2),
                                  find_address (memory_map[i].target_label));
               break;

            case OPCODE_A8:  /* this is an offset for jump instructions! */
               {
                  int offset;

                  offset = (int) find_address (memory_map[i].target_label) -
                                                                      (i + 2);
                  if (offset > 127 || offset < -128)
                  {
                     char msg[MAXMSGLEN];

                     sprintf
                        (msg, "relative jump too far at address %04X", i);
                     error (msg);
                  }
               }
               memory_write_byte
                  (i, (byte) (((int) find_address
                             (memory_map[i].target_label) - (i + 2)) & 0xFF));
               break;

            case DATA_ONLY_16:
               memory_write_word ((word) i,
                                  find_address (memory_map[i].target_label));
               break;

            case DATA_ONLY_8:
               memory_write_byte
                  (i, (byte) ((int) find_address
                             (memory_map[i].target_label) & 0xFF));
               break;
         }
      }
   }

   return;
}


word find_address (char *label)
{
   char msg[MAXMSGLEN];
   int i = 0;

   while (i < labels_counter)
   {
      if (!strcmp (label, source_labels[i].name))
         return source_labels[i].address;
      i++;
   }

   sprintf (msg, "label '%s' not found", label);
   error (msg);

   return 0;  /* never reached */
}

/*****************************************************************/

int is_label (char s[])
{
   int i, len;

   i = 0;
   len = strlen (s);

   if (len == 0)
      return 0;

   /* first character */
   if (!(s[i] >= 'a' && s[i] <= 'z' ||
         s[i] >= 'A' && s[i] <= 'Z' ||
         s[i] == '_'))
      return 0;

   /* others */
   i++;
   while (i < len - 1)
   {
      if (s[i] >= '0' && s[i] <= '9' ||
          s[i] >= 'a' && s[i] <= 'z' ||
          s[i] >= 'A' && s[i] <= 'Z' ||
          s[i] == '_')
         i++;
      else
         return 0;
   }

   if (s[i] == ':')
      return 1;
   else
      return 0;
}


int is_hex (char s[])
{
   int i;

   i = 0;

   /* first digit */
   if (s[i] == '\0' || s[i] < '0' || s[i] > '9')
      return 0;
      
   /* others */
   i++;
   while (s[i] != '\0')
   {
      if (s[i] >= '0' && s[i] <= '9' ||
          s[i] >= 'a' && s[i] <= 'f' ||
          s[i] >= 'A' && s[i] <= 'F')
         i++;
      else
         return 0;
   }

   return 1;
}


int htoi (char s[])
{
   int n, i;

   i = 0;
   n = 0;

   while (s[i] != '\0')
   {
      n = n * 16 + hex_digit_value (s[i]);
      i++;
   }

   return n;
}


int hex_digit_value (char d)
{
   char msg [MAXMSGLEN];

   if (d >= '0' && d <= '9')
      return d - '0';
   else if (d >= 'a' && d <= 'f')
      return 10 + d - 'a';
   else if (d >= 'A' && d <= 'F')
      return 10 + d - 'A';
   else
   {
      sprintf (msg, "invalid hexadecimal digit %c", d);
      error (msg);
   }

   return 0;  /* never reached */
}
      
/*****************************************************************/

void write_exe (FILE *fp)
{
   int i = 0;

   fprintf (fp, "%04X\n", entry_point_address); 

   while (i < current_address)
   {
      fprintf (fp, "%02X | %s\n",
               memory_map[i].cell, memory_map[i].source_line);
      i++;
   }

   return;
}

/*****************************************************************/

void error (char *msg)
{
   printf ("ERROR AT LINE %1d: %s\n", current_line, msg);
   exit (EXIT_FAILURE);
}
