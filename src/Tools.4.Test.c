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
#include <Tools.h>
#include <stdio.h>

char b64(char c) {
	if (c>=10) {
        c-=10;
		if (c>=26) {
			c-=26;
			if (c>=26) {
				c = (c==26)?'&':'$';
			} else {
				c = 'a'+c;
			}
		} else {
			c = 'A'+c;
		}
	} else {
		c = '0'+c;
	}
    return c;
}
char *DataInit(void) {
	char *r,c;
	int i;
	rnPush(r,0x2000);
	for (i=0;i<0x2000;i+=2) {
		c = (i>>7)&0x3f;
		r[i] = b64(c);
		c = (i>>1)&0x3f;
		r[i+1] = b64(c);
	}
	return r;
}

void LstEltSet(TlsIdxList *L,int dDat,char *Data,int b,int e) {
	int i,l;
	i = b; l = 0x1000-dDat;
	if ((i+l)>e) { l = e-i; }
	Call(L,EltSet,3(i,Data+(2*dDat),l));
	i+=l; l = 0x1000;
	while (i<e) {
		if ((i+l)>e) { l = e-i; }
        Call(L,EltSet,3(i,Data,l));
		i+=l;
	}
}
int LstEltCheck(TlsIdxList *L,char *Data,int oDat,int b,int e) {
	int ok,f,j,k,l;
	char Output[0x80];
	ok = (0==0);
	j = b; f = -1;
	while (j<e) {
		l = 40;
		if (j+l>e) { l = (e-j); }
		Call(L,EltGet,3(Output,j,l));
        for (k=0;k<(l*2);k++) {
		    int c0,c1;
		    c0 = Output[k]; c1 = Data[(((j+oDat-b)*2)+k)&0x1fff];
			if (ok && (c0!=c1)) { f = j+(k/2); }
		    ok = ok && (c0==c1);
	    }
		j+=l;
	}
	return f;
}

/*--------------------------------------*/

void FrontInsertTest(TlsIdxList *L,char *Data,int i) {
	int j,l,k,ok,f;
	printf("\n    Insertion: %d Elts",i);
	Call(L,Insert,2(0,i));
    printf("... Filling:");
	LstEltSet(L,0,Data,0,i);
    f = LstEltCheck(L,Data,0,0,i);
	if (f<0) { printf("... Ok."); } else {printf("... Failed at %d.",f);}
}

void FrontTest(TlsIdxList *L,char *Data) {
	int i;
	for (i=0;i<=256;i+=16) {
		FrontInsertTest(L,Data,i);
	    printf("   Clearing.");
	    L = Call(L,Clear,0);
	}
	for (i=480;i<=560;i+=16) {
		FrontInsertTest(L,Data,i);
	    printf("   Clearing.");
	    L = Call(L,Clear,0);
	}
	for (i=992;i<=1100;i+=16) {
		FrontInsertTest(L,Data,i);
	    printf("   Clearing.");
	    L = Call(L,Clear,0);
	}
	for (i=8160;i<=8256;i+=16) { 
		FrontInsertTest(L,Data,i);
	    printf("   Clearing.");
	    L = Call(L,Clear,0);
	}
	for (i=8192+480;i<=8192+544;i+=16) {
		FrontInsertTest(L,Data,i);
	    printf("   Clearing.");
	    L = Call(L,Clear,0);
	}
}

/*--------------------------------------*/

void AddInsertTest(TlsIdxList *L,char *Data,int Base,int Add) {
	int ok,bi,ei;
	int dD0,dD1,dD2;
	dD0 = 0x11; dD1 = 0xd3; dD2 = 0x131;
	printf("\n    Adding %d Elts in %d. At Start:",Add,Base);
	Call(L,Insert,2(0,Base));
	LstEltSet(L,0,Data,0,Base);
	Call(L,Insert,2(0,Add));
	LstEltSet(L,dD0,Data,0,Add);
	ok = LstEltCheck(L,Data,dD0,0,Add);
	if (ok<0) { ok = LstEltCheck(L,Data,0,Add,Base+Add); }
	if (ok<0) { printf("Ok"); } else { printf("Failed(%d)",ok); }
	Call(L,Clear,0);

	printf(" At End:");
	Call(L,Insert,2(0,Base));
	LstEltSet(L,0,Data,0,Base);
	Call(L,Insert,2(Base,Add));
	LstEltSet(L,dD0,Data,Base,Base+Add);
	ok = LstEltCheck(L,Data,0,0,Base);
	if (ok<0) { ok = LstEltCheck(L,Data,dD0,Base,Add+Base); }
	if (ok<0) { printf("Ok"); } else { printf("Failed(%d)",ok); }
	Call(L,Clear,0);

	printf(" Middle:");
	Call(L,Insert,2(0,Base));
	LstEltSet(L,0,Data,0,Base);
	Call(L,Insert,2(Base/2,Add));
	LstEltSet(L,dD0,Data,Base/2,(Base/2)+Add);
	ok = LstEltCheck(L,Data,0,0,Base/2);
	if (ok<0) { ok = LstEltCheck(L,Data,dD0,Base/2,Add+(Base/2)); }
	if (ok<0) { ok = LstEltCheck(L,Data,Base/2,Add+(Base/2),Add+Base); }
	if (ok<0) { printf("Ok"); } else { printf("Failed(%d)",ok); }

	printf(" All three:");
	Call(L,Insert,2(0,Add));
	LstEltSet(L,dD1,Data,0,Add);
	Call(L,Insert,2(Base+(2*Add),Add));
	LstEltSet(L,dD2,Data,Base+(2*Add),Base+(3*Add));
	           bi = 0; ei = Add;     ok = LstEltCheck(L,Data,dD1,bi,ei);
	if (ok<0) { bi=ei; ei+=(Base/2); ok = LstEltCheck(L,Data,0,bi,ei); }
	if (ok<0) { bi=ei; ei+=Add;      ok = LstEltCheck(L,Data,dD0,bi,ei); }
	if (ok<0) { bi=ei; ei+=Base-(Base/2); ok = LstEltCheck(L,Data,Base/2,bi,ei); }
	if (ok<0) { bi=ei; ei+=Add;      ok = LstEltCheck(L,Data,dD2,bi,ei); }
	if (ok<0) { printf("Ok"); } else { printf("Failed(%d)",ok); }
	Call(L,Clear,0);
}

void AddTest(TlsIdxList *L,char *Data) {
	void xInsert(int a,int b) {
		AddInsertTest(L,Data,a,b);
		AddInsertTest(L,Data,b,a);
	}
    AddInsertTest(L,Data,0,0);
	xInsert(2,0);
	AddInsertTest(L,Data,2,2);
	xInsert(0,16);
	AddInsertTest(L,Data,16,16);
	xInsert(0,32);
	xInsert(16,32);
	AddInsertTest(L,Data,32,32);
	xInsert(0,48);
	xInsert(16,48);
	xInsert(32,48);
	AddInsertTest(L,Data,48,48);
	xInsert(48,64);
	xInsert(0,64);
	xInsert(0,128);
	xInsert(16,128);
	xInsert(32,128);
	xInsert(0,500);
	xInsert(16,500);
	xInsert(30,500);
	xInsert(64,500);
	xInsert(0,512);
	xInsert(16,512);
	xInsert(32,512);
	xInsert(64,512);
	AddInsertTest(L,Data,512,512);
	xInsert(400,512);
	xInsert(0,8192-16);
	xInsert(16,8192-16);
	xInsert(32,8192-16);
	xInsert(512,8192-16);
	xInsert(0,8192);
	xInsert(16,8192);
	xInsert(32,8192);
	xInsert(70,8192);
	xInsert(512,8192);
	xInsert(484,8192);
	xInsert(560,8192);
	xInsert(0,8192+32);
	xInsert(16,8192+32);
	xInsert(32,8192+32);
	xInsert(70,8192+32);
	xInsert(512,8192+32);
	xInsert(484,8192+32);
	xInsert(570,8192+32);
}

/*--------------------------------------*/

void SomeRemoveTest(TlsIdxList *L,char *Data,int Base,int Sub) {
	int ok,i;
	printf("\n    Remove: %d Elts from %d",Sub,Base);
	Call(L,Insert,2(0,Base));
	LstEltSet(L,0,Data,0,Base);
	printf(" From start:");
    Call(L,Remove,2(0,Sub));
    ok = LstEltCheck(L,Data,Sub,0,Base-Sub);
	if (ok<0) { printf("Ok "); } else { printf("Failed(%d) ",ok);}
	Call(L,Clear,0);

	Call(L,Insert,2(0,Base));
	LstEltSet(L,0,Data,0,Base);
	printf(" end:");
	Call(L,Remove,2(Base-Sub,Sub));
    ok = LstEltCheck(L,Data,0,0,Base-Sub);
	if (ok<0) { printf("Ok "); } else { printf("Failed(%d) ",ok);}
	Call(L,Clear,0);

	Call(L,Insert,2(0,Base));
	LstEltSet(L,0,Data,0,Base);
	i = (Base-Sub)/2;
	printf(" 1/2:");
	Call(L,Remove,2(i,Sub));
    ok = LstEltCheck(L,Data,0,0,i);
	if (ok<0) { ok = LstEltCheck(L,Data,i+Sub,i,Base-Sub); }
	if (ok<0) { printf("Ok "); } else { printf("Failed(%d) ",ok);}
	Call(L,Clear,0);

	if (Sub+1<=Base) {
	    Call(L,Insert,2(0,Base));
	    LstEltSet(L,0,Data,0,Base);
	    printf("1:");
	    Call(L,Remove,2(1,Sub));
        ok = LstEltCheck(L,Data,0,0,1);
	    if (ok<0) { ok = LstEltCheck(L,Data,1+Sub,1,Base-Sub); }
	    if (ok<0) { printf("Ok "); } else { printf("Failed(%d) ",ok);}
	    Call(L,Clear,0);

	    Call(L,Insert,2(0,Base));
	    LstEltSet(L,0,Data,0,Base);
	    printf("Frlst:");
	    Call(L,Remove,2(Base-(Sub+1),Sub));
        ok = LstEltCheck(L,Data,0,0,Base-(Sub+1));
	    if (ok<0) { ok = LstEltCheck(L,Data,Base-1,Base-(Sub+1),Base-Sub); }
	    if (ok<0) { printf("Ok "); } else { printf("Failed(%d) ",ok);}
	    Call(L,Clear,0);
	}
}

void RemoveTest(TlsIdxList *L,char *Data) {
	SomeRemoveTest(L,Data,0,0);
	SomeRemoveTest(L,Data,12,0);
	SomeRemoveTest(L,Data,12,12);
	SomeRemoveTest(L,Data,16,4);
	SomeRemoveTest(L,Data,16,12);
	SomeRemoveTest(L,Data,32,0);
	SomeRemoveTest(L,Data,32,12);
	SomeRemoveTest(L,Data,32,32);
	SomeRemoveTest(L,Data,36,12);
	SomeRemoveTest(L,Data,64,0);
	SomeRemoveTest(L,Data,64,64);
	SomeRemoveTest(L,Data,64,16);
	SomeRemoveTest(L,Data,64,32);
	SomeRemoveTest(L,Data,64,40);
	SomeRemoveTest(L,Data,510,0);
	SomeRemoveTest(L,Data,510,510);
	SomeRemoveTest(L,Data,510,12);
	SomeRemoveTest(L,Data,510,32);
	SomeRemoveTest(L,Data,510,70);
	SomeRemoveTest(L,Data,512,0);
	SomeRemoveTest(L,Data,512,510);
	SomeRemoveTest(L,Data,512,512);
	SomeRemoveTest(L,Data,512,70);
	SomeRemoveTest(L,Data,1080,0);
	SomeRemoveTest(L,Data,1080,512);
	SomeRemoveTest(L,Data,1080,600);
	SomeRemoveTest(L,Data,1080,70);
	SomeRemoveTest(L,Data,8192,0);
	SomeRemoveTest(L,Data,8192,32);
	SomeRemoveTest(L,Data,8192,80);
	SomeRemoveTest(L,Data,8192,512);
	SomeRemoveTest(L,Data,8192,600);
	SomeRemoveTest(L,Data,8192,8192);
	SomeRemoveTest(L,Data,8392,0);
	SomeRemoveTest(L,Data,8392,32);
	SomeRemoveTest(L,Data,8392,80);
	SomeRemoveTest(L,Data,8392,512);
	SomeRemoveTest(L,Data,8392,600);
	SomeRemoveTest(L,Data,8392,8192);
}

/*--------------------------------------*/

typedef struct {
	TlsIdxListIterator TlsIdxListIterator;
	int Begin,End,Step,Ok,Error,Nb,Idx;
	char *Data;
} stForEachTest;

int MyForEachFound(TlsIdxListIterator *this,int Idx,void *Data) {
	char *p,*q;
	ThisToThat(stForEachTest,TlsIdxListIterator);
    p = that->Data+((Idx&0x0fff)*2);
	q = Data;
	that->Ok = that->Ok && (p[0]==q[0]) && (p[1]==q[1]);
	that->Ok = that->Ok && (Idx>=that->Begin) && (Idx<that->End) && (((Idx-that->Begin)%that->Step)==0);
	that->Ok = that->Ok && (Idx==that->Idx);
	that->Nb--; that->Idx += that->Step;
	if (!that->Ok) { that->Error = Idx; }
	return !(that->Ok);
}

TlsIdxListIterator *MyForEachInit(stForEachTest *r,char *Data,int Begin,int End,int Step) {
	static struct TlsIdxListIterator Static = {MyForEachFound};
	r->TlsIdxListIterator.Static = &Static;
	r->Data = Data;
	r->Begin = Begin;
	r->End = End;
	r->Step = Step;
	r->Ok = (0==0);
	r->Nb = (End+(Step-1)-Begin)/Step;
	r->Idx = Begin;
	r->Error = -1;
	return &r->TlsIdxListIterator;
}

void ForEachSomeTest(TlsIdxList *L,char *Data,int Content,int Begin,int End,int Step) {
	stForEachTest Itr;
	printf("\n     In %d, for each [%d,%d[, step %d :",Content,Begin,End,Step);
	Call(L,Insert,2(0,Content));
	LstEltSet(L,0,Data,0,Content);
	MyForEachInit(&Itr,Data,Begin,End,Step);
	Call(L,ForEach,4(Begin,End,Step,&Itr.TlsIdxListIterator));
	if ((Itr.Ok)&&(Itr.Nb==0)) {
		printf("Ok");
	} else {
		printf("Failed(%d)",Itr.Error);
	}
	Call(L,Clear,0);
}
void ForEachTest(TlsIdxList *L,char *Data) {
	ForEachSomeTest(L,Data,16,0,16,1);
	ForEachSomeTest(L,Data,32,0,32,1);
	ForEachSomeTest(L,Data,32,5,31,2);
	ForEachSomeTest(L,Data,128,0,128,1);
	ForEachSomeTest(L,Data,128,1,129,2);
	ForEachSomeTest(L,Data,128,2,130,4);
	ForEachSomeTest(L,Data,128,42,86,4);
	ForEachSomeTest(L,Data,128,2,130,32);
	ForEachSomeTest(L,Data,128,2,167,55);
	ForEachSomeTest(L,Data,720,0,720,1);
	ForEachSomeTest(L,Data,720,1,721,2);
	ForEachSomeTest(L,Data,720,6,720,16);
	ForEachSomeTest(L,Data,720,142,186,4);
	ForEachSomeTest(L,Data,720,100,500,34);
	ForEachSomeTest(L,Data,720,2,707,71);
	ForEachSomeTest(L,Data,9000,0,9000,7);
	ForEachSomeTest(L,Data,9000,2,8800,600);
}


/*--------------------------------------*/

main() {
	TlsIdxList *L;
	char Default[16];
	char *Data;
	EnvOpen(4096,4096);
	Data = DataInit();
	printf("\nDebut: ");
	Default[0]=Default[1]=Default[2]=Default[3]=' ';
	printf("\n    Creation: EltSz==2; Growth==0x20");
    L = TlsBigIdxListNew(2,32);
	FrontTest(L,Data);
	AddTest(L,Data);
	RemoveTest(L,Data);
    ForEachTest(L,Data);
	printf("\nEnd\n");
	EnvClose();
}

