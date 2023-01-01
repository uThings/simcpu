
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


 Library functions can be stored in separate assembly language source
 files. 

 With the include.c utility program the #include directives in a main
 assembly language source file are expanded and the source code of each
 library function is actually included into the final source file.

 1) To compile the utility program include.c 
    (see make_include.bat in "source" directory):
    gcc -o include.exe include.c

 2) To create a complete assembly language source file foobar.axx starting 
    from a main source file foobar_main.axx containing #include directives 
    to include foobar.ixx files
    (see files *_main.axx in lib/include_samples directory):

   C:\> include foobar_main.axx foobar.axx
