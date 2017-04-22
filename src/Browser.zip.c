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
#include <Browser.h>
#include <zlib.h>

typedef struct {
	TlsMemPool *Pool;
	int BufEnd;
	BrwStreamCtrl *Src;
	z_stream State;
	void *eBuf,*p,*ep,*bBuf[1024];
} StreamState;

typedef struct {
	BrwStreamCtrl BrwStreamCtrl;
	MemStack *Mem;
	StreamState *State;
} Stream;

/*__________________________________________________
 |
 | Providing our own alloc/free primitives allow
 | us to savagely leave the process without having
 | to properly clean all the mess. If that does
 | ever happen, the standard procedure would be to
 | simply unstack and free the whole memory branch.
 |__________________________________________________
*/
static void *MyAlloc(void *this,unsigned int items,unsigned int size) {
	StreamState *that;
	TlsMemPool *p,**r;
	int sz,psz,psz0,i;
	that = this;
	sz = (items*size)+TlsAtomSize(TlsMemPool *);
	psz = sz; psz0 = 0; i=1; while (psz!=psz0) { psz0 = psz; psz = psz|(psz>>i); i=i<<1; }
	psz++; 
	if (psz<32) {
		psz = 32;
	} else {
		psz0 = psz>>1; 
		i = (psz0+psz)>>1; if (sz<=i) { psz = i; } else { psz0 = i; }
		i = (psz0+psz)>>1; if (sz<=i) { psz = i; }
		if (sz<=psz0) psz = psz0;
	}
	p = Call(that->Pool,SizeSelect,1(psz));
    r = Call(p,Alloc,0);
	*r = p;
	return r+1;
}
static void MyFree(void *this,void *data) {
	TlsMemPool **pp,*p;
	pp = data; p = pp[-1];
	Call(p,Free,1(data));
}

static void *zipRead(BrwStream *this,void *b,void *e) {
	void *p;
	ThisToThat(Stream,BrwStreamCtrl.BrwStream);
    if (that->State) {
		int end,flush,res;
		StreamState *s;
		z_stream *z;
		s = that->State;
		z = &s->State;
        p = b; end =(0!=0);
		while ((!end)&&(p!=e)) {
			while ((s->p==s->ep)&&(!s->BufEnd)) {
                s->p = s->bBuf;
				s->ep = Call(&s->Src->BrwStream,Read,2(s->p,s->eBuf));
				s->BufEnd = (s->ep==BrwEof);
				if (s->BufEnd) s->ep = s->p;
			}
			flush = Z_NO_FLUSH;
			if (s->p!=s->ep) {
			    z->next_in = s->p;
				z->avail_in = s->ep-s->p;
			} else {
				z->next_in = Z_NULL;
                z->avail_in = 0;
			}
			z->next_out = p;
			z->avail_out = e-p;
			res = inflate(z,flush);
			p = z->next_out;
			if (!s->BufEnd) s->p = z->next_in;
			end = (res!=Z_OK);
		}
		if (end) {
            Call(s->Src,Close,0);
            inflateEnd(z);
			mLeave(that->Mem);
			that->State = 0;
		}
	} else {
		p = &BrwEof;
	}
	return p;
}

static void zipClose(BrwStreamCtrl *this) {
	ThisToThat(Stream,BrwStreamCtrl);
	if (that->State) {
		Call(that->State->Src,Close,0);
		inflateEnd(&(that->State->State));
		mLeave(that->Mem);
		that->State = 0;
	}
}

BrwStreamCtrl *BrwSerialUnziped(BrwStreamCtrl *Org) {
	static struct BrwStream Static = {zipRead};
	static struct BrwStreamCtrl CtrlStatic = {zipClose};
	Stream *r;
	StreamState *s;
    z_stream *z;
	rPush(r);
	r->BrwStreamCtrl.BrwStream.Static = &Static;
	r->BrwStreamCtrl.Static = &CtrlStatic;
	r->Mem = rFork(256);
	mEnter(r->Mem);
	mPush(r->Mem,r->State);
	mIn(r->Mem,r->State->Pool = TlsMemPoolNew(1024));
	s = r->State;
	s->eBuf = r->State->bBuf+1024;
	s->Src = Org;
	s->ep = s->p = s->bBuf;
	s->BufEnd = (0!=0);
	z = &s->State;
	z->next_in = Z_NULL;
	z->avail_in = 0;
	z->total_in = 0;
	z->next_out = 0;
	z->avail_out = 0;
	z->total_out = 0;
	z->zalloc = MyAlloc;
	z->zfree = MyFree;
	z->opaque = s;
	if (inflateInit(z)!=Z_OK) {
		Call(&r->BrwStreamCtrl,Close,0);
	}
	return &r->BrwStreamCtrl;
}

