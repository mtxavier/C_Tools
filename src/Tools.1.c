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

/*____________________________________
 |
 | Bit Fields manipulation
 |____________________________________
*/


int TlsBitFieldGetField(char *buf,int bField,int eField) {
    int r;
    r = 0;
    if (bField<eField) {
        int s,esi,si;
        unsigned int v,m;
        char *p,*ep;
        unsigned char v0;
        s = bField&7; p = buf+(bField>>3); ep = buf+((eField-1)>>3);
        v0 = *p; v = v0; if (p<ep) { v0=p[1]; v = v|(v0<<8);}
        si=0; esi=(eField-bField)-8;
        while (si<esi) {
        r = r|(((v>>s)&0xff)<<si);
        v=v>>8; si+=8; p++;
            if (p<ep) { v0=p[1]; v=v|(v0<<8); }
        }
        m = 0xff>>((-esi)&7);
        r = r|(((v>>s)&m)<<si);
    }
    return r;
}

void TlsBitFieldSetField(char *buf,int bField,int eField,int Value) {
    if (bField<eField) {
        int s,esi,si;
        unsigned int v,m,cm,em,ecm,val;
        char *p,*ep;
        unsigned char v0;
        s = bField&7; p = buf+(bField>>3); ep = buf+((eField-1)>>3);
        v0 = *p; v = v0; val = Value;
        m = (0xff<<s); cm = 0xffff^m;
        si=0; esi=(eField-bField)-8;
        while (p<ep) {
            v0 = p[1]; v = ((v|(v0<<8))&cm)|(((val>>si)<<s)&m); 
            *p = v&0xff; p++; v=v>>8; si+=8;
        }
        em = m&(0xff>>((-eField)&7));
        ecm = 0xffff^em;
        v = (v&ecm)|(((val>>si)<<s)&em);
        *p = v&0xff;
    }
}

void TlsBitFieldFillField(char *buf,int bField,int eField,int val) {
    /* ToDo: we are just lazy here, make an optimized version !! */
    if (bField<eField) {
        char *p,*bp,*ep;
        int i;
        unsigned char bm,m,em;
        bp = buf+(bField>>3); bm = 0xff<<(bField&7);
        ep = buf+((eField-1)>>3); em = 0xff>>((-eField)&7);
        if (val) {
            if (bp==ep) {
                m = bm & em; *bp = *bp|m;
            } else {
                *bp = *bp|bm; *ep = *ep|em;
                for (p=bp+1;p<ep;p++) *p = 0xff;
            }
        } else {
            if (bp==ep) {
                m = bm & em; *bp = *bp&(0xff^m);
            } else {
                *bp = *bp&(0xff^bm); *ep = *ep|(0xff^em);
                for (p=bp+1;p<ep;p++) *p = 0;
            }
        }
    }
}

/*____________________________________
 |
 | Basic Number/String conversions
 |____________________________________
*/

static char *BaseStringToInt(int base,int *Exp,int *R,char *s) {
    int e,r,cnt;
    r = 0; e = 0;
    cnt = (*s!=0);
    while (cnt) {
        int c;
        c = *s;
        if ((c>='0')&&(c<='9')) {
            c = c-'0';
        } else {
            if ((c>='A')&&(c<='F')) {
                c = 10 + (c-'A');
            } else {
                if ((c>='a')&&(c<='f')) {
                    c = 10 + (c-'a');
                } else {
                    c = -1;
                }
            }
        }
        cnt = ((c>=0)&&(c<base));
        if (cnt) { e++; s++; r=(r*base)+c; cnt = (*s!=0); }
    }
    *Exp = e; *R = r;
    return s;
}
static int to10ExpMinusN(int x,int exp) {
    int mul,div;
    mul = 10; div = 1;
    while ((exp>0)&&((div*mul)<x)) {
    if (exp & 1) div = (div*mul);
        exp = exp/2; mul = mul*mul;
    }
    if (div<=0) { div = (x<0)?(1-x):(x+1); } // Safeguard for outrageous numbers.
    return (exp>0)?0:(x/div);
}
static int to10ExpPlusN(int x,int exp) {
    int mul;
    mul=10;
    while (exp>0) {
        if (exp & 1) x = x*mul;
        exp = exp/2; mul = mul*mul;
    }
    return x;
}
static int to10ExpN(int x,int exp) {
    return (exp>0)?to10ExpPlusN(x,exp):((exp<0)?to10ExpMinusN(x,-exp):x);
}
int TlsStringToInt(char *s) {
    int r;
    int sgn,base,dec,decexp,exp,dummy;
    char *p;
    r = 0; p = s;
    sgn = (0!=0); base=0; dec=0; decexp=0; exp=0;
    while ((*p=='-')||(*p=='+')) {
        sgn = (sgn!=(*p=='-'));
        p++;
    }
    while (*p=='0') p++;
	switch (*p) {
	case 'o':
        p=BaseStringToInt(8,&dummy,&base,p+1);
	break;
	case 'x':
        p=BaseStringToInt(16,&dummy,&base,p+1);
	break;
	case 'b':
        p=BaseStringToInt(2,&dummy,&base,p+1);
	break;
	case '.':
	    p=BaseStringToInt(10,&decexp,&dec,p+1);
	break;
	default:
	    p=BaseStringToInt(10,&dummy,&base,p);
	break;
    }
    if ((*p=='e')||(*p=='E')) {
        exp = TlsStringToInt(p+1);
    }
    if (dec) { dec = to10ExpN(dec,exp-decexp); }
    if (exp) { base = to10ExpN(base,exp); }
    r = dec+base;
    if (sgn) { r=-r; }
    return r;
}

char *TlsStringDuplicate(char *s) {
    int L,aL;
    char *p,*r,*e;
    p = s; while (*p) p++;
    L = 1+(p-s); aL = TlsRoundUp(int,L);
    rnPush(r,aL);
    p = r; e = p+L;
    while (p<e) { *p++ = *s++; }
    return r;
}

#include <List.h>

/*_____________________________________________
 |
 | MemPools
 |_____________________________________________
*/

typedef struct {
    TlsMemPool TlsMemPool;
    BinTree/*<SizedPool.Size>*/ Size;
    int ChunkSize;
    struct MemPools *Pools;
    List/*<MemPoolChunk.n>*/ Free;
} SizedPool;
typedef struct MemPools {
    TlsMemPool TlsMemPool;
    MemStack *Mem;
    BinTree/*<SizedPool.Size>*/ Pools;
    SizedPool *Current;
} MemPools;
typedef union {
    List/*<MemPoolChunk.n>*/ n;
    TlsMemPool *Org;
} MemPoolChunk;

static TlsMemPool *SizedPoolSizeSelect(TlsMemPool *this,int Size) {
    TlsMemPool *r;
    ThisToThat(SizedPool,TlsMemPool);
    if (Size==that->Size.Id) {
        r = this;
    } else {
        r = Call(&(that->Pools->TlsMemPool),SizeSelect,1(Size));
    }
    return r;
}
static void *SizedPoolAlloc(TlsMemPool *this) {
    void *r;
    MemPoolChunk *cnk;
    ThisToThat(SizedPool,TlsMemPool);
    if (that->Free.n!=ListEnd) {
        List *l;
        l = that->Free.n;
        that->Free.n = l->n;
        cnk = CastBack(MemPoolChunk,n,l);
    } else {
        char *n;
        mnPush(that->Pools->Mem,n,that->ChunkSize);
        cnk = (MemPoolChunk *)n;
    }
    cnk->Org = this;
    r = cnk+1;
    return r;
}
static void SizedPoolFree(TlsMemPool *this,void *Data) {
    MemPoolChunk *d0,*d1;
    ThisToThat(SizedPool,TlsMemPool);
    d1 = Data; d0 = d1-1;
    d0->n.n = that->Free.n;
    that->Free.n = &d0->n;
}
static SizedPool *SizedPoolNew(MemPools *Pools,int Size) {
    SizedPool *r;
    static struct TlsMemPool Static = {
        SizedPoolSizeSelect,SizedPoolAlloc,SizedPoolFree
    };
    mPush(Pools->Mem,r);
    r->TlsMemPool.Static = &Static;
    r->Pools = Pools;
    r->Free.n = ListEnd;
    Size = TlsRoundUp(int,Size);
    r->Size.Id = Size;
    r->ChunkSize = Size+TlsAtomSize(MemPoolChunk);
    return r;
}

static TlsMemPool *MemPoolsSizeSelect(TlsMemPool *this,int Size) {
    BinTree *f;
    SizedPool *r;
    ThisToThat(MemPools,TlsMemPool);
    Size = TlsRoundUp(int,Size);
    f = BinTreeSeek(&that->Pools,Size);
    if (f!=BinTreeEnd) {
        r = CastBack(SizedPool,Size,f);
    } else {
    mIn(that->Mem,r=SizedPoolNew(that,Size));
        BinTreeSeekOrInsert(&that->Pools,Size,&r->Size);
    }
    that->Current = r;
    return &r->TlsMemPool;
}
static void *MemPoolsAlloc(TlsMemPool *this) {
    void *r;
    ThisToThat(MemPools,TlsMemPool);
    r = 0;
    if (that->Current) {
        r = Call(&that->Current->TlsMemPool,Alloc,0);
    }
    return r;
}
static void MemPoolsFree(TlsMemPool *this,void *Data) {
    MemPoolChunk *d0,*d1;
    d1 = Data; d0 = d1-1;
    Call(d0->Org,Free,1(Data));
}
TlsMemPool *TlsMemPoolNew(int Growth) {
    MemStack *Mem;
    MemPools *r;
    static struct TlsMemPool Static = {
        MemPoolsSizeSelect,MemPoolsAlloc,MemPoolsFree
    };
    Mem = rFork(Growth);
    rPush(r);
    r->Mem = Mem;
    r->TlsMemPool.Static = &Static;
    r->Current = 0;
    BinTreeInit(&r->Pools);
    return &r->TlsMemPool;
}

/*____________________________________________________________________________
 |
 | Dynamic structures
 |____________________________________________________________________________
*/

/*_________________________________________
 |
 | TlsDyndList
 |_________________________________________
*/

typedef struct TlsDyndListNod TlsDyndListNod;
typedef struct TlsDyndList TlsDyndList;

struct TlsDyndListNod {
    dList/*<TlsDyndListNod.dList>*/ dList;
    void *Val;
};
struct TlsDyndList {
    TlsMemPool *Mem;
    dList/*<TlsDyndListNod.dList>*/ r;
};

int TlsIntdListIsEmpty(TlsDyndList *L) {
    dList *n,*r;
    r = &L->r; n=r->n;
    return (r==n);
}

void *TlsDyndListGetItem(TlsDyndListNod *n) { return n->Val; }

TlsDyndListNod *TlsDyndListNext(TlsDyndList *L,TlsDyndListNod *N) {
    dList *rt,*r;
    TlsDyndListNod *R;
    rt = &L->r; r = N->dList.n;
    if (r==rt) { r=r->n; }
    R = (r==rt)?0:CastBack(TlsDyndListNod,dList,r);
    return R;
}
TlsDyndListNod *TlsDyndListPrevious(TlsDyndList *L,TlsDyndListNod *N) {
    dList *rt,*r;
    TlsDyndListNod *R;
    rt = &L->r; r=N->dList.p;
    if (r==rt) { r=r->p; };
    R = (r==rt)?0:CastBack(TlsDyndListNod,dList,r);
    return R;
}

TlsDyndListNod *TlsDyndListPush(TlsDyndList *L,void *Val) {
    TlsDyndListNod *r;
    dList *n,*rt;
    r = Call(L->Mem,Alloc,0);
    r->Val = Val;
    rt=&L->r; n=&r->dList;
    n->p=rt; n->n=rt->n; n->p->n=n->n->p=n;
    return r;
}
TlsDyndListNod *TlsDyndListQueue(TlsDyndList *L,void *Val) {
    TlsDyndListNod *r;
    dList *n,*rt;
    r = Call(L->Mem,Alloc,0); r->Val = Val;
    rt=&L->r; n=&r->dList;
    n->n=rt; n->p=rt->p; n->p->n=n->n->p=n;
    return r;
}
TlsDyndListNod *TlsDyndListChainBefore(TlsDyndList *L,TlsDyndListNod *O,void *Item) {
    TlsDyndListNod *r;
    dList *n,*o;
    r = Call(L->Mem,Alloc,0); 
    r->Val = Item; 
    n = &r->dList; o = &O->dList;
    n->n=o; n->p=o->p; n->n->p=n->p->n=n;
    return r;
}
TlsDyndListNod *TlsDyndListChainAfter(TlsDyndList *L,TlsDyndListNod *O,void *Item) {
    TlsDyndListNod *r;
    dList *n,*o;
    r = Call(L->Mem,Alloc,0);
    r->Val = Item;
    n = &r->dList; o = &O->dList;
    n->n=o->n; n->p=o; n->n->p=n->p->n=n;
    return r;
}
void *TlsDyndListRemove(TlsDyndList *L,TlsDyndListNod *N) {
    void *r;
    dList *p,*n;
    r=N->Val;
    n = &N->dList; p=n->p;
    p->n=n->n; p->n->p=p; n->p=n->n=n;
    Call(L->Mem,Free,1(N));
    return r;
}
void *TlsDyndListPop(TlsDyndList *L) {
    void *r;
    dList *rt,*n;
    rt=&L->r; n=rt->n;
    if (rt!=n) {
        TlsDyndListNod *N;
        N = CastBack(TlsDyndListNod,dList,n);
        r = TlsDyndListRemove(L,N);
    } else {
        r = 0;
    }
    return r; 
}
void *TlsDyndListUnqueue(TlsDyndList *L) {
    void *r;
    dList *rt,*p;
    rt = &L->r; p=rt->p;
    if (rt!=p) {
        TlsDyndListNod *P;
        P = CastBack(TlsDyndListNod,dList,p);
        r = TlsDyndListRemove(L,P);
    } else {
        r = 0;
    }
    return r;
}
void TlsDyndListFree(TlsDyndList *L) {
    TlsMemPool *Mem;
    dList *rt,*n,*f;
    rt = &L->r; n=rt->n;
    while (n!=rt) {
        f=n; n=n->n;
        Call(L->Mem,Free,1(f));
    }
    Mem = Call(L->Mem,SizeSelect,1(sizeof(TlsDyndList)));
    Call(Mem,Free,1(L));
}
TlsDyndList *TlsDyndListAlloc(TlsMemPool *Mem) {
    TlsDyndList *r;
    dList *rt;
    Mem=Call(Mem,SizeSelect,1(sizeof(TlsDyndList)));
    r = Call(Mem,Alloc,0);
    r->Mem = Call(Mem,SizeSelect,1(sizeof(TlsDyndListNod)));
    rt = &r->r;
    rt->n=rt->p=rt;
    return r;
}
void *TlsDyndListForEach(TlsDyndList *L,int (*fnd)(void *item,void *clos),void *Clos) {
    void *r;
    int f;
    TlsDyndListNod *p;
    dList *rt,*n;
    r=0; f=(0!=0);
    rt=&L->r; n=rt->n;
    while ((n!=rt)&&(!f)) {
        p=CastBack(TlsDyndListNod,dList,n);
        f=fnd(p->Val,Clos);
        n=n->n;
    }
    if (f) { r=p->Val; }
    return r;
}
void *TlsDynRvsdListForEach(TlsDyndList *L,int (*fnd)(void *item,void *clos),void *Clos) {
    void *r;
    int f;
    TlsDyndListNod *n;
    dList *rt,*p;
    r=0; f=(0!=0);
    rt=&L->r; p=rt->p;
    while ((p!=rt)&&(!f)) {
        n=CastBack(TlsDyndListNod,dList,p);
        f=fnd(n->Val,Clos);
        p=p->p;
    }
    if (f) { r=n->Val; }
    return r;
}

/*____________________________________________________________________________
 |
 | TlsStringVal
 |____________________________________________________________________________
*/

static TlsCharString AsciiStringNullVal(TlsStringVal *this) {
    static unsigned char Blnk[]="";
    static TlsCharString R = {Blnk+0,Blnk+0};
    return R;
}
static struct TlsStringVal StringValNullStatic = {AsciiStringNullVal};
TlsStringVal TlsStringValNull = {&StringValNullStatic};


