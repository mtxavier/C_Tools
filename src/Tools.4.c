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

/*_____________________________________________________________________________
 |
 |_____________________________________________________________________________
*/

/*______________________________
 |
 | Persistent structures
 |______________________________
*/

typedef struct IdxNod IdxNod;
typedef struct {
	TlsMemPool *Pool;
	int PoolChunkSize;
	int EltSize,LeafCntnt; // EltSize
	int LeafSz,LinkSz; // LeafSz == LeafCntnt*EltSize
	int Lvl;
	IdxNod *Root;
} IdxDesc;

struct IdxNod {
	int End;
	union {
		struct { void *b,*e; } Leaf;
		struct { struct IdxNod **b,**e; } Link;
	} Data;
};

#define idxLeafGetLength(l,L,Sz) {\
	l = (L)->Data.Leaf.e-(L)->Data.Leaf.b;\
	if (l<=0) { l+=(Sz); } else { if (l==(Sz)) {l=0;} } \
}

#define idxLinkGetLength(l,L) {\
	if ((L)->Data.Link.b==(L)->Data.Link.e) {\
		l = 0;\
	} else {\
		l = (L)->Data.Link.e[-1]->End;\
	}\
}

#define idxNodGetLength(l,Lvl,L,Sz) {\
	if ((Lvl)>0) {\
		idxLinkGetLength(l,L)\
	} else  {\
		idxLeafGetLength(l,L,Sz)\
	}\
}

#define idxNodIsEmpty(empty,d,N,lvl) {\
	empty = (lvl==0); \
	if (empty) { empty = ((N)->Data.Leaf.b==((N)->Data.Leaf.e+(d)->LeafSz));} \
}

/*______________________________
 |
 | Local structures
 |______________________________
*/

typedef struct {
	TlsCircleBuffer Data;
	int LeafSz;
} LeafNod;

typedef struct {
	int lvl,length;
	IdxDesc *desc;
	IdxNod **b,**e,**eBuff;
} LinkNod;

typedef struct {
	IdxNod **pp,*p;
	int b,e;
} PosPtr;

#define posPtrGet(dst,src,i) { \
	(dst).pp = (src).b+(i); (dst).p = *((dst).pp);\
	(dst).e = (dst).p->End;\
	(dst).b = ((i)==0)?0:(dst).pp[-1]->End;\
}

#define LeafNodGet(dst,src,Ls) {\
	void *buff; buff = (src)+1; \
	dst.LeafSz = Ls;\
	dst.Data.eBuff = dst.Data.bBuff = buff; \
	dst.Data.eBuff += dst.LeafSz;\
	dst.Data.bElt = (src)->Data.Leaf.b;\
	dst.Data.eElt = (src)->Data.Leaf.e;\
}

#define LeafNodUpdate(dst,src) {\
	(dst)->Data.Leaf.b = src.Data.bElt; \
	(dst)->Data.Leaf.e = src.Data.eElt; \
}

#define LinkNodGet(dst,src,Lvl) {\
	dst.lvl = Lvl;\
	{ void *ap; ap = (src)+1; dst.b = ap; }\
	dst.e = (src)->Data.Link.e;\
    dst.eBuff = dst.b+dst.desc->LinkSz;\
	dst.length = (dst.b==dst.e)?0:dst.e[-1]->End;\
}


#define LinkNodUpdate(dst,src) {\
    (dst)->Data.Link.e = src.e;\
	if (src.e!=src.b) {\
		((dst)->Data.Link.e[-1])->End = src.length;\
	}\
}

static IdxDesc *idxDescNew(IdxDesc *r,int EltSz,int LeafCntnt);

static void IdxNodFree(IdxDesc *d,IdxNod *f);
static void IdxLinkFree(IdxDesc *d,int Lvl,IdxNod *f);

static IdxNod *idxLinkSplit(IdxDesc *Default,int *cdrlvl,int *carlvl,IdxNod **car,int idx);
static IdxNod *idxLinkCat(IdxDesc *Default,int *rlvl,int carLvl,IdxNod *car,int cdrLvl,IdxNod *cdr);

/*------------*/

static IdxNod *idxLeafAlloc(IdxDesc *Default,int Size) {
	IdxNod *r;
	int s;
	void *bB;
	r = Call(Default->Pool,Alloc,0);
	r->End = Size;
	bB = r+1;
	s = Size;
	if (Size==0) { 
		s = Default->LeafSz; 
	} else {
		if (Size == Default->LeafSz) { s = 0; }
	}
    r->Data.Leaf.b = bB;
	r->Data.Leaf.e = r->Data.Leaf.b+s;
	return r;
}

/*------------*/

static IdxDesc *idxDescNew(IdxDesc *r,int EltSz,int LeafCntnt) {
	int ChunkSize;
	ChunkSize = (TlsCBf_RequiredMem(LeafCntnt*EltSz)-TlsAtomSize(TlsCircleBuffer))+TlsAtomSize(IdxNod);
	r->Pool = TlsMemPoolNew(16*ChunkSize);
	r->Pool = Call(r->Pool,SizeSelect,1(ChunkSize));
	r->PoolChunkSize = ChunkSize;
	r->Lvl = 0;
	r->EltSize = EltSz;
	r->LeafSz = EltSz*LeafCntnt;
	r->LeafCntnt = LeafCntnt;
	r->LinkSz = (ChunkSize-TlsAtomSize(IdxNod))/TlsAtomSize(IdxNod **);
	r->Root = idxLeafAlloc(r,0);
	return r;
}

/*------------*/

static PosPtr *idxGetPosPtr(PosPtr *r,LinkNod *src,int idx) {
	PosPtr s;
	int min,max,mmax,found;
	min = 0; mmax = max = src->e-src->b;
	found = (0!=0);
	while ((min+1<max)&&(!found)) {
		int m;
		m = (min+max)>>1;
		posPtrGet(s,*src,m);
		found = (idx>=s.b)&&(idx<=s.e);
        if (!found) { if (idx<s.b) { max = m; } else { min = m; } }
	}
	if ((!found)&&(min<mmax)) {
		posPtrGet(s,*src,min);
		found = (idx>=s.b)&&(idx<=s.e);
		if (!found) {
			found = (max<mmax);
			if (found) { posPtrGet(s,*src,max); }
		}
	}
	if (found) {
		*r = s;
	} else {
		r->pp = src->b;
		r->p = *r->pp;
	   	r->b = 0; r->e = 0;
	}
	return r;
}

/*-----------------*/

static IdxNod *idxTreeAlloc(IdxDesc *d,int *lvl,int nb) {
	int lv,lg,ex;
	IdxNod *r;
    IdxNod *treeAlloc(int *length,int lvl,int nb) {
		IdxNod *r;
	    void *data;
		r = Call(d->Pool,Alloc,0);
		data = r+1;
		if (lvl==0) {
			r->Data.Leaf.b = data;
			if (d->LeafSz>nb) {
				if (!nb) {
				    r->Data.Leaf.e = data + d->LeafSz; // Buffer empty
				} else {
				    r->Data.Leaf.e = data + nb;
				}
				*length = nb;
			} else {
				r->Data.Leaf.e = r->Data.Leaf.b; // Buffer Full
				*length = d->LeafSz;
			}
		} else {
			int idx,lg,lv0;
			IdxNod **p,**e,*n;
			r->Data.Link.b = data; lv0 = lvl-1;
			p = data; e = p+d->LinkSz; idx = 0;
			while ((p<e)&&(idx<nb)) {
				n = treeAlloc(&lg,lv0,nb-idx);
				idx += lg; n->End = idx;
                *p++ = n;
			}
			r->Data.Link.e = p;
			*length = idx;
		}
		return r;
	}
	lg = d->LeafSz; lv = 0; ex = d->LinkSz;
	while (lg<nb) { lv++; lg = lg * ex; }
	*lvl = lv;
	r = treeAlloc(&lg,lv,nb);
	r->End = lg;
	return r;
}

static void IdxLinkFree(IdxDesc *d,int Lvl,IdxNod *f) {
	if (Lvl>0) {
		IdxNod **p,**e;
		{void *data; data=f+1; p = data; }
		e = f->Data.Leaf.e;
		while (p<e) { 
			IdxLinkFree(d,Lvl-1,*p);
			p++; 
		}
	}
	Call(d->Pool,Free,1(f));
}

/*___________________________________
 |
 | Get/Set/ForEach 
 |___________________________________
*/

static int idxNodGet(IdxDesc *d,int lvl,IdxNod *N,void *dst,int nb,int idx) {
	int di;
	if (lvl==0) {
		int lg,e;
        LeafNod lf;
		LeafNodGet(lf,N,d->LeafSz);
		idxLeafGetLength(lg,N,d->LeafSz);
        if (lg>=(idx+nb)) { e = idx+nb; } else { e = lg; }
		di = e-idx;
		TlsCBfGet(dst,dst+di,&lf.Data,idx);
	} else {
		int e,i,b;
		LinkNod nd;
		PosPtr pos;
		nd.desc = d;
		LinkNodGet(nd,N,lvl);
        idxGetPosPtr(&pos,&nd,idx);
		b = idx-pos.b;
		i = idx; e = i + nb;
		while ((i<e)&&(pos.pp<nd.e)) {
            di = idxNodGet(d,lvl-1,*pos.pp,dst,e-i,b);
			b = 0; pos.pp++; dst+=di; i+=di;
		}
		di = i-idx;
	}
	return di;
}

   /*------------------*/

static int idxNodSet(IdxDesc *d,int lvl,IdxNod *N,void *src,int nb,int idx) {
	int di;
	if (lvl==0) {
		int lg,e;
        LeafNod lf;
		LeafNodGet(lf,N,d->LeafSz);
		idxLeafGetLength(lg,N,d->LeafSz);
        if (lg>=(idx+nb)) { e = idx+nb; } else { e = lg; }
		di = e-idx;
		TlsCBfOverwrite(&lf.Data,idx,src,src+di);
	} else {
		int e,i,b;
		LinkNod nd;
		PosPtr pos;
		nd.desc = d;
		LinkNodGet(nd,N,lvl);
        idxGetPosPtr(&pos,&nd,idx);
		b = idx-pos.b;
		i = idx; e = i + nb;
		while ((i<e)&&(pos.pp<nd.e)) {
            di = idxNodSet(d,lvl-1,*pos.pp,src,e-i,b);
			b = 0; pos.pp++; src+=di; i+=di;
		}
		di = i-idx;
	}
	return di;
}

   /*------------------*/

static int idxNodForEach(IdxDesc *d,int lvl,IdxNod *N,int *B,int step,int *bRange,int eRange,int Step,TlsIdxListIterator *Itr) {
	int b,found,bR;
	b = *B; bR = *bRange;
	found = (0!=0);
	if (lvl==0) {
		int lg;
        LeafNod lf;
		void *p,*e,*ee,*er;
		LeafNodGet(lf,N,d->LeafSz);
		idxLeafGetLength(lg,N,d->LeafSz);
		p = lf.Data.bElt; e = p+lg; ee = lf.Data.eBuff; er = p+eRange;
		if (e-er>0) { e = er; }
		if (ee-e>0) { ee = e; }
	    p = p + bR;
		while ((p<ee)&&(!found)) {
			found = Call(Itr,Found,2(b,p));
			p += Step; bR += Step; b += step;
		}
		if ((p<e)&&(!found)) {
		    p-=lf.LeafSz; e-=lf.LeafSz;
			while ((p<e)&&(!found)) {
				found = Call(Itr,Found,2(b,p));
				p += Step; bR += Step; b += step;
			}
		}
	} else {
		LinkNod nd;
		PosPtr pos;
		int idx,lR,lg,e;
		nd.desc = d;
		LinkNodGet(nd,N,lvl);
        idxGetPosPtr(&pos,&nd,bR);
		idx = pos.b;
        idxLinkGetLength(e,N);
		if (eRange<e) { e = eRange; }
		while ((!found)&&(idx<e)&&(pos.pp<nd.e)) {
			idxNodGetLength(lg,lvl-1,*pos.pp,d->LeafSz);
			if (bR<idx+lg) {
			    lR = bR-idx;
                found = idxNodForEach(d,lvl-1,*pos.pp,&b,step,&lR,eRange-idx,Step,Itr);
			    bR = lR+idx; 
			}
			idx+=lg; pos.pp++;
		}
	}
	*B = b; *bRange = bR;
	return found;
}

/*___________________________________
 |
 | Interface
 |___________________________________
*/

typedef struct {
	TlsIdxList TlsIdxList;
	IdxDesc Data;
	int EltNb;
} IdxList;

static int IdxListEltNb(TlsIdxList *this) {
	ThisToThat(IdxList,TlsIdxList);
	return that->EltNb;
}
static void *IdxListEltGet(TlsIdxList *this,void *Dst,int idx,int nb) {
	int Idx,dIdx,Nb;
	ThisToThat(IdxList,TlsIdxList);
	Idx = idx * that->Data.EltSize;
	Nb = nb * that->Data.EltSize;
	if (Idx<0) { Nb+=Idx; Idx=0; idx=0; }
	if (idx>that->EltNb) { Nb = 0; }
	if (Nb>0) {
        dIdx = idxNodGet(&that->Data,that->Data.Lvl,that->Data.Root,Dst,Nb,Idx);
	} else {
		dIdx = 0;
	}
	return Dst+dIdx;
}
static void *IdxListEltSet(TlsIdxList *this,int idx,void *src,int nb) {
	int Idx,dIdx,Nb;
	ThisToThat(IdxList,TlsIdxList);
    Idx = idx * that->Data.EltSize;
	Nb = nb * that->Data.EltSize;
	if (Idx<0) { Nb+=Idx; Idx=0; idx=0; }
	if (idx>that->EltNb) { Nb = 0; }
	if (Nb>0) {
        dIdx = idxNodSet(&that->Data,that->Data.Lvl,that->Data.Root,src,Nb,Idx);
	} else {
		dIdx = 0;
	}
	return src+dIdx;
}
static int IdxListInsert(TlsIdxList *this,int Idx,int Nb) {
	IdxNod *n,*a,*b;
	int nlvl,alvl,blvl,lvl;
	int idx,nb;
	ThisToThat(IdxList,TlsIdxList);
	idx = Idx*that->Data.EltSize;
	nb  = Nb*that->Data.EltSize;
	n = idxTreeAlloc(&that->Data,&nlvl,nb);
	a = that->Data.Root; alvl = that->Data.Lvl;
	b = idxLinkSplit(&that->Data,&blvl,&alvl,&a,idx);
	a = idxLinkCat(&that->Data,&lvl,alvl,a,nlvl,n);
	that->Data.Root = idxLinkCat(&that->Data,&alvl,lvl,a,blvl,b);
    that->Data.Lvl = alvl;
	that->EltNb += Nb;
	return 0;
}
static void IdxListRemove(TlsIdxList *this,int Idx,int Nb) {
	IdxNod *o,*a,*b;
	int olvl,alvl,blvl,lvl;
	int idx,nb;
	ThisToThat(IdxList,TlsIdxList);
	idx = Idx*that->Data.EltSize;
	nb = Nb*that->Data.EltSize;
    a = that->Data.Root; alvl = that->Data.Lvl;
	o = idxLinkSplit(&that->Data,&olvl,&alvl,&a,idx);
	b = idxLinkSplit(&that->Data,&blvl,&olvl,&o,nb);
	that->Data.Root = idxLinkCat(&that->Data,&lvl,alvl,a,blvl,b);
	that->Data.Lvl = lvl;
	that->EltNb -= Nb;
    IdxLinkFree(&that->Data,olvl,o);
}
static void IdxListForEach(TlsIdxList *this,int bRange,int eRange,int Step,TlsIdxListIterator *Itr) {
	int bR,eR,st;
	ThisToThat(IdxList,TlsIdxList);
	if (Step<=0) {
		if (Step) {
		    int nb;
		    Step = -Step;
		    nb = (bRange-eRange)/Step;
		    eRange = bRange+Step;
		    bRange = eRange-(Step*nb);
		} else {
			Step = 1; bRange = eRange = 0;
		}
	}
    if (bRange<0) {
        bR = -bRange; eR = bR%Step;
		if (eR) { bRange = Step-eR; } else { bRange = 0; }
	}
	if (eRange>that->EltNb) { eRange = that->EltNb; }
	if ((bRange<eRange)&&(bRange<that->EltNb)) {
		bR = bRange*that->Data.EltSize;
		eR = eRange*that->Data.EltSize;
		st = Step*that->Data.EltSize;
        idxNodForEach(&that->Data,that->Data.Lvl,that->Data.Root,&bRange,Step,&bR,eR,st,Itr);
	}
}
static TlsIdxList *IdxListClear(TlsIdxList *this) { 
	ThisToThat(IdxList,TlsIdxList);
	IdxLinkFree(&that->Data,that->Data.Lvl,that->Data.Root);
	that->Data.Lvl = 0;
	that->Data.Root = idxLeafAlloc(&that->Data,0);
	that->EltNb = 0;
	return this;
}

TlsIdxList *TlsBigIdxListNew(int EltSize,int Growth) {
	static struct TlsIdxList Static = {
		IdxListEltNb,IdxListEltGet,IdxListEltSet,
		IdxListInsert,IdxListRemove,IdxListForEach,IdxListClear
	};
	IdxList *r;
	rPush(r);
	r->TlsIdxList.Static = &Static;
	idxDescNew(&r->Data,EltSize,Growth);
	r->EltNb = 0;
	return &r->TlsIdxList;
}

/*_____________________________________________________________________________
 |
 |
 |_____________________________________________________________________________
*/

static IdxNod *idxEmptyNodAlloc(IdxDesc *d,int End) {
	IdxNod *r,**b;
	r = Call(d->Pool,Alloc,0);
	r->End = End;
	{ void *data; data = r+1; b = r->Data.Link.b = r->Data.Link.e = data; }
	return r;
}

/*_____________________________________________________________________________
 |
 | Split
 |_____________________________________________________________________________
*/

static IdxNod *idxLeafSplit(IdxDesc *d,IdxNod **Car,int idx) {
	IdxNod *car,*cdr,*x;
	LeafNod cad,cdd;
	int length,dl;
    car = *Car;
    LeafNodGet(cad,car,d->LeafSz);
	idxLeafGetLength(length,car,d->LeafSz);
	if ((idx>0)&&(idx<length)) {
	    dl = length-idx;
        cdr = idxLeafAlloc(d,dl);
	    LeafNodGet(cdd,cdr,d->LeafSz);
	    TlsCBfCpy(&cdd.Data,0,dl,&cad.Data,idx);
	    TlsCBfRemove(&cad.Data,idx,dl);
		car->End = idx;
	    LeafNodUpdate(cdr,cdd);
	    LeafNodUpdate(car,cad);
	} else {
		cdr = idxLeafAlloc(d,0);
		if (idx<=0) { x = cdr; cdr = car; car = x; *Car = car; }
	}
	return cdr;
}

static IdxNod *idxLinkSplit(IdxDesc *d,int *cdrlvl,int *carlvl,IdxNod **Car,int idx) {
	PosPtr pos;
	LinkNod cad,cdd;
	IdxNod *cdr,*x,*car;

	IdxNod *LvlCleanSplit(int cdrOffset,int idx) {
		int i,l,lvl;
		IdxNod **p,**q,**e,*cdr;
	    cdr = idxEmptyNodAlloc(d,cad.length-idx);
		car->End = idx;
		i = cdrOffset;
		q = cdr->Data.Link.b;
		if ((idx==pos.b)||(pos.pp>=car->Data.Link.e)) {
			p = pos.pp;
		} else {
			p = pos.pp+1;
		}
		e = car->Data.Link.e;
		car->Data.Link.e = p;
		lvl = (*carlvl)-1;
		if (lvl<=0) {
            while (p<e) {
                idxLeafGetLength(l,*p,d->LeafSz);
			    i += l; (*p)->End = i; *q++ = *p++;
		   	}
		} else {
            while (p<e) {
                idxLinkGetLength(l,*p);
			    i += l; (*p)->End = i; *q++ = *p++;
		   	}
		}
		cdr->Data.Link.e = q;
		return cdr;
	}

	IdxNod *LvlMiddleSplit() {
		IdxNod *cdr,*ucdr;
		int carl,ucdrl,l;
		cdr = LvlCleanSplit(0,pos.e);
		car->End = pos.b; car->Data.Link.e = pos.pp;
        carl = (*carlvl)-1;
		pos.p->End = (pos.e-pos.b);
		ucdr = idxLinkSplit(d,&ucdrl,&carl,&pos.p,idx-pos.b);
        *Car = car = idxLinkCat(d,carlvl,*carlvl,car,carl,pos.p);
		cdr = idxLinkCat(d,cdrlvl,ucdrl,ucdr,*cdrlvl,cdr);
        return cdr;
	}

	IdxNod *LvlFrontSplit() {
		IdxNod *cdr,*ucdr;
		int carl,ucdrl;
		cdr = LvlCleanSplit(0,pos.e);
		carl = (*carlvl)-1;
		ucdr = idxLinkSplit(d,&ucdrl,&carl,&pos.p,idx-pos.b);
		cdr = idxLinkCat(d,cdrlvl,ucdrl,ucdr,*cdrlvl,cdr);
		Call(d->Pool,Free,1(car));
		*carlvl = carl; *Car = pos.p;
		return cdr;
	}

	IdxNod *LvlBackSplit() {
		IdxNod *cdr;
		int carl;
        car->End = pos.b; car->Data.Link.e--;
		carl = (*carlvl)-1;
		cdr = idxLinkSplit(d,cdrlvl,&carl,&pos.p,idx-pos.b);
		*Car = idxLinkCat(d,carlvl,*carlvl,car,carl,pos.p);
		return cdr;
	}

	cad.desc = cdd.desc = d;
	car = *Car;
	*cdrlvl = *carlvl;
	if (*carlvl<=0) {
		cdr = idxLeafSplit(d,&car,idx);
		*Car = car;
	} else {
	    LinkNodGet(cad,car,*carlvl);
		if ((idx>0)&&(idx<cad.length)) {
			IdxNod **p,**ep,**q,*split;
			int plvl,splitlvl,empty;
		    idxGetPosPtr(&pos,&cad,idx);
			if ((idx==pos.b)||(idx==pos.e)) {
				cdr = LvlCleanSplit(0,idx);
			} else {
                if ((pos.pp==car->Data.Link.b)){
                    cdr = LvlFrontSplit();
			    } else {
				    if (pos.pp==(car->Data.Link.e-1)) {
					    cdr = LvlBackSplit();
					} else {
					    cdr = LvlMiddleSplit();
				    }
			    }
			}
		} else {
			if (idx<=0) {
				*cdrlvl = *carlvl; cdr = car; 
				*carlvl = 0;
				*Car = car = idxLeafAlloc(d,0);
			} else {
				*cdrlvl = 0;
                cdr = idxLeafAlloc(d,0);
			}
		}
	}
	return cdr;
}

/*_____________________________________________________________________________
 |
 | Cat
 |_____________________________________________________________________________
*/

static IdxNod *idxLinkLvlUpAlloc(IdxDesc *d,int lvl,int lv,IdxNod *org) {
	IdxNod *r;
	int length;
	r = org;
	idxNodGetLength(length,lvl,r,d->LeafSz);
	while (lv<lvl) {
		IdxNod *n;
		lv++;
		n = idxEmptyNodAlloc(d,0);
		n->Data.Link.e[0]=r; r->End = length; n->Data.Link.e++;
		r = n;
	}
	return r;
}

/*--------------*/

static IdxNod **idxLeafThoroughPack(IdxDesc *d,IdxNod **b,IdxNod **e) {
	IdxNod **p,**q,*qc,*pc;
	LeafNod dp,dq;
	int ql,pl,tl,idx,leafsz;
	e++; leafsz = d->LeafSz;
	q = b; qc = *q; LeafNodGet(dq,qc,leafsz);
	idxLeafGetLength(ql,qc,leafsz); 
	idx = qc->End-ql;
	p = q+1;
	while (p<e) {
		pc = *p; LeafNodGet(dp,pc,leafsz);
		idxLeafGetLength(pl,pc,leafsz);
		do {
            tl = leafsz-ql; if (tl>pl) tl = pl;
			TlsCBfInsert(&dq.Data,ql,tl);
		    TlsCBfCpy(&dq.Data,ql,ql+tl,&dp.Data,0);
			TlsCBfRemove(&dp.Data,0,tl);
		    ql+=tl; pl-=tl;
			if (ql>=leafsz) {
				idx += ql; qc->End = idx; 
				LeafNodUpdate(pc,dp);
				LeafNodUpdate(qc,dq);
				q++; qc = *q; 
				LeafNodGet(dq,qc,leafsz); 
				idxLeafGetLength(ql,qc,leafsz);
		    }
		} while ((pl>0)&&(q!=p));
		LeafNodUpdate(pc,dp);
		p++;
	}
	LeafNodUpdate(qc,dq); p = q+1;
	while (p<e) { Call(d->Pool,Free,1(*p)); p++; }
	return q;
}
static IdxNod *idxLeafShallowPack(IdxDesc *d,IdxNod *lc) {
	struct {
        IdxNod **pNod;
		int offset;
	} wndw[8],*bbw,*ebw,*bw,*ew,*ow;
	IdxNod **p,**q;
	LinkNod l;
	int leafsz,ofs,lng,minsz;
	l.desc = d;
	LinkNodGet(l,lc,1);
	if (l.b<l.e) {
	    leafsz = d->LeafSz;
	    bbw = wndw; ebw = wndw+8;
		q = p = l.b; minsz = 0;
	    idxLeafGetLength(ofs,*p,leafsz);
	    ow = bw = ew = bbw; 
		ew->offset = 0; ew->pNod = p++;
	    ew++;
	    while (p<l.e) {
			if (ew==bw) { 
				*q++ = *bw->pNod; 
				bw++; if (bw>=ebw) bw = bbw; 
			} else {
				minsz += leafsz;
			}
			ow = ew; 
		    idxLeafGetLength(lng,*p,leafsz);
		    ew->offset = ofs; ew->pNod = p++;
		    ofs += lng;
			ew++; if (ew>=ebw) ew = bbw;
		    if (ow!=bw) {
			    if ((ofs-bw->offset)<=minsz) {
				    q = idxLeafThoroughPack(d,bw->pNod,ow->pNod);
				    idxLeafGetLength(lng,*q,leafsz);
				    bw = bbw; ew = bw+1;
				    bw->pNod = q;
				    bw->offset = ofs-lng; minsz = 0;
			    }
		    }
	    }
        while ((bw!=ew)||(minsz)) {
			minsz = 0;
		    *q++ = *bw->pNod; 
			bw++; if (bw>=ebw) bw = bbw;
	    }
	    l.e = q;
	    LinkNodUpdate(lc,l);
	}
	return lc;
}

static void idx2LinksPack(IdxDesc *d,int lvl,IdxNod *a,IdxNod *b) {
	IdxNod **p,**q,**ep,**eq;
	int End,l,leafsz;
	lvl--;
	p = b->Data.Link.b; ep = b->Data.Link.e;
	q = a->Data.Link.e; eq = a->Data.Link.b+d->LinkSz;
	leafsz = d->LeafSz;
	idxLinkGetLength(End,a);
	while ((p<ep)&&(q<eq)) {
		idxNodGetLength(l,lvl,*p,leafsz);
		End+=l; (*p)->End = End;
		*q++ = *p++;
	}
	a->Data.Link.e = q;
	End = 0; q = b->Data.Link.b;
	while (p<ep) {
		idxNodGetLength(l,lvl,*p,leafsz);
		End += l; (*p)->End = End;
		*q++ = *p++;
	}
	b->Data.Link.e = q;
}

static IdxNod **idxLinkThoroughPack(IdxDesc *d,int lvl,IdxNod **b,IdxNod **e) {
	IdxNod **p,*cp,**q,*cq;
	int End,l,linksz;
	e++;
	linksz = d->LinkSz;
	q = b; cq = *q;
	idxLinkGetLength(l,(*q));
	End = cq->End-l;
	p = q+1; if (p<e) cp = *p;
	while (p<e) {
		idx2LinksPack(d,lvl,cq,cp);
		if (cp->Data.Link.b == cp->Data.Link.e) { p++; if(p<e) cp = *p; }
		if ((cq->Data.Link.e-cq->Data.Link.b)>=linksz) {
			idxLinkGetLength(l,cq); End += l; cq->End = End;
			q++; cq = *q;
			if ((q==p)&&(p<e)) { p++; if (p<e) cp = *p; }
		}
	}
	idxLinkGetLength(l,cq); End += l; cq->End = End;
	p = q+1;
	while (p<e) { Call(d->Pool,Free,1(*p)); p++; }
	return q;
}

static IdxNod *idxLinkShallowPack(IdxDesc *d,int *Lvl,IdxNod *lc) {
	LinkNod ld,pd;
	IdxNod **p,**q;
	struct { IdxNod **pNod; int offset; } wndw[4],*bw,*ew,*bbw,*ebw,*ow;
	int ofs,lnksz,minsz,lvl;
	ld.desc = pd.desc = d;
	LinkNodGet(ld,lc,lvl);
	q = p = ld.b; lvl = (*Lvl)-1; lnksz = d->LinkSz;
	bbw = wndw; ebw = wndw+4;
	if (p<ld.e) {
   	    minsz = 0; ew = bw = bbw;
	   	ew->offset = 0; ew->pNod = q;
		ofs = (*q)->Data.Link.e-(*q)->Data.Link.b;
		p++; ew++;
	    while (p<ld.e) {
			if (ew==bw) {
				*q++ = *bw->pNod;
				bw++; if (bw>=ebw) bw=bbw;
			} else {
				minsz += lnksz;
			}
			ow = ew; ew->pNod = p; ew->offset = ofs;
		    ofs += (*p)->Data.Link.e-(*p)->Data.Link.b;
		    if ((ofs-bw->offset)<=minsz) {
                q = idxLinkThoroughPack(d,lvl,bw->pNod,ow->pNod);
			    ew = bw = bbw; minsz = 0;
			    ew->pNod = q; ew->offset = 0; 
				ofs = (*q)->Data.Link.e-(*q)->Data.Link.b;
		    }
			p++; ew++; if (ew>=ebw) ew=bbw;
	    }
	    while ((bw!=ew)||(minsz)) {
			minsz = 0;
		    *q++ = *bw->pNod;
		    bw++; if (bw>=ebw) bw = bbw;
	    }
	    ld.e = q;
	    LinkNodUpdate(lc,ld);
	}
	return lc;
}

static IdxNod *idxLvlPack(IdxDesc *d,int *lvl,IdxNod *lnk) {
	if ((*lvl)>1) {
		lnk = idxLinkShallowPack(d,lvl,lnk);
	} else {
		lnk = idxLeafShallowPack(d,lnk);
	}
	return lnk;
}

/*--------------*/

static IdxNod *idxLvlCat(IdxDesc *d,int *lvr,int lv,IdxNod *car,IdxNod *cdr) {
	IdxNod *r;

	IdxNod *CatOverflow(int sz0,int sz1) {
		IdxNod **p,*r;
		r = idxEmptyNodAlloc(d,sz0+sz1);
		p = r->Data.Link.b;
		p[0] = car; p[1] = cdr; r->Data.Link.e = p+2;
		p[0]->End = sz0; p[1]->End = sz0+sz1;
		return r;
	}

	IdxNod *LeafCat(int *lvr) {
		LeafNod cad,cdd;
		int l0,l1,sz0,sz1,lm;
		idxLeafGetLength(l0,car,d->LeafSz);
		idxLeafGetLength(l1,cdr,d->LeafSz);
		sz0 = l0; sz1 = l1; lm = d->LeafSz;
		if (l0+l1>lm) {
			*lvr = 1;
			r = CatOverflow(sz0,sz1);
		} else {
            LeafNodGet(cad,car,lm);
			LeafNodGet(cdd,cdr,lm);
			TlsCBfInsert(&cad.Data,l0,l1);
			TlsCBfCpy(&cad.Data,l0,l0+l1,&cdd.Data,0);
			LeafNodUpdate(car,cad);
			Call(d->Pool,Free,1(cdr));
			car->End += l1;
			*lvr = 0;
		    r = car;
		}
		return r;
	}

	IdxNod *LinkCat(int *lvr) {
		LinkNod cad,cdd;
		int l0,l1,lm,sz,idx;
		lm = d->LinkSz;
		cad.desc = cdd.desc = d;
        LinkNodGet(cad,car,lv);
		LinkNodGet(cdd,cdr,lv);
		l0 = cad.e-cad.b;
        l1 = cdd.e-cdd.b;
		if (l0+l1<=lm) {
            IdxNod **p,**q,**e;
	        *lvr = lv;
			p = cad.e; e = p+l1; q = cdd.b;
			idx = cad.length;
			if (lv>1) {
			    while (p<e) {
		            idxLinkGetLength(sz,*q);
					idx += sz; (*q)->End = idx; 
					*p++ = *q++; 
			    } 
			} else {
				while (p<e) {
					idxLeafGetLength(sz,*q,d->LeafSz);
					idx += sz; (*q)->End = idx;
					*p++ = *q++;
				}
			}
			car->End += cdd.length;
			cad.length = idx; cad.e = p;
			LinkNodUpdate(car,cad);
			Call(d->Pool,Free,1(cdr));
		    r = idxLvlPack(d,lvr,car);
		} else {
			*lvr = lv+1;
			r = CatOverflow(cad.length,cdd.length);
		}
	    return r;
	}

	if (lv==0) {
		r = LeafCat(lvr);
	} else {
		IdxNod **ar,**dr;
		ar = car->Data.Link.e;
		dr = cdr->Data.Link.b;
		if ((ar>car->Data.Link.b)&&(dr<cdr->Data.Link.e)) {
            int l0,l1,lv0,ovflw;
			IdxNod *ocar,*ocdr,*scat;
			ar--; ocar = *ar; ocdr = *dr;
			if (lv<=1) {
				idxLeafGetLength(l0,ocar,d->LeafSz);
				idxLeafGetLength(l1,ocdr,d->LeafSz);
				ovflw = (l0+l1)>d->LeafSz;
			} else {
				l0 = ocar->Data.Link.e-ocar->Data.Link.b;
				l1 = ocdr->Data.Link.e-ocdr->Data.Link.b;
				ovflw = (l0+l1)>d->LinkSz;
			}
			if (!ovflw) {
				IdxNod **p,**e;
				idxNodGetLength(l0,lv-1,ocar,d->LeafSz);
				l1 = ocdr->End;
				car->End -= l0; cdr->End += l0;
				car->Data.Link.e--;
			    ocdr = *dr = idxLvlCat(d,&lv0,lv-1,ocar,ocdr);
				ocdr->End = l0+l1;
                p = cdr->Data.Link.b+1; e = cdr->Data.Link.e;
				while (p<e) { (*p)->End+=l0; p++; }
			}
		}
		r = LinkCat(lvr);
	}
	return r;
}

static IdxNod *idxLinkCat(IdxDesc *d,int *lvp,int lvcar,IdxNod *car,int lvcdr,IdxNod *cdr) {
	IdxNod *r;

	IdxNod *RearCat(int *rlvl) {
		LinkNod cad;
		IdxNod *p,*q,*r;
		int l,plvl;
		cad.desc = d;
		p = cdr; plvl = lvcdr;
		LinkNodGet(cad,car,lvcar);
		if (lvcar>1) {
		    while ((cad.e!=cad.b)&&(plvl<lvcar)) {
			    cad.e--; q = cad.e[0]; 
			    idxLinkGetLength(l,q);
			    q->End = l; car->End -= l; cad.length -= l;
                p = idxLinkCat(d,&plvl,cad.lvl-1,q,plvl,p);
		    }
		} else {
		    while ((cad.e!=cad.b)&&(plvl<lvcar)) {
			    cad.e--; q = cad.e[0]; 
			    idxLeafGetLength(l,q,d->LeafSz);
			    q->End = l; car->End -= l; cad.length -= l;
                p = idxLinkCat(d,&plvl,cad.lvl-1,q,plvl,p);
		    }
		}
		LinkNodUpdate(car,cad);
		if (cad.e==cad.b) {
			r = p; 
			Call(d->Pool,Free,1(car));
		} else {
			p = idxLinkLvlUpAlloc(d,cad.lvl,plvl,p);
		    r = idxLvlCat(d,&plvl,cad.lvl,car,p);
		}
		*rlvl = plvl;
		return r;
	}

    IdxNod *UpdateCdr(IdxNod *cdr,LinkNod *cdd) {
		int l,idx;
		IdxNod **p,**q,**e;
		q = cdr->Data.Link.b; p = cdd->b; e = cdd->e;
		idx = 0;
		if (cdd->lvl>1) { 
		    while (p<e) {
			    idxLinkGetLength(l,*p);
			    idx += l; (*p)->End = idx; *q = *p;
			    p++; q++;
	        }
		} else {
		    while (p<e) {
			    idxLeafGetLength(l,*p,d->LeafSz);
			    idx += l; (*p)->End = idx; *q = *p;
			    p++; q++;
	        }
		}
		cdr->Data.Link.e = cdd->e = q;
		return cdr;
	}

	IdxNod *FrontCat(int *rlvl) {
		LinkNod cdd;
		IdxNod *p,*q,*r;
		int plvl,l;
		plvl = lvcar; p = car;
		cdd.desc = d;
		LinkNodGet(cdd,cdr,lvcdr);
		if (cdd.lvl>1) {
		    while ((plvl<cdd.lvl)&&(cdd.b!=cdd.e)) {
			    q = *cdd.b; cdd.b++;
			    idxLinkGetLength(l,q); q->End = l;
			    p = idxLinkCat(d,&plvl,plvl,p,cdd.lvl-1,q);
		    }
		} else {
		    while ((plvl<cdd.lvl)&&(cdd.b!=cdd.e)) {
			    q = *cdd.b; cdd.b++;
			    idxLeafGetLength(l,q,d->LeafSz); q->End = l;
			    p = idxLinkCat(d,&plvl,plvl,p,cdd.lvl-1,q);
		    }
		}
		if (cdd.b==cdd.e) {
			r = p; *rlvl = plvl;
			Call(d->Pool,Free,1(cdr));
		} else {
            cdr = UpdateCdr(cdr,&cdd);
			p = idxLinkLvlUpAlloc(d,cdd.lvl,plvl,p);
		    r = idxLvlCat(d,lvp,cdd.lvl,p,cdr);
		}
		return r;
	}

	if (lvcdr==lvcar) {
		r = idxLvlCat(d,lvp,lvcar,car,cdr);
	} else {
		if (lvcar>lvcdr) {
            r = RearCat(lvp);
		} else {
			r = FrontCat(lvp);
		}
	}
	return r;
}


