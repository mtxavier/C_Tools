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
#include <ScrBuilder.h>

#include <List.h>
#include <Patricia.h>

/*______________________________
 |
 | Vocabulary
 |______________________________
*/

static char *VocabularyNullGetLabel(ScrVocabulary *this,char *Label) { return Label;}
static char *VocabularyNullInternalLabel(ScrVocabulary *this,int Num) { return "";}
static char *VocabularyNullCheckLabel(ScrVocabulary *this,char *Label) { return 0;}
static struct ScrVocabulary ScrVocabularyNullStatic = {
	VocabularyNullGetLabel,VocabularyNullInternalLabel,VocabularyNullCheckLabel
};
ScrVocabulary ScrVocabularyNull = {&ScrVocabularyNullStatic};

/*-------------*/

typedef struct {
	ScrVocabulary ScrVocabulary;
	MemStack *NodMem,*WordMem;
	PatriciaNod AllWords;
} Vocabulary;
static int StrSize(char *s) { char *p; p = s; while (*p) p++; return 1+(p-s); }
static char *StrDuplicate(char *s) {
	char *p,*q,*r;
	int siz;
	siz = StrSize(s);
	rnPush(r,siz);
	p = s; q = r;
   	while (*p) { *q++=*p++; } 
	*q = 0;
	return r;
}
static char *VocabularyGetLabel(ScrVocabulary *this,char *Label) {
	PatriciaNod *p;
	ThisToThat(Vocabulary,ScrVocabulary);
	p = PatriciaSeek(&that->AllWords,Label);
	if (p==&that->AllWords) {
		char *nl;
        mIn(that->WordMem,nl = StrDuplicate(Label));
		mPush(that->NodMem,p);
		PatriciaSeekOrInsert(&that->AllWords,nl,p);
	}
    return p->Key;
}
static char *VocabularyInternalLabel(ScrVocabulary *this,int Num) {
	unsigned int n;
	char Val[3*sizeof(int)],*p;
	n = Num;
	p = Val; *p++ = '#';
	while (n) {
		int c;
		c = n&0x3f; n = n>>6;
		if (c<36) {
			if (c<10) { *p++ = c+'0'; } else { *p++ = c+'a'-10; }
		} else {
			if (c<62) { *p++ = c+'A'-36; } else { *p++ = (c==62)?'_':'#'; }
		}
	}
    *p = 0;
	return Call(this,GetLabel,1(Val));
}
static char *VocabularyCheckLabel(ScrVocabulary *this,char *Label) {
	PatriciaNod *p;
	ThisToThat(Vocabulary,ScrVocabulary);
	p = PatriciaSeek(&that->AllWords,Label);
	return (p==&that->AllWords)?0:p->Key;
}
ScrVocabulary *ScrVocabularyNew(void) {
	Vocabulary *r;
	MemStack *WordMem,*NodMem;
	static struct ScrVocabulary Static = {
		VocabularyGetLabel,VocabularyInternalLabel,VocabularyCheckLabel
	};
	NodMem = rFork(8*sizeof(PatriciaNod));
	WordMem = rFork(128);
	rPush(r);
	r->ScrVocabulary.Static = &Static;
	r->NodMem = NodMem;
	r->WordMem = WordMem;
	PatriciaInit(&r->AllWords);
	return &r->ScrVocabulary;
}

    /*--------------*/

typedef struct {
	ScrVocabulary ScrVocabulary;
	ScrVocabulary *Base,*Spec;
} VocJargon;
static char *JargonGetLabel(ScrVocabulary *this,char *Label) {
	char *r;
	ThisToThat(VocJargon,ScrVocabulary);
	r = Call(that->Base,CheckLabel,1(Label));
	if (!r) { r = Call(that->Spec,GetLabel,1(Label)); }
	return r;
}
static char *JargonInternalLabel(ScrVocabulary *this,int Num) {
	ThisToThat(VocJargon,ScrVocabulary);
	return Call(that->Spec,InternalLabel,1(Num));
}
static char *JargonCheckLabel(ScrVocabulary *this,char *Label) {
	char *r;
	ThisToThat(VocJargon,ScrVocabulary);
	r = Call(that->Base,CheckLabel,1(Label));
	if (!r) { r = Call(that->Spec,CheckLabel,1(Label)); }
	return r;
}
ScrVocabulary *ScrVocabularyJargon(ScrVocabulary *Language) {
	VocJargon *r;
	static struct ScrVocabulary Static = {
		JargonGetLabel,JargonInternalLabel,JargonCheckLabel
	};
	rPush(r);
	r->ScrVocabulary.Static = &Static;
	r->Base = Language;
	r->Spec = ScrVocabularyNew();
	return &r->ScrVocabulary;
}

/*____________________________
 |
 | UnkBound
 |____________________________
*/

typedef struct {
	BinTree/*<UnkBoundEltChunk.uIdx>*/ uIdx;
	void *Elts[16];
} UnkBoundEltChunk;

typedef struct {
	ScrUnkBound ScrUnkBound;
	MemStack *Mem;
	BinTree/*<UnkBoundEltChunk.uIdx>*/ Elts;
	int Size;
} UnkBound;

static int UnkBoundSize(ScrUnkBound *this) { 
	ThisToThat(UnkBound,ScrUnkBound);
	return that->Size;
}
static int UnkBoundAdd(ScrUnkBound *this,void *Elt) {
	int idx;
    UnkBoundEltChunk *chunk;
	ThisToThat(UnkBound,ScrUnkBound);
	idx = that->Size; that->Size++;
	if (!(idx&0xf)) {
        mPush(that->Mem,chunk);
		BinTreeSeekOrInsert(&that->Elts,idx>>4,&chunk->uIdx);
	} else {
		BinTree *f;
		f = BinTreeSeek(&that->Elts,idx>>4);
		chunk = CastBack(UnkBoundEltChunk,uIdx,f);
	}
	chunk->Elts[idx&0xf] = Elt;
	return idx;
}
static void *UnkBoundGet(ScrUnkBound *this,int idx) {
    UnkBoundEltChunk *c;
	BinTree *f;
	ThisToThat(UnkBound,ScrUnkBound);
	f = BinTreeSeek(&that->Elts,idx>>4);
	c = CastBack(UnkBoundEltChunk,uIdx,f);
	return c->Elts[idx&0xf];
}
static void *UnkBoundPack(ScrUnkBound *this) {
	void **r,**p,**q,**e,**ec;
	int c;
	ThisToThat(UnkBound,ScrUnkBound);
	rnPush(r,that->Size);
	p = r; e = p+that->Size; c = 0;
    while (p<e) {
		UnkBoundEltChunk *ch;
		BinTree *f;
		f = BinTreeSeek(&that->Elts,c);
		ch = CastBack(UnkBoundEltChunk,uIdx,f);
		q = ch->Elts; c++;
		ec = p+16; if (ec>e) ec = e;
		while (p<ec) { *p++ = *q++; }
	}
	mLeave(that->Mem);
	return r;
}
ScrUnkBound *ScrOneShotUnkBound(void) {
	UnkBound *r;
	MemStack *Mem;
	static struct ScrUnkBound Static = {
		UnkBoundSize,UnkBoundAdd,UnkBoundGet,UnkBoundPack
	};
	Mem = rFork(256); mEnter(Mem);
	mPush(Mem,r); r->Mem = Mem; r->ScrUnkBound.Static = &Static;
	r->Size = 0; BinTreeInit(&r->Elts);
	return &r->ScrUnkBound;
}

/*____________________________
 |
 | Catalog
 |____________________________
*/

static ScrCatalog *PackedCatalog(int EntryNb,void *Void);

static ScrCatalogEntry CatalogData(ScrCatalog *this,char *Label) {
	int num;
	ScrCatalogEntry r;
	r.Data = 0;
	num = Call(this,GetNum,1(Label));
	if (num>0) { r = Call(this,GetData,1(num)); }
	return r;
}
static ScrCatalog *CatalogPack(ScrCatalog *this) {
	ScrCatalog *r;
	int i,nb;
	nb = Call(this,EltNb,0);
	r = PackedCatalog(nb,0);
    for (i=0;i<nb;i++) {
		ScrCatalogEntry e;
		char *l;
		e = Call(this,GetData,1(i));
		l = Call(this,GetLabel,1(i));
        if (*l) { Call(r,SetLabel,2(i,l)); /* don't do this for anonymous data. */ }
		Call(r,SetData,2(i,e));
	}
	return r;
}
static ScrCatalogEntry *CatalogStripPack(ScrCatalog *this,int *EntryNb){
	ScrCatalogEntry *r,*p,*e;
	int nb,i;
	nb = Call(this,EltNb,0);
	rnPush(r,nb);
	p = r; e = p+nb; i = 0;
	while (p<e) { *p = Call(this,GetData,1(i)); p++; i++; }
	*EntryNb = nb;
	return r;
}

     /*-------------*/

typedef struct {
	BinTree/*<uCatalogEntry.Num>*/ Num;
	PatriciaNod/*<uCatalogEntry.Label>*/ Label;
	ScrCatalogEntry Data;
} uCatalogEntry;
typedef struct {
	MemStack *Mem;
	ScrCatalog ScrCatalog;
	PatriciaNod/*<uCatalogEntry.Label>*/ Label;
	BinTree/*<uCatalogEntry.Num>*/ Num;
	ScrCatalogEntry Void;
	int EntryNb;
} uCatalog;
static int UnpackEltNb(ScrCatalog *this) {
	ThisToThat(uCatalog,ScrCatalog);
	return that->EntryNb;
}
static int UnpackSetLabel(ScrCatalog *this,int Dst,char *Label) {
	int r;
	PatriciaNod *p;
	uCatalogEntry *e;
	ThisToThat(uCatalog,ScrCatalog);
	p = PatriciaSeek(&that->Label,Label);
	if ((p==&that->Label)&&(*Label)) {
		BinTree *q;
		r = Dst;
		if (r<0) r = that->EntryNb++;
		q = BinTreeSeek(&that->Num,r);
		if (q == BinTreeEnd) {
			mPush(that->Mem,e);
			e->Data = that->Void;
			BinTreeSeekOrInsert(&that->Num,r,&e->Num);
		    PatriciaSeekOrInsert(&that->Label,Label,&e->Label);
		} else {
			e = CastBack(uCatalogEntry,Num,q);
			if (!(*e->Label.Key)) {
		        PatriciaSeekOrInsert(&that->Label,Label,&e->Label);
			} else {
				r = -1;
			}
		}
	} else {
		if (*Label) {
		    e = CastBack(uCatalogEntry,Label,p);
			r = e->Num.Id;
		} else {
			r = -1;
		}
	}
	return r;
}
static int UnpackGetNum(ScrCatalog *this,char *Label) {
	int r;
	PatriciaNod *p;
	ThisToThat(uCatalog,ScrCatalog);
    p = PatriciaSeek(&that->Label,Label);
	if (p==&that->Label) {
		r = -1;
	} else {
		uCatalogEntry *P;
		P = CastBack(uCatalogEntry,Label,p);
		r = P->Num.Id;
	}
	return r;
}
static char *UnpackGetLabel(ScrCatalog *this,int Num) {
	BinTree *p;
	char *r;
	ThisToThat(uCatalog,ScrCatalog);
	p = BinTreeSeek(&that->Num,Num);
	if (p==BinTreeEnd) {
		r = "";
	} else {
        uCatalogEntry *P;
		P = CastBack(uCatalogEntry,Num,p);
		r = P->Label.Key;
	}
	return r;
}
static int UnpackSetData(ScrCatalog *this,int num,ScrCatalogEntry data) {
	BinTree *p;
	uCatalogEntry *n;
	ThisToThat(uCatalog,ScrCatalog);
	if (num<0) { num = that->EntryNb++;}  
	p = BinTreeSeek(&that->Num,num);
	if (p!=BinTreeEnd) {
		n = CastBack(uCatalogEntry,Num,p);
		n->Data = data;
	} else { // Anonymous data
		mPush(that->Mem,n);
		n->Data = data;
		n->Label.Key = "";
		BinTreeSeekOrInsert(&that->Num,num,&n->Num);
	}
	return num;
}
static ScrCatalogEntry UnpackGetData(ScrCatalog *this,int Num) {
	BinTree *p;
	ScrCatalogEntry r;
	ThisToThat(uCatalog,ScrCatalog);
	p = BinTreeSeek(&that->Num,Num);
    if (p==BinTreeEnd) {
		r = that->Void;
	} else {
		uCatalogEntry *P;
		P = CastBack(uCatalogEntry,Num,p);
		r = P->Data;
	}
	return r;
}
static void UnpackClose(ScrCatalog *this) {
	ThisToThat(uCatalog,ScrCatalog);
	that->EntryNb = 0;
	PatriciaInit(&that->Label);
	BinTreeInit(&that->Num);
	mLeave(that->Mem);
	mEnter(that->Mem);
}
ScrCatalog *ScrCatalogNew(void *Void) {
	uCatalog *r;
	MemStack *Mem;
	static struct ScrCatalog Static = {
		UnpackEltNb,
		UnpackSetLabel,UnpackGetNum,UnpackGetLabel,
		UnpackSetData,UnpackGetData,
		CatalogData,CatalogPack,CatalogStripPack,
        UnpackClose
	};
	Mem = rFork(64);
	mPush(Mem,r);
	mEnter(Mem);
	r->Mem = Mem;
	r->ScrCatalog.Static = &Static;
	r->Void.Data = Void;
	r->EntryNb = 0;
	PatriciaInit(&r->Label);
	BinTreeInit(&r->Num);
	return &r->ScrCatalog;
}

    /*------------*/

typedef struct {
	PatriciaNod/*<pCatalogEntry.Label>*/ Label;
	ScrCatalogEntry Data;
} pCatalogEntry;
typedef struct {
	ScrCatalog ScrCatalog;
	int EntryNb,EntryCount;
	pCatalogEntry *Entries;
	ScrCatalogEntry Void;
	PatriciaNod/*pCatalogEntry.Label*/ Label;
} PackedCtlg;

static int PackedEltNb(ScrCatalog *this) {
	ThisToThat(PackedCtlg,ScrCatalog);
	return that->EntryNb;
}
static int PackedSetLabel(ScrCatalog *this,int Dst,char *Label) { 
	PatriciaNod *p;
	int r;
	ThisToThat(PackedCtlg,ScrCatalog);
	p = PatriciaSeek(&that->Label,Label);
	if (p==&that->Label) {
        pCatalogEntry *P;
		P = CastBack(pCatalogEntry,Label,p);
        r = P - that->Entries;
	} else {
		if (Dst<0) { Dst=that->EntryCount++; }
		if ((Dst>=0)&&(Dst<that->EntryNb)) {
            pCatalogEntry *n;
			n = that->Entries+Dst;
			PatriciaSeekOrInsert(&that->Label,Label,&n->Label);
		} else {
			r = -1;
		}
	}
	return r;
}
static int PackedGetNum(ScrCatalog *this,char *Label) {
	PatriciaNod *p;
	int r;
	ThisToThat(PackedCtlg,ScrCatalog);
    p = PatriciaSeek(&that->Label,Label);
	if (p==&that->Label) {
		r = -1;
	} else {
		pCatalogEntry *P;
		P = CastBack(pCatalogEntry,Label,p);
		r = P - that->Entries;
	}
	return r;
}
static char *PackedGetLabel(ScrCatalog *this,int Num) {
	char *r;
	ThisToThat(PackedCtlg,ScrCatalog);
	if ((Num>=0)&&(Num<that->EntryNb)) {
        r = that->Entries[Num].Label.Key;
	} else {
		r = "";
	}
	return r;
}
static int PackedSetData(ScrCatalog *this,int num,ScrCatalogEntry data) {
	int r;
	ThisToThat(PackedCtlg,ScrCatalog);
	if (num<0) num = that->EntryCount++;
	r = (num<that->EntryNb);
    if (r) { that->Entries[num].Data=data;}
	return r;
}
ScrCatalogEntry PackedGetData(ScrCatalog *this,int num) {
	ScrCatalogEntry r;
	ThisToThat(PackedCtlg,ScrCatalog);
    if ((num>=0)&&(num<that->EntryNb)) {
        r = that->Entries[num].Data;
	} else {
		r = that->Void;
	}
	return r;
}
static void PackedClose(ScrCatalog *this) { }
static ScrCatalog *PackedCatalog(int EntryNb,void *Void) {
    PackedCtlg *r;
	pCatalogEntry *p,*e;
	static struct ScrCatalog Static = {
		PackedEltNb,
		PackedSetLabel,PackedGetNum,PackedGetLabel,
		PackedSetData,PackedGetData,
        CatalogData,CatalogPack,CatalogStripPack,
		PackedClose
	};
    rPush(r);
	r->EntryNb = EntryNb;
	r->EntryCount = 0;
	rnPush(r->Entries,EntryNb);
	r->Void.Data = Void;
	PatriciaInit(&r->Label);
	p = r->Entries;
	e = p+EntryNb;
	while (p<e) { p->Data.Data = Void; p->Label.Key = ""; p++; }
	return &r->ScrCatalog;
}

/*______________________________
 |
 | Strings
 |______________________________
*/

static int StringNullSize(ScrString *this) { return 0; }
static void StringNullCopy(ScrString *this,char *dst,int b,int e) { while (b<e) {*dst++=0; b++;} }
static struct ScrString ScrStringNullStatic = {StringNullSize,StringNullCopy};
ScrString ScrStringNull = {&ScrStringNullStatic};

typedef struct {
	ScrString ScrString;
	char *b,*e;
} DirectString;
static int DirectStringSize(ScrString *this) { ThisToThat(DirectString,ScrString); return that->e-that->b;}
static void DirectStringCopy(ScrString *this,char *dst,int b,int en) {
	char *p,*e,*ee,*q;
	ThisToThat(DirectString,ScrString);
    p = that->b+b;
	ee = that->b+en;
	q = dst;
	e = that->b; if (e>ee) e=ee;
	while (p<e) { *q++=0; p++; }
	e = that->e; if (e>ee) e=ee;
	while (p<e) { *q++=*p++; }
	while (p<ee) { *q++=0; p++; }
}
static ScrString *FixedString(char *b,char *e) {
	DirectString *r;
	static struct ScrString Static = {DirectStringSize,DirectStringCopy};
	rPush(r);
	r->ScrString.Static = &Static;
	r->b = b; r->e = e;
	return &r->ScrString;
}
ScrString *ScrStringDirect(char *txt) {
	char *e;
	e = txt; while (*e) e++; // no terminal 0
	return FixedString(txt,e);

}
    
    /*--  String Cat --*/

typedef struct {
	ScrString ScrString;
	int sa,sb;
	ScrString *a,*b;
} StringCat;
static int StringCatSize(ScrString *this) {
	ThisToThat(StringCat,ScrString);
    return that->sa+that->sb;
}
static void StringCatCopy(ScrString *this,char *dst,int be,int en) {
	ThisToThat(StringCat,ScrString);
	if (that->sa<be) {
		Call(that->b,Copy,3(dst,be-that->sa,en-that->sa));
	} else {
		if (that->sa<en) {
			Call(that->a,Copy,3(dst,be,that->sa));
			Call(that->b,Copy,3(dst+(that->sa-be),0,en-that->sa));
		} else {
			Call(that->a,Copy,3(dst,be,en));
		}
	}
}
ScrString *ScrStringCat(ScrString *a,ScrString *b) {
	StringCat *r;
	static struct ScrString Static = {StringCatSize,StringCatCopy};
	rPush(r);
	r->ScrString.Static = &Static;
	r->a = a; r->sa = Call(a,Size,0);
	r->b = b; r->sb = Call(b,Size,0);
	return &r->ScrString;
}

    /*--- String Dec Value ---*/

ScrString *ScrStringDecimal(int a) {
	char *Val,*e,*p,*q;
	int c;
	rnPush(Val,3*sizeof(int)); // 2<(ln(256)/ln(10))<3 ; we align on int
    p = Val; c = a;
    if (c<0) { *p++ = '-';  c=-c; }
	q = p;
	do { *p = '0'+(c%10); p++; c=c/10; } while(c!=0);
	e = p; p--;
	while (q<p) { int x; x=*q; *q=*p; *p=x; q++; p--; }
	return FixedString(Val,e);
}

/*_________________________________
 |
 | Arrays
 |_________________________________
*/

typedef struct {
	ScrArray ScrArray;
	ScrCatalogEntry Default;
} tArrayNull;

static void ArrayNullDim(ScrArray *this,int *min,int *max,int *EltNb){ *min =0; *max=0; *EltNb=0;}
static void ArrayNullSetDefault(ScrArray *this,ScrCatalogEntry Elt) { ThisToThat(tArrayNull,ScrArray); that->Default=Elt;}
static void ArrayNullSetElt(ScrArray *this,int num,ScrCatalogEntry Elt) {}
static ScrCatalogEntry ArrayNullGetDefault(ScrArray *this){ ThisToThat(tArrayNull,ScrArray); return that->Default;}
static ScrCatalogEntry ArrayNullGetElt(ScrArray *this,int num) { ThisToThat(tArrayNull,ScrArray); return that->Default;}
static int ArrayNullIsSet(ScrArray *this,int num) { return (0!=0); }
static void ArrayNullForEach(ScrArray *this,int(*NextElt)(int n,ScrCatalogEntry Elt,void *Clos),void *Clos) {}
static ScrArray *ArrayNullTranslate(ScrArray *this,ScrCatalogEntry def,
	ScrCatalogEntry (*Translate)(int num,ScrCatalogEntry Elt,void *Clos),void *Clos) { return this;}

ScrArray *ScrArrayNull(void *Default) {
	tArrayNull *r;
	static struct ScrArray Static = {
		ArrayNullDim,ArrayNullSetDefault,ArrayNullSetElt,ArrayNullGetDefault,ArrayNullGetElt,ArrayNullIsSet,
		ArrayNullForEach,ArrayNullTranslate
	};
	rPush(r); r->ScrArray.Static = &Static;
	r->Default.Data = Default;
	return &r->ScrArray;
}

/*------------------------*/

typedef struct {
	ScrArray ScrArray;
	ScrCatalogEntry Default,Elt;
} OneArray;

static void OneArrayDim(ScrArray *this,int *min,int *max,int *EltNb) { *min =0; *max =1; *EltNb = 1;}
static void OneArraySetDefault(ScrArray *this,ScrCatalogEntry Elt) {ThisToThat(OneArray,ScrArray); that->Default = Elt;}
static void OneArraySetElt(ScrArray *this,int num,ScrCatalogEntry Elt) {
	ThisToThat(OneArray,ScrArray);
	if (num==0) { that->Elt = Elt; }
}
static ScrCatalogEntry OneArrayGetDefault(ScrArray *this) { ThisToThat(OneArray,ScrArray); return that->Default;}
static ScrCatalogEntry OneArrayGetElt(ScrArray *this,int num) { 
	ThisToThat(OneArray,ScrArray); 
    return num?that->Default:that->Elt;
}
static int OneArrayIsSet(ScrArray *this,int num) { return (num==0); }
static void OneArrayForEach(ScrArray *this,int (*NextElt)(int num,ScrCatalogEntry Elt,void *Clos),void *Clos) {
	ThisToThat(OneArray,ScrArray); 
	NextElt(0,that->Elt,Clos);
}
static ScrArray *OneArrayTranslate(ScrArray *this,ScrCatalogEntry Default,
	ScrCatalogEntry (*TranslateElt)(int num,ScrCatalogEntry Elt,void *Clos),void *Clos) {
	ScrCatalogEntry elt;
	ThisToThat(OneArray,ScrArray);
    elt = TranslateElt(0,that->Elt,Clos);
	return ScrOneEltArray(Default.Data,elt.Data);
}
ScrArray *ScrOneEltArray(void *Default,void *Elt) {
	OneArray *r;
	static struct ScrArray Static = {
        OneArrayDim,OneArraySetDefault,OneArraySetElt,OneArrayGetDefault,
		OneArrayGetElt,OneArrayIsSet,OneArrayForEach,OneArrayTranslate
	};
	rPush(r); r->ScrArray.Static = &Static;
	r->Default.Data = Default; r->Elt.Data = Elt;
	return &r->ScrArray;
}

/*------------------------*/

struct rDefaultTranslate {
	ScrArray *Tgt;
	ScrCatalogEntry (*Translate)(int n,ScrCatalogEntry Elt,void *Clos);
	void *Clos;
};
static int rDefaultTranslate(int num,ScrCatalogEntry Elt,void *Clos) {
    struct rDefaultTranslate *clos;
	ScrCatalogEntry elt;
	clos = Clos;
    elt = clos->Translate(num,Elt,clos->Clos);
	Call(clos->Tgt,SetElt,2(num,elt));
	return (0!=0);
}
static ScrArray *DefaultTranslate(ScrArray *Array,ScrCatalogEntry Default,
		ScrCatalogEntry (*Translate)(int n,ScrCatalogEntry Elt,void *Clos),void *Clos) {
	int min,max,enb;
    struct rDefaultTranslate clos;
	clos.Translate = Translate;
	clos.Clos = Clos;
	Call(Array,Dim,3(&min,&max,&enb));
    if (enb*4>max) {
		clos.Tgt = ScrSetSizedArray(max,Default.Data);
	} else {
		clos.Tgt = ScrUnknownSizedArray(Default.Data);
	}
	return clos.Tgt;
}

/*_______________________________________
 |
 | Set size
 |_______________________________________
*/

static ScrCatalogEntry ScrCatalogEntryNull = {0};
typedef struct {
	ScrArray ScrArray;
	int Size,EltNb;
	ScrCatalogEntry *Elt;
	ScrCatalogEntry Default;
} SetSize;
static SetSize *SetSizeNew(int Size,void *Default);

static void SetSizeDim(ScrArray *this,int *min,int *max,int *EltNb) {
	ThisToThat(SetSize,ScrArray);
	*min = 0; *max=that->Size; *EltNb=that->EltNb;
}
static void SetSizeSetDefault(ScrArray *this,ScrCatalogEntry Elt) {
	ThisToThat(SetSize,ScrArray);
	that->Default = Elt;
}
static void SetSizeSetElt(ScrArray *this,int num,ScrCatalogEntry Elt) {
	ThisToThat(SetSize,ScrArray);
	if (num>=0 && num<that->Size) { 
		if(that->Elt[num].Data == &ScrCatalogEntryNull) { that->EltNb++; }
		that->Elt[num]=Elt; 
	}
}
static ScrCatalogEntry SetSizeGetDefault(ScrArray *this) {
    ThisToThat(SetSize,ScrArray);
	return that->Default;
}
static ScrCatalogEntry SetSizeGetElt(ScrArray *this,int num) {
	ScrCatalogEntry r;
	ThisToThat(SetSize,ScrArray);
	r = that->Default;
	if (num>=0 && num<that->Size) { r = that->Elt[num]; }
	if (r.Data==&ScrCatalogEntryNull) { r = that->Default; }
	return r;
}
static int SetSizeIsSet(ScrArray *this,int num) {
	int r;
	ThisToThat(SetSize,ScrArray);
	r = (num>=0 && num<that->Size);
	if (r) { r=(that->Elt[num].Data!=&ScrCatalogEntryNull);}
	return r;
}
static void SetSizeForEach(ScrArray *this,int (*NextElt)(int num,ScrCatalogEntry Elt,void *Clos),void *Clos) {
	ScrCatalogEntry *p,*e;
	int i;
	ThisToThat(SetSize,ScrArray);
    p = that->Elt; e = p+that->Size; i = 0;
	while (p<e) { 
	    if (p->Data!=&ScrCatalogEntryNull) { 
			if (NextElt(i,*p,Clos)) {p=e-1;}
	    }
	    p++; i++;
	}
}
static ScrArray *SetSizeTranslate(ScrArray *this,ScrCatalogEntry Default,
		ScrCatalogEntry (*Translate)(int num,ScrCatalogEntry Elt,void *Clos),void *Clos) {
	SetSize *r;
	ScrCatalogEntry *p,*e,*q;
	int i;
	ThisToThat(SetSize,ScrArray);
	r = SetSizeNew(that->Size,0);
	r->Default = Default;
	p = r->Elt; e = p+that->Size; i=0;
	q = that->Elt;
	while (p<e) {
		if (q->Data!=&ScrCatalogEntryNull) {
		    *p = Translate(i,*q,Clos);
			that->EltNb++;
		} else {
			*p = *q;
		}
		i++; p++; q++; 
	}
	return &r->ScrArray;
}
static SetSize *SetSizeNew(int Size,void *Default) {
	SetSize *r;
	static struct ScrArray Static = {
		SetSizeDim, SetSizeSetDefault, SetSizeSetElt, SetSizeGetDefault, SetSizeGetElt, 
		SetSizeIsSet, SetSizeForEach, SetSizeTranslate
	};
	rPush(r); r->ScrArray.Static = &Static;
	r->Size = Size;
	rnPush(r->Elt,Size);
	if (Default) {
		ScrCatalogEntry *p,*e;
        p = r->Elt; e = p+Size;
		while (p<e) { p->Data=&ScrCatalogEntryNull; p++; }
	}
	r->EltNb = 0;
	r->Default.Data = Default;
	return r;
}
ScrArray *ScrSetSizedArray(int Size,void *Default) { return &SetSizeNew(Size,Default)->ScrArray; }

/*------------------------*/

typedef struct {
	BinTree/*<UnknownSizeElt.Num>*/ Num;
	ScrCatalogEntry Elt;
} UnknownSizeElt;

typedef struct {
	MemStack *Mem;
	ScrArray ScrArray;
	int Min,Max,EltNb;
	BinTree/*<UnknownSizeElt.Num>*/ Elts;
	ScrCatalogEntry Default;
} UnknownSize;
static void UnknownDim(ScrArray *this,int *min,int *max,int *EltNb) {
	ThisToThat(UnknownSize,ScrArray);
	*min = that->Min; *max = that->Max; *EltNb = that->EltNb;
}
static void UnknownSetDefault(ScrArray *this,ScrCatalogEntry Elt) {
	ThisToThat(UnknownSize,ScrArray);
	that->Default = Elt;
}
static void UnknownSetElt(ScrArray *this,int num,ScrCatalogEntry Elt) {
	BinTree *B;
	UnknownSizeElt *b;
	ThisToThat(UnknownSize,ScrArray);
    B = BinTreeSeek(&that->Elts,num);
	if (B!=BinTreeEnd) {
		b = CastBack(UnknownSizeElt,Num,B);
		b->Elt = Elt;
	} else {
		mPush(that->Mem,b);
		b->Elt = Elt;
		BinTreeSeekOrInsert(&that->Elts,num,&b->Num);
		if (!that->EltNb) { that->Min=that->Max=num;}
		that->EltNb++;
		if (num>that->Max) that->Max = num;
		if (num<that->Min) that->Min = num;
	}
}
static ScrCatalogEntry UnknownGetDefault(ScrArray *this) {
	ThisToThat(UnknownSize,ScrArray);
    return that->Default;
}
static ScrCatalogEntry UnknownGetElt(ScrArray *this,int num) {
	BinTree *B;
	ScrCatalogEntry r;
	ThisToThat(UnknownSize,ScrArray);
	r = that->Default;
	B = BinTreeSeek(&that->Elts,num);
	if (B!=BinTreeEnd) {
		UnknownSizeElt *b;
		b = CastBack(UnknownSizeElt,Num,B);
		r = b->Elt;
	}
    return r;
}
static int UnknownIsSet(ScrArray *this,int num) {
	BinTree *B;
	ThisToThat(UnknownSize,ScrArray);
	B = BinTreeSeek(&that->Elts,num);
	return (B!=BinTreeEnd);
}

struct rUnknownForEach { 
	void *Closure;
    int (*NextElt)(int num,ScrCatalogEntry Elt,void *clos);
};
static int rUnknownForEach(BinTree *B,void *Clos) {
	UnknownSizeElt *b;
	struct rUnknownForEach *clos;
	b = CastBack(UnknownSizeElt,Num,B);
	clos = Clos;
	return clos->NextElt(B->Id,b->Elt,clos->Closure);
}
static void UnknownForEach(ScrArray *this,int (*NextElt)(int num,ScrCatalogEntry Elt,void *Clos),void *Clos) {
	struct rUnknownForEach clos;
	ThisToThat(UnknownSize,ScrArray);
	clos.Closure = Clos;
	clos.NextElt = NextElt;
	BinTreeForEach(&that->Elts,rUnknownForEach,&clos);
}
struct rUnknownTranslate {
	ScrArray *Tgt;
	void *clos;
	ScrCatalogEntry (*Translate)(int num,ScrCatalogEntry Elt,void *Clos);
	ScrCatalogEntry Default;
};
int rUnknownTranslate(BinTree *B,void *Clos) {
	UnknownSizeElt *b;
	struct rUnknownTranslate *clos;
	clos = Clos; b = CastBack(UnknownSizeElt,Num,Clos);
	Call(clos->Tgt,SetElt,2(B->Id,clos->Translate(B->Id,b->Elt,clos->clos)));
	return (0!=0);
}
static ScrArray *UnknownTranslate(ScrArray *this,ScrCatalogEntry Default,
			ScrCatalogEntry (*Translate)(int num,ScrCatalogEntry Elt,void *Clos),void *Clos) {
	struct rUnknownTranslate clos;
	ThisToThat(UnknownSize,ScrArray);
	if (that->EltNb*4>=that->Max) { // UnknownSizeElt takes 4 times as much space as a pointer
		clos.Tgt = ScrSetSizedArray(that->Max,Default.Data);
	} else { // The array is mostly empty.
	    clos.Tgt = ScrUnknownSizedArray(Default.Data);
	}
    clos.Default = Default;
	clos.clos = Clos;
	BinTreeForEach(&that->Elts,rUnknownTranslate,&clos);
	return clos.Tgt;
}
ScrArray *ScrUnknownSizedArray(void *Default) {
	UnknownSize *r;
	MemStack *Mem;
	static struct ScrArray Static = {
		UnknownDim,UnknownSetDefault,UnknownSetElt,UnknownGetDefault,
		UnknownGetElt,UnknownIsSet,UnknownForEach,UnknownTranslate
	};
	Mem = rFork(256);
	mPush(Mem,r); r->ScrArray.Static = &Static; r->Mem = Mem;
	r->Min = r->Max = r->EltNb = 0;
	r->Default.Data = Default;
	BinTreeInit(&r->Elts);
	return &r->ScrArray;
}

/*_______________
 |
 | Array cat
 |_______________
*/

typedef struct {
	ScrArray ScrArray;
	ScrArray *A,*B;
	int AMax;
	ScrCatalogEntry Default;
} ArrayCat;
static void ArrayCatDim(ScrArray *this,int *min,int *max,int *EltNb) {
	int amin,amax,anb,bmin,bmax,bnb;
	ThisToThat(ArrayCat,ScrArray);
	Call(that->A,Dim,3(&amin,&amax,&anb));
	Call(that->B,Dim,3(&bmin,&bmax,&bnb));
	if (anb && bnb) {
	    *min = amin; *max = amax+bmax; *EltNb = anb+bnb;
	} else {
		if (anb) {
			*min = amin; *max = amax; *EltNb = anb;
		} else {
			*min = bmin; *max = bmax; *EltNb = bnb;
		}
    }
}
static void ArrayCatSetDefault(ScrArray *this,ScrCatalogEntry Elt) {
	ThisToThat(ArrayCat,ScrArray);
	that->Default = Elt;
}
static void ArrayCatSetElt(ScrArray *this,int num,ScrCatalogEntry Elt) {
	ThisToThat(ArrayCat,ScrArray);
	if (num>=that->AMax) {
		Call(that->B,SetElt,2(num-that->AMax,Elt));
	} else {
		Call(that->A,SetElt,2(num,Elt));
	}
}
static ScrCatalogEntry ArrayCatGetDefault(ScrArray *this){ 
	ThisToThat(ArrayCat,ScrArray);
	return that->Default;
}
static ScrCatalogEntry ArrayCatGetElt(ScrArray *this,int num) {
	ScrCatalogEntry r;
	ThisToThat(ArrayCat,ScrArray);
	if (num>=that->AMax) {
	    r = Call(that->B,GetElt,1(num-that->AMax));
	} else {
	    r = Call(that->A,GetElt,1(num));
	}
	return r;
}
static int ArrayCatIsSet(ScrArray *this,int num) {
	int r;
	ThisToThat(ArrayCat,ScrArray);
	if (num>=that->AMax) {
		r = Call(that->B,IsSet,1(num-that->AMax));
	} else {
	    r = Call(that->A,IsSet,1(num));
	}
	return r;
}
struct ArrayCatForEach {
	int AMax;
	void *Clos;
	int (*NextElt)(int num,ScrCatalogEntry Elt,void *clos);
};
static int rArrayCatForEach(int num,ScrCatalogEntry Elt,void *Clos) {
	struct ArrayCatForEach *clos;
	clos = Clos;
	return clos->NextElt(num+clos->AMax,Elt,clos->Clos);
}
static void ArrayCatForEach(ScrArray *this,int (*NextElt)(int num,ScrCatalogEntry Elt,void *clos),void *Clos) {
	struct ArrayCatForEach clos;
	ThisToThat(ArrayCat,ScrArray);
	clos.AMax=that->AMax;
	clos.NextElt = NextElt;
	clos.Clos = Clos;
	Call(that->A,ForEach,2(NextElt,Clos));
	Call(that->B,ForEach,2(rArrayCatForEach,&clos));
}
struct ArrayCatTranslate {
	int AMax;
	void *Clos;
	ScrArray *Tgt;
	ScrCatalogEntry (*Translate)(int num,ScrCatalogEntry Elt,void *Clos);
};
static int rArrayCatTranslate(int n,ScrCatalogEntry Elt,void *Clos) {
	struct ArrayCatTranslate *clos;
	ScrCatalogEntry elt;
	clos = Clos;
    elt = clos->Translate(n,Elt,clos->Clos);
	Call(clos->Tgt,SetElt,2(n+clos->AMax,elt));
	return (0!=0);
}
static ScrArray *ArrayCatTranslate(ScrArray *this,ScrCatalogEntry Default,
	ScrCatalogEntry (*Translate)(int n,ScrCatalogEntry Elt,void *clos),void *Clos) {
	int min,max,nb;
	struct ArrayCatTranslate clos;
	ThisToThat(ArrayCat,ScrArray);
    clos.AMax = that->AMax;
	clos.Clos = Clos;
	clos.Translate = Translate;
	Call(this,Dim,3(&min,&max,&nb));
	if ((nb*4)>=max) {
		clos.Tgt = ScrSetSizedArray(max,Default.Data);
	} else {
	    clos.Tgt = ScrUnknownSizedArray(Default.Data);
	}
	clos.AMax = 0;
	Call(that->A,ForEach,2(rArrayCatTranslate,&clos));
	clos.AMax = that->AMax;
	Call(that->B,ForEach,2(rArrayCatTranslate,&clos));
	return clos.Tgt;
}
ScrArray *ScrConcatArray(ScrArray *A,ScrArray *B) {
	ArrayCat *r;
	int amax,amin,anb;
	static struct ScrArray Static = {
		ArrayCatDim,ArrayCatSetDefault,ArrayCatSetElt,ArrayCatGetDefault,
		ArrayCatGetElt,ArrayCatIsSet,ArrayCatForEach,ArrayCatTranslate
	};
	rPush(r); r->ScrArray.Static = &Static;
	r->A = A; r->B = B;
	Call(A,Dim,3(&amin,&amax,&anb));
	r->Default = Call(B,GetDefault,0);
	r->AMax = amax;
	return &r->ScrArray;
}

/*__________________________________________
 |
 | Altered array: when A elt is not set
 | we take B elt instead.
 |__________________________________________
*/

typedef struct {
	ScrArray ScrArray;
	ScrArray *A,*B;
	int EltNb;
} AlteredArray;
static void AlteredArrayDim(ScrArray *this,int *min,int *max,int *EltNb) {
	int amin,amax,anb,bmin,bmax,bnb;
	ThisToThat(AlteredArray,ScrArray);
	Call(that->A,Dim,3(&amin,&amax,&anb));
	Call(that->B,Dim,3(&bmin,&bmax,&bnb));
	if (anb && bnb) {
	    *min = (amin<bmin)?amin:bmin;
	    *max = (amax>bmax)?amax:bmax;
	    *EltNb = that->EltNb;
	} else {
		if (anb) {
			*min = amin; *max = amax; *EltNb = anb;
		} else {
			*min = bmin; *max = bmax; *EltNb = bnb;
		}
	}
}
static void AlteredArraySetDefault(ScrArray *this,ScrCatalogEntry Elt) {
	ThisToThat(AlteredArray,ScrArray);
	Call(that->A,SetDefault,1(Elt));
}
static void AlteredArraySetElt(ScrArray *this,int num,ScrCatalogEntry Elt) {
	int elt,n;
	ThisToThat(AlteredArray,ScrArray);
	n = Call(that->A,IsSet,1(num));
	if (!n) { if (!Call(that->B,IsSet,1(num))) { that->EltNb++;} }
	Call(that->A,SetElt,2(num,Elt));
}
static ScrCatalogEntry AlteredArrayGetDefault(ScrArray *this) {
	ThisToThat(AlteredArray,ScrArray)
	return Call(that->A,GetDefault,0);
}
static ScrCatalogEntry AlteredArrayGetElt(ScrArray *this,int num) {
	ScrCatalogEntry r;
	ThisToThat(AlteredArray,ScrArray);
	if (Call(that->A,IsSet,1(num))) {
        r = Call(that->A,GetElt,1(num));
	} else {
		r = Call(that->B,GetElt,1(num));
	}
	return r;
}
static int AlteredArrayIsSet(ScrArray *this,int num) {
	int r;
	ThisToThat(AlteredArray,ScrArray);
	r = Call(that->B,IsSet,1(num));
	if (!r) r = Call(that->A,IsSet,1(num));
	return r;
}
struct AlteredArrayForEach {
	ScrArray *A;
	int (*NextElt)(int num,ScrCatalogEntry Elt,void *Clos);
	void *Clos;
};
static int AlteredLeftForEach(int n,ScrCatalogEntry Elt,void *Clos) {
	struct AlteredArrayForEach *clos;
	int r;
	clos = Clos;
    r = !Call(clos->A,IsSet,1(n));
	if (r) { r = clos->NextElt(n,Elt,clos->Clos); }
    return r;
}
static void AlteredArrayForEach(ScrArray *this,int (*NextElt)(int num,ScrCatalogEntry Elt,void *Clos),void *Clos) {
	struct AlteredArrayForEach clos;
	ThisToThat(AlteredArray,ScrArray);
	clos.A = that->A;
	clos.NextElt = NextElt; clos.Clos = Clos;
	Call(that->A,ForEach,2(NextElt,Clos));
	Call(that->B,ForEach,2(AlteredLeftForEach,&clos));
}
static ScrArray *AlteredArrayTranslate(ScrArray *this,ScrCatalogEntry Default,
	ScrCatalogEntry (*Translate)(int num,ScrCatalogEntry Elt,void *Clos),void *Clos) {
	return DefaultTranslate(this,Default,Translate,Clos);
}
static int AlteredArrayCount(int n,ScrCatalogEntry Elt,void *Clos) {
	int *clos;
    clos = Clos; *clos++;
	return (0!=0);
}
ScrArray *ScrAlteredArray(ScrArray *A,ScrArray *B) {
	AlteredArray *r;
	static struct ScrArray Static = {
		AlteredArrayDim, AlteredArraySetDefault, AlteredArraySetElt, AlteredArrayGetDefault,
		AlteredArrayGetElt, AlteredArrayIsSet, AlteredArrayForEach, AlteredArrayTranslate
	};
	rPush(r); r->ScrArray.Static = &Static;
	r->A = A; r->B = B; r->EltNb = 0;
	Call(&r->ScrArray,ForEach,2(AlteredArrayCount,&r->EltNb));
	return &r->ScrArray;
}

/*__________________________________________
 |
 | RegularArray : Elt given by a function
 |__________________________________________
*/

typedef struct {
	ScrArray ScrArray;
	int Size;
	ScrCatalogEntry Default;
	void *(*Elt)(int idx,void *Clos);
	void *Clos;
} RegularArray;
static void RegularDim(ScrArray *this,int *min,int *max,int *EltNb) {
	ThisToThat(RegularArray,ScrArray);
	*min = 0; *max = that->Size; *EltNb = that->Size;
}
static void RegularSetDefault(ScrArray *this,ScrCatalogEntry Elt) {
	ThisToThat(RegularArray,ScrArray);
	that->Default = Elt;
}
static void RegularSetElt(ScrArray *this,int num,ScrCatalogEntry Elt) { /* Error: values are fixed */ }
static ScrCatalogEntry RegularGetDefault(ScrArray *this) {
	ThisToThat(RegularArray,ScrArray);
	return that->Default;
}
static ScrCatalogEntry RegularGetElt(ScrArray *this,int num) {
	ScrCatalogEntry r;
	ThisToThat(RegularArray,ScrArray);
	r = that->Default;
	if ((num>=0) && ((num<that->Size)||(that->Size<0))) {
		r.Data = that->Elt(num,that->Clos);
	}
	return r;
}
static int RegularIsSet(ScrArray *this,int num) {
	ThisToThat(RegularArray,ScrArray);
    return ((num>=0)&&((num<that->Size)||(that->Size<0)));
}
static void RegularForEach(ScrArray *this,int (*NextElt)(int num,ScrCatalogEntry Elt,void *Clos),void *Clos) {
	int cont,i,e;
	ThisToThat(RegularArray,ScrArray);
	i = 0; e = that->Size;
	cont = (i<e)||(e<0);
	while (cont) {
		ScrCatalogEntry d;
		d.Data = that->Elt(i,that->Clos);
		cont = NextElt(i,d,Clos);
		if (cont) {i++; cont = (i<e)||(e<0);}
	}
}

struct RegularTranslate {
	ScrCatalogEntry (*Translate)(int num,ScrCatalogEntry Elt,void *Clos);
	void *TranslateClos;
	void *(*ModeleElt)(int idx,void *Clos);
	void *ModeleClos;
};
static void *RegularTranslateElt(int Idx,void *Clos) {
	ScrCatalogEntry d;
	struct RegularTranslate *clos;
	clos = Clos;
	d.Data = clos->ModeleElt(Idx,clos->ModeleClos);
	return clos->Translate(Idx,d,clos->TranslateClos).Data;
}
static ScrArray *RegularTranslate(ScrArray *this,ScrCatalogEntry Default,
		ScrCatalogEntry (*TranslateElt)(int num,ScrCatalogEntry Elt,void *Clos),void *Clos
	) {
	struct RegularTranslate *clos;
	ThisToThat(RegularArray,ScrArray);
	rPush(clos);
	clos->Translate = TranslateElt;
	clos->TranslateClos = Clos;
	clos->ModeleElt = that->Elt;
	clos->ModeleClos = that->Clos;
	return ScrRegularArray(Default.Data,that->Size,RegularTranslateElt,clos);
}

ScrArray *ScrRegularArray(void *Default,int Size,void *(*Elt)(int idx,void *Clos),void *Clos) {
	RegularArray *r;
	static struct ScrArray Static = {
		RegularDim,RegularSetDefault,RegularSetElt,RegularGetDefault,RegularGetElt,RegularIsSet,
		RegularForEach,RegularTranslate
	};
	rPush(r); r->ScrArray.Static = &Static;
	r->Default.Data = Default; r->Size = Size; r->Elt = Elt; r->Clos = Clos;
	return &r->ScrArray;
}



