
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXSTRLEN 1024
#define MAXINCLUDEFILES 1024


void expand_include_directives (char *include_file_name, FILE *fout);
int get_include_file_name (char *line, char *fname);
int lookfor_include_file (char *fname);

int main(int argc, char *argv[])
{
   FILE *fout;

   if (argc != 3)
   {
      printf ("usage: include <input file> <output file>\n");
      exit (EXIT_FAILURE);
   }

   if ((fout = fopen (argv[2], "w")) == NULL)
   {
      printf ("error opening output file\n");
      exit (EXIT_FAILURE);
   }

   expand_include_directives (argv[1], fout);

   fclose (fout);

   return EXIT_SUCCESS;
}

void expand_include_directives (char *include_file_name, FILE *fout)
{
   FILE *fin;
   char line [MAXSTRLEN];

   if ((fin = fopen (include_file_name, "r")) == NULL)
   {
      printf ("error opening input file %s\n", include_file_name);
      exit (EXIT_FAILURE);
   }

   while (fgets (line, MAXSTRLEN, fin) != NULL)
   {
      /* include directive? */
      if (get_include_file_name (line, include_file_name))
      {
         if (!lookfor_include_file (include_file_name))
         {
            expand_include_directives (include_file_name, fout);
         }
         else
         {
            fprintf (fout, "\n; multiple include of file %s suppressed\n",
                     include_file_name);
         }
      }
      else
      {
         fputs (line, fout);
      }
   }

   fclose (fin);

   return;
}


int get_include_file_name (char *line, char *fname)
{
   int i, j;

   if (!strncmp (line, "#include", 8))
   {
      i = 9;

      while (line[i] != '\0' && line[i] != '"')
         i++;

      if (line[i] != '"')
      {
         printf ("incorrect syntax in #include directive\n");
         exit (EXIT_FAILURE);
      }

      i++;
      j = 0;
      while (line[i] != '\0' && line[i] != '"')
         fname[j++] = line[i++];

      if (line[i] != '"')
      {
         printf ("incorrect syntax in #include directive\n");
         exit (EXIT_FAILURE);
      }

      fname[j] = '\0';

      return 1;
   }
   else
   {
      return 0;
   }
}


int lookfor_include_file (char *fname)
{
   static char include_file_list [MAXINCLUDEFILES][MAXSTRLEN];
   static int n_include_files = 0;

   int i;

   for (i = 0; i < n_include_files; i++)
   {
      if (!strcmp (fname, include_file_list [i]))
         return 1;
   }

   if (n_include_files < MAXINCLUDEFILES)
   {
      strcpy (include_file_list[n_include_files], fname);
      n_include_files++;
   }
   else
   {
      printf ("no more room for include file list\n");
      exit (EXIT_FAILURE);
   }

   return 0;
}
