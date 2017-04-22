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

#include <StackEnv.h>
#include <Classes.h>
#include <RawSwitch.h>
#include <Thd.h>

typedef struct {
	ThdThread ThdThread;
	ThdFiberRawFrame Raw;
	ThdItf *Desc;
	MemStack *p,*r;
	int Running;
} thdEnvThread;
static int envDescSwitch(ThdThread *this) {
	MemStack *x;
	ThisToThat(thdEnvThread,ThdThread);
    x = Env.r; Env.r = that->r; that->r = x;
	x = Env.p; Env.p = that->p; that->p = x;
	ThdFiberRawSwitch(&that->Raw);
	// !!! Be careful here, we mustn't switch the state back,
	// !!! because the return will be done with a new call to envDescSwitch()  !!!
	return that->Running;
}

static void envDescMain(ThdFiberRawFrame *this) {
	MemStack *x;
	ThisToThat(thdEnvThread,Raw);
    x = Env.r; Env.r = that->r; that->r = x;
	x = Env.p; Env.p = that->p; that->p = x;
	that->Running = (0==0);
	Call(that->Desc,Main,1(&that->ThdThread));
	that->Running = (0!=0);
	while (0==0) { Call(&that->ThdThread,Wait,0);}
}

ThdThread *ThdEnvFiberLaunch(ThdItf *Main,int StackSize,int pGrowth,int rGrowth) {
	static struct ThdFiberRawFrame Raw = {envDescMain};
	static struct ThdThread Thrd = {envDescSwitch,envDescSwitch};
	thdEnvThread *r;
	MemStack *P,*R;
	rPush(r);
	r->ThdThread.Static = &Thrd;
	r->Raw.Static = &Raw;
	r->Desc = Main;
   	// !!! the new pStack has to be stack on the rStack (like every other variables) !!!
	r->r = rFork(rGrowth);
	r->p = rFork(pGrowth);
	r->Running = (0==0);
	ThdFiberRawSetStack(&r->Raw,StackSize);
	ThdFiberRawLaunch(&r->Raw);
	return &r->ThdThread;
}

/*_____________________________________________________________
 |
 | Fiber implementation with RawSwitch.
 | You will have to chose your implementation of RawSwitch
 | at link time:
 |     RawSwitch.o is quite fast but not very portable, being mainly
 | assembly language and all.
 |     RawSwitch.C.o uses longjmp for better compatibility, and
 | is only as good as longjmp. You will have to adjust it for
 | your platform thou: there's only one assembler line.
 |     I wouldnt recommand NativeFrame.o: it is slow and has
 | awful memory management. It is portable, if all else fail.
 | If it comes to that, you'd better write your own routines for 
 | your system: only two short routines are needed !
 |_____________________________________________________________
*/

#include <RawSwitch.h>

typedef struct {
	ThdThread ThdThread;
	ThdFiberRawFrame Raw;
	ThdItf *Desc;
	int Running;
} rawFiber;

static int rawFiberSwitch(ThdThread *this) {
	ThisToThat(rawFiber,ThdThread);
	ThdFiberRawSwitch(&that->Raw);
	return (that->Running);
}

static void rawFiberMain(ThdFiberRawFrame *this) {
	ThisToThat(rawFiber,Raw);
	that->Running = (0==0);
	Call(that->Desc,Main,1(&that->ThdThread));
	that->Running = (0!=0);
	while (0==0) {
		ThdFiberRawSwitch(this); 
	}
}

ThdThread *ThdFiberLaunch(ThdItf *Main,int StackSize) {
	rawFiber *r;
	static struct ThdFiberRawFrame Raw = {rawFiberMain};
	static struct ThdThread Static = {rawFiberSwitch,rawFiberSwitch};
	rPush(r);
	r->Desc = Main;
	r->ThdThread.Static = &Static;
	r->Raw.Static = &Raw;
	r->Running = (0==0);
	ThdFiberRawSetStack(&r->Raw,StackSize);
	ThdFiberRawLaunch(&r->Raw);
	return &r->ThdThread;
}

#include <OsThd.c.h>


