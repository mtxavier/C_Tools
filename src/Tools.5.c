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

#include <Patricia.h>

/*___________________________________________________
 |
 | Lexic
 |___________________________________________________
*/

static char *Empty="";
static char *GetLabelNull(TlsLexic *this,char *Label) { return Empty; }
static char *CheckLabelNull(TlsLexic *this,char *Label) { return (*Label)?0:Empty; }
static struct TlsLexic LexicNullStatic = {GetLabelNull,CheckLabelNull};
TlsLexic TlsLexicNull= {&LexicNullStatic};


typedef struct {
	TlsLexic TlsLexic;
	MemStack *Nods,*Keys;
	PatriciaNod Root;
} Lexic;
static char *LexicGetLabel(TlsLexic *this,char *Label) {
	PatriciaNod *r;
	char *s;
	ThisToThat(Lexic,TlsLexic);
	s = Empty;
	if (*Label) {
	r = PatriciaSeek(&that->Root,Label);
	if (r==&that->Root) {
		char *e,*nKey;
        e = Label; while (*e) {e++;}
		e++;
		mnPush(that->Keys,nKey,(e-Label));
		mPush(that->Nods,r);
		PatriciaSeekOrInsert(&that->Root,nKey,r);
	} }
	s = r->Key;
	return s;
}
static char *LexicCheckLabel(TlsLexic *this,char *Label) {
	PatriciaNod *r;
	char *s;
	ThisToThat(Lexic,TlsLexic);
	s = Empty;
	if (*Label) {
	    r = PatriciaSeek(&that->Root,Label);
	    s = (r == &that->Root)?0:r->Key;
	}
	return s;
}
TlsLexic *TlsLexicNew(void) {
	Lexic *r;
	MemStack *Mem;
	static struct TlsLexic Static = {LexicGetLabel,LexicCheckLabel};
	Mem = rFork(2048);
	mPush(Mem,r);
	r->TlsLexic.Static = &Static;
	r->Nods = Mem;
	r->Keys = MemStackFork(Mem,2048);
	PatriciaInit(&r->Root);
	return &r->TlsLexic;
}

/*_______________________________
 |
 |
 |_______________________________
*/

typedef struct {
	Lexic Lexic;
	TlsLexic *Root;
} Slang;
static char *SlangGetLabel(TlsLexic *this,char *Label) { char *r;
	ThisToThat(Slang,Lexic.TlsLexic);
    r = Call(that->Root,CheckLabel,1(Label));
	if (!r) { r = LexicGetLabel(this,Label); }
	return r;
}
static char *SlangCheckLabel(TlsLexic *this,char *Label) {
	char *r;
	ThisToThat(Slang,Lexic.TlsLexic);
	r = Call(that->Root,CheckLabel,1(Label));
	if (!r) { r = LexicCheckLabel(this,Label); }
	return r;
}
TlsLexic *TlsSlangNew(TlsLexic *Base) {
	Slang *r;
	MemStack *Mem;
	struct TlsLexic Static = {SlangGetLabel,SlangCheckLabel};
	Mem = rFork(2048);
	mPush(Mem,r);
	r->Lexic.TlsLexic.Static = &Static;
	r->Lexic.Nods = Mem;
	r->Lexic.Keys = MemStackFork(Mem,2048);
	PatriciaInit(&r->Lexic.Root);
	r->Root = Base;
	return &r->Lexic.TlsLexic;
}

/*_______________________________________________________
 |
 | MemSpace
 |_______________________________________________________
*/

static void *MemSpaceNullGetOrAdd(Tls1dMemSpace *this,int idx) { return 0; }
static void *MemSpaceNullGet(Tls1dMemSpace *this,int idx) { return 0;}
static TlsRange *MemSpaceNullScope(Tls1dMemSpace *this,TlsRange *rScope) { rScope->b = rScope->e =0; return rScope; }
static void MemSpaceNullForEach(Tls1dMemSpace *this,TlsRange *s,int (*f)(int i,void *d,void *C),void *clos) {}
static void MemSpaceNullRelease(Tls1dMemSpace *this,TlsRange *range) {}
static struct Tls1dMemSpace MemSpace1dNullStatic = {
	MemSpaceNullGetOrAdd,MemSpaceNullGet,MemSpaceNullScope,MemSpaceNullForEach,MemSpaceNullRelease
};
Tls1dMemSpace Tls1dMemSpaceNull = {&MemSpace1dNullStatic};

/*----------------------------*/

typedef struct Mem1dNod {
	union Mem1dElt {
		struct Mem1dNod *Nod;
		void *Leaf;
	} Data[0X20];
	unsigned int Used;
} Mem1dNod;
typedef struct {
	Tls1dMemSpace Tls1dMemSpace;
	TlsMemPool *Leaf,*Nod;
	int Shft;
	TlsRange Range;
	Mem1dNod *Root; 
} MemSpace1d;

static Mem1dNod *Mem1dNodAlloc(MemSpace1d *t,int shft) {
	Mem1dNod *r;
	union Mem1dElt *p,*e;
	r = Call(t->Nod,Alloc,0);
    p = r->Data; e = p+32;
	if (shft>=5) {
	    while (p<e) { p->Nod = 0; p++; }
	} else {
        while (p<e) { p->Leaf = 0; p++; }
	}
	r->Used = 0;
	return r;
}

static void *rSpace1dGet(int shft,Mem1dNod *d,int idx) {
	void *r;
	if (shft<5) {
		r = d->Data[idx&0x1f].Leaf;
	} else {
        Mem1dNod *d0;
		d0 = d->Data[(idx>>shft)&0x1f].Nod;
		if (d0) {
		    r = rSpace1dGet(shft-5,d0,idx);
		} else {
			r = 0;
		}
	}
	return r;
}
static void *rSpace1dGetOrAdd(MemSpace1d *t,int shft,Mem1dNod *d,int idx) {
    void *r;
	int idx0;
	if (shft<5) {
		idx0 = idx&0x1f;
		r = d->Data[idx0].Leaf;
		if (!r) {
			r = d->Data[idx0].Leaf = Call(t->Leaf,Alloc,0);
			d->Used = d->Used | (1<<idx0);
		}
	} else {
		Mem1dNod *d0;
		idx0 = (idx>>shft)&0x1f;
        d0 = d->Data[idx0].Nod;
        if (!d0) {
			d0 = d->Data[idx0].Nod = Mem1dNodAlloc(t,shft-5);
			d->Used = d->Used | (1<<idx0);
		}
		r = rSpace1dGetOrAdd(t,shft-5,d0,idx);
	}
	return r;
}
static void Space1dResize(MemSpace1d *t,int b,int e){
	unsigned int shft,sz,msk0;
	int ib,ie;
	Mem1dNod *r;
	Mem1dNod *n0,*n1,*n;
	shft = t->Shft; 
	msk0 = (0x1<<shft)-1; 
	ib = b>>shft; ie = (e+msk0)>>shft; 
	sz = ((e-b)+msk0)>>shft;
	n = t->Root;
	while (sz>0x20) {
		shft+=5;
		n0 = n;
		n = Mem1dNodAlloc(t,shft);
		if (((ib&0x1f)>=(ie&0x1f))&&(ie&0x1f)) {
			union Mem1dElt *p,*q,*ep;
			unsigned int msk,u0,u1;
			n1 = Mem1dNodAlloc(t,shft-5);
			msk = 1;
			q = n1->Data; p = n0->Data; ep = p+(ie&0x1f);
			while (p<ep) { *q = *p; p->Leaf=0; q++; p++; msk = msk|(msk<<1); }
			n0->Used = (n0->Used)&(~msk);
			n1->Used = msk;
		} else {
			n1 = 0;
		}
		ib = ib>>5; ie = (ie+0x1f)>>5;
		n->Data[ib&0x1f].Nod = n0;  n->Used = 0x1<<(ib&0x1f);
		if (n1) { n->Data[(ib+1)&0x1f].Nod = n1; n->Used = n->Used | (1<<((ib+1)&0x1f)); }
		sz = sz>>5;
	}
	t->Root = n;
	t->Shft = shft;
	t->Range.b=ib<<shft; t->Range.e=ie<<shft;
}

static int rSpace1dCut(MemSpace1d *t,int shft,Mem1dNod *n,int bc,int ec) {
	int b,e,b1,b0,e0,msk,d,clr;
	union Mem1dElt *p,*ep;
	if (!shft) {
		b0 = (bc)&0x1f;
		p = n->Data+b0; ep = p+(ec-bc);
		msk = 1<<b0;
		while (p<ep) { 
			if (p->Leaf) {
				Call(t->Leaf,Free,1(p->Leaf)); 
				p->Leaf=0; 
			} 
			p++; msk = msk|(msk<<1);
		}
		n->Used = n->Used & (~msk);
	} else {
		d = 1<<shft; msk = d-1;
	    b0 = bc>>shft;
		b1 = (b0+1)<<shft; b = bc;
		p = n->Data+(b0&0x1f);
		msk = 1<<(b0&0x1f); clr = 0;
		while (b<ec) {
			if (p->Nod) {
			    e = (b1<ec)?b1:ec;
                if (rSpace1dCut(t,shft-5,p->Nod,b,e)) {
                    clr = clr|msk;
				    Call(t->Nod,Free,1(p->Nod));
				    p->Nod = 0;
			    }
			}
			p++; b = b1; b1 += d; msk = msk<<1;
		}
		n->Used = n->Used & (~clr);
	}
	return (n->Used==0);
}
static int Space1dFirstUsed(unsigned int u0) {
	int bv,ev,mv;
	bv = 0; ev = 0x1f;
	while ((bv+1)<ev) {
	    mv = (bv+ev)>>1;
		if (u0==(u0&((-1)<<mv))) { bv = mv; } else { ev = mv; }
	}
	return (u0==(u0&((-1)<<ev)))?ev:bv;
}
static int Space1dLastUsed(unsigned int u0) {
	int bv,ev,mv;
	bv = 0; ev = 0x1f;
	while ((bv+1)<ev) {
		mv = (bv+ev)>>1;
        if (u0==(u0&(~((-1)<<mv)))) { ev=mv; } else { bv=mv; }
	}
	return (u0==(u0&(~((-1)<<bv))))?bv:ev;
}
static void MemSpace1dShrink(MemSpace1d *t) {
	int shft;
	shft = t->Shft;
	if (shft>0) {
        TlsRange s,l,a;
		unsigned int u,u0,u1;
		u = t->Root->Used;
		if (!u) {
            t->Range.b = t->Range.e = 0;
		    t->Shft = 0;
		} else {
		s.b = t->Range.b>>shft;
		s.e = t->Range.e>>shft;
		l.b = s.b&0x1f; l.e = s.e&0x1f;
		u0 = u>>l.b; u1 = u<<(0x20-l.b); u = (u0|u1) & 0xffffffff;
        a.b = Space1dFirstUsed(u);
		a.e = Space1dLastUsed(u);
        if (a.e<=a.b+1) {
			int single,bo;
			bo = 0;
			single = (a.b==a.e);
			if (!single) {
				Mem1dNod *b,*e;
				b = t->Root->Data[(l.b+a.b)&0x1f].Nod;
				e = t->Root->Data[(l.b+a.e)&0x1f].Nod;
                if (b->Used) { a.b = Space1dFirstUsed(b->Used); } else { a.b = 0x20; }
				if (e->Used) { a.e = Space1dLastUsed(e->Used); } else { a.e = 0; }
				single = (a.e<=a.b);
				if (single) {
					union Mem1dElt *p,*ep,*q;
                    bo = a.b;
					p = e->Data; ep = p+a.e; q = b->Data;
					while (p<ep) *q++=*p++;
					b->Used = b->Used | e->Used;
					Call(t->Nod,Free,1(e));
					t->Root->Data[(l.b+a.e)&0x1f].Nod = 0; // technically unecessary
				}
			}
			if (single) {
				int base;
				{
				    Mem1dNod *b;
				    b = t->Root->Data[l.b].Nod;
				    Call(t->Nod,Free,1(t->Root));
                    t->Root = b;
				}
				base = s.b&(-0x20);
				shft -= 5;
                t->Range.b = (((base+l.b)<<5)+bo)<<shft;
				t->Range.e = (((base+l.b+1)<<5)+bo)<<shft;
				t->Shft = shft;
				MemSpace1dShrink(t);
			}
		}
	    }
	}
}

static void *MemSpace1dGetOrAdd(Tls1dMemSpace *this,int idx) {
	TlsRange R;
	ThisToThat(MemSpace1d,Tls1dMemSpace);
	R = that->Range;
	if (R.b==R.e) {
		that->Range.b = idx;
		that->Range.e = idx+1;
	} else {
	    if ((R.b>idx)||(R.e<=idx)) {
		    R.b = (idx<R.b)?idx:R.b;
		    R.e = (idx>=R.e)?idx+1:R.e;
		    Space1dResize(that,R.b,R.e);
	    }
	}
	return rSpace1dGetOrAdd(that,that->Shft,that->Root,idx);
}
static void *MemSpace1dGet(Tls1dMemSpace *this,int idx) {
	void *r;
	ThisToThat(MemSpace1d,Tls1dMemSpace);
	if ((idx<that->Range.b)||(idx>=that->Range.e)) {
		r = 0;
	} else {
        r = rSpace1dGet(that->Shft,that->Root,idx);
	}
	return r;
}
static void MemSpace1dRelease(Tls1dMemSpace *this,TlsRange *range) {
	TlsRange R;
	ThisToThat(MemSpace1d,Tls1dMemSpace);
	TlsRangeClip(R,that->Range,*range);
	if (R.e>R.b) {
		int ie,ib,shft;
		ie = R.e; shft = that->Shft;
		ib = R.b>>shft;
		while (shft) { ie = (ie+0x1f)>>5; shft-=5; }
		if (((ib&0x1f)>=(ie&0x1f))&&(ie&0x1f)) {
			int cut;
			cut = ((ib>>5)+1)<<(shft+5);
			rSpace1dCut(that,that->Shft,that->Root,R.b,cut);
			rSpace1dCut(that,that->Shft,that->Root,cut,R.e);
		} else {
            rSpace1dCut(that,that->Shft,that->Root,R.b,R.e);
		}
		if (!that->Root->Used) { 
			that->Shft=0; that->Range.b=that->Range.e=0; 
		} else {
			MemSpace1dShrink(that);
		}
	}
}
static TlsRange *MemSpace1dScope(Tls1dMemSpace *this,TlsRange *r) {
	ThisToThat(MemSpace1d,Tls1dMemSpace);
	*r = that->Range;
	return r;
}
static int rMemSpace1dForEach(Mem1dNod *n,int shft,int b,int e,int (*fn)(int idx,void *Data,void *Clos),void *clos) {
	int i,fnd;
	fnd = (0!=0);
	i = b;
	if (!shft) {
		while ((i<e)&&(!fnd)) {
			if (n->Data[i&0x1f].Nod) { fnd = fn(i,n->Data[i&0x1f].Nod,clos); }
			i++;
		}
	} else {
        int inc,bp,ebp;
		union Mem1dElt *p;
		inc = 0x1<<shft;
		bp = b;
		ebp = (bp&(-inc))+inc;
		p = n->Data+((b>>shft)&0x1f);
		while ((bp<e)&&(!fnd)) {
			if (p->Nod) { fnd = rMemSpace1dForEach(p->Nod,shft-5,bp,ebp,fn,clos);}
			p++; bp = ebp; ebp+=inc; if (ebp>e) ebp = e;
		}
	}
	return fnd;
}
static void MemSpace1dForEach(Tls1dMemSpace *this,TlsRange *Scope,int (*fn)(int idx,void *Data,void *Closure),void *closure) {
	int fnd,cut;
	TlsRange clp;
	ThisToThat(MemSpace1d,Tls1dMemSpace);
    TlsRangeClip(clp,*Scope,that->Range);
	cut = 0x20<<that->Shft;
	if ((cut>=clp.b)&&(cut<clp.e)) {
		fnd = rMemSpace1dForEach(that->Root,that->Shft,clp.b,cut,fn,closure);
		if (!fnd) {
			rMemSpace1dForEach(that->Root,that->Shft,cut,clp.e,fn,closure);
		}
	} else {
		rMemSpace1dForEach(that->Root,that->Shft,clp.b,clp.e,fn,closure);
	}
}
static Tls1dMemSpace *tls1dMemSpaceMount(MemSpace1d *r,int LeafSz,TlsMemPool *pool) {
	TlsMemPool *LeafPool;
	static struct Tls1dMemSpace Static = {
		MemSpace1dGetOrAdd,MemSpace1dGet,MemSpace1dScope,MemSpace1dForEach,MemSpace1dRelease
	};
    r->Tls1dMemSpace.Static = &Static;
	LeafPool = Call(pool,SizeSelect,1(LeafSz));
	r->Nod = Call(pool,SizeSelect,1(sizeof(Mem1dNod)));
	r->Leaf = LeafPool;
	r->Shft = 0;
	r->Range.b = r->Range.e = 0;
	r->Root = Mem1dNodAlloc(r,0);
	return &r->Tls1dMemSpace;
}

Tls1dMemSpace *Tls1dMemSpaceMount(int LeafSz,TlsMemPool *pool) {
	MemSpace1d *r;
	rPush(r);
	return tls1dMemSpaceMount(r,LeafSz,pool);
}
Tls1dMemSpace *Tls1dMemSpaceNew(int LeafSz) {
	MemSpace1d *r;
	rPush(r);
	return tls1dMemSpaceMount(r,LeafSz,TlsMemPoolNew(2048)); 
}

/*____________________________________________________________________
 |
 |
 |____________________________________________________________________
*/

typedef struct {
	Tls1dIntSpace Tls1dIntSpace;
	int Default;
	Tls1dMemSpace *Data;
} IntSpace1d;
static void IS1dSet(Tls1dIntSpace *this,int idx,int v) {
	int *d;
	int i;
	ThisToThat(IntSpace1d,Tls1dIntSpace);
    i = idx>>5;
	d = Call(that->Data,Get,1(i));
	if (!d) {
		int *p,*e;
		p = d = Call(that->Data,GetOrAdd,1(i));
        e = d+0x20;
		while (p<e) { *p++ = that->Default; }
	}
	d[idx&0x1f] = v;
}
static int IS1dGet(Tls1dIntSpace *this,int idx) {
	int i,r;
    int *d;
	ThisToThat(IntSpace1d,Tls1dIntSpace);
	i = idx>>5; r = that->Default;
	d = Call(that->Data,Get,1(i));
    if (d) r = d[idx&0x1f];
	return r;
}
static TlsRange *IS1dScope(Tls1dIntSpace *this,TlsRange *rScope) {
	ThisToThat(IntSpace1d,Tls1dIntSpace);
	Call(that->Data,Scope,1(rScope));
	rScope->b = rScope->b<<5; rScope->e = rScope->e<<5;
	return rScope;
}
struct stIS1dForEach {
	int (*f)(int idx,int *b,int *e,void *C);
	void *clos;
	int fnd;
};
static int fnIS1dForEach(int idx,void *Data,void *C) {
	struct stIS1dForEach *c;
	int *data;
	c = C; data = Data;
	c->fnd = c->f(idx<<5,data,data+0x20,c->clos);
	return c->fnd;
}
static void IS1dForEach(Tls1dIntSpace *this,TlsRange *Scope,int (*fn)(int idx,int *b,int *e,void *Clos),void *clos) {
	TlsRange r;
	int *d,*p,*ep,fnd;
	ThisToThat(IntSpace1d,Tls1dIntSpace);
	r.b = Scope->b>>5;
	r.e = Scope->e>>5;
	if (r.b==r.e) {
		d = Call(that->Data,Get,1(r.b));
		if (d) { fn(Scope->b,d+(Scope->b&0x1f),d+(Scope->e&0x1f),clos); }
	} else {
		int b,e;
	    fnd = (0!=0);
		b = Scope->b;
		e = (r.b+1)<<5;
		d = Call(that->Data,Get,1(r.b));
		if (d) { fnd = fn(Scope->b,d+(Scope->b&0x1f),d+0x20,clos); }
		if (!fnd) {
			struct stIS1dForEach C;
			r.b++;
			C.f = fn; C.clos = clos; C.fnd =fnd;
			Call(that->Data,ForEach,3(&r,fnIS1dForEach,&C));
			fnd = C.fnd;
		}
		if (!fnd) {
            d = Call(that->Data,Get,1(r.e));
			if (d) { fn((Scope->e)&(-0x20),d,d+(Scope->e&0x1f),clos); }
        }
	}
}
static void IS1dRelease(Tls1dIntSpace *this,TlsRange *range) {
	TlsRange r;
	int *d,*p,*ep;
	ThisToThat(IntSpace1d,Tls1dIntSpace);
	r.b = (range->b+0x1f)>>5;
	r.e = (range->e)>>5;
	if (r.e>r.b) { Call(that->Data,Release,1(&r)); }
	r.b = range->b>>5;
	if (r.b==r.e) {
		d = Call(that->Data,Get,1(r.b));
		if (d) {
			p = d+(range->b&0x1f);
			ep = d+(range->e&0x1f);
			while (p<ep) { *p++ = that->Default; }
		}
	} else {
		d = Call(that->Data,Get,1(r.b));
		if (d) {
			p = d+(range->b&0x1f);
			ep = d+0x20;
			while (p<ep) { *p++ = that->Default; }
		}
		d = Call(that->Data,Get,1(r.e));
		if (d) {
			p = d;
			ep = d+(range->e&0x1f);
			while (p<ep) { *p++ = that->Default; }
		}
	}
}

Tls1dIntSpace *Tls1dIntSpaceNew(int Default) {
	IntSpace1d *r;
	static struct Tls1dIntSpace Static = {
		IS1dSet,IS1dGet,IS1dScope,IS1dForEach,IS1dRelease
	};
	rPush(r);
	r->Tls1dIntSpace.Static = &Static;
	r->Default = Default;
	r->Data = Tls1dMemSpaceNew(0x20*sizeof(int));
	return &r->Tls1dIntSpace;
}

/*___________________________________________________
 |
 | 2d Map
 |___________________________________________________
*/

typedef struct Nod2d {
	union Nod2dElt {
		struct Nod2d *Nod;
		void *Leaf;
	} Data[0x40];
	unsigned char Used[8];
} Nod2d;
typedef struct {
	Tls2dMemSpace Tls2dMemSpace;
	TlsMemPool *Nod,*Leaf;
	Nod2d *Root;
	int Shft;
	Tls2dArea Diam;
} Space2d;

/*-------------*/

static Nod2d *Nod2dAlloc(Space2d *t,int shft) {
    Nod2d *r;
	union Nod2dElt *p,*e;
	unsigned char *u,*ue;
	r = Call(t->Nod,Alloc,0);
	p = r->Data; e = p+0x40;
    if (shft>=3) {
		while (p<e) { p->Nod = 0; p++; }
	} else {
		while (p<e) { p->Leaf = 0; p++; }
	}
    u = r->Used; ue = u+8; while (u<ue) {*u++=0;}
    return r;	
}

#define Nod2dNum(x,y,shft) (((x>>shft)&7)+((y>>(shft-3))&0x38))
static void *Nod2dGet(Nod2d *n,int x,int y,int shft) {
	void *r;
	r = (void *)0;
	if (shft<3) {
		r = n->Data[(x&7)+((y&7)<<3)].Leaf;
	} else {
        Nod2d *nn;
		nn = n->Data[Nod2dNum(x,y,shft)].Nod;
		if (nn) {
			r = Nod2dGet(nn,x,y,shft-3);
		}
	}
	return r;
}
static void *Nod2dAdd(Space2d *t,Nod2d *n,int x,int y,int shft) {
	void *r;
	int ax,ay;
	if (shft<3) {
        r = Call(t->Leaf,Alloc,0);
		n->Data[(x&0x7)+((y&0x7)<<3)].Leaf = r;
		ax = x&0x7; ay = y&0x7;
		n->Used[ay] |= (0x1<<ax);
	} else {
		Nod2d *nn;
		int nodnum;
        nodnum = Nod2dNum(x,y,shft);
		nn = n->Data[nodnum].Nod;
		if (!nn) {
            nn = Nod2dAlloc(t,shft-3);
			n->Data[nodnum].Nod = nn;
		    ax = (x>>shft)&0x7; ay = (y>>shft)&0x7;
			n->Used[ay] |= (1<<ax);
		}
		r = Nod2dAdd(t,nn,x,y,shft-3);
	}
	return r;
}
static void Space2dGrow(Space2d *t,int x,int y) {
	int bx,ex,by,ey;
	int shft,msk;
	Nod2d *r;
	shft =t->Shft; msk = (1<<(shft+3))-1;
	bx = (x<t->Diam.w.b)?x:t->Diam.w.b;
	ex = (x>t->Diam.w.e)?x:t->Diam.w.e;
	by = (y<t->Diam.l.b)?y:t->Diam.l.b;
	ey = (y>t->Diam.l.e)?y:t->Diam.l.e;
	bx = bx>>shft; ex = (ex+msk)>>shft;
	by = by>>shft; ey = (ey+msk)>>shft;
	r = t->Root;
	while (((ex-bx)>8)||((ey-by)>8)) {
		Nod2d *n[4];
		union Nod2dElt *p,*dp,*le,*lee,*e,*de,*q,*q1;
		unsigned char *pu,*dpu,*qu0,*qu1,usd,msk;
		int bx0,by0,bx1,by1,dx,dy;
		bx0 = (bx>>3)&7; bx1 = (bx0+1)&7;
		by0 = (by&0x38); by1 = (by0+8)&0x38;
		n[0]=n[1]=n[2]=n[3]=r;
        r = Nod2dAlloc(t,shft+3);
		r->Data[by0+bx0].Nod=n[0];
		if (bx&7) { r->Data[by0+bx1].Nod = n[1] = Nod2dAlloc(t,shft); }
		if (by&7) { r->Data[by1+bx0].Nod = n[2] = n[3] = Nod2dAlloc(t,shft); 
		    if (bx&7) { r->Data[by1+bx1].Nod = n[3] = Nod2dAlloc(t,shft); } 
		}
		msk = 0xff<<(bx&7);
		p = dp = n[0]->Data; de = p+0x40;
		dpu = pu = n[0]->Used;
		dy = 2; lee = p+((by&7)<<3);
		while (p<de) {
			while (p<lee) {
			    dx = 1; le = p+0x8; e = p+(bx&0x7);
			    while (p<le) {
			        q = (n[dx+dy]->Data)+(p-dp);
			        if (q!=p) {
			            while (p<e) { *q++=*p; p->Nod=0; p++; }
			        } else {
				        p = e;
			        }
			        e = le; dx = 0;
			    }
				qu0 = (n[0+dy]->Used)+(pu-dpu);
				qu1 = (n[1+dy]->Used)+(pu-dpu);
				usd = *pu; 
				// order of affectation to pu, qu0 and qu1 is important here since they might point to the same object.
				*pu++=0; *qu1 = usd&(~msk); *qu0 = usd&msk;
			}
			dy = 0; lee = de;
		}
		bx = bx>>3; by=by>>3;
		ex = (ex+7)>>3; ey=(ey+7)>>3;
		shft+=3;
	}
	t->Root = r;
	t->Shft = shft;
	t->Diam.w.b = bx<<shft; t->Diam.w.e = ex<<shft;
	t->Diam.l.b = by<<shft; t->Diam.l.e = ey<<shft;
}

#define IsIn2dArea(x,y,A) ((x>=(A).w.b)&&(x<(A).w.e)&&(y>=(A).l.b)&&(y<(A).l.e))

static Tls2dArea *Space2dClip(Tls2dArea *r,Space2d *Space,Tls2dArea *Are) {
	int bx,by,ex,ey,cx,cy,shft;
	int cbx,cby,cex,cey;
	shft = Space->Shft;
	bx = Space->Diam.w.b; ex = Space->Diam.w.e;
	by = Space->Diam.l.b; ex = Space->Diam.l.e;
	cx = (bx+(8<<shft))&((-8)<<shft);
	cy = (by+(8<<shft))&((-8)<<shft);
	cbx = (Are->w.b>bx)?Are->w.b:bx;
	cex = (Are->w.e<ex)?Are->w.e:ex;
    cby = (Are->l.b>by)?Are->l.b:by;
	cey = (Are->l.e<ey)?Are->l.e:ey;
	if ((cbx>=cex)||(cby>=cey)) { cbx=cex=bx; cby=cey=by; }
	if (cx>cex) cx=cex;
	if (cy>cey) cy=cey;
	r[0].w.b = r[2].w.b = cbx; r[0].w.e = r[2].w.e = cx;
	r[0].l.b = r[1].l.b = cby; r[0].l.e = r[1].l.e = cy;
	r[1].w.b = r[3].w.b = cx; r[1].w.e = r[3].w.e = cex;
	r[2].l.b = r[3].l.b = cy; r[2].l.e = r[3].l.e = cey;
	return r;
}

static int Nod2dForEach(Nod2d *r,int shft,Tls2dArea *Area,int (*fn)(int x,int y,void *Data,void *clos),void *clos) {
	Tls2dArea clip;
	int fnd,bx,by,ex,ey,x,y,dv;
	union Nod2dElt *p,*np;
	fnd = (0!=0);
	bx = Area->w.b; ex = Area->w.e;
	by = Area->l.b; ey = Area->l.e;
	p = r->Data+((bx>>shft)&0x7)+(((by>>shft)&0x7)<<3);
	if (shft<3) {
		y = by;
		while ((y<ey)&&(!fnd)) {
			np = p+8; x = bx;
			while ((x<ex)&&(!fnd)) {
				fnd = fn(x,y,p->Leaf,clos);
				p++; x++;
			}
            p=np; y++;
		}
	} else {
		dv = (1<<shft);
		x = (bx&((-1)<<shft))+dv;  if (x>ex) x=ex;
		y = (by&((-1)<<shft))+dv;  if (y>ey) y=ey;
		clip.l.b = by; clip.l.e = y;
		while ((clip.l.b<clip.l.e)&&(!fnd)) {
			np = p+8;
			clip.w.b = bx; clip.w.e = x;
			while ((clip.w.b<clip.w.e)&&(!fnd)) {
				if (p->Nod) { fnd = Nod2dForEach(p->Nod,shft-3,&clip,fn,clos); }
				clip.w.b = clip.w.e; 
				clip.w.e+=dv; if (clip.w.e>ex) clip.w.e = ex;
				p++;
			}
			clip.l.b = clip.l.e;
			clip.l.e += dv; if (clip.l.e>ey) clip.l.e = ey;
			p=np;
		}
	}
	return fnd;
}

static int Nod2dReleased(Space2d *th,Nod2d *nod,int shft,Tls2dArea *Area) {
	int bx,ex,by,ey,empty;
	union Nod2dElt *p,*np,*ep,*eep;
	unsigned char *pu,u,*eu,umsk,umsk0;
	bx = Area->w.b; ex = Area->w.e;
	by = Area->l.b; ey = Area->l.e;
	if (shft<3) {
		p = nod->Data+(bx&0x7)+((by&0x7)<<3);
		ep = nod->Data+(ex&0x7)+((by&0x7)<<3);
		eep = nod->Data+(bx&0x7)+((ey&0x7)<<3);
		umsk0 = 1<<(bx&0x7);
		pu = nod->Used+(by&0x7);
        while (p<eep) {
			np = p+8;
			u = *pu; umsk = umsk0;
			while (p<ep) {
				if (p->Leaf) {
					Call(th->Leaf,Free,1(p->Leaf));
					p->Leaf = 0; u = u & (~umsk);
				}
				p++; umsk = umsk<<1;
			}
			*pu++ = u; p = np; ep+=8;
		}
	} else {
	    Tls2dArea clip;
		int dv,elx;
		dv = 1<<shft; 
		elx = (bx&((-1)<<shft))+dv; if (elx>ex) elx = ex;
        clip.l.b = by; clip.l.e = (by&((-1)<<shft))+dv; if (clip.l.e>ey) clip.l.e = ey;
		p   = nod->Data+((bx>>shft)&0x7)+(((by>>shft)&0x7)<<3);
		umsk0 = 1<<((bx>>shft)&0x7);
		pu = nod->Used+((by>>shft)&0x7);
		while (clip.l.b<clip.l.e) {
			np = p+8; u = *pu; umsk = umsk0;
		    clip.w.b = bx; clip.w.e = elx;
			while (clip.w.b<clip.w.e) {
				if (p->Nod) {
					if (Nod2dReleased(th,p->Nod,shft-3,&clip)) {
						Call(th->Nod,Free,1(p->Nod));
						p->Nod = 0; u = u & (~umsk);
					}
				}
				clip.w.b = clip.w.e; clip.w.e+=dv; if (clip.w.e>ex) clip.w.e = ex;
                p++; umsk = umsk<<1;
			}
			clip.l.b = clip.l.e; clip.l.e+=dv; if (clip.l.e>ey) clip.l.e = ey;
			*pu++ = u; p = np;
		}
	}
    pu = nod->Used; eu = pu+8;
	do { empty = ((*pu)==0); pu++; } while((pu<eu)&&(empty));
	return empty;
}

static void Space2dShrink(Space2d *th) {
	unsigned char *up,*ue,u;
	int fail,x,y;
   	up = th->Root->Used; ue = up+8;
	fail = (0!=0);
	while ((th->Shft>3)&&(!fail)) {
	    x = y = -1;
	    do {
		    if (x<0) y++;
		    u = *up++;
		    fail = (u!=0)&&(x>=0);
		    if ((x<0)&&(u!=0)) {
			    if (u&0xf0) {
				    if (u&0xc0) { x = (u&0x80)?7:6; } else { x = (u&0x20)?5:4; }
			    } else {
				    if (u&0xc) { x = (u&0x8)?3:2; } else { x = (u&0x2)?1:0; }
			    }
			    fail = (u!=(1<<x));
		    }
	    } while ((up<ue)&&(!fail));
		if (!fail) {
            if (x<0) {
				th->Shft = 0;
				th->Diam.w.b = th->Diam.w.e = th->Diam.l.b = th->Diam.l.e = 0;
			} else {
				Nod2d *r;
				Tls2dArea Diam;
				int shft,dv;
				shft = th->Shft; dv = 1<<shft;
				Diam.w.b = ((th->Diam.w.b)&((-8)<<shft))+(x<<shft);
				if (Diam.w.b<th->Diam.w.b) Diam.w.b+=(dv<<3);
				Diam.w.e = Diam.w.b+dv;
				Diam.l.b = ((th->Diam.l.b)&((-8)<<shft))+(y<<shft);
				if (Diam.l.b<th->Diam.l.b) Diam.l.b+=(dv<<3);
				Diam.l.e = Diam.l.b+dv;
				r = th->Root->Data[x+(y<<3)].Nod;
				Call(th->Nod,Free,1(th->Root));
				th->Root=r;
				th->Diam = Diam;
				th->Shft-=3;
			}
		}
	}
}

/*----------*/

static void *Space2dGetOrAdd(Tls2dMemSpace *this,int x,int y) {
	void *r;
	ThisToThat(Space2d,Tls2dMemSpace);
	if (IsIn2dArea(x,y,that->Diam)) {
		r = Nod2dGet(that->Root,x,y,that->Shft);
		if (!r) {
			r = Nod2dAdd(that,that->Root,x,y,that->Shft);
		}
	} else {
		Space2dGrow(that,x,y);
		r = Nod2dAdd(that,that->Root,x,y,that->Shft);
	}
	return r;
}
static void *Space2dGet(Tls2dMemSpace *this,int x,int y) {
	void *r;
	ThisToThat(Space2d,Tls2dMemSpace);
	if (IsIn2dArea(x,y,that->Diam)) {
	    r = Nod2dGet(that->Root,x,y,that->Shft);
	} else {
		r = (void *)0;
	}
    return r;
}
#define EmptyArea(a) ((a.w.b>=a.w.e)||(a.l.b>=a.l.e))
static void Space2dForEach(Tls2dMemSpace *this,Tls2dArea *Scope,int (*fn)(int x,int y,void *Data,void *Closure),void *clos) {
    Tls2dArea clip[4];
	int found;
	ThisToThat(Space2d,Tls2dMemSpace);
	if (Scope) {
	    Space2dClip(clip,that,Scope);
	} else {
		Space2dClip(clip,that,&that->Diam);
	}
	found = (0!=0);
	if (!EmptyArea(clip[0])) { found = Nod2dForEach(that->Root,that->Shft,clip+0,fn,clos);}
	if ((!found)&&(!EmptyArea(clip[1]))) { found = Nod2dForEach(that->Root,that->Shft,clip+1,fn,clos); }
	if ((!found)&&(!EmptyArea(clip[2]))) { found = Nod2dForEach(that->Root,that->Shft,clip+2,fn,clos); }
	if ((!found)&&(!EmptyArea(clip[3]))) { found = Nod2dForEach(that->Root,that->Shft,clip+3,fn,clos); }
}
static void Space2dRelease(Tls2dMemSpace *this,Tls2dArea *Scope) {
	Tls2dArea clip[4];
	ThisToThat(Space2d,Tls2dMemSpace);
	if (Scope) {
		Space2dClip(clip,that,Scope);
	} else {
		Space2dClip(clip,that,&that->Diam);
	}
    Nod2dReleased(that,that->Root,that->Shft,clip+0);
    Nod2dReleased(that,that->Root,that->Shft,clip+1);
    Nod2dReleased(that,that->Root,that->Shft,clip+2);
    Nod2dReleased(that,that->Root,that->Shft,clip+3);
	Space2dShrink(that);
}

Tls2dMemSpace *TlsSpace2dNew(int LeafSz) {
	Space2d *r;
	TlsMemPool *Pool;
	static struct Tls2dMemSpace Static = {
		Space2dGetOrAdd,Space2dGet,Space2dForEach,Space2dRelease
	};
	rPush(r);
	r->Tls2dMemSpace.Static = &Static;
	Pool = TlsMemPoolNew(2048);
	r->Nod = Call(Pool,SizeSelect,1(sizeof(Nod2d)));
	r->Leaf = Call(Pool,SizeSelect,1(LeafSz));
	r->Root = Nod2dAlloc(r,0);
	r->Shft = 0;
	r->Diam.w.b = r->Diam.l.b = 0;
	r->Diam.w.e = r->Diam.l.e = 8;
	return &r->Tls2dMemSpace;
}

