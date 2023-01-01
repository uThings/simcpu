
/*****************************************************************

 Copyright 2004   PIER LUCA MONTESSORO

 University of Udine
 ITALY

 montessoro@uniud.it
 www.montessoro.it

 This file is part of a freeware open source software package.
 It can be freely used (as it is or modified) as long as this
 copyright note is not removed.

******************************************************************/


 SIMCPU is a package to simulate a low level programming
 environment simplified for didactic purposes.

 It consists in:

 SYSTEM SPECIFICATIONS
 - CPU architecture definition (architecture.txt)
 - assembly language definition (languages.txt)
 - machine language definition (languages.txt)

 CPU SIMULATOR SPECIFICATIONS
 - file formats definitions (file_formats.txt)
 - assembly language to machine language translator module
   (source code in simcpu.h and assembler.c, in C language)
 - CPU simulation module (source code in simcpu.h, executor.c
   and user_interface.c, in C language)

 UTILITIES
 - include.c, a C program to handle library subroutines in separate
   source files and merge them in a single source assembly language program


 The C code has been developed to be compiled with the GNU C/C++
 compiler under Windows 95/98/ME/NT/2000/XP or unix/linux environments.


 Assembler module
 ----------------

 1) To compile (see make_assembler.bat):

    gcc -oassembler.exe assembler.c


 2) To translate assembly language file foo.axx into machine code 
    file foo.exx (assembly source file format is documented in 
    file_formats.txt):

    C:\>assembler foo

    (if no file name is given on the command line, the program asks for one)



 CPU simulation module
 ---------------------

 1) To compile (see make_simcpu.bat):
    gcc -g -osimcpu.exe executor.c user_interface.c

 2) To run in interactive mode (simcpu executable file format is 
    documented in file_formats.txt):

    C:\>simcpu foo.exx

    (if no file name is given on the command line, the program asks for one)

    The command line interpreter is called at the beginning of the 
    simulation and whenever the simulation is stopped. While executing 
    endless loops, the command line interpreter may be activated by 
    pressing CONTOL-C. 

    Note: the command line interpreter reads only the first character 
    of each command (H for on-line help).

 3) To run in batch mode put the simulation commands in a text file (see 
    an example in run/batch_input_commands.txt)
    To start the simulation and log the trace in a text file:
    C:\>simcpu foo.exx < batch_input_commands.txt > simulation_trace.txt



 Library subroutines include utility
 -----------------------------------

 1) To compile (see make_include.bat):
    gcc -o include.exe include.c

 2) To create a complete assembly language source file foobar.axx starting 
    from a main source file foobar_main.axx containing #include directives 
    to include foobar.ixx files
    (see files *_main.axx in lib/include_samples directory):

   C:\> include foobar_main.axx foobar.axx
