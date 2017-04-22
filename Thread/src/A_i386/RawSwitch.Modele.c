#include <RawSwitch.h>
#include <Classes.h>

// This is the modele for the actual file (RawSwitch.s). For those assembler
// unfriendly users (like me).
// You will have to generate the .s file by stoping gcc at the assembly stage
// (gcc -o RawSwitch.s -S ......).
// Then edit the assembly file. You want 
// ThdFiberRawLaunch() and ThdFiberRawSwitch to have exactly the same 
// calling convention and stack structure.

// Gcc calling conventions: ebp,ebx,esi,edi must be preserved by the Callee
// All other registers: Caller preserved. That goes for the FP stack 
// and, I presume, for the *MX stack as well.
//
// However, we will save those registers directly on the stack. Only the 
// stack pointer needs to be reserved.

// We want 

extern ThdFiberRawFrame *ThdFiberRawSwitch1(ThdFiberRawFrame *Next);

ThdFiberRawFrame *ThdFiberRawLaunch(ThdFiberRawFrame *Main) {
	void *Sp,*Sp0;
    asm (
		"pushl %ebx\n\t"
		"pushl %esi\n\t"
		"pushl %edi\n\t"
	);
	Sp = Main->Switch.Sp;
	asm (
		"movl  %%esp,%0\n\t"
		"movl  %1,%%esp\n\t"
		:"=c"(Sp0) /* output */
		:"d"(Sp)  /* input */
	);
	Main->Switch.Sp = Sp0;
	Call(Main,Main,0);
	asm (
		"popl  %edi\n\t"
	    "popl  %esi\n\t"
		"popl  %ebx\n\t"
	);
	return Main;
}

ThdFiberRawFrame *ThdFiberRawSwitch(ThdFiberRawFrame *Next) {
	void *Sp,*Sp0;
	Sp = Next->Switch.Sp;
    asm (
		"pushl %%ebx\n\t"
		"pushl %%esi\n\t"
		"pushl %%edi\n\t"
		"movl  %%esp,%0\n\t"
		"movl  %1,%%esp\n\t"
		:"=c"(Sp0) /* output */
		:"d"(Sp)  /* input */
	);
	Next->Switch.Sp = Sp0;
	asm (
		"popl  %edi\n\t"
	    "popl  %esi\n\t"
		"popl  %ebx\n\t"
	);
	return Next;
}

