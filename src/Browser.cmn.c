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

/*_____________________________________________________________________________
 |
 | BrwPos
 |_____________________________________________________________________________
*/

static BrwPos *PosNullDuplicate(BrwPos *this) { return this; }
static void PosNullMove(BrwPos *this,int dMantisse,int dLg2exp) {}
static int PosNullPartVal(BrwPos *this,void *b,void *e,int idx) { return -idx; }
static struct BrwPos PosNullStatic = {PosNullDuplicate,PosNullMove,PosNullPartVal};
BrwPos BrwPosNull = {&PosNullStatic};

/*------------*/

typedef struct {
	BrwPos BrwPos;
	unsigned int Val;
} Int32Pos;
static BrwPos *Int32Duplicate(BrwPos *this) {
	ThisToThat(Int32Pos,BrwPos);
	return (BrwPosInt32(that->Val,0));
}
static void Int32Move(BrwPos *this,int dM,int dXp) {
	ThisToThat(Int32Pos,BrwPos);
	that->Val = (that->Val+(dM<<dXp)) & 0xffffffff;
}

#define DataPartCpy(b,e,idx,v) { \
	unsigned char *q,*eq,*p,*ep;\
	q = b; eq = e; \
	b = &(v); e = (&(v))+1; \
	p = b; ep = e; p+=idx; \
	while ((q<eq)&&(p<ep)) { *q++ = *p++; } \
	e = ep; b = p;\
}

static int Int32PartVal(BrwPos *this,void *b,void *e,int idx) {
	ThisToThat(Int32Pos,BrwPos);
    DataPartCpy(b,e,idx,that->Val);
	return e-b;
}
BrwPos *BrwPosInt32(unsigned int dM,int dXp) {
	Int32Pos *r;
	static struct BrwPos Static = { Int32Duplicate,Int32Move,Int32PartVal };
    r->BrwPos.Static = &Static;
	r->Val = (dM<<dXp) & 0xffffffff;
	return &r->BrwPos;
}

/*------*/
static BrwPos *IntDuplicate(BrwPos *this) {
	ThisToThat(Int32Pos,BrwPos);
	return BrwPosInt(that->Val);
}
static void IntMove(BrwPos *this,int dM,int dXp) {
	ThisToThat(Int32Pos,BrwPos);
	that->Val += (dM<<dXp);
}
static int IntPartVal(BrwPos *this,void *b,void *e,int idx) {
	ThisToThat(Int32Pos,BrwPos);
    DataPartCpy(b,e,idx,that->Val);
	return e-b;
}
BrwPos *BrwPosInt(int oM) {
	Int32Pos *r;
	static struct BrwPos Static = {IntDuplicate,IntMove,IntPartVal};
    r->BrwPos.Static = &Static;
	r->Val = oM;
	return &r->BrwPos;
}

/*------*/

typedef struct {
	BrwPos BrwPos;
	long long Val;
} Int64Pos;
static BrwPos *Int64Duplicate(BrwPos *this) {
	BrwPos *r;
	unsigned int v;
	ThisToThat(Int64Pos,BrwPos);
	v = (that->Val) & 0xffffffff;
	r = BrwPosInt64(v,0);
    Call(r,Move,2((that->Val)>>32,32));
	return r;
}
static void Int64Move(BrwPos *this,int dM,int dXp) {
	long long dm;
	ThisToThat(Int64Pos,BrwPos);
	dm = dM; dm = dm<<dXp;
	that->Val += dm;
}
static int Int64PartVal(BrwPos *this,void *b,void *e,int idx) {
	ThisToThat(Int64Pos,BrwPos);
    DataPartCpy(b,e,idx,that->Val);
	return e-b;
}
BrwPos *BrwPosInt64(unsigned int dM,int dXp) {
	Int64Pos *r;
	long long val;
	static struct BrwPos Static = { Int64Duplicate,Int64Move,Int64PartVal };
	r->BrwPos.Static = &Static;
	val = dM; val = val<<dXp; r->Val = val;
	return &r->BrwPos;
}

/*_____________________________________________________________________________
 |
 | This file must be included from the architecture dependant Browser.c
 |_____________________________________________________________________________
*/

void *BrwEof = &BrwEof;

/*-----------------*/

static BrwPos *SerialNullGetPos(BrwRSerial *this,int From) { return &BrwPosNull; }
static void SerialNullSeek(BrwRSerial *this,BrwPos *Idx) {}
static void *SerialNullRead(BrwRSerial *this,void *b,void *e) { return &BrwEof; }
static struct BrwRSerial BrwRSerialStatic = {SerialNullGetPos,SerialNullSeek,SerialNullRead};
BrwRSerial BrwRSerialNull = {&BrwRSerialStatic};

/*-----------------*/

static void *StreamNullRead(BrwStream *this,void *b,void *e) { return &BrwEof;}
static struct BrwStream BrwStreamStatic = {StreamNullRead};
BrwStream BrwStreamNull = {&BrwStreamStatic};

static void StreamCtrlNullClose(BrwStreamCtrl *this) { }
static struct BrwStreamCtrl StreamCtrlNullStatic = {&StreamCtrlNullClose};
BrwStreamCtrl BrwStreamCtrlNull = {{&BrwStreamStatic},&StreamCtrlNullStatic};

/*_____________________________________________________________________________
 |
 | Stream
 |_____________________________________________________________________________
*/

typedef struct {
	BrwStream BrwStream;
	BrwRSerial *Src;
} SerialStream;

static void *SerialStreamRead(BrwStream *this,void *b,void *e) {
	ThisToThat(SerialStream,BrwStream);
	return Call(that->Src,Read,2(b,e));
}
BrwStream *BrwSerialStream(BrwRSerial *s) {
	SerialStream *r;
	static struct BrwStream Static = {SerialStreamRead};
	rPush(r);
	r->BrwStream.Static = &Static;
	r->Src = s;
	return &r->BrwStream;
}

/*_____________________________________________________________________________
 |
 |
 |_____________________________________________________________________________
*/

void BrwRSerialClose(BrwRSerial *S) {
	BrwPos *p;
	char dummy[16];
	void *d,*e;
	d = dummy;
	pOpen
	    pIn(p = Call(S,GetPos,1(BrwPosEnd)));
	    Call(S,Seek,1(p));
		e = Call(S,Read,2(d,d+16));
        if (e!=0)  {  Call(S,Read,2(d,d+16)); }
	pClose
}

  /*--------*/

typedef struct {
	BrwRSerial BrwRSerial;
	void *b,*e,*c;
} RawSerial;

static BrwPos *RawGetPos(BrwRSerial *this,int From) {
	int r;
	ThisToThat(RawSerial,BrwRSerial);
	if (From==BrwPosCur) {
		if (that->c>that->e) {
			r = that->e-that->b;
		} else {
		    r = that->c-that->b;
		}
	} else {
		if (From==BrwPosSet) {
			r = 0;
		} else {	
			r = that->e-that->b;
		}
	}
	return BrwPosInt(r);
}
static void RawSeek(BrwRSerial *this,BrwPos *Idx) {
	int l;
	ThisToThat(RawSerial,BrwRSerial);
	Call(Idx,PartVal,3(&l,(&l)+1,0));
	that->c = that->b + l;
}
static void *RawRead(BrwRSerial *this,void *Q,void *eQ) {
	unsigned char *r,*p,*q,*ep,*eq;
	ThisToThat(RawSerial,BrwRSerial);
	p = that->c; ep = that->e; q = Q; eq = eQ;
	r = q;
	if ((eq>q)&&(ep>p)) {
		if ((ep-p)>(eq-q)) { ep = p+(eq-q); }
		while (p<ep) { *q++ = *p++; }
		r = q; that->c = p;
	} else {
		if ((eq>q)||(p>ep)) { r = BrwEof; that->c = ep+1; }
	}
	return r;
}
BrwRSerial *BrwRSerialRaw(void *b,void *e) {
	RawSerial *r;
	static struct BrwRSerial Static = {RawGetPos,RawSeek,RawRead};
	rPush(r);
	r->BrwRSerial.Static = &Static;
	r->c = r->b; r->b = b; r->e = e;
	return &r->BrwRSerial;
}

/*_____________________________________________________________________________
 |
 | Random
 |_____________________________________________________________________________
*/

static unsigned int lrand0(unsigned int *s) {
	unsigned int i;
	i = *s;
	i = ((1103515245*i)+12345); // Unix
	*s = i;
	return i&0x7fffffff;
}
static unsigned int lrand1(unsigned int *s) {
	unsigned int i;
	i = *s;
	i = ((1664525*i)+1013904223); // Numerical receipe
	*s = i;
	return i&0xffffffff;
}

typedef struct {
	BrwPos BrwPos;
    unsigned int Seed;
	unsigned int Last,Last0;
	int LastBit;
} randSerial;

static BrwPos *randSerialDuplicate(BrwPos *this) {
	randSerial *r;
	ThisToThat(randSerial,BrwPos);
	rPush(r);
	*r = *that;
	return &r->BrwPos;
}
static void randSerialMove(BrwPos *this,int dM,int dLg) { }
static int randSerialPartVal(BrwPos *this,void *b,void *e,int idx) {
	ThisToThat(randSerial,BrwPos);
	DataPartCpy(b,e,idx,that->Seed);
	return e-b;
}

typedef struct {
	BrwRSerial BrwRSerial;
	randSerial State;
} RandSerial;

static BrwPos *RandomGetPos(BrwRSerial *this,int From) {
	BrwPos *r;
	ThisToThat(RandSerial,BrwRSerial);
	r = Call(&that->State.BrwPos,Duplicate,0);
	return r;
}
static void RandomSeek(BrwRSerial *this,BrwPos *Idx) {
	randSerial *state;
	ThisToThat(RandSerial,BrwRSerial);
	if (Idx->Static == that->State.BrwPos.Static) {
	    state = CastBack(randSerial,BrwPos,Idx);
	    that->State = *state;
	}
}
static void *RandomRead0(BrwRSerial *this,void *b,void *E) {
	unsigned int *p,*e;
	int l;
	unsigned int s,s0;
	ThisToThat(RandSerial,BrwRSerial);
    p = b; e = E;
	l = that->State.LastBit;
	s = that->State.Last; s0 = that->State.Last0;
	while (p<e) {
		while ((p<e)&&(l>=8)) {
			*p++ = s&0xff; s = ((s>>8)|s0); l-=8;
		}
		if (p<e) {
			s0 = (s>>1)<<24;
			s = (s<<31)|lrand0(&that->State.Seed);
			l += 31;
		}
	}
	that->State.Last = s; that->State.Last0 = s0;
	that->State.LastBit = l;
	return e;
}
static void *RandomRead1(BrwRSerial *this,void *b,void *E) {
	unsigned int *p,*e;
	int l;
	unsigned int s;
	ThisToThat(RandSerial,BrwRSerial);
    p = b; e = E;
	l = that->State.LastBit;
	s = that->State.Last;
	while (p<e) {
		while ((p<e)&&(l>=8)) {
			*p++ = s&0xff; s = s>>8; l-=8;
		}
		if (p<e) {
			s = lrand1(&that->State.Seed);
			l = 32;
		}
	}
	that->State.Last = s;
	that->State.LastBit = l;
	return e;
}

BrwRSerial *BrwSerialRandom(unsigned int Seed) {
	RandSerial *r;
	static struct BrwRSerial Static = {
        RandomGetPos,RandomSeek,RandomRead0
	};
	static struct BrwPos PosStatic = {
		randSerialDuplicate,randSerialMove,randSerialPartVal
	};
	rPush(r);
	r->BrwRSerial.Static = &Static;
	r->State.BrwPos.Static = &PosStatic;
	r->State.Seed = Seed;
	r->State.Last = r->State.Last0 = 0;
	r->State.LastBit = 0;
	return &r->BrwRSerial;
}

BrwRSerial *BrwSerialRandom1(unsigned int Seed) {
	RandSerial *r;
	static struct BrwRSerial Static = {
        RandomGetPos,RandomSeek,RandomRead1
	};
	static struct BrwPos PosStatic = {
		randSerialDuplicate,randSerialMove,randSerialPartVal
	};
	rPush(r);
	r->BrwRSerial.Static = &Static;
	r->State.BrwPos.Static = &PosStatic;
	r->State.Seed = Seed;
	r->State.Last = r->State.Last0 = 0;
	r->State.LastBit = 0;
	return &r->BrwRSerial;
}

    /*--------*/

typedef struct {
	BrwPos BrwPos;
	unsigned long long Last,LastMax;
	BrwPos *Src;
} diceRandom;
static BrwPos *diceRandomDuplicate(BrwPos *this) {
	diceRandom *r;
	ThisToThat(diceRandom,BrwPos);
    rPush(r);
	*r = *that;
	r->Src = Call(that->Src,Duplicate,0);
	return &r->BrwPos;
}
static void diceRandomMove(BrwPos *this,int dM,int dXp) {}
static int diceRandomPartVal(BrwPos *this,void *b,void *e,int idx) {  return e-b; }

typedef struct {
	BrwRSerial BrwRSerial;
	BrwRSerial *Src;
	int Sides,SrcCap;
	diceRandom State;
} DiceRandom;
static BrwPos *uDiceGetPos(BrwRSerial *this,int From) {
	ThisToThat(DiceRandom,BrwRSerial);
	return Call(&that->State.BrwPos,Duplicate,0);
}
static void uDiceSetPos(BrwRSerial *this,BrwPos *Pos) {
	ThisToThat(DiceRandom,BrwRSerial);
    if (Pos->Static == that->State.BrwPos.Static) {
	    diceRandom *p;
	    p = CastBack(diceRandom,BrwPos,Pos);
	    Call(that->Src,Seek,1(p->Src));
	    that->State = *p;
	}
}

static void *ucDiceRead(BrwRSerial *this,void *b,void *E) {
	unsigned int *p,*e,sides,base;
	unsigned long long l,lmax;
	ThisToThat(DiceRandom,BrwRSerial);
	p = b; e = E;
	l = that->State.Last;
	lmax = that->State.LastMax;
	sides = that->Sides;
	base = that->SrcCap;
	while (p<e) {
		int cr,ovf;
		if (!that->Sides) {
			sides = *p; 
			while (((p+1)<e)&&(sides<2)) { *p++=0; sides = *p; }
			if (sides<2) sides= 1;
		}
        while (lmax<sides) {
			unsigned int v[1];
			Call(that->Src,Read,2(v,v+1));
			lmax = lmax*base;
			l = (l*base)+v[0];
		}
		cr = lmax%sides; ovf = lmax-cr;
		if (l<ovf) {
			*p++ = l%sides;
			lmax = lmax/sides;
            l = l/sides;
		} else {
			lmax = cr;
			l = l%sides;
		}
	}
	that->State.Last = l;
	that->State.LastMax = lmax;
	return e;
}

BrwRSerial *BrwCappedDice(int Sides,int RndCap,BrwRSerial *Rnd) {
	DiceRandom *r;
	static struct BrwRSerial Static = {
		uDiceGetPos,uDiceSetPos,ucDiceRead
	};
	static struct BrwPos PosStatic = {
		diceRandomDuplicate,diceRandomMove,diceRandomPartVal
	};
	rPush(r);
	r->BrwRSerial.Static = &Static;
	r->Sides = Sides;
	r->Src = Rnd;
	r->SrcCap = RndCap;
	r->State.BrwPos.Static = &PosStatic;
	r->State.Last = 0;
	r->State.LastMax = 1;
	return &r->BrwRSerial;
}

BrwRSerial *BrwMixedDice(int RndCap,BrwRSerial *Rnd) { return BrwCappedDice(0,RndCap,Rnd); }

    /*--------*/

typedef struct {
	BrwPos BrwPos;
	int idx,ShuffleNb,SrcDup;
	unsigned char *bShuffle;
	BrwPos *Src;
} diceShuffle;
static BrwPos *diceShuffleDuplicate(BrwPos *this) {
	diceShuffle *r;
	unsigned char *p,*q,*e;
	int shsz;
	ThisToThat(diceShuffle,BrwPos);
	rPush(r);
	r->BrwPos.Static = this->Static;
	r->idx = that->idx;
	r->ShuffleNb = that->ShuffleNb;
	shsz = TlsRoundUp(int,that->ShuffleNb);
	rnPush(r->bShuffle,shsz);
	p = that->bShuffle; e = p+that->ShuffleNb; q = r->bShuffle; while (p<e) { *q++ = *p++; }
	if (that->SrcDup) {
		r->Src = Call(that->Src,Duplicate,0);
	} else {
		r->Src = that->Src;
	}
	r->SrcDup = (0==0);
	return &r->BrwPos;
}
static void diceShuffleMove(BrwPos *this,int dMantisse,int dLg2exp) { }
static int diceShufflePartVal(BrwPos *this,void *b,void *e,int idx) { return e-b; }

typedef struct {
	BrwRSerial BrwRSerial;
	BrwRSerial *Src;
	diceShuffle State;
} DiceShuffle;

static BrwPos *DiceShuffleGetPos(BrwRSerial *this,int From) {
	ThisToThat(DiceShuffle,BrwRSerial);
	that->State.SrcDup = (0!=0);
	that->State.Src = Call(that->Src,GetPos,1(From));
	that->State.SrcDup = (0==0);
	return Call(&that->State.BrwPos,Duplicate,0);
}
static void DiceShuffleSetPos(BrwRSerial *this,BrwPos *Pos) {
	diceShuffle *st;
	unsigned char *p,*q,*e;
	ThisToThat(DiceShuffle,BrwRSerial);
	if (Pos->Static == that->State.BrwPos.Static) {
	    st = CastBack(diceShuffle,BrwPos,Pos);
		if (st->ShuffleNb == that->State.ShuffleNb) {
			that->State.idx = st->idx;
	        p = that->State.bShuffle; e = p+st->ShuffleNb; q = st->bShuffle;
	        while (p<e) { *p++ = *q++; }
	        Call(that->Src,Seek,1(st->Src));
		}
	}
}
static void *DiceShuffleRead(BrwRSerial *this,void *b,void *E) {
	unsigned int *p,*e;
	unsigned char *s;
	int idx;
	ThisToThat(DiceShuffle,BrwRSerial);
	Call(that->Src,Read,2(b,E));
	p = b; e = E;
	idx = that->State.idx;
	s = that->State.bShuffle;
	while (p<e) {
		unsigned int lst,i;
		i = *p;
		lst = s[i]; s[i] = s[idx]; s[idx] = lst;
		*p++ = lst;
		idx++; if (idx>=that->State.ShuffleNb) idx = 0;
	}
	that->State.idx = idx;
	return e;
}

BrwRSerial *BrwShuffledDice(unsigned char *bDistr,unsigned char *eDistr,int RndCap,BrwRSerial *Rnd) {
	DiceShuffle *r;
	int shnb,shsz;
	unsigned char *p,*q,*e;
	static struct BrwRSerial Static = {
		DiceShuffleGetPos,DiceShuffleSetPos,DiceShuffleRead
	};
	static struct BrwPos PosStatic = {
		diceShuffleDuplicate,diceShuffleMove,diceShufflePartVal
	};
	rPush(r);
	r->BrwRSerial.Static = &Static;
	shsz = shnb = eDistr-bDistr;
    r->State.ShuffleNb = shnb;
	shsz = TlsRoundUp(int,shnb);
	rnPush(r->State.bShuffle,shsz);
	r->State.BrwPos.Static = &PosStatic;
	r->State.idx = 0;
    r->State.SrcDup = (0!=0);
	p = bDistr; e = eDistr; q = r->State.bShuffle; while (p<e) { *q++=*p++; }
	if (shnb!=RndCap) {
	    r->Src = BrwCappedDice(shnb,RndCap,Rnd);
	} else {
		r->Src = Rnd;
	}
	return &r->BrwRSerial;
}

/*_____________________________________________________________________________
 |
 | BrwType
 |_____________________________________________________________________________
*/

static int TypeNullIsLeaf(BrwType *this) {return (0==0); }
static BrwNod *TypeNullFields(BrwType *this) { return &BrwNodNull; }
static struct BrwType TypeNullStatic = {TypeNullIsLeaf,TypeNullFields};
BrwType BrwTypeNull = {&TypeNullStatic};

/*_____________________________________________________________________________
 |
 | BrwNode
 |_____________________________________________________________________________
*/

static int NodNullEntriesNb(BrwNod *this) { return 0; }
static void NodNullBrowse(BrwNod *this,BrwNodEntries *Tgt,int bi,int e) {}
static BrwNod *NodNullEntry(BrwNod *this,BrwType **type,char *Label) { return &BrwNodNull; }
static BrwRSerial *NodNullLeaf(BrwNod *this,BrwType **type,char *Label) { return &BrwRSerialNull;}
static struct BrwNod NodNullStatic = {NodNullEntriesNb,NodNullBrowse,NodNullEntry,NodNullLeaf};
BrwNod BrwNodNull = {&NodNullStatic};

