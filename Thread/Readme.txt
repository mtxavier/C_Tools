This is the threads/fiber/coroutines implementation.
Threads are programs elements that can act in true parallelism way. As such 
special care has to be taken to protect critical datas from becoming 
incoherent. This can really get pretty confusing pretty fast, so be sure to
know what you're doing. 
I suggest using the safer fibers and coroutines until your program is 
reasonably debugged before going full parallelism.

I was also unable to check with certainty that my thread implementation, using
the linux thread library, was fullproof. Using only the 'wait()' primitive 
ensures that only one thread is active at a time (and also that a thread is 
active at all times), so it is pretty safe. The 'ack()' primitive unlocks a 
waiting thread, or waits for it if it isn't waiting yet. It allows true 
parallelism, but you are never guaranteed that this will actually be the 
case so be ready for anything !!! (more on this later).

Usage:
   The main interface is in "Thd.h" in the main directory.
   To use thread facility you will have to:
       * Derive your own 'ThdItf' object. You only have to provide the 'Main()'
method and a constructor. (You can even afford to dispose of the constructor
primitive if you don't intent to share your thread object). The two threads 
are supposed to be communicating using the private fields in the ThdItf, so
be sure to include the appropriate fields.
       * Obtain a 'ThdItf' object by calling your constructor. Even if they
have the same Main() method, they usually will need their own private data,
so you will have to provide one object for each new thread.
       * Create your thread by calling the relevant launch thread primitive.
The following are provided:
            * 'ThdFiberLaunch(ThdItf *Main,...)'
            * 'ThdEnvFiberLaunch(ThdItf *Main,...)'
            * 'ThdNativeFiberLaunch(ThdItf *Main,...)'
            * 'ThdThreadLaunch(ThdItf *Main,...)'
            * 'ThdProcessLaunch(ThdItf *Main,...)' Not implemented yet.
       * Those primitives will block the current process and call the 'Main()'
method of the provided Main interface object. 
       * Your 'Main()' method is called with a 'ThdThread' object as a 
parameter. It might use it to give the hand back to its root thread. This is
done by calling one of the 'Wait()' or 'Ack()' method. But for now, it should
concentrate on doing its own initialisations. As the parent thread is blocked,
operations and data are safe during the initialisation phase. When done, the
child thread should give the hand back to its parent by calling the 'Wait()'
method.
       * Back to the parent thread. At the return of the launch thread 
primitive, the parent thread should also receive, as the result, a 
'ThdThread' object. It can use it to give the hand back to the child process.
       * The child and the parent thread can use their respective 'ThdThread'
object to pass the program flow to one another. They will use the 'Wait()'
and 'Ack()' methods to do so.
       * Very important !!! The 'ThdThread' objects are personal and under no
circumstance should threads share them with other threads. One of the 
consequences is that threads will be organised as a tree hierarchy. The 
program flow can only be handed between parents and direct childrens.
       * The 'Ack()' ans 'Wait()' directive will return true (0==0) if the
child is running and false (0!=0) when the child has ended, that is when it
has left its main method. Obviously, the child will always get (0==0).

Explanation on the primitives:

    * The Wait() method will block the calling thread and unblock the paired
one, effectively handing the program flow to the other thread.
    * The Ack() method will unblock the paired thread if it is blocked, or 
wait for it if it isn't. In the case of Fibers, it will act exactly as the
Wait() method, and is indeed the same function.

    * Fiber implementation are very architecture dependent. The 
        * ThdNativeFiberLaunch() use setjmp/longjmp to give a somewhat 
portable implementation. It is (relatively) slow and will use your main 
stack to store the various fiber stacks. As a result you will have to
work with very fragmented and short stacks. It might be enough in some case,
so you can use this as a last resort.
       * ThdFiberLaunch()/ThdEnvFiberLaunch(). This implementations use 
routines present in the 'RawSwitch.s' assembler file to do the actual stack
switching. It is very fast and safe, but obviously not very portable. The one
that is currently provided will work on linux+i64 architecture.
There are only two short routines, so that shouldn't be very difficult to
port to other architectures, but I won't do it since I wouldn't be able to 
test it.
    The difference between the two is that ThdEnvFiberLaunch() will set 
a locals rStack and pStack for the new threads and a switching of the stacks
will be done each time the program flow pass from one fiber to the other.
    If you use your fiber to only manage created program components, and 
never intend to allocate anything within it, you can get by with only 
ThdFiberLaunch(). In the other case you might be better of with 
ThdEnvFiberLaunch().
       * 'ThdThreadLaunch()' use your OS thread library to create threads.
For now, it might be your only way to access true parallelism on your new
shiny multicore system. However true parallelism comes with a heavy price
that you will all too soon learn to pay. One word: unpredictability. 
Compared to fibers, the switching process is definitely slower, so you have
no advantage at all using threads instead of fiber on monoprocessors systems. 

Other considerations:
   * Compiling:
This package is dependant on the root package. If you haven't done so yet,
build the main package by typing in the previous directory:
        make all
Then, in this directory, type
        make all

   * Linking with your own application:
You will need to link your program with the following files:
    RawSwitch.o RawStack.o Thd.o
NativeFrame.o is optional, if you want to use ThdNativeFiberLaunch().
You will also have to provide the '-lpthread' option to GCC at linking stage.

   * As we use thread local variables in multithreading environment, *especially* for
the rStack and pStack pointers, multithreading as the dubious side effect off bleeding
on all other modules. In peculiar, the 'Env' variable (containing the stack pointers)
is prefixed with the __thread keyword. What it means is that all access to it will
be done via the 'GS:' segment (or equivalent). I don't know the hit performance, but
at the very least, it generates a bigger code, so it can't be too good. 

Bugs:
    * In the case of threads, the Ack() method doesn't seems to act properly.
I have experienced some strange behaviours, like bad Wait/Ack pairing, the
target never awakening, and random deadlock. It might have to do with the use
of the print() primitive in all the threads. 
Operations on the same file descriptor breaks the rule of strict tree hierarchy.
Anyway, you might want to stick to the 'Wait()' primitive until further 
investigations. And if you have solid thread experience, you might also help me
we some useful hints !!!

