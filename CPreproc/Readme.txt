These is an implementation of C Preprocessor.

This is not finished yet, 
   * issuing line numbers isn't supported. Yet.
   * Neither are macro definition in the main environment.

The main interface is in the "../inc/C_Lexic.h".

As it is a very interconnected module, there aren't many sub-modules
that can be made public.

Anyway: functionnalities are:

    - Support for file inclusion.
    - Support for macro expansion.
    - Reading on the fly. You won't need to use an intermediate file
to use this module. Just set the environment (for the moment only the
path can be set), add a file and do the reading Lexem by Lexem.

-------------------------------------

Compilation:

On linux platform (I suppose it would work on any unix platform):

In order to link correctly, you might need to make the objects in the in
the previous directory.

To do so, in the previous directory, type 

    make all


Type

    make all

in the main directory.

That should compile the object files and the test application.

Not much to say. If you don't intend to use the modules, you can still launch
the various test applications. They do stuff.



