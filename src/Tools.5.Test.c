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
#include <Tools.h>
#include <stdio.h>

typedef int IntBlock40[0x40];

typedef struct {
	int Seed;
} stRandom;

static int NextRandom(stRandom *rnd) {
	rnd->Seed = (rnd->Seed)*0x356d9adb+0xeadb304d;
	return rnd->Seed;
}

static void MemSpaceBlockSet(Tls1dMemSpace *dst,int idx,stRandom *Start) {
	int *p,*e;
	stRandom rnd;
	rnd.Seed = Start->Seed;
	p = Call(dst,GetOrAdd,1(idx));
	e = p+0x40;
	while (p<e) { *p++ = NextRandom(&rnd);}
	Start->Seed = rnd.Seed;
}
static int MemSpaceBlockCheck(Tls1dMemSpace *dst,int idx,stRandom *Start) {
	void *datornull;
	int *p,*e,r;
	stRandom rnd;
	p = Call(dst,Get,1(idx));
	r = (p!=((void *)0x0));
    if (r) {
	    rnd.Seed = Start->Seed;
		e = p+0x40;
        while ((p<e)&&r) {
			r = (*p==NextRandom(&rnd));
			p++;
		}
	}
	Start->Seed = rnd.Seed;
	return r;
}

#define div40(q,r,a) { r=a&0x3f; if (a<0) {q=-((-(a-r))/0x40);} else { q=a/0x40;}}

static void MemSpaceRangeSet(Tls1dMemSpace *dst,int b,int e,stRandom *cntnt) {
	int pb,rb,pe,re;
	int *bblk,*p,*eblk;
	div40(pb,rb,b)
	div40(pe,re,e)
	if (pb==pe) {
		if (rb!=re) {
		    bblk = Call(dst,GetOrAdd,1(pb));
		    p = bblk+rb; eblk = bblk+re;
		    while (p<eblk) { *p++=NextRandom(cntnt); }
		}
	} else {
		while (pb<pe) {
			bblk = Call(dst,GetOrAdd,1(pb));
			p = bblk+rb; eblk = bblk+0x40;
			while (p<eblk) { *p++=NextRandom(cntnt); }
			pb++; rb=0;
		}
		if (re) {
			bblk = Call(dst,GetOrAdd,1(pb));
			p = bblk; eblk = p+re;
			while (p<eblk) { *p++=NextRandom(cntnt); }
		}
	}
}
static int MemSpaceRangeCheck(Tls1dMemSpace *dst,int b,int e,stRandom *cntnt) {
	int pb,rb,pe,re,r;
	int *bblk,*p,*eblk;
	r = (0==0);
	div40(pb,rb,b)
	div40(pe,re,e)
	if (pb==pe) {
		if (rb!=re) {
			bblk = Call(dst,Get,1(pb));
			p = bblk+rb; eblk = bblk+re;
			r = (bblk!=((void *)0));
			while (r && (p<eblk)) { r = (*p==NextRandom(cntnt)); p++;}
		}
	} else {
		while (r&&(pb<pe)) {
			bblk = Call(dst,Get,1(pb));
			p = bblk+rb; eblk = bblk+0x40;
			r = (bblk!=((void *)0));
            while (r&&(p<eblk)) { r = (*p==NextRandom(cntnt)); p++;}
			pb++; rb=0;
		}
		if (re&&r) {
			bblk = Call(dst,Get,1(pb));
			p = bblk; eblk = p+re;
			r = (bblk!=((void *)0));
			while (r&&(p<eblk)) { r = (*p==NextRandom(cntnt)); p++;}
		}
	}
	return r;
}

main() {
	Tls1dMemSpace *Mem1,*Mem2;
	int Check,i,j,k;
	stRandom rands;
	EnvOpen(4096,4096);
	Mem1 = Tls1dMemSpaceNew(sizeof(IntBlock40));
	rands.Seed = 300;
	MemSpaceBlockSet(Mem1,20000,&rands);
	rands.Seed = 300;
    Check = MemSpaceBlockCheck(Mem1,20000,&rands);
	if (Check) { printf("Test 1: Pass !\n"); } else { printf("Test 1: Failed !\n"); }
	rands.Seed = 1; Check = MemSpaceBlockCheck(Mem1,20000,&rands);
	if (!Check) { printf("Test 2: Pass !\n"); } else { printf("Test 2: Failed !\n"); }
	rands.Seed = 300; Check = MemSpaceBlockCheck(Mem1,15000,&rands);
	if (!Check) { printf("Test 3: Pass !\n"); } else { printf("Test 3: Failed !\n"); }
	for (i=0; i<20; i++) { rands.Seed = 24+121*i; MemSpaceBlockSet(Mem1,-200000+i*36033,&rands); }
	rands.Seed = 57; i=1; j=1;
    while (i<0x100) { 
		MemSpaceRangeSet(Mem1,-200+j,-200+(j+i),&rands); 
		j+=2*i; 
		i++; 
	}
	rands.Seed = 9;
    MemSpaceRangeSet(Mem1,(0x24000)+5,(0x24000)+0xb9,&rands);
    MemSpaceRangeSet(Mem1,(0x24000)+0x113,(0x24000)+0x245,&rands);
    MemSpaceRangeSet(Mem1,(0x24000)+0xb9,(0x24000)+0x113,&rands);
    /*----*/
	Check = (0==0); i = 0;
	while ((Check)&&(i<20)) { rands.Seed = 24+121*i; Check = MemSpaceBlockCheck(Mem1,-200000+i*36033,&rands); i++; }
	if (Check) { printf("Test 4: Pass !\n"); } else { printf("Test 4: Failed !\n"); }
	rands.Seed = 57; i=1; j=1; Check = (0==0);
	while ((i<0x100)&&(Check)) { 
		Check = MemSpaceRangeCheck(Mem1,-200+j,-200+(j+i),&rands); 
		j+=2*i; 
		i++; 
	}
	if (Check) { printf("Test 5: Pass !\n"); } else { printf("Test 5: Failed !\n"); }
	rands.Seed = 9; Check=(0==0);
    Check = Check && MemSpaceRangeCheck(Mem1,(0x24000)+5,(0x24000)+0xb9,&rands);
    Check = Check && MemSpaceRangeCheck(Mem1,(0x24000)+0x113,(0x24000)+0x245,&rands);
    Check = Check && MemSpaceRangeCheck(Mem1,(0x24000)+0xb9,(0x24000)+0x113,&rands);
	if (Check) { printf("Test 6: Pass !\n"); } else { printf("Test 6: Failed !\n"); }
	EnvClose();
}

