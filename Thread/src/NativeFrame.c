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
#include <StackEnv.h>
#include <setjmp.h>
#include <Thd.h>

typedef struct {
	ThdThread ThdThread;
	ThdItf *Desc;
	int StackSize;
	int Running;
	jmp_buf *Switch;
	MemStack *r,*p;
} NativeFrame;

static int nativeSwitch(ThdThread *this) {
	jmp_buf Ret,*Me;
	MemStack *x;
	ThisToThat(NativeFrame,ThdThread);
	Me = that->Switch; that->Switch = &Ret;
	x = Env.r; Env.r = that->r; that->r = x;
	x = Env.p; Env.p = that->p; that->p = x;
	if (!setjmp(Ret)) { longjmp(*Me,(0==0)); }
	return that->Running;
}

typedef struct GrowthTop {
	jmp_buf Start;
	NativeFrame NewThread;
} GrowthTop;

static __thread struct {
	GrowthTop *Top;
} Lcl = {0};

static void EscalateMore(char *Base,char *Add,GrowthTop *Target);

ThdThread *ThdNativeFiberLaunch(ThdItf *Main,int MainStackSize,int StackSize,int pGrowth,int rGrowth) {
	static struct ThdThread Static = {nativeSwitch,nativeSwitch};
	char Base[sizeof(void *)];
	jmp_buf Here;
	ThdThread *r;
	MemStack *x;
	if (!Lcl.Top) {
		GrowthTop Root;
		Lcl.Top = &Root;
		Root.NewThread.StackSize = MainStackSize;
		if (!setjmp(Root.Start)) {
			EscalateMore(Base,Base,&Root);
		}
	}
	r = &Lcl.Top->NewThread.ThdThread;
	Lcl.Top->NewThread.ThdThread.Static = &Static;
	Lcl.Top->NewThread.Desc = Main;
	Lcl.Top->NewThread.StackSize = StackSize+sizeof(jmp_buf);
	Lcl.Top->NewThread.Switch = &Here;
	Lcl.Top->NewThread.p = Env.p; Env.p = rFork(pGrowth);
	Lcl.Top->NewThread.r = Env.r; Env.r = rFork(rGrowth);
	Lcl.Top->NewThread.Running = (0==0);
	if (!setjmp(Here)) {
		longjmp(Lcl.Top->Start,1);
	}
	return r;
}
static void TopReached(GrowthTop *ThdStart) {
	GrowthTop NewTop;
	char Base[sizeof(void *)];
	Lcl.Top = &NewTop;
	if (!setjmp(NewTop.Start)) {
        longjmp(ThdStart->Start,1);
	} else {
		if (!setjmp(NewTop.Start)) {
            EscalateMore(Base,Base,&NewTop);
		} else {
			NewTop.NewThread.Running = (0==0);
			Call(NewTop.NewThread.Desc,Main,1(&NewTop.NewThread.ThdThread));
			NewTop.NewThread.Running = (0!=0);
			while (0==0) { Call(&NewTop.NewThread.ThdThread,Wait,0); }
		}
	}
}

static void MakeSpace64k(char *Base,GrowthTop *Tgt) { char Extra[0x10000]; EscalateMore(Base,Extra,Tgt); }
static void MakeSpace32k(char *Base,GrowthTop *Tgt) { char Extra[0x8000]; EscalateMore(Base,Extra,Tgt); }
static void MakeSpace16k(char *Base,GrowthTop *Tgt) { char Extra[0x4000]; EscalateMore(Base,Extra,Tgt); }
static void MakeSpace8k(char *Base,GrowthTop *Tgt) { char Extra[0x2000]; EscalateMore(Base,Extra,Tgt); }
static void MakeSpace4k(char *Base,GrowthTop *Tgt) { char Extra[0x1000]; EscalateMore(Base,Extra,Tgt); }
static void MakeSpace2k(char *Base,GrowthTop *Tgt) { char Extra[0x800]; EscalateMore(Base,Extra,Tgt); }
static void MakeSpace1k(char *Base,GrowthTop *Tgt) { char Extra[0x400]; EscalateMore(Base,Extra,Tgt); }
static void MakeSpace512(char *Base,GrowthTop *Tgt) { char Extra[0x200]; EscalateMore(Base,Extra,Tgt); }

typedef void (*MakeMoreSpace)(char *Base,GrowthTop *Tgt);

static void EscalateMore(char *Base,char *Add,GrowthTop *Target) {
	int allcated,needed;
    char Moar[sizeof(void *)];
	allcated = ((Moar-Base)>0)?Moar-Base:Base-Moar;
	needed = Target->NewThread.StackSize-allcated;
	if (needed<=0) {
		TopReached(Target);
	} else {
		static MakeMoreSpace Moooar[] = {
			MakeSpace64k,MakeSpace32k,MakeSpace16k,MakeSpace8k,
			MakeSpace4k, MakeSpace2k, MakeSpace1k, MakeSpace512
		};
		MakeMoreSpace *p,*e;
		int incr;
		incr = 0x10000;
        p = Moooar; e = Moooar+7;
		while ((p<e)&&(incr>needed)) { p++; incr = incr>>1; }
        p[0](Base,Target);
	}
}

