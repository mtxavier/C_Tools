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

#include <stdlib.h>
#include <Classes.h>
#include <StackEnv.h>
#include <List.h>

/*___________________________________________________________________
 |
 |___________________________________________________________________
*/

typedef enum {
    SplitWhole
    ,SplitBottom
    ,SplitTop
    ,SplitMiddle
} ChunkSplit;

typedef struct {
    int Split;
    void *Top;
    dList dList;
} MemChunk;

#define MCBottom(m) ((void *)((m)+1))

MemChunk *MemChunkAlloc(int Size) {
    MemChunk *M;
    char *Top;
    void *p;
    p = malloc(Size+sizeof(MemChunk));
    M = p; p = M+1;
    Top = p; Top += Size;
    M->Top = Top;
    M->dList.n = M->dList.p = &M->dList;
    M->Split = SplitWhole;
    return M;
}

MemChunk *MemChunkSplit(MemChunk *M,void *p,int Size) {
    MemChunk *R;
    R = p; p = R+2;
    if (p>M->Top) {
	R = MemChunkAlloc(Size);
    } else {
	R->Top = M->Top;
	R->dList.n = R->dList.p = &R->dList;
	R->Split = SplitTop;
	M->Split++;
	M->Top = R;
    }
    return R;
}

void MemChunkPatch(MemChunk *M) {
    MemChunk *Top;
    Top = M->Top;
    switch(M->Split) {
    case SplitBottom:
	M->Top = Top->Top;
	M->Split = SplitWhole;
    break;
    case SplitMiddle:
        M->Top = Top->Top;
	M->Split = SplitTop;
    break;
    } 
}

void MemChunkFree(MemChunk *M) {
    MemChunk *Top;
    Top = M->Top;
    switch(M->Split) {
    case SplitBottom:
	M->Top = Top->Top;
	M->Split = SplitWhole;
    case SplitWhole: {
	free(M);
    } break;
    case SplitMiddle:
	M->Top = Top->Top;
	M->Split = SplitTop;
    break;
    }
}

typedef struct {
    MemStack MemStack;
    MemChunk *Mem;
    int Growth;
    List Mark;
} MyMemStack;

MemStack *StackOpenInChunk(MemChunk *M,int GrowthSize) {
    MyMemStack *R;
    R = MCBottom(M);
    R->Mem = M;
    R->MemStack.Top = R->Mem->Top;
    R->MemStack.Bottom = MCBottom(M);
    R->MemStack.p = R+1;
    R->Growth = GrowthSize;
    R->Mark.n = R->MemStack.p;
    return &R->MemStack;
}

MemStack *MemStackOpen(int GrowthSize) {
    return StackOpenInChunk(
        MemChunkAlloc(sizeof(MyMemStack)+sizeof(List)+GrowthSize)
        ,GrowthSize
    );
}

void MemStackClose(MemStack *S) {
    MyMemStack *s;
    s = CastBack(MyMemStack,MemStack,S);
    MemStackPop(S,s+1);
    s->Mem->dList.p->n = s->Mem->dList.n;
    s->Mem->dList.n->p = s->Mem->dList.p;
    free(s->Mem);
}

MemStack *MemStackFork(MemStack *root,int GrowthSize) {
    MemStack *r;
    MyMemStack *R,*Root;
    MemChunk *M;
    r = MemStackOpen(GrowthSize);
    R = CastBack(MyMemStack,MemStack,r);
    Root = CastBack(MyMemStack,MemStack,root);
    M = MemChunkSplit(Root->Mem,root->p,Root->Growth);
    R->Mem->dList.p = &Root->Mem->dList;
    R->Mem->dList.n = &M->dList;
    M->dList.p = &R->Mem->dList;
    M->dList.n = Root->Mem->dList.n;
    M->dList.n->p = &M->dList;
    R->Mem->dList.p->n = &R->Mem->dList;
    Root->Mem = M;
    root->Top = M->Top;
    root->Bottom = MCBottom(M);
    root->p = root->Bottom;
    return r;
}

void *MemStackPush(MemStack *Stack,void *Last) {
    int s,As;
    MemChunk *Mm;
    MyMemStack *S;
    void *p;
    char *Bottom;
    S = CastBack(MyMemStack,MemStack,Stack);
    {
	char *b,*e;
	b = Last; e = Stack->p; s = e-b;
    }
    As = s>S->Growth ? s:S->Growth;
    Mm = MemChunkAlloc(As);
    Bottom = MCBottom(Mm);
    Stack->Bottom = Bottom;
    Stack->p = Bottom+s;
    Stack->Top = Mm->Top;
    Mm->dList.n = S->Mem->dList.n;
    Mm->dList.p = &S->Mem->dList;
    S->Mem->dList.n = &Mm->dList;
    Mm->dList.n->p = &Mm->dList;
    S->Mem = Mm;
    return Bottom;
}

void MemStackPop(MemStack *Stack,void *p) {
    if (p>Stack->Top || p<Stack->Bottom) {
	dList *m;
	MemChunk *M;
        MyMemStack *S;
        S = CastBack(MyMemStack,MemStack,Stack);
	M = S->Mem;
	m = &M->dList;
	do {
	    m = m->p;
	    M->dList.p->n = M->dList.n;
	    M->dList.n->p = M->dList.p;
	    MemChunkFree(M);
	    M = CastBack(MemChunk,dList,m);
	} while(p>M->Top || p<MCBottom(M));
        MemChunkPatch(M);
	S->Mem = M;
	Stack->Bottom = MCBottom(M);
	Stack->Top = M->Top;
    }
    Stack->p = p;
}

void mEnter(MemStack *Stack) {
    List *Mp;
    MyMemStack *S;
    S = CastBack(MyMemStack,MemStack,Stack);
    mPush(Stack,Mp);
    Mp->n = S->Mark.n;
    S->Mark.n = Mp;
}

void mLeave(MemStack *Stack) {
    void *p;
    MyMemStack *S;
    S = CastBack(MyMemStack,MemStack,Stack);
    p = S->Mark.n;
    S->Mark.n = S->Mark.n->n;
    MemStackPop(Stack,p);
}

/*___________________________________________________________________
 |
 |___________________________________________________________________
*/

__thread Env_Env Env;

int xEnvOpen(Env_Env *E,long ParamSize,long ResultSize) {
    E->p = MemStackOpen(ParamSize);
    E->r = MemStackOpen(ResultSize);
    return (0==0);
}

int EnvOpen(long ParamSize,long ResultSize) {
    return xEnvOpen(&Env,ParamSize,ResultSize);
}

int xEnvClose(Env_Env *E) {
    MemStackClose(E->p);
    MemStackClose(E->r);
}

int EnvClose(void) {
    xEnvClose(&Env);
}

