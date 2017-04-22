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
#include <List.h>

#include <GeoText.h>

/*___________________________________________
 |
 | Text
 |___________________________________________
*/
static int lnullLength(GeoTextLine *this){ return 0; }
static char *lnullGetPlainText(GeoTextLine *this,char *r,int bPos,int ePos) { return r;}
static char *lnullGetAttrText(GeoTextLine *this,char *r,int bPos,int ePos){ return r;}
static void lnullInsert(GeoTextLine *this,char *Attribut,int Pos,char *b,char *e) {}
static void lnullRemove(GeoTextLine *this,int Pos,int Length) {}
static void lnullAttrSet(GeoTextLine *this,int bPos,int ePos,char *Mask,char *Attr) {}

static struct GeoTextLine lnullStatic = {
	lnullLength,lnullGetPlainText,lnullGetAttrText,lnullInsert,lnullRemove,lnullAttrSet
};
GeoTextLine GeoTextLineNull = {&lnullStatic};
   
    /*----*/

static int tnullLineNb(GeoText *this) { return 0; }
static GeoTextLine *tnullGetLine(GeoText *this,int *Layout,int num) { return &GeoTextLineNull; }
static void tnullInsert(GeoText *this,int num,int nb,int Layout){}
static void tnullRemove(GeoText *this,int num,int nb) {}
static void tnullLayoutSet(GeoText *this,int num,int nb,int Layout) {}

static struct GeoText tnullStatic = {
	tnullLineNb,tnullGetLine,tnullInsert,tnullRemove,tnullLayoutSet
};
GeoText GeoTextNull = {&tnullStatic};

/*________________________________________________________
 |
 | Lines are cut in chunks allocated in a chunk manager.
 |________________________________________________________
*/

typedef struct {
	MemStack *Mem;
	int BlockSize;
	dList/*<TextChunk.n>*/ Free;
} TextChunkManager;
typedef struct {
	TextChunkManager *Alloc;
	int AtomSize,ChunkLength;
	dList/*<TextChunk.n>*/ Data,*Current;
	int CurrentOffset,length;
} TextChain;
typedef struct { // about 20-30 char header-> Chunks should be at least 128 char longs to compensate.
	TextChain *Chain;
	dList/*<TextChunk.n>*/ n;
	char *b,*e;
} TextChunk;

typedef struct {
	MemStack *Mem;
	TextChunkManager *Chunks;
	dList/*<TextLineChunk.TextLine.n>*/ Free;
} TextLineManager;
typedef struct TextLine {
	GeoTextLine GeoTextLine;
	TextLineManager *Pool;
	dList/*<TextLine.n>*/ n;
	int Layout,AttrSize,CharSize;
	TextChain Data;
} TextLine;

/*----------------*/

static char *tcGetSize(TextChunk *t,int *length) {
	int l;
	char *e;
	l = t->Chain->ChunkLength;
	e = t->b+(t->Chain->AtomSize*l);
    *length = l;
	return e;
}

static TextChunk *tcAlloc(TextChain *ch) {
	TextChunkManager *m;
	TextChunk *r;
	m = ch->Alloc;
	if (m->Free.n!=&m->Free) {
	    dList *n;
		n = m->Free.n;
		n->p->n = n->n; n->n->p = n->p;
		n->n = n->p = n;
		r = CastBack(TextChunk,n,n);
	} else {
		char *nc;
		void *nv,*ne;
		mnPush(m->Mem,nc,m->BlockSize); 
		nv = nc;
		r = nv; nv = r+1;
		r->b = nv;
		r->Chain = ch;
		r->n.n = r->n.p = &r->n;
	}
	r->e = r->b;
	return r;
}
static void tcFree(TextChunk *f) {
	TextChunkManager *m;
	m = f->Chain->Alloc;
	f->n.p = f->n.n = &f->n;
    f->n.p = &m->Free; f->n.n = m->Free.n;
	f->n.p->n = f->n.n->p = &f->n;
}
static void tcFreeRoot(dList *root) {
	TextChunkManager *m;
	TextChunk *f;
	if (root->n!=root) {
		f = CastBack(TextChunk,n,root->n);
		m = f->Chain->Alloc;
        root->n->p = &m->Free; root->p->n = m->Free.n;
		root->p->n->p = root->p; root->n->p->n = root->n;
		root->n = root->p = root;
	}
}

static TextLine *tlcAlloc(TextLineManager *Pool) {
	TextLine *r;
	dList *l;
	if (Pool->Free.n!=&Pool->Free) {
		l = Pool->Free.n;
		l->n->p = l->p; l->p->n = l->n; l->p = l->n;
		r = CastBack(TextLine,n,l);
	} else {
		mPush(Pool->Mem,r);
        r->Pool = Pool;
		r->Data.Alloc = Pool->Chunks;
		l = &r->n; l->n = l->p = l;
	}
	return r;
}
static void tlcFree(TextLine *f) {
	dList *l,*root;
	TextLineManager *Pool;
	tcFreeRoot(&f->Data.Data);
	Pool = f->Pool;
    l = &f->n; l->p = &Pool->Free; l->n = Pool->Free.n; l->p->n = l->n->p = l;
}

    /*-------*/

static TextChunk *tcChunkMergeNext(TextChunk *nod) {
	dList *l,*root;
	int ns;
	TextChunk *pn;
	root = &(nod->Chain->Data);
	ns = (nod->Chain->ChunkLength)*(nod->Chain->AtomSize);
	l = &nod->n;
	if (l->n!=root) {
		pn = CastBack(TextChunk,n,l->n);
        if (((pn->e-pn->b)+(nod->e-nod->b))<ns) {
			char *p,*q,*ep;
			if (l->n==nod->Chain->Current) {
				nod->Chain->Current = l;
				nod->Chain->CurrentOffset -= (nod->e-nod->b)/(nod->Chain->AtomSize);
			}
			p = nod->b; ep = nod->e; q = pn->e; while (p<ep) { *q++=*p++; }
			pn->e = q;
			l->p->n = l->n; l->n->p = l->p; l->p = l->n = l;
			tcFree(nod);
			nod = pn;
		}
	}
	return nod;
}

static TextChunk *tcChunkMergePrevious(TextChunk *nod) {
	dList *l;
	l = &nod->n;
    if (l->p!=&(nod->Chain->Data)) {
	    TextChunk *pn;
        pn = CastBack(TextChunk,n,l->p);
		nod = tcChunkMergeNext(pn);
	}
	return nod;
}

static TextChunk *tcChunkMerge(TextChunk *nod) {
	return tcChunkMergeNext(tcChunkMergePrevious(nod));
}

    /*------*/

static void tcThoroughPack(dList *b,dList *e) {
	int as,ts;
    char *p,*q,*ep,*eq;
	dList *root,*n,*ob;
	TextChunk *cn,*cb,*ce;
	cb = CastBack(TextChunk,n,b);
	root = &cb->Chain->Data;
	as = cb->Chain->AtomSize;
	ts = as*cb->Chain->ChunkLength;
	ob = b; n = b->n;
	if (n!=e) {
	    q = cb->e; eq = cb->b+ts; 
	    cn = CastBack(TextChunk,n,n);
		if (n==cb->Chain->Current) {
	        cb->Chain->Current = &cb->n;
			cb->Chain->CurrentOffset -= (cb->e-cb->b)/as;
		}
		p = cn->b; ep = cn->e;
	    while (n!=e) {
		    while ((q<eq)&&(p<ep)) {*q++=*p++;}
		    if (q==eq) {
			    cb->e = eq; b = b->n;
			    cb = CastBack(TextChunk,n,b);
			    q = cb->b; eq = q+ts;
		    }
		    if (p==ep) {
			    cn->e = cn->b; n = n->n;
			    if (n!=e) {
					if (n==cb->Chain->Current) {
						cb->Chain->Current = &cb->n;
						cb->Chain->CurrentOffset -= ((q-cb->b)/as);
				    }
			        cn = CastBack(TextChunk,n,n);
			        p = cn->b; ep = cn->e;
			    }
		    }
	    }
	    cb->e = q; n = &cn->n;
	    while ((n!=ob)&&(cn->b==cn->e)) {
			dList *p;
			p = n->p;
		    n->p->n = n->n; n->n->p = n->p; n->n = n->p; 
		    tcFree(cn); n = p; cn = CastBack(TextChunk,n,p);
	    }
	}
}

static void tcQuickPack(TextChain *dst,int *bl,int *el) {
	int ts,lng,cap,max;
	dList *bc,*ec;
	TextChunk *cc;
	int *bbl;
	if (dst->Data.p!=dst->Data.n) {
	ts = dst->AtomSize*dst->ChunkLength;
	max = (el-bl)*ts;
	lng = 0; cap = 0;
	bc = ec = dst->Data.n;
	bbl = bl; while (bbl<el) { *bbl++=0;}
	bbl = bl;
	while (ec!=&dst->Data) {
		int s;
		cc = CastBack(TextChunk,n,ec);
		s = cc->e-cc->b;
		if (cap<max) { cap+=ts;}
		lng += s-*bbl; *bbl=s;
		if ((cap-lng)>=ts) {
			int *p,l;
			ec = ec->n;
			tcThoroughPack(bc,ec);
			p = bbl; 
			*p = 0; p = (p==bl)?el-1:p-1; 
			*p = lng%ts;     p = (p==bl)?el-1:p-1;
			for (l=lng/ts; l!=0; l--) {
				*p = ts; p = (p==bl)?el-1:p-1;
			}
		} else {
		    bbl++; if (bbl==el) bbl=bl;
		    ec = ec->n; if (cap==max) { bc=bc->n; }
		}
	}
	}
}

/*-------------------------*/

static dList *tcGetChunk(TextChain *chain,int pos,int *ChunkStart) {
	TextChunk *r;
	dList *p;
	int as,fwd,cpos,bpos,epos,end;
	as = chain->AtomSize;
	bpos = chain->CurrentOffset;
	epos = chain->length;
	fwd = ((pos>=bpos)&&(pos<((bpos+epos)>>1)));
	fwd = fwd||(pos<(bpos>>1));
	if (fwd) {
		if (pos>=bpos) {
			p = chain->Current;
		} else {
			p = chain->Data.n;
			bpos = 0;
		}
		do {
			end = (p==&chain->Data);
			if (!end) {
				r = CastBack(TextChunk,n,p);
				epos = bpos+((r->e-r->b)/as);
				end = (bpos<=pos)&&(epos>pos);
				if (!end) { p = p->n; bpos = epos; }
			}
		} while(!end);
	} else {
		if (pos>=bpos) {
            p = &chain->Data;
		} else {
		    p = chain->Current;
		    epos = bpos;
		}
		bpos = -1;
		do {
			p = p->p;
			end = (p==&chain->Data);
			if (!end) {
                r = CastBack(TextChunk,n,p);
			    bpos = epos-((r->e-r->b)/as);
				end = (bpos<=pos)&&(epos>pos);
				if (!end) { epos = bpos; }
			}
		} while(!end);
	}
    *ChunkStart = bpos;
	return p;
}

static void tcMove(TextChain *chain,int pos) {
	dList *c;
	int start;
	c = tcGetChunk(chain,pos,&start);
	if (c!=&chain->Data) {
		chain->Current = c;
		chain->CurrentOffset = start;
	}
}

/*---------------------*/

static void tcRawInsert(TextChain *dst,int pos,char *b,char *e) {
	// It is assumed that dst contains at least one chunk.
	int cl,as,el,il,ml,bpos,BC;
	dList *c;
	TextChunk *cc;
	as = dst->AtomSize;
	el = dst->ChunkLength;
	il = (e-b)/as;

	if (pos<0) pos = 0;
	if (pos>=dst->length) { pos=dst->length; }
	BC = (pos<dst->CurrentOffset);
    c = tcGetChunk(dst,pos,&bpos);
	if (c==&dst->Data) {
		if (pos<=0) {
			c = c->n; bpos = 0;
		} else {
	        TextChunk *p;
			c = dst->Data.p;
			p = CastBack(TextChunk,n,c);
			bpos = dst->length - ((p->e-p->b)/as);
		}
	}
	if ((pos==bpos)&&(c->p!=&dst->Data)) {//insert at the end of previous chunk
        TextChunk *p;
		c = c->p;
		p = CastBack(TextChunk,n,c);
		bpos = bpos - ((p->e-p->b)/as);
	    BC = (pos<=dst->CurrentOffset);
	}
	if (BC) { dst->CurrentOffset+=il; }
	dst->length += il;

    cc = CastBack(TextChunk,n,c);
	cl = (cc->e-cc->b)/as;
	pos -= bpos;

	if (pos<0) pos = 0;
	if (pos>cl) pos = cl;
	if (il+cl<=el) {
		char *p,*q,*ep;
		q = cc->b + ((il+cl)*as);
		p = cc->e; ep = cc->b +(pos*as);
		cc->e = q;
		while (ep<p) { q--; p--; *q=*p; }
		p = e; ep = b;
		while (ep<p) { q--; p--; *q=*p; }
	} else {
		int rl;
		char *p,*q,*ep,*pp,*efl;
        TextChunk *last,*forlast;
		last = tcAlloc(dst);
		rl = (il+cl)%el;
		last->e = last->b + (as*rl);
		if (il+cl>(2*el)) { 
			forlast = tcAlloc(dst); 
			efl=forlast->b;
		} else { 
			forlast = cc; 
			efl = forlast->b+(as*pos);
		}
		q = last->e;
		p = cc->e; ep = cc->b+(pos*as);
		forlast->e = forlast->b+(as*el);
		cc->e = cc->b+(as*el);
		while ((ep<p)&&(last->b<q)) { q--; p--; *q=*p; }
		if (last->b<q) {
			p = e;
			while (last->b<q) { q--; p--; *q=*p; }
			q = forlast->e;
		} else {
			q = forlast->e;
			while (ep<p) { q--; p--; *q=*p; }
			p = e;
		}
		while (efl<q) {q--; p--; *q=*p; }
		if (cc!=forlast) {
		    dList *l;
		    pp = p;
			q = cc->b + (pos*as);
			p = b; ep = b + ((el-pos)*as);
			while (p<ep) { *q=*p; q++; p++; }
			l = &last->n;
			l->p = c; l->n = c->n; l->p->n = l->n->p = l;
			l = &forlast->n;
			l->p = c; l->n = c->n; l->p->n = l->n->p = l;
			p = pp;
			while (p>ep) {
				TextChunk *n;
		        n = tcAlloc(dst);
				n->e = n->b+(el*as);
				q = n->e;
				while (q>n->b) { q--; p--; *q=*p; }
				l = &n->n;
			    l->p = c; l->n = c->n; l->p->n = l->n->p = l;
			}
		} else {
			dList *l;
			l = &last->n;
			l->p = c; l->n = c->n; l->p->n = l->n->p = l;
		}
	}
}

static void tcCheckRawInsert(TextChain *dst,int pos,char *b,char *e) {
	if (b!=e) { 
		if (dst->Data.n==&dst->Data) {
			TextChunk *n;
			dList *c;
			n = tcAlloc(dst);
            c = &n->n;
		    c->n = &dst->Data; c->p = &dst->Data;
		    c->n->p = c->p->n = c;
		}
		tcRawInsert(dst,pos,b,e); 
	}
}

/*---------------------*/

static void tcRemove(TextChain *dst,int bpos,int epos) {
	TextChunk *cc;
	if (bpos<0) bpos=0;
	if (epos>=dst->length) epos = dst->length;
	if (bpos<epos) {
		dList *c;
        int as,el,bc,ec;
	    as = dst->AtomSize;
	    el = dst->ChunkLength;
		dst->length -= (epos-bpos);
		c = tcGetChunk(dst,bpos,&bc);
		while ((bpos<epos) && (c!=&dst->Data)) {
			int rmv;
			char *p,*q,*ep,*eq;
			cc = CastBack(TextChunk,n,c);
			ec = bc+((cc->e-cc->b)/as);
			rmv = 0;
			if (epos<ec) {
				rmv = epos-bpos;
				q = cc->b+((bpos-bc)*as);
				p = cc->b+((epos-bc)*as);
				ep = cc->b+ec;
				while (p<ep) { *q=*p; p++; q++; }
				cc->e = q;
			} else {
				if (bpos<=bc) {
					rmv = ec-bc;
					cc->e = cc->b;
				} else {
					rmv = ec-bpos;
					cc->e = cc->b+((bpos-bc)*as);
				}
			}
			if ((bpos<=dst->CurrentOffset)&&(c!=dst->Current)) { dst->CurrentOffset -= rmv;}
			epos -= rmv;
			if ((cc->b==cc->e)&&(c->p!=c->n)) {
				dList *n;
                if (c==dst->Current) { dst->Current=c->n; }
				n = c->n;
				c->p->n = c->n; c->n->p = c->p; c->p = c->n = c;
				tcFree(cc);
				c = n;
			} else {
			    c = c->n;
			}
		}
		if (c!=&dst->Data) {
			cc = CastBack(TextChunk,n,c);
			tcChunkMerge(cc);
		}
	}
}

/*--------------------*/

static void tcFill(TextChain *chain,int bpos,int epos,int offset,char *bMask,char *eMask,char *Val) {
	if (bpos<0) bpos = 0;
	if (epos>chain->length) epos=chain->length;
	if (bpos<epos) {
		dList *c;
	    int as,oa,bc;
		TextChunk *cc;
	    as = chain->AtomSize; oa = as-(eMask-bMask);
		c = tcGetChunk(chain,bpos,&bc);
		while (bpos<epos) {
			char *p,*m,*q,*em,*eq;
			cc = CastBack(TextChunk,n,c);
			q = cc->b+((bpos-bc)*as)+offset; eq = cc->e;
			while ((bpos<epos)&&(q<eq)) {
			    p = Val; m = bMask; em = eMask;
				while (m<em) {
					char v;
					v = (*q&*m)|*p; *q = v;
					q++; p++; m++;
				}
				q+=oa; bpos++;
			}
			bc = bpos;
			c = c->n;
		}
	}
}

/*__________________________________*/

static void tcCopy(TextChain *chain,char *r,int bPos,int ePos,int offset,int width) {
	if (bPos<0) bPos = 0;
	if (ePos>chain->length) ePos=chain->length;
	if (bPos<ePos) {
		dList *c;
		int as,oa,bc;
		char *p,*q,*ep,*eep;
		TextChunk *cc;
		as = chain->AtomSize; oa = as-width;
		c = tcGetChunk(chain,bPos,&bc);
		q = r;
		while (bPos<ePos) {
			cc = CastBack(TextChunk,n,c);
		    p = cc->b+((bPos-bc)*as)+offset; ep = cc->e;
			while ((bPos<ePos)&&(p<ep)) {
				eep = p+width;
				while (p<eep) { *q++ = *p++;}
				p += oa; bPos++;
			}
			bc = bPos;
			c = c->n;
		}
	}
}

/*____________________________________
 |
 |
 |____________________________________
*/


static int tlLength(GeoTextLine *this) {
	ThisToThat(TextLine,GeoTextLine);
	return that->Data.length;
}
static char *tlGetPlainText(GeoTextLine *this,char *r,int bPos,int ePos) {
	ThisToThat(TextLine,GeoTextLine);
    tcCopy(&that->Data,r,bPos,ePos,that->AttrSize,that->CharSize);
	return r + ((ePos-bPos)*that->CharSize);
}
static char *tlGetAttrText(GeoTextLine *this,char *r,int bPos,int ePos){
	ThisToThat(TextLine,GeoTextLine);
	tcCopy(&that->Data,r,bPos,ePos,0,that->AttrSize+that->CharSize);
	return r + ((ePos-bPos)*(that->AttrSize+that->CharSize));
}
static void tlInsert(GeoTextLine *this,char *Attribut,int Pos,char *b,char *e) {
	int nb;
	ThisToThat(TextLine,GeoTextLine);
	nb = (e-b)/that->CharSize;
	if (Pos<0) Pos=0;
	if (Pos>=that->Data.length) Pos=that->Data.length;
	if (nb) {
		TextChunk *tmp;
		int st;
		char *bt,*et,*q,*p,*ep,*eq;
		tmp = tcAlloc(&that->Data);
		bt = tmp->b; et = tcGetSize(tmp,&st);
		if (st>nb) { st = nb; et = bt+st*that->Data.AtomSize; }
        q = bt; eq = et;
		while (q<eq) {
			p = Attribut; ep = p+that->AttrSize;
			while (p<ep) { *q++=*p++; }
			q += that->CharSize;
		}
        p = b;
		while (p<e) {
            q = bt;
			while ((q<eq)&&(p<e)) {
				ep = p+that->CharSize;
				q += that->AttrSize;
				while (p<ep) { *q++=*p++; } 
			}
		    tcMove(&that->Data,Pos);
            tcCheckRawInsert(&that->Data,Pos,bt,q);
			Pos += st;
		}
		tcFree(tmp);
	}
}
static void tlRemove(GeoTextLine *this,int bPos,int ePos) {
	ThisToThat(TextLine,GeoTextLine);
    tcRemove(&that->Data,bPos,ePos);
}
static void tlAttrSet(GeoTextLine *this,int bPos,int ePos,char *Mask,char *Attr) {
	char *b,*e,full;
	int end;
	ThisToThat(TextLine,GeoTextLine);
	b = Mask; e = Mask+that->AttrSize; end = (b==e);
	full = -1;
	while ((b<e)&&(!end)) { 
		end = (*b!=full);
		if (!end) { b++; Attr++; }
	}
	while ((b<e)&&(!end)) {
		end = ((*(e-1))!=full);
        if (!end) { e--; }
	}
	if (b<e) tcFill(&that->Data,bPos,ePos,b-Mask,b,e,Attr);
}

static TextChain *tcmChainFill(TextChain *r,TextChunkManager *mng,int AtomSize) {
	int datasize;
	datasize = mng->BlockSize - sizeof(TextChunk); 
	r->Alloc = mng;
	r->AtomSize = AtomSize;
	r->ChunkLength = (datasize/AtomSize);
    r->Data.n = r->Data.p = &r->Data;
	r->Current = &r->Data;
	r->CurrentOffset = 0; r->length = 0;
	return r;
}

static TextLine *TextLineNew(TextLineManager *Pool,int AttrSize,int CharSize) {
	TextLine *r;
	static struct GeoTextLine Static = {
		tlLength,tlGetPlainText,tlGetAttrText,tlInsert,tlRemove,tlAttrSet
	};
	r = tlcAlloc(Pool);
	r->GeoTextLine.Static = &Static;
	r->Pool = Pool;
	r->Layout = 0; r->AttrSize = AttrSize; r->CharSize = CharSize;
	tcmChainFill(&r->Data,Pool->Chunks,AttrSize+CharSize);
	r->n.p = r->n.n = &r->n;
	return r;
}

/*----------*/

static TextChunkManager *tcmNew(int BlockSize) {
	MemStack *Mem;
	TextChunkManager *r;
	Mem = rFork(2048);
	rPush(r);
	r->Mem = Mem;
	r->BlockSize = BlockSize;
	r->Free.n = r->Free.p = &r->Free;
	return r;
}

static TextLineManager *tclmNew(TextChunkManager *Chunks) {
	TextLineManager *r;
	mPush(Chunks->Mem,r);
	r->Mem = Chunks->Mem;
	r->Chunks = Chunks;
	r->Free.n = r->Free.p = &r->Free;
	return r;
}

/*_____________________________________
 |
 |
 |_____________________________________
*/

typedef struct {
	GeoText GeoText;
	TextLineManager *Pool;
	dList/*<TextLine.n>*/ Data;
	int LineNb;
	int AttrSize,CharSize;
} Text;

static int txtLineNb(GeoText *this) { 
	ThisToThat(Text,GeoText);
	return that->LineNb;
}
static GeoTextLine *txtGetLine(GeoText *this,int *Layout,int num) {
	dList *l;
	GeoTextLine *r;
	ThisToThat(Text,GeoText);
	r = &GeoTextLineNull;
	*Layout = 0;
	if (num<0) num = 0;
	if (num>=that->LineNb) num = 0;
	l = that->Data.n;
	while (num) { l=l->n; num--; /*ToDo: Do better than that !!!! */}
    if (l!=&that->Data) {
		TextLine *R;
		R = CastBack(TextLine,n,l);
		r = &R->GeoTextLine;
		*Layout = R->Layout;
	}
	return r;

}
static void txtInsert(GeoText *this,int num,int nb,int Layout) {
	ThisToThat(Text,GeoText);
	if (nb>0) {
	    dList *ipt,*elt;
	    if (num<0) num = 0;
	    if (num>that->LineNb) num = that->LineNb;
		that->LineNb += nb;
	    ipt = &that->Data; while (num) { ipt=ipt->n; num--; }
	    while (nb) {
	        TextLine *t;
            t = TextLineNew(that->Pool,that->AttrSize,that->CharSize);
		    t->Layout = Layout;
		    elt = &t->n;
		    elt->p = ipt; elt->n = ipt->n; elt->p->n = elt->n->p = elt;
		    nb--;
	    }
	}
}
static void txtRemove(GeoText *this,int num,int nb) {
	ThisToThat(Text,GeoText);
	if (num<0) { nb+=num; num = 0; }
	if (num+nb>=that->LineNb) nb = that->LineNb-num;
	if (nb>0) {
	    dList *ipt,*elt;
		that->LineNb -= nb;
		ipt = &that->Data; while (num) { ipt=ipt->n; num--; }
		while (nb) {
			TextLine *t;
			elt = ipt->n;
			t = CastBack(TextLine,n,elt);
			elt->n->p = elt->p; elt->p->n = elt->n; elt->p = elt->n = elt;
			tlcFree(t);
			nb--;
		}
	}
}
static void txtLayoutSet(GeoText *this,int num,int nb,int Layout) {
	ThisToThat(Text,GeoText);
	if (num<0) { nb+=num; num=0; }
	if (num+nb>=that->LineNb) nb = that->LineNb-num;
	if (nb>0) {
		dList *elt;
		elt = that->Data.n; while (num) { elt=elt->n; num--; }
		while (nb) {
			TextLine *t;
			t = CastBack(TextLine,n,elt);
            t->Layout = Layout;
			elt = elt->n; nb--;
		}
	}
}

static GeoText *geoTextNew(TextLineManager *Pool,int AttrSize,int CharSize) {
	Text *r;
	static struct GeoText Static = {
		txtLineNb,txtGetLine,txtInsert,txtRemove,txtLayoutSet
	};
	rPush(r);
	r->GeoText.Static = &Static;
	r->Pool = Pool;
	r->AttrSize = AttrSize;
	r->CharSize = CharSize;
	r->LineNb = 0;
	r->Data.n = r->Data.p = &r->Data;
    return &r->GeoText;
}

/*__________________________________
 |
 |
 |__________________________________
*/

GeoText *GeoTextEdit(int AttrWidth,int TextWidth) {
	return geoTextNew(tclmNew(tcmNew(2048)),AttrWidth,TextWidth);
}




