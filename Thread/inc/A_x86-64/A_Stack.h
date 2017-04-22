/*  Fondement Michtam
 *  Copyright (C) 2011 Xavier Lacroix
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _A_Stack_h_
#define _A_Stack_h_

// Gcc calling conventions: ebp,ebx,esi,edi must be preserved by the Callee
// All other registers: Caller preserved. That goes for the FP stack 
// and, I presume, for the *MX stack as well.
//
// However, we will save those registers directly on the stack. Only the 
// stack pointer needs to be reserved.

#include <setjmp.h>
typedef struct {
	void *Sp,*bSp;
	jmp_buf *Cont;
} Arch_StackRegisters;

#endif
