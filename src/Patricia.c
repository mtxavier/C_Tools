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

/*____________________________________________________________________________
 |
 | Patricia trees AKA Radix trees.
 |
 | Each node has a "BitToTest" field. This Field strictly increase has we 
 | progress down the tree. When this field stop increasing, we know we have 
 | reached a leaf.
 |
 | The radix tree has some properties:
 |   (1) Each node is both an internal and a leaf node. It is referenced by
 | exactly two pointers: one descending (pointing toward the node aspect) and 
 | one ascending (pointing toward the leaf aspect).
 |   (2) The keys in each subtrees of a node begin with the same sequence of 
 | bits, up to (but not including) the "BitToTest" bit. Also, the "BitToTest" 
 | bit of every node in the [0] subtree are equal to 0. It is equal to 1 for 
 | the nodes of the [1] subtree.
 |
 | Additional constraints for ease of implementation:
 |    Each non empty tree has a special unique node called 1stOne, with these 
 | properties:
 |     * 1stOne.BitToTest == -1
 |     * 1stOne.Next[0] == &1stOne
 |     * 1stOne.Next[1] == NULL (or the root node, as a convention).
 |    Here how that helps:
 |     * 1stOne is the only node with the NULL pointer in the tree.
 |     * 1stOne has no subtree.
 | There is always exactly 1 NULL pointer in the tree, the 1stOne node ensures
 | that this pointer is localised in an atomic Node. 
 | With the BitToTest set to -1, the 1stOne is already marked as "Leaf" 
 | without the need to explore. When descending the tree, it ensures that we
 | never encounter the NULL pointer.
 |
 | When inserting and removing node, maintening the 1stOne node semantic should
 | be inforced:
 |    * When inserting a node at leaf position, the new node is normally 
 | inserted between the leaf and the parent node. So the 1stOne naturally 
 | stays at leaf position.
 |    * When removing the 1stOne, a new 1stOne must be chosen. The previous
 | node does the job nicely. To do so, the only thing needed is to lift up
 | its other subtree at its position.
 |_____________________________________________________________________________
*/


#include <Patricia.h>

#define MaxInt (((unsigned)-1)>>1)
#define uCharFirst1(r,vl) { \
	unsigned int _v;\
	_v = vl;\
	if (_v&0xf0){\
		r=(_v&0xc0)?((_v&0x80)?0:1):((_v&0x20)?2:3);\
	} else {\
		r=(_v&0x0c)?((_v&0x08)?4:5):((_v&0x02)?6:(_v?7:8));\
	}\
}

PatriciaNod *cPatriciaSeek(PatriciaCtx *Ctx,PatriciaNod *Root,char *s) {
	PatriciaNod *N;
	int d,b,bv;
	d = b = -1;
	N = Root->Next[0];
	while ((b<N->BitToTest)&&(d==b)) {
	    b = N->BitToTest;
		d = Ctx->CheckRadixNext(Ctx,N->Key,b);
		N = N->Next[Ctx->BitVal];
	}
	if ((d==b)&&(N!=Root)) {
		if (Ctx->CheckRadixNext(Ctx,N->Key,MaxInt)!=MaxInt) N=Root;
	} else {
		N = Root;
	}
	return N;
}

PatriciaNod *cPatriciaSeekOrInsert(
	PatriciaCtx *Ctx,PatriciaNod *Root,char *s,PatriciaNod *NewNod
) {
	PatriciaNod *N,**pN;
	pN = Root->Next+0;
	N = *pN;
	if (N==Root) {
		NewNod->Key = s;
		NewNod->BitToTest = -1;
		NewNod->Next[0] = NewNod;
		NewNod->Next[1] = Root;
		Root->Next[0] = NewNod;
        N = NewNod;
	} else {
		int b,bn,d;
		b = d = -1;
		bn = N->BitToTest;
		while ((b==d)&&(b<bn)) {
			b = bn;
			d = Ctx->CheckRadixNext(Ctx,N->Key,b);
            if (d==b) {
                pN = N->Next+Ctx->BitVal;
				N = *pN;
				bn = N->BitToTest;
			}
		}
		if (d==b) {
			b = MaxInt;
			d = Ctx->CheckRadixNext(Ctx,N->Key,b);
		}
		if (d!=b) {
			NewNod->Key = s;
			NewNod->BitToTest = d;
			NewNod->Next[Ctx->BitVal] = NewNod;
			NewNod->Next[Ctx->BitVal^1] = N;
			*pN = NewNod;
			N = NewNod;
		}
	}
	return N;
}

PatriciaNod *cPatriciaRemove(PatriciaCtx *Ctx,PatriciaNod *Root,char *s) {
	PatriciaNod *Rmv,*lfRmv,**pp;
	lfRmv = Root;
	pp = lfRmv->Next+0; Rmv = *pp;
	if (Rmv!=Root) {
		PatriciaNod *p,**plfRmv;
		int b,bn,d,lfv;
		b=d=-1;
		bn = Rmv->BitToTest;
	    plfRmv = pp = Root->Next+0;
        while ((b==d)&&(b<bn)) {
            b = bn;
			d = Ctx->CheckRadixNext(Ctx,Rmv->Key,b);
			plfRmv = pp; lfRmv = Rmv;
			pp = lfRmv->Next+Ctx->BitVal; Rmv = *pp;
            bn = Rmv->BitToTest;
		}
		if (b==d) {
			b = MaxInt;
		    lfv = Ctx->BitVal;
			d = Ctx->CheckRadixNext(Ctx,Rmv->Key,b);
		}
		if (b!=d) {
			Rmv = Root;
		} else { 
		if ((lfRmv==Rmv)||(Rmv->BitToTest==-1)) {
			/* This is the case where Rmv points on itself. */
			if (Rmv->BitToTest==-1) {
				/* This is the case where we remove the 1stOne. */
				if (lfRmv==Root) {
					/* Since the 1stOne points only on itself and Root,
					There can only be one node left. */
					Root->Next[0] = Root;
				} else {
                    /* There are many nodes, we must chose the new 1stOne.
					   lfRmv will do.*/
				    *plfRmv = lfRmv->Next[lfv^1];
				    lfRmv->BitToTest = -1;
					lfRmv->Next[0] = lfRmv;
					lfRmv->Next[1] = Root;
				}
			} else {
			    *plfRmv = lfRmv->Next[lfv^1];
			}
		} else {
            PatriciaNod *ndRmv;
			int ndv;
			ndRmv = Root; ndv = 0;
			p = ndRmv->Next[0];
			while (p!=Rmv) {
				ndRmv = p;
				ndv = Ctx->GetBitVal(s,ndRmv->BitToTest);
				p = ndRmv->Next[ndv];
			}
			*plfRmv = lfRmv->Next[lfv^1];
            lfRmv->Next[0] = Rmv->Next[0];
			lfRmv->Next[1] = Rmv->Next[1];
			lfRmv->BitToTest = Rmv->BitToTest;
			ndRmv->Next[ndv] = lfRmv;
		}}
	}
	return Rmv;
}

typedef struct {
    PatriciaNod *(*Fn)(PatriciaNod *,void *closure);
    PatriciaNod *Result;
    void *Closure;
} ForEachClosure;

static void rForEach(PatriciaNod *Node,ForEachClosure *Closure) {
	int tt;
	PatriciaNod *p;
	tt = Node->BitToTest; p = Node->Next[0];
	if (tt>=p->BitToTest) {
	    Closure->Result = Closure->Fn(p,Closure->Closure);
	} else {
		rForEach(p,Closure);
	}
	if (!Closure->Result) {
		p = Node->Next[1];
		if (tt>=p->BitToTest) {
			Closure->Result = Closure->Fn(p,Closure->Closure);
		} else {
			rForEach(p,Closure);
		}
	}
}

PatriciaNod *PatriciaForEach(
	PatriciaNod *Root,
	PatriciaNod *(*fn)(PatriciaNod *,void *closure),void *Closure
) {
	PatriciaNod *p;
	ForEachClosure C;
	p = Root->Next[0];
	if (p!=Root) {
		if (p->BitToTest==-1) {
			C.Result = fn(p,Closure);
		} else {
	        C.Fn=fn;
	        C.Closure=Closure;
	        C.Result=0;
	        rForEach(p,&C);
		}
	} else {
		C.Result = 0;
	}
	return C.Result;
}

     /*------------*/

static int StringBitVal(const char *Key,int n) {
	return (Key[n>>3]>>((7^n)&7))&1;
}
static int StringCheckNext(PatriciaCtx *Ctx,const char *Ref,int max){
	int r;
	const char *p,*q,*lst,*Key;
	unsigned char pv,df;
	Key = Ctx->Key;
	lst = Key+(max>>3);
	p = Ctx->c;
	q = Ref+(p-Key);
	while ((p<lst)&&((*p)==(*q))&&(*p)) { p++; q++; }
	pv = *p;
	if (p==lst) {
		int mm;
		mm = (7^(max&7));
		df = ((pv^(*q))&(0xfe<<mm));
		if (!df) {
			r = max; Ctx->BitVal = (pv>>mm)&1;
		}
	} else {
	    df = (pv^*q);
		if (!df) {
		    r = max; Ctx->BitVal = 0;
		}
	}
	if (df) {
		int m;
		uCharFirst1(m,df);
		r = ((p-Key)<<3)+m;
		Ctx->BitVal = (pv>>(7^m))&1;
	}
	Ctx->c = p;
	return r;
}

void PatriciaInit(PatriciaNod *Root) {
    static char NullString[4];
	NullString[0] = 0;
	Root -> BitToTest = -1;
	Root -> Key = NullString;
	Root -> Next[0] = Root -> Next[1] = Root;
}

PatriciaNod *PatriciaSeek(PatriciaNod *Root,char *s) {
	PatriciaCtx Ctx;
	Ctx.CheckRadixNext = StringCheckNext;
	Ctx.Key = Ctx.c = s;
    return cPatriciaSeek(&Ctx,Root,s);
}

PatriciaNod *PatriciaSeekOrInsert(
	PatriciaNod *Root,char *s,PatriciaNod *NewNod
){
	PatriciaCtx Ctx;
	Ctx.CheckRadixNext = StringCheckNext;
	Ctx.Key = Ctx.c = s;
    return cPatriciaSeekOrInsert(&Ctx,Root,s,NewNod);
}

PatriciaNod *PatriciaRemove(PatriciaNod *Root,char *s){
	PatriciaCtx Ctx;
	Ctx.CheckRadixNext = StringCheckNext;
	Ctx.GetBitVal = StringBitVal;
	Ctx.Key = Ctx.c = s;
    return cPatriciaRemove(&Ctx,Root,s);
}

/*-----------------------------------------------------------------------------
	Test
-----------------------------------------------------------------------------*/

#ifdef _PatriciaC_ToTest_
	struct {
		char Buffer[20000];
		char *SP;
	} Stack;
	void StackInit() {
		Stack.SP = Stack.Buffer;
	}
	PatriciaNod *PatPush() {
		PatriciaNod *Result;
		Result = (PatriciaNod *)Stack.SP;
		Stack.SP += sizeof(PatriciaNod);
		return Result;
	}
	PatriciaNod *PatPrint(PatriciaNod *Node) {
		printf("%d %s\n",Node->BitToTest,Node->Key);
		return 0;
	}
	void main() {
		PatriciaNod Root,*NewNod;
		FILE *F;
		long EndOfFile=0;
		StackInit();
		PatriciaInit(&Root);
		NewNod = PatPush();
		F = fopen("Patricia.Test","rt");
		while (!EndOfFile) {
			char *P;
			int car;
			P = Stack.SP;
			do {
				car = getc(F);
				if (car!=EOF && car!='\n') *P++=car;
				EndOfFile = (car==EOF);
			} while(!EndOfFile && car!='\n');
			*P = 0;
			if (Stack.SP[0]=='1') {
				printf("Insert %s",Stack.SP+1);
				if(PatriciaSeekOrInsert(&Root,Stack.SP+1,NewNod) == NewNod) {
					Stack.SP += strlen(Stack.SP)+1; 
					NewNod = PatPush();
				}
				printf(" done.\n");
			} else {
				printf("Remove %s",Stack.SP+1);
				PatriciaRemove(&Root,Stack.SP+1);
				printf(" done.\n");
			}
			PatriciaForEach(&Root,&PatPrint,0);
			printf("\n");
		}
		fclose(F);
	}

#endif

