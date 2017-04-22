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

#include <Classes.h>
#include <RawSwitch.h>
#include <setjmp.h>

//
// This is the version using jmp_buf for storing continue value.
// There's an direct SP allocation somewhere, to allocate memory.
//

ThdFiberRawFrame *ThdFiberRawSwitch(ThdFiberRawFrame *Next) {
	jmp_buf *old,Cont;
    old = Next->Switch.Cont;
	Next->Switch.Cont = &Cont;
	if (!setjmp(Cont)) { longjmp(*old,(0==0)); }
	return Next;
}

ThdFiberRawFrame *ThdFiberRawLaunch(ThdFiberRawFrame *Main) {
	jmp_buf Cont;
	Main->Switch.Cont = &Cont;
	if (!setjmp(Cont)) {
	    void **NF;
		NF = ((void **)(Main->Switch.Sp))-1; 
		asm ( 
			"movl %0,%%esp"
			:
			:"r"(NF)
		);
		Call(Main,Main,0);
		while (0==0) { 
			Main = ThdFiberRawSwitch(Main);
		}
	}
	return Main;
}

