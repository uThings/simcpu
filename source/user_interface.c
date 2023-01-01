
/*****************************************************************

 Copyright 2002   PIER LUCA MONTESSORO

 University of Udine
 ITALY

 montessoro@uniud.it
 www.montessoro.it

 This file is part of a freeware open source software package.
 It can be freely used (as it is or modified) as long as this
 copyright note is not removed.

 SIMCPU: user interface module (user_interface.h)

******************************************************************/

#define USER_INTERFACE_HELP \
"\n" \
"Available commands (can be abbreviated down to the first letter only):\n" \
"\n" \
"   Set Trace_mode Instruction\n" \
"   Set Trace_mode Extended\n" \
"   Set Trace_mode Disabled\n" \
"   Set Breakpoint <address (hex)> Enabled\n" \
"   Set Breakpoint <address (hex)> Disabled\n" \
"   Run Next\n" \
"   Run Forever\n" \
"   Load Input-buffer $<string, up to the end of line>\n" \
"   Display Memory <address (hex)> <length (dec)>\n" \
"   Display Cpu\n" \
"   Display Input-buffer\n" \
"   EXit\n" \
"   Quit\n" \
"   Help\n" \
"   Control-C (to enter command line mode while simulation is running)\n" \
"\n" \
"   SHORTCUTS:\n" \
"   Next = Run Next\n" \
"   Cpu = Display Cpu\n" \
"\n"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>


#define MAXBUFLEN      256
#define MAX_FNAME_LEN  256
#define EXECUTABLE_EXT ".exx"

#define PARALLELISM  16UL
#define MEMSIZE      1UL << PARALLELISM

#define TOKEN_VOID     0

#define TOKEN_SET      1
#define TOKEN_RUN      2
#define TOKEN_LOAD     3
#define TOKEN_DISPLAY  4
#define TOKEN_EXIT     5
#define TOKEN_QUIT     6
#define TOKEN_HELP     7

#define TOKEN_TRACE_MODE  65
#define TOKEN_BREAKPOINT  66
#define TOKEN_NEXT        67
#define TOKEN_FOREVER     68
#define TOKEN_INPUTBUFFER 69
#define TOKEN_MEMORY      70
#define TOKEN_CPU         71

#define TOKEN_ENABLED     129
#define TOKEN_DISABLED    130

#define TOKEN_INSTRUCTION 131
#define TOKEN_EXTENDED    132

#define TOKEN_ASCII_DATA  253
#define TOKEN_HEX_DATA    254
#define TOKEN_DEC_DATA    255


#define TRACE_MODE_DISABLED     0
#define TRACE_MODE_INSTRUCTION  1
#define TRACE_MODE_EXTENDED     2

typedef unsigned short int  word;

struct TAG_user_interface_commands
{
   char *command;
   int code;
} user_interface_commands[] =
   { { "set", TOKEN_SET },
     { "run", TOKEN_RUN },
     { "load", TOKEN_LOAD },
     { "display", TOKEN_DISPLAY },
     { "exit", TOKEN_EXIT },
     { "quit", TOKEN_QUIT },
     { "help", TOKEN_HELP },
     { "trace_mode", TOKEN_TRACE_MODE },
     { "breakpoint", TOKEN_BREAKPOINT },
     { "next", TOKEN_NEXT },
     { "forever", TOKEN_FOREVER },
     { "input-buffer", TOKEN_INPUTBUFFER },
     { "memory", TOKEN_MEMORY },
     { "cpu", TOKEN_CPU },
     { "enabled", TOKEN_ENABLED },
     { "disabled", TOKEN_DISABLED },
     { "instruction", TOKEN_INSTRUCTION },
     { "extended", TOKEN_EXTENDED } };

#define N_COMMANDS 18

extern char input_buffer[];
extern int input_index;
extern unsigned char breakpoints[MEMSIZE];
extern unsigned char single_step;
extern unsigned char trace_mode;

void user_control (word current_program_counter);
void prompt (char *command, char *options);
int get_command (FILE *input_stream, char *prompt, char *options,
                 int interactive_mode);
int get_ascii_data (FILE *input_stream, char *prompt,
                    char *ascii_data, int interactive_mode);
int get_hex_data (FILE *input_stream, char *prompt,
                  int *hex_data, int interactive_mode);
int get_dec_data (FILE *input_stream, char *prompt,
                  int *dec_data, int interactive_mode);
int find_option_code (char *command, char *options);
int find_token_code (char *command);
int ishdigit (char c);
int hdigitvalue (char c);
void purge_input (FILE* input_stream);
void set_break_on_next_instruction (int d);
static void int_disabled (int arg);
static void int_handler (int arg);
void int_setup (void);
word initialize_executor (FILE *fp);
word executor (void);
const char *sim_version (void);
void message (char *msg);

/* utility functions prototypes (see executor.c) */

void show_memory (word starting_address, int size);
void show_cpu (void);
void show_input_buffer (void);


/*****************************************************************/

int main (int argc, char *argv[])
{
   FILE *fp;
   word starting_address;
   char filename[MAX_FNAME_LEN];

   if (argc != 2)
   {
      printf ("SimCPU executable file (without .exx extension): ");
      scanf ("%s", filename);
      while (getchar() != '\n');
   }
   else
   {
      strcpy (filename, argv[1]);
   }

   strcat (filename, EXECUTABLE_EXT);

   if ((fp = fopen (filename, "r")) == NULL)
   {
      printf ("Cannot open input file %s\n", filename);
      exit (EXIT_FAILURE);
   }

   printf ("SIMCPU %s starting...\n", sim_version());

   /* setup the contol-C handling */
   int_setup ();

   starting_address = initialize_executor (fp);
   user_control (starting_address);

   return EXIT_SUCCESS;
}

/*****************************************************************/

void user_control (word current_program_counter)
{
   int addr, len;
   int interactive_mode = 1; /* non-interactive mode not yet implemented */
   FILE *input_stream = stdin; /* default */

   while (1)
   {
      addr = len = 0;

      printf ("\nENTERING COMMAND MODE: current program counter = %04X\n",
              (unsigned int) current_program_counter);

      switch (get_command (input_stream, "> ",
              "set, run, load, display, exit, quit, help, next, cpu",
              interactive_mode))
      {
         case TOKEN_EXIT:
         case TOKEN_QUIT:
            return;

         case TOKEN_SET:
            switch (get_command (input_stream, "set",
                                 "trace_mode, breakpoint",
                                 interactive_mode))
            {
               case TOKEN_TRACE_MODE: /* set trace_mode */
                  switch (get_command (input_stream,
                                       "set trace_mode",
                                       "instruction, extended, disabled",
                                       interactive_mode))
                  {
                     case TOKEN_INSTRUCTION: /* set trace_mode instruction */
                        trace_mode = TRACE_MODE_INSTRUCTION;
                        break;

                     case TOKEN_EXTENDED: /* set trace_mode extended */
                        trace_mode = TRACE_MODE_EXTENDED;
                        break;

                     case TOKEN_DISABLED: /* set trace_mode disabled */
                        trace_mode = TRACE_MODE_DISABLED;
                        break;

                     default: 
                        printf ("Wrong command\n");
                        purge_input (input_stream);
                  }
                  break;

               case TOKEN_BREAKPOINT: /* set breakpoint */
                  if (get_hex_data (input_stream,
                                    "breakpoint address (hex): ",
                                    &addr, interactive_mode)
                         != TOKEN_HEX_DATA)
                  {
                     printf ("Invalid address\n");
                     purge_input (input_stream);
                     break;
                  }
                  if (addr < 0 || addr >= (int) MEMSIZE)
                  {
                     printf ("Invalid address\n");
                     purge_input (input_stream);
                     break;
                  }
                  switch (get_command (input_stream,
                                       "set breakpoint",
                                       "enabled, disabled",
                                       interactive_mode))
                  {
                     case TOKEN_ENABLED: /* set breakpoint enabled */
                        breakpoints[addr] = 1;
                        break;

                     case TOKEN_DISABLED: /* set breakpoint enabled */
                        breakpoints[addr] = 0;
                        break;

                     default: 
                        printf ("Wrong command\n");
                        purge_input (input_stream);
                  }
                  break;

               default: 
                  printf ("Wrong command\n");
                  purge_input (input_stream);
            }
            break;

         case TOKEN_RUN:
            switch (get_command (input_stream, "run", "next, forever",
                                 interactive_mode))
            {
               case TOKEN_NEXT: /* run next */
                  single_step = 1;
                  current_program_counter = executor();
                  break;

               case TOKEN_FOREVER: /* run forever */
                  current_program_counter = executor();
                  break;

               default: 
                  printf ("Wrong command\n");
                  purge_input (input_stream);
            }
            break;

         case TOKEN_DISPLAY:
            switch (get_command (input_stream, "display",
                                 "memory, cpu, input-buffer",
                                 interactive_mode))
            {
               case TOKEN_CPU: /* display CPU */
                  show_cpu ();
                  break;

               case TOKEN_MEMORY: /* display memory */
                  if (get_hex_data (input_stream, "memory address (hex): ",
                                    &addr, interactive_mode)
                         != TOKEN_HEX_DATA)
                  {
                     printf ("Invalid address\n");
                     purge_input (input_stream);
                     break;
                  }
                  if (addr < 0 || addr >= (int) MEMSIZE)
                  {
                     printf ("Invalid address\n");
                     purge_input (input_stream);
                     break;
                  }
                  if (get_dec_data (input_stream, "length (decimal): ",
                                    &len, interactive_mode)
                         != TOKEN_DEC_DATA)
                  {
                     printf ("Invalid length\n");
                     purge_input (input_stream);
                     break;
                  }
                  show_memory (addr, len);
                  break;

               case TOKEN_INPUTBUFFER: /* display input-buffer */
                  show_input_buffer ();
                  break;

               default: 
                  printf ("Wrong command\n");
                  purge_input (input_stream);
            }
            break;

         case TOKEN_LOAD:
            switch (get_command (input_stream, "load", "input-buffer",
                                 interactive_mode))
            {
               case TOKEN_INPUTBUFFER: /* load input-buffer */
                  if (get_ascii_data (input_stream,
                                      "ASCII input (must begin with '$'): ",
                                  input_buffer, interactive_mode)
                        != TOKEN_ASCII_DATA)
                  {
                     printf ("Invalid input\n");
                     purge_input (input_stream);
                  }
                  input_index = 0;
                  break;

               default: 
                  printf ("Wrong command\n");
                  purge_input (input_stream);
            }
            break;

         case TOKEN_HELP:
            printf (USER_INTERFACE_HELP);
            break;

         case TOKEN_NEXT: /* run next */
            single_step = 1;
            current_program_counter = executor();
            break;

         case TOKEN_CPU: /* display CPU */
            show_cpu ();
            break;

         default:
            printf ("Wrong command\n");
            purge_input (input_stream);

      }  /* end switch */
   }  /* end while */
}

/*****************************************************************/

/*
   In the following, the status variable means:
      0 = looking for the token
      1 = inside the token
      2 = token terminated
*/

int cr_flag = 1;   /* remember if last character was a CR */


void write_prompt (char *command, char *options)
{
   if (!strcmp (command, "> "))
   {
      /* top-level promtp, options are hidden */
      printf ("> ");
   }
   else
   {
      printf ("%s (%s): ", command, options);
   }

   return;
}


int get_command (FILE *input_stream, char *prompt, char *options,
                 int interactive_mode)
{
   char ch;
   char buffer [MAXBUFLEN];
   int i, status;

   i = 0;
   status = 0;

   if (cr_flag && interactive_mode)
   {
      write_prompt (prompt, options);
   }

   while (status != 2 && (ch = fgetc (input_stream)) != EOF)
   {
      cr_flag = 0;
      if (status == 0)
      {
         if (ch == '\n' && interactive_mode)
         {
            cr_flag = 1;
            write_prompt (prompt, options);
         }
         else if (ch != ' ' && ch != '\t')
         {
            status = 1;
            buffer [i++] = ch;
         }
      }
      else /* status == 1 */
      {
         if (ch != '\n' &&  ch != ' ' && ch != '\t')
         {
            buffer [i++] = ch;
         }
         else
         {
            status = 2;
            if (ch == '\n')
               cr_flag = 1;
         }
      }
   }

   if (i == 0)
      return TOKEN_EXIT;
   else
   {
      buffer [i] = '\0';
      return find_option_code (buffer, options);
   }
}


/*****************************************************************/

/* To INPUTBUFFERSIZE must be given the same value as in executor.h */
#define INPUTBUFFERSIZE 256

int get_ascii_data (FILE *input_stream, char *prompt,
                    char *ascii_data, int interactive_mode)
{
   char ch;
   int i, status;

   i = 0;
   status = 0;

   if (cr_flag && interactive_mode)
   {
      printf ("%s", prompt);
   }

   while (i < INPUTBUFFERSIZE-1 && status != 2
          && (ch = fgetc (input_stream)) != EOF)
   {
      cr_flag = 0;
      if (status == 0)
      {
         if (ch == '\n' && interactive_mode)
         {
            cr_flag = 1;
            printf ("%s", prompt);
         }
         else if (ch != ' ' && ch != '\t')
         {
            if (ch == '$')
               status = 1;
            else
               return TOKEN_VOID;
         }
      }
      else /* status == 1 */
      {
         if (ch != '\n')
         {
            ascii_data [i++] = ch;
         }
         else
         {
            ascii_data [i++] = ch;
            status = 2;
            cr_flag = 1;
         }
      }
   }

   if (status != 2)
      return TOKEN_EXIT;

   ascii_data [i] = '\0';
   return TOKEN_ASCII_DATA;
}

/*****************************************************************/

int get_hex_data (FILE *input_stream, char *prompt,
                  int *hex_data, int interactive_mode)
{
   char ch;
   int i, status;
   int data;

   i = 0;
   status = 0;

   if (cr_flag && interactive_mode)
   {
      printf ("%s", prompt);
   }

   while (status != 2 && (ch = fgetc (input_stream)) != EOF)
   {   
      cr_flag = 0;
      if (status == 0)
      {
         if (ch == '\n' && interactive_mode)
         {
            cr_flag = 1;
            printf ("%s", prompt);
         }
         else if (ch != ' ' && ch != '\t')
            if (ishdigit(ch))
            {
               data = hdigitvalue (ch);
               status = 1;
            }
            else
            {
               return TOKEN_VOID;
            }
      }
      else /* status == 1 */
      {
         if (ch != '\n' && ch != ' ' && ch != '\t')
         {
            if (ishdigit(ch))
            {
               data = data * 16 + hdigitvalue (ch);
            }
            else
            {
               return TOKEN_VOID;
            }
         }
         else
         {
            status = 2;
            if (ch == '\n')
               cr_flag = 1;
         }
      }
   }

   if (status != 2)
      return TOKEN_EXIT;

   *hex_data = data;
   return TOKEN_HEX_DATA;
}

/*****************************************************************/
                 
int get_dec_data (FILE *input_stream, char *prompt,
                  int *dec_data, int interactive_mode)
{
   char ch;
   int i, status;
   int data;

   i = 0;
   status = 0;

   if (cr_flag && interactive_mode)
   {
      printf ("%s", prompt);
   }

   while (status != 2 && (ch = fgetc (input_stream)) != EOF)
   {   
      cr_flag = 0;
      if (status == 0)
      {
         if (ch == '\n' && interactive_mode)
         {
            cr_flag = 1;
            printf ("%s", prompt);
         }
         else if (ch != ' ' && ch != '\t')
            if (isdigit(ch))
            {
               data = toupper (ch) - '0';
               status = 1;
            }
            else
            {
               return TOKEN_VOID;
            }
      }
      else /* status == 1 */
      {
         if (ch != '\n' && ch != ' ' && ch != '\t')
         {
            if (isdigit(ch))
            {
               data = data * 10 + toupper (ch) - '0';
            }
            else
            {
               return TOKEN_VOID;
            }
         }
         else
         {
            status = 2;
            if (ch == '\n')
               cr_flag = 1;
         }
      }
   }

   if (status != 2)
      return TOKEN_EXIT;

   *dec_data = data;
   return TOKEN_DEC_DATA;
}

/*****************************************************************/

int ishdigit (char c)
{
   return isdigit (c) || c >= 'a' && c <= 'f' || c >= 'A' && c <= 'F';
}

int hdigitvalue (char c)
{
   if (isdigit (c))
      return c - '0';
   else
      return toupper (c) - 'A' + 10;
}

/*****************************************************************/

int find_option_code (char *command, char *options)
{
   /* options format:  "xxxxxxx, yyyyyyy, ..." */

   int i, j;
   char temp_option[MAXBUFLEN];
   char temp_command[MAXBUFLEN];

   for (i = 0; command[i] != '\0'; i++)
      temp_command[i] = tolower (command[i]);
   temp_command[i] = '\0';

   i = j = 0;

   do
   {
      if (options[i] == ',' || options[i] == '\0')
      {
         temp_option[j] = '\0';
         if (!strncmp (temp_command, temp_option, strlen (temp_command)))
            return (find_token_code (temp_option));
         j = 0;
      }
      else if (options[i] != ' ')
      {
         temp_option[j++] = tolower (options[i]);
      }
   } while (options[i++] != '\0');

   return TOKEN_VOID;
}


int find_token_code (char *command)
{
   int i;
   for (i = 0; i < N_COMMANDS; i++)
   {
      if (!strncmp (command, user_interface_commands[i].command,
                    strlen (command)))
         return user_interface_commands[i].code;
   }

   return TOKEN_VOID;
}

/*****************************************************************/

void purge_input (FILE* input_stream)
{
   char ch;

   if (!cr_flag)
      while ((ch = fgetc (input_stream)) != EOF && ch != '\n');
   cr_flag = 1;

   return;
}

/*****************************************************************/

void message (char *msg)
{
   printf ("%s\n", msg);
   return;
}

/*****************************************************************/

/* control-C handling */

void set_break_on_next_instruction (int d)
{
   /* argument d is not used, but required by the signal standard
      library function */
   single_step = 1;
   return;
}

/* dummy function, to be used while Control-C is already being processed */
static void int_disabled (int arg)
{
   /* keep Control-C disabled */
   signal (SIGINT, int_disabled);

   return;
}

/* local interrupt handler function */
static void int_handler (int arg)
{
   /* disable Control-C */
   signal (SIGINT, int_disabled);

   /* call the user's interrupt handler */
   set_break_on_next_instruction (1);

   /* enable Control-C */
   signal (SIGINT, int_handler);

   return;
}

/* function to be called to initialize the interrupt handling */

void int_setup (void)
{
   /* save the user's interrupt handler, and setup the signal to activate the
      local interrupt handler function */
   signal (SIGINT, int_handler);
   return;
}

