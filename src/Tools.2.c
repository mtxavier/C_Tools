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

/*_____________________________________________
 |
 | Circle Buffer
 |_____________________________________________
*/

void *TlsCBfWrite(TlsCircleBuffer *dst,void *sb,void *se) {
	char *p,*q,*eq,*eeq;
	int al,sl,bl;
	bl = TlsCBfMaxContent(dst);
	al = TlsCBfOccupied(dst);
	sl = (se-sb); if ((al+sl)>bl) { sl = bl-al; se=sb+sl; }
	p = sb;
	q = dst->eElt; eq = eeq = q+sl;
	if (eq>=dst->eBuff) { eq = dst->eBuff; }
	while (q<eeq) {
		while (q<eq) *q++ = *p++;
        if (eq!=eeq) { q = dst->bBuff; eq = eeq-bl; eeq = eq; }
	}
	if (q==dst->eBuff) { if (sb!=se) q=dst->bBuff; }
    dst->eElt = q;
	return p;
}

void *TlsCBfRead(void *db,void *de,TlsCircleBuffer *src) {
	char *p,*q,*ep,*eep;
	int bl,al,dl;
	bl = TlsCBfMaxContent(src);
	al = TlsCBfOccupied(src);
    dl = (de-db); if (dl>al) { dl = al; de = db+dl; }
	q = db;
	p = src->bElt; ep = eep = p+dl; 
	if (ep>=src->eBuff) { ep = src->eBuff; }
	while (p<eep) {
		while (p<ep) { *q++ = *p++; }
		if (ep!=eep) { p = src->bBuff; ep = eep-bl; eep = ep; }
	}
	if (p==src->eBuff) p = src->bBuff;
	src->bElt = p;
	if (p==src->eElt) { if (db!=de) { TlsCBfClear(src); } }
	return de;
}

int TlsCBfInsert(TlsCircleBuffer *src,int pos,int nb) {
	char *p,*q,*ep,*eep,*eq,*eeq;
	int bl,al,il;
	bl = TlsCBfMaxContent(src);
	al = TlsCBfOccupied(src);
	if (pos<0) pos = 0; if (pos>al) pos = al;
	il = nb; if (il+al>bl) { il = bl-al; }
	if (pos>(al>>1)) {
		p = src->eElt; 
		ep = eep = p-(al-pos); if (ep<src->bBuff) { ep = src->bBuff; }
		q = src->eElt+il; if (q>=src->eBuff) { q -= bl; }
		if (src->eElt==src->eBuff) { if (il>0) { src->eElt = q; } } else { src->eElt = q; }
		eq = eeq = q-(al-pos); if (eq<src->bBuff) { eq = src->bBuff; }
		while ((p>eep)||(q>eeq)) {
			while ((p>ep)&&(q>eq)) { p--; q--; *q = *p; }
			if ((p==ep)&&(ep!=eep)) { p = src->eBuff; ep = eep+bl; eep = ep; }
			if ((q==eq)&&(eq!=eeq)) { q = src->eBuff; eq = eeq+bl; eeq = eq; }
		}
	} else {
		p = src->bElt;
		ep = eep = p + pos; if (ep>=src->eBuff) { ep = src->eBuff; }
		q = src->bElt - il; if (q<src->bBuff) { q += bl; }
		src->bElt = q; if ((il>0)&&(src->eElt==src->eBuff)) { src->eElt = src->bBuff; }
		eeq = eq = q + pos; if (eq>=src->eBuff) { eq = src->eBuff; }
		while ((p<eep)||(q<eeq)) {
			while ((p<ep)&&(q<eq)) { *q++=*p++; }
			if ((p==ep)&&(ep!=eep)) { p = src->bBuff; ep = eep-bl; eep = ep; }
			if ((q==eq)&&(eq!=eeq)) { q = src->bBuff; eq = eeq-bl; eeq = eq; }
		}
	}
	return nb-il;
}


int TlsCBfRemove(TlsCircleBuffer *src,int pos,int nb) {
	char *p,*q,*ep,*eep,*eq,*eeq;
	int al,bl,rl,ll;
	bl = TlsCBfMaxContent(src);
	al = TlsCBfOccupied(src);
	if (pos<0) pos = 0; if (pos>al) pos = al;
	rl = nb; if (pos+rl>al) { rl = (al-pos); }
	if (rl>=al) {
		TlsCBfClear(src);
	} else {
	    if (pos>((al-rl)>>1)) {
	        ll = al-(pos+rl);
		    q = src->bElt+pos; if (q>=src->eBuff) { q -= bl; }
		    eq = eeq = q+ll; if (eq>=src->eBuff) { eq=src->eBuff;}
		    p = q + rl; if (p>=src->eBuff) { p -= bl;}
		    ep = eep = p+ll; if (ep>src->eBuff) { ep = src->eBuff; }
		    while ((p<eep)||(q<eeq)) {
			    while ((p<ep)&&(q<eq)) { *q++ = *p++; }
			    if ((p==ep)&&(ep!=eep)) { p = src->bBuff; ep=eep-bl; eep=ep; }
			    if ((q==eq)&&(eq!=eeq)) { q = src->bBuff; eq=eeq-bl; eeq=eq; }
		    }
		    src->eElt = (eeq==src->eBuff)?src->bBuff:eeq;
	    } else {
		    q = src->bElt+pos+rl; if (q>src->eBuff) { q-=bl; }
			eq = eeq = q-pos; if (eq<src->bBuff) { eq = src->bBuff; }
			p = q-rl; if (p<=src->bBuff) { p+=bl; }
			ep = eep = p-pos; if (ep<src->bBuff) { ep = src->bBuff; }
			while ((p>eep)||(q>eeq)) {
				while ((p>ep)&&(q>eq)) { q--; p--; *q=*p; }
				if ((p==ep)&&(ep!=eep)) { p = src->eBuff; ep=eep+bl; eep = ep; }
				if ((q==eq)&&(eq!=eeq)) { q = src->eBuff; eq=eeq+bl; eeq = eq; }
			}
			src->bElt = (eeq==src->eBuff)?src->bBuff:eeq;
	    }
	}
	return nb-rl;
}

void *TlsCBfOverwrite(TlsCircleBuffer *dst,int pos,void *sb,void *se) {
	char *p,*q,*eq,*eeq;
	int bl,al;
	bl = TlsCBfMaxContent(dst);
	al = TlsCBfOccupied(dst);
	if ((pos+(se-sb))>al) { se = sb+(al-pos); }
	p = sb;
	q = dst->bElt+pos; if (q>=dst->eBuff) { q-=bl; }
	eq = eeq = q + (se-sb); if ( eq>dst->eBuff ) { eq = dst->eBuff; }
	while (q<eeq) {
		while (q<eq) { *q++ = *p++; }
		if (eq!=eeq) { q = dst->bBuff; eeq-=bl; eq=eeq; }
	}
	return se;
}

void *TlsCBfGet(void *db,void *de,TlsCircleBuffer *src,int pos) {
	char *p,*q,*ep,*eep;
	int bl,al;
	bl = TlsCBfMaxContent(src);
	al = TlsCBfOccupied(src);
	if ((pos+(de-db))>al) { de = db+(al-pos); } if (de<db) { de=db; } 
	q = db;
	p = src->bElt+pos; if (p>=src->eBuff) { p-=bl; }
	eep = ep = p+(de-db); if (ep>src->eBuff) { ep=src->eBuff; }
	while (p<eep) {
		while (p<ep) { *q++ = *p++; }
		if (p!=eep) { p=src->bBuff; eep -= bl; ep = eep; }
	}
	return q;
}

int TlsCBfFill(TlsCircleBuffer *dst,int bpos,int epos,void *fb,void *fe) {
	int bl,al;
	char *p,*q,*ep,*eq,*eeq;
	bl = TlsCBfMaxContent(dst);
	al = TlsCBfOccupied(dst);
	if (bpos<0) bpos = 0; if (epos>al) epos = al;
	if (bpos<epos) {
		q = dst->bElt+bpos;  if (q>=dst->eBuff) { q -= bl; }
		eq = eeq = q + (epos-bpos); if (eq>dst->eBuff) { eq=dst->eBuff; }
		p = fb; ep = fe;
		while (q<eeq) {
			while ((q<eq)&&(p<ep)) { *q++ = *p++; }
			if (p==ep) { p = fb; }
			if ((q==eq)&&(eq!=eeq)) { q = dst->bBuff; eeq -= bl; eq = eeq; }
		}
	}
	return epos;
}

void TlsCBfCpy(TlsCircleBuffer *dst,int db,int de,TlsCircleBuffer *src,int sb) {
	int dbl,sbl,dal,bal;
	char *p,*q,*ep,*eep,*eq,*eeq;
	dbl = TlsCBfMaxContent(dst);
	sbl = TlsCBfMaxContent(src);
	q = dst->bElt+db; while (q>=dst->eBuff) q-=dbl; 
	eeq = q+(de-db); eq = (eeq>dst->eBuff)?dst->eBuff:eeq; 
	p = src->bElt+sb; while (p>=src->eBuff) p-=sbl;
	ep = src->eBuff;
	while (q<eeq) {
		while ((q<eq)&&(p<ep)) { *q++=*p++; }
		if (q>=eq) { q=dst->bBuff; eeq-= dbl; eq=eeq; if (eq>dst->eBuff) eq=dst->eBuff;  }
		if (p>=ep) { p=src->bBuff; }
	}
}

int TlsCBf_RequiredMem(int Size) {
	void *b,*e;
	int as,aas,is;
	TlsCircleBuffer *b0;
	b = 0; e = b;
    b0 = b; e = b0+1;
	as = Size;
	is = TlsAtomSize(int);
	aas = (as+is-1)&(-is);
    e = e + aas;
	return e-b;
}

TlsCircleBuffer *TlsCBf_MemFormat(void *chunk,int Size) {
	TlsCircleBuffer *dst;
	int as;
	char *buff;
	dst = chunk; chunk=dst+1;
	buff = chunk;
	as = Size; 
	dst->bBuff = buff; dst->eBuff=buff+as;
	TlsCBfClear(dst);
	return dst;
}

void TlsCBfAlloc(TlsCircleBuffer *dst,int Size) {
	int as,aas,is;
	char *buff;
	is = TlsAtomSize(int);
	as = Size; aas = (as+is-1)&(-is);
	rnPush(buff,aas);
	dst->bBuff = buff; dst->eBuff=buff+as;
	TlsCBfClear(dst);
}

TlsCircleBuffer *TlsCBfNew(int Size) {
	TlsCircleBuffer *r;
	rPush(r);
	TlsCBfAlloc(r,Size);
	return r;
}

