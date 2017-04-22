Fondement Michtam is a library of widely experimental primitives that MIGHT be
useful in more well defined projects.

This directory contains the definition of a script language. This is largely work in progress. As such, it hasn't gotten true interface yet.

The inc directory contains the public headers.
The src directory contains the code. Local headers should also be kept here.
The obj directory contains the object files.

For now, the implemented functionalites are:

    * Type definition (type checking is not implemented yet).
    * Data declaration.
    * Functions declarations, as part of data declaration. 
         * Functions are considered data.
         * Recursion is accepted.
    * Lazy evaluation: Internal representation of data and functions are actually a formula form. You have to force the evaluation to get the actual value.
    * Modules division. Datas and types are grouped in modules.
         * Modules have interfaces and dependencies.
         * Loading a module will loads its dependencies.
    
So, for now, it is largely a declarative language. It is the aim of the author to add an imperative part to allow, you know, scripting capabilities.

-------------------------------------

Compilation:

This module is dependent on the main components and on the CPreproc component.
You should build them prior to building this.

On linux platform (I suppose it would work on any unix platform):

Main component:
    In the previous directory type:
        make all

CPreproc component:
    In the CPreproc directory type:
       make all


Then, in this directory, type
    make all


That should compile the object files and link the test application.

Not much to say. If you don't intend to use the modules, you can still launch
the various test applications. They do stuff.



