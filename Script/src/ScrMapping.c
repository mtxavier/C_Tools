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
#include <ScrDataExpr.h>
#include <List.h>
#include <Builder.Loc.h>


/*____________________________
 |
 | Mem Mapping
 |____________________________
*/


typedef struct { struct ScrMapType *Static; } ScrMapType;
extern ScrMapType ScrMapTypeNull;
struct ScrMapType {
	int (*Size)(ScrMapType *this,int *align);
	ScrVar *(*Map)(ScrMapType *this,void *Data);
};

/*----------------------*/

static int MapTypeNullSize(ScrMapType *this,int *align) { *align = 1; return 0;}
static ScrVar *MapTypeNullMap(ScrMapType *this,void *Data) { return &ScrVarNull; }
static struct ScrMapType MapTypeNullStatic = {MapTypeNullSize,MapTypeNullMap};
ScrMapType ScrMapTypeNull = {&MapTypeNullStatic};

/*________________________
 |
 | Values
 |________________________
*/

typedef struct {
	ScrDataVal ScrDataVal;
	ScrVar ScrVar;
	void *Val;
} MapVar;

static ScrDataVal *MapVarBrowse(ScrDataVal *this,ScrDataBrowser *Tgt) { return &ScrDataValNull; }
static ScrDataVal *MapVarEval(ScrDataVal *this,ScrDataCtx *Ctx,ScrVar *Result,ScrDataVal *Param) { return this; }
static int MapVarInt(ScrDataVal *this) { return 0;}
static ScrString *MapVarString(ScrDataVal *this) { return 0;}
static ScrPath *MapVarPath(ScrDataVal *this) { return &ScrPathNull; }
static ScrVar *MapVarPtrTgt(ScrDataVal *this) { return &ScrVarNull; }
static ScrArray *MapVarArray(ScrDataVal *this) { return ScrArrayNull(&ScrVarNull);}
static ScrDataVal *MapVarGetField(ScrDataVal *this,int Idx) { return this;}

static ScrDataVal *MapVarValue(ScrVar *this) {
	ThisToThat(MapVar,ScrVar);
	return &that->ScrDataVal;
}
static ScrVar *MapVarElt(ScrVar *this,int idx) { return this; }
static void MapVarAddress(ScrVar *this,ScrVar **Base,int *Idx) { 
	ThisToThat(MapVar,ScrVar);
	*Base = &ScrVarNull;
	*Idx = ((that->Val)-((void *)0));
}
static void MapVarSet(ScrVar *this,ScrDataVal *Value) {}

/*------*/

static void MapIntSet(ScrVar *this,ScrDataVal *Value) {
	int *v;
	ThisToThat(MapVar,ScrVar);
	v = that->Val;
	*v = Call(Value,Int,0);
}
static int MapIntInt(ScrDataVal *this) {
	int *v;
	ThisToThat(MapVar,ScrDataVal);
	v = that->Val;
	return *v;
}

static int MapTypeIntSize(ScrMapType *this,int *align) { *align=sizeof(int); return sizeof(int); }
static ScrVar *MapTypeIntMap(ScrMapType *this,void *Data) {
	MapVar *r;
	static struct ScrVar VarStatic = { MapVarValue,MapVarElt,MapVarAddress,MapIntSet };
	static struct ScrDataVal DataStatic = {
		MapVarBrowse,MapVarEval,MapIntInt,MapVarString,MapVarPath,MapVarPtrTgt,MapVarArray,MapVarGetField
	};
	rPush(r); r->ScrVar.Static = &VarStatic;
	r->ScrDataVal.Static = &DataStatic;
	r->Val = Data;
	return &r->ScrVar;
}
static struct ScrMapType MapTypeIntStatic = {MapTypeIntSize,MapTypeIntMap};
static ScrMapType ScrMapTypeInt = {&MapTypeIntStatic};


/*--------------*/

static void MapCharSet(ScrVar *this,ScrDataVal *Value){
	char *v;
	ThisToThat(MapVar,ScrVar);
	v = that->Val;
	*v = Call(Value,Int,0);
}
static int MapCharInt(ScrDataVal *this) {
	char *v;
	ThisToThat(MapVar,ScrDataVal);
	v = that->Val;
	return *v;
}
static int MapTypeCharSize(ScrMapType *this,int *align) { *align = (sizeof(char)); return (sizeof(char));}
static ScrVar *MapTypeCharMap(ScrMapType *this,void *data) {
	MapVar *r;
	static struct ScrVar VarStatic = { MapVarValue,MapVarElt,MapVarAddress,MapCharSet };
	static struct ScrDataVal DataStatic = {
		MapVarBrowse,MapVarEval,MapCharInt,MapVarString,MapVarPath,MapVarPtrTgt,MapVarArray,MapVarGetField
	};
	rPush(r); r->ScrVar.Static = &VarStatic;
	r->ScrDataVal.Static = &DataStatic;
	r->Val = data;
	return &r->ScrVar;
}
static struct ScrMapType MapTypeCharStatic = {MapTypeCharSize,MapTypeCharMap};
static ScrMapType ScrMapTypeChar = {&MapTypeCharStatic};

/*---------------*/

static void MapStringSet(ScrVar *this,ScrDataVal *Value) {
	ScrString **v;
	ThisToThat(MapVar,ScrVar);
	v = that->Val;
	*v = Call(Value,String,0);
}
static ScrString *MapStringString(ScrDataVal *this){
	ScrString **v;
	ThisToThat(MapVar,ScrVar);
	v = that->Val;
	return *v;
}
static int MapTypeStringSize(ScrMapType *this,int *align) { *align=sizeof(ScrString *); return (sizeof(ScrString *));}
static ScrVar *MapTypeStringMap(ScrMapType *this,void *data) {
	MapVar *r;
	static struct ScrVar VarStatic = { MapVarValue,MapVarElt,MapVarAddress,MapStringSet };
	static struct ScrDataVal DataStatic = {
		MapVarBrowse,MapVarEval,MapVarInt,MapStringString,MapVarPath,MapVarPtrTgt,MapVarArray,MapVarGetField
	};
	rPush(r); r->ScrVar.Static = &VarStatic;
	r->ScrDataVal.Static = &DataStatic; r->Val = data;
	return &r->ScrVar;
}
static struct ScrMapType MapTypeStringStatic = {MapTypeStringSize,MapTypeStringMap};
static ScrMapType ScrMapTypeString = {&MapTypeStringStatic};

/*--------------*/

typedef struct {
	MapVar MapVar;
	ScrMapType *on;
} MapPtr;
static void MapPtrSet(ScrVar *this,ScrDataVal *Val) {
	ScrVar *Base;
	int idx;
	void **v;
	ThisToThat(MapPtr,MapVar.ScrVar);
	Base = Call(Val,PtrTgt,0);
	Call(Base,Address,2(&Base,&idx));
	v = that->MapVar.Val;
	*v = ((void *)0)+idx;
}
static ScrVar *MapPtrPtrTgt(ScrDataVal *this) {
	void **v;
	ThisToThat(MapPtr,MapVar.ScrVar);
	v = that->MapVar.Val;
	return Call(that->on,Map,1(*v));
}

typedef struct {
	ScrMapType ScrMapType;
	ScrMapType *on;
} MapTypePtr;
static int MapTypePtrSize(ScrMapType *this,int *align) { *align = sizeof(void*); return sizeof(void*);}
static ScrVar *MapTypePtrMap(ScrMapType *this,void *Data) {
	MapPtr *r;
	static struct ScrVar VarStatic = { MapVarValue,MapVarElt,MapVarAddress,MapPtrSet };
	static struct ScrDataVal DataStatic = {
		MapVarBrowse,MapVarEval,MapVarInt,MapVarString,MapVarPath,MapPtrPtrTgt,MapVarArray,MapVarGetField
	};
	ThisToThat(MapTypePtr,ScrMapType);
	rPush(r); r->MapVar.ScrVar.Static = &VarStatic;
	r->MapVar.ScrDataVal.Static = &DataStatic; r->MapVar.Val = Data;
	r->on = that->on;
	return &r->MapVar.ScrVar;
}

static ScrMapType *ScrMapTypePtr(ScrMapType *On) {
	MapTypePtr *r;
	static struct ScrMapType Static = {MapTypePtrSize,MapTypePtrMap};
	rPush(r); r->ScrMapType.Static = &Static; r->on = On;
	return &r->ScrMapType;
}

/*--------------*/

typedef struct {
	MapVar MapVar;
	ScrArray *Values;
	ScrMapType *on;
	int eltsize;
} MapArray;
static void *MapArrayEltVal(int idx,void *Closure) {
	ScrVar *v;
	MapArray *that;
	that = Closure;
    v = Call(that->on,Map,1(that->MapVar.Val+(that->eltsize*idx)));
	return Call(v,Value,0);
}
static int ArrayEltSet(int num,ScrCatalogEntry Elt,void *Clos) {
	MapArray *that;
	ScrDataVal *val;
	ScrVar *v;
	that = Clos; val = Elt.Data;
	rOpen
		v = Call(that->on,Map,1(that->MapVar.Val+(that->eltsize*num)));
	    Call(v,Set,1(val));
	rClose
	return (0!=0);
}
static ScrVar *MapArrayElt(ScrVar *this,int idx) {
	ThisToThat(MapArray,MapVar.ScrVar);
	return Call(that->on,Map,1(that->MapVar.Val+(that->eltsize*idx)));
}
static void MapArraySet(ScrVar *this,ScrDataVal *data) {
	ScrArray *Array;
	ThisToThat(MapArray,MapVar.ScrVar);
	Array = Call(data,Array,0);
	rOpen
	    Call(Array,ForEach,2(ArrayEltSet,that));
	rClose
}
static ScrArray *MapArrayArray(ScrDataVal *this) {
	ThisToThat(MapArray,MapVar.ScrDataVal);
	return that->Values;
}
static ScrDataVal *MapArrayGetField(ScrDataVal *this,int idx) {
	ScrDataVal *Val;
	ThisToThat(MapArray,MapVar.ScrDataVal);
	Val = Call(that->Values,GetElt,1(idx)).Data;
	return Val;
}

typedef struct {
	ScrMapType ScrMapType;
	ScrMapType *on;
	int nb;
} MapTypeArray;
static int MapArraySize(ScrMapType *this,int *align) {
	int size,al;
	ThisToThat(MapTypeArray,ScrMapType);
	size = Call(that->on,Size,1(align));
	al = *align;
    if (that->nb>1) { size = ((size+(al-1))&(-al)); }
	return that->nb*size;
}
static ScrVar *MapArrayMap(ScrMapType *this,void *Data) {
	MapArray *r;
	int size,align;
	static struct ScrVar VarStatic = { MapVarValue,MapArrayElt,MapVarAddress,MapArraySet};
	static struct ScrDataVal ValStatic = {
		MapVarBrowse,MapVarEval,MapVarInt,MapVarString,MapVarPath,MapVarPtrTgt,MapArrayArray,MapArrayGetField
	};
	ThisToThat(MapTypeArray,ScrMapType);
	rPush(r); r->MapVar.ScrDataVal.Static = &ValStatic;
	r->MapVar.ScrVar.Static = &VarStatic; r->MapVar.Val = Data;
	r->on = that->on;
	size = Call(r->on,Size,1(&align));
	r->eltsize = ((size+align-1)&(-align));
	r->Values = ScrRegularArray(&ScrDataValNull,that->nb,MapArrayEltVal,r);
	return &r->MapVar.ScrVar;
}

static ScrMapType *ScrMapTypeArray(ScrMapType *On,int nb) {
	MapTypeArray *r;
	static struct ScrMapType Static = {MapArraySize,MapArrayMap};
	rPush(r); r->ScrMapType.Static = &Static;
	r->on = On; r->nb = nb;
	return &r->ScrMapType;
}

/*________________________
 |
 | Struct
 |________________________
*/

typedef struct {
	int offset;
	ScrMapType *Type;
} MappedField;
typedef struct {
	MapVar MapVar;
	MappedField *b,*e;
	int align,size;
} MapStruct;
static ScrDataVal *MapStructGetField(ScrDataVal *this,int Idx) {
	MappedField *f;
	ScrVar *v;
	ThisToThat(MapStruct,MapVar.ScrDataVal);
	f = that->b+Idx;
	v = Call(f->Type,Map,1(that->MapVar.Val+f->offset));
	return Call(v,Value,0);
}
static ScrVar *MapStructElt(ScrVar *this,int idx) {
	MappedField *f;
	ThisToThat(MapStruct,MapVar.ScrVar);
	f = that->b+idx;
	return Call(f->Type,Map,1(that->MapVar.Val+f->offset));
}
static void MapStructSet(ScrVar *this,ScrDataVal *Value) {
	MappedField *f;
	int idx;
	ThisToThat(MapStruct,MapVar.ScrVar);
	f = that->b; idx = 0;
    while (f<that->e) {
		ScrVar *v;
		ScrDataVal *d;
		rOpen
		v = Call(f->Type,Map,1(that->MapVar.Val+f->offset));
		d = Call(Value,GetField,1(idx));
		Call(v,Set,1(d));
		rClose
		f++; idx++;
	}
}

typedef struct {
	ScrMapType ScrMapType;
	MappedField *b,*e;
	int align,size;
} MapTypeStruct;

static int MapTypeStructSize(ScrMapType *this,int *align) {
	ThisToThat(MapTypeStruct,ScrMapType);
	*align = that->align;
	return that->size;
}
static ScrVar *MapTypeStructMap(ScrMapType *this,void *data) {
	static struct ScrVar VarStatic = { MapVarValue,MapStructElt,MapVarAddress,MapStructSet};
	static struct ScrDataVal ValStatic = {
		MapVarBrowse,MapVarEval,MapVarInt,MapVarString,MapVarPath,MapVarPtrTgt,MapVarArray,MapStructGetField
	};
	MapStruct *r;
	ThisToThat(MapTypeStruct,ScrMapType);
	rPush(r); r->MapVar.ScrVar.Static =  &VarStatic;
	r->MapVar.ScrDataVal.Static = &ValStatic; r->MapVar.Val = data;
	r->b = that->b; r->e = that->e; r->align = that->align; r->size = that->size;
	return &r->MapVar.ScrVar;
}

static ScrMapType *ScrMapTypeStruct(MappedField *b,MappedField *e,int align,int size) {
	MapTypeStruct *r;
	static struct ScrMapType Static = {MapTypeStructSize,MapTypeStructMap};
	rPush(r); r->ScrMapType.Static = &Static;
    r->b = b; r->e = e; r->align = align; r->size = size;
	return &r->ScrMapType;
}

/*________________________
 |
 | Browser
 |________________________
*/

typedef struct {
	BinTree/*<MappedType.Public>*/ Public;
	ScrMapType *Map;
} MappedType;
typedef struct {
	List List;/*<TypeHole.List>*/
	int offset,space;
} TypeHole;

typedef struct TypeRecord TypeRecord;

typedef struct {
	MemStack *Mem;
	List/*<StructDef.List>*/ List;
	List/*<TypeHole.List>*/ Hole;
	ScrTypeStructBuilder *Public;
    MappedField *b,*e;
	int Size,Align;
} StructDef;
struct TypeRecord {
	MemStack *Mem;
	ScrTypeBrowser ScrTypeBrowser;
	ScrTypeStructBrowser ScrTypeStructBrowser;
	BinTree/*<MappedType.Public>*/ Mapped;
	List/*<TypeHole.List>*/ FreeHole;
	List/*<StructDef.List>*/ FreeStruct;
	StructDef Struct;
    ScrMapType *Result;
};

static TypeHole *TrGetFreeFreeSpace(TypeRecord *tr) {
	List *f;
	TypeHole *r;
	f = tr->FreeHole.n;
	if (f==ListEnd) {
		mPush(tr->Mem,r);
	} else {
		tr->FreeHole.n = f->n;
		r = CastBack(TypeHole,List,f);
	}
	return r;
}
static void TrStructRelease(TypeRecord *tr) {
	List *f;
	f = tr->Struct.Hole.n;
	while (f!=ListEnd) { List *n; n=f->n; f->n=tr->FreeHole.n; tr->FreeHole.n=f; f=n; }
	if (tr->Struct.List.n!=ListEnd) {
		List *O;
		StructDef *o;
		mLeave(tr->Struct.Mem);
		O = tr->Struct.List.n;
		o = CastBack(StructDef,List,O);
		tr->Struct = *o;
		O->n = tr->FreeStruct.n;
		tr->FreeStruct.n = O;
	}
}
static void TrInsertFreeSpace(TypeRecord *tr,List *Insert,int offset,int space) {
	TypeHole *p;
	p = TrGetFreeFreeSpace(tr);
	p->offset = offset; p->space = space;
	p->List.n = Insert->n; Insert->n = &p->List;
}
static int AllocateSpace(TypeRecord *tr,int size,int Align) {
	List *P,*PP,*pc;
	TypeHole *p,*c;
	int mAlign,offset,score;
	mAlign = Align-1;
	offset = (tr->Struct.Size+mAlign)&(-Align);
	score = 1; c = 0; PP = pc = &tr->Struct.Hole; P = P->n;
	while (P!=ListEnd) {
		int b,e,ee,sc;
		p = CastBack(TypeHole,List,P);
		ee = p->offset+p->space;
		b = (p->offset+mAlign)&(-Align);
		e = b+size;
		if (ee>=e) {
			sc = 2;
			if (b==p->offset) sc+=6;
			if (e==ee) sc+=4;
		} else {
			sc = 0;
		}
		if (sc>score) { score = sc; c = p; pc = PP; }
		PP = P; P = P->n;
	}
	if (score>1) {
		int b,e,bc,ec;
		TypeHole *p,*n;
		List *f;
		bc = c->offset; ec = c->offset+c->space;
		b = (bc+mAlign)&(-Align); e = b+size;
		f = pc->n; pc->n = f->n; f->n = tr->FreeHole.n; tr->FreeHole.n = f;
		if (bc!=b) { TrInsertFreeSpace(tr,pc,bc,b-bc); pc = pc->n; }
		if (ec!=e) { TrInsertFreeSpace(tr,pc,e,ec-e); } 
		offset = b;
	} else {
		if (offset!=tr->Struct.Size) {
			pc = &tr->Struct.Hole; while(pc->n!=ListEnd) {pc=pc->n;}
			TrInsertFreeSpace(tr,pc,tr->Struct.Size,offset-tr->Struct.Size);
		}
		tr->Struct.Size = offset+size;
	}
	return offset;
}
static ScrMapType *TrGetType(TypeRecord *tr,ScrType *Type) {
	BinTree *F;
	MappedType *f;
	int id;
	{void *t,*n; t = Type; n = 0; id = t-n; }
	F = BinTreeSeek(&tr->Mapped,id);
	if (F==BinTreeEnd) {
		ScrMapType *o;
        mPush(tr->Mem,f);
		o = tr->Result;
		Call(Type,Browse,1(&tr->ScrTypeBrowser));
		f->Map = tr->Result;
		tr->Result = o;
		BinTreeSeekOrInsert(&tr->Mapped,id,&f->Public);
	} else {
		f = CastBack(MappedType,Public,F);
	}
	return f->Map;
}
static void TypeStructBrowserAdd(ScrTypeStructBrowser *this,int num,char *Label,ScrType *Type) {
	MappedField *c;
	ThisToThat(TypeRecord,ScrTypeStructBrowser);
	c = that->Struct.b+num;
    if (c<that->Struct.e) {
		int size,align;
		c->Type = TrGetType(that,Type);
		size = Call(c->Type,Size,1(&align));
		c->offset = AllocateSpace(that,size,align);
		if (align>that->Struct.Align) { that->Struct.Align = align; }
	}
}
static ScrType *TypeStructBrowserEnd(ScrTypeStructBrowser *this) {
	ThisToThat(TypeRecord,ScrTypeStructBrowser);
	mIn(that->Mem,that->Result = ScrMapTypeStruct(that->Struct.b,that->Struct.e,that->Struct.Align,that->Struct.Size));
    TrStructRelease(that);
	return &ScrTypeNull; /* ToDo */
}
static void TypeStructBrowserMap(ScrTypeStructBrowser *this,int offset,int num,char *Label,ScrType *Type){
	MappedField *c;
	ThisToThat(TypeRecord,ScrTypeStructBrowser);
	Call(that->Struct.Public,AddField,3(0,Label,Type));
	c = that->Struct.b + num;
	if (c<that->Struct.e) {
		int align;
	    c->Type = TrGetType(that,Type);
	    c->offset = offset;
		Call(c->Type,Size,1(&align));
        if (align>that->Struct.Align) { that->Struct.Align = align;}
	}
}
static ScrType *TypeStructBrowserMapEnd(ScrTypeStructBrowser *this,char *Label,int align,int size) {
	ScrType *r;
	ThisToThat(TypeRecord,ScrTypeStructBrowser);
	mIn(that->Mem,r = Call(that->Struct.Public,Close,2(&ScrTypeBrowserNull,Label)));
	mIn(that->Mem,that->Result = ScrMapTypeStruct(that->Struct.b,that->Struct.e,align,size));
    TrStructRelease(that);
	return r;
}

static ScrType *TypeRecordAtom(ScrTypeBrowser *this,ScrTypeBase Atom) {
	ThisToThat(TypeRecord,ScrTypeBrowser);
	switch (Atom) {
	case ScbShort: case ScbLong:
	case ScbInt: that->Result = &ScrMapTypeInt; break;
	case ScbChar: that->Result = &ScrMapTypeChar; break;
	case ScbString: that->Result = &ScrMapTypeString; break;
	default: that->Result = &ScrMapTypeNull; break;
	}
	return &ScrTypeNull;
}
static ScrTypeStructBrowser *TypeRecordStruct(ScrTypeBrowser *this,char *Label,ScrType *Base,int FieldNb) {
	StructDef *N;
	MappedField *p,*e;
	ThisToThat(TypeRecord,ScrTypeBrowser);
	if (that->FreeStruct.n==ListEnd) {
		mPush(that->Mem,N);
		N->Mem = MemStackFork(that->Mem,256);
	} else {
		N = CastBack(StructDef,List,that->FreeStruct.n);
		that->FreeStruct.n = N->List.n;
	}
	mEnter(N->Mem); 
	mIn(N->Mem,N->Public = ScrTypeStructNew());
	*N = that->Struct; that->Struct.List.n = &N->List;
	N = &that->Struct;
	mnPush(that->Mem,N->b,FieldNb);
	N->e = N->b + FieldNb;
    p = N->b; e = N->e;
	while (p<e) { p->offset = 0; p->Type = &ScrMapTypeNull; p++; }
	N->Size = 0; N->Align = 1;
	N->Hole.n = ListEnd;
	return &that->ScrTypeStructBrowser;
}
static ScrType *TypeRecordFn(ScrTypeBrowser *this,ScrType *Result,ScrType *Params) {
	ThisToThat(TypeRecord,ScrTypeBrowser);
	that->Result = &ScrMapTypeNull;
	return &ScrTypeNull;
}
static ScrType *TypeRecordPtr(ScrTypeBrowser *this,ScrType *On) { 
	ScrMapType *on;
	ThisToThat(TypeRecord,ScrTypeBrowser);
	on = TrGetType(that,On);
	mIn(that->Mem,that->Result = ScrMapTypePtr(on));
	return &ScrTypeNull;
}
static ScrType *TypeRecordArray(ScrTypeBrowser *this,int EltNb,ScrType *Of) {
	ScrMapType *of;
	ThisToThat(TypeRecord,ScrTypeBrowser);
	of = TrGetType(that,Of);
    mIn(that->Mem,that->Result = ScrMapTypeArray(of,EltNb));
	return &ScrTypeNull;
}
static ScrType *TypeRecordList(ScrTypeBrowser *this,ScrType *Of) {
	ThisToThat(TypeRecord,ScrTypeBrowser);
	that->Result = &ScrMapTypeNull;
	return &ScrTypeNull;
}
static ScrType *TypeRecordLabeled(ScrTypeBrowser *this,char *Label,ScrType *Actual) {
	ThisToThat(TypeRecord,ScrTypeBrowser);
	that->Result = TrGetType(that,Actual);
	return &ScrTypeNull;
}
static TypeRecord *TypeRecordNew(void) {
	MemStack *Mem;
	TypeRecord *r;
	static struct ScrTypeStructBrowser StructBrowserStatic = { 
		TypeStructBrowserAdd,TypeStructBrowserEnd,TypeStructBrowserMap,TypeStructBrowserMapEnd
	};
	static struct ScrTypeBrowser BrowserStatic = {
		TypeRecordAtom,TypeRecordStruct,TypeRecordFn,TypeRecordPtr,TypeRecordArray,TypeRecordList,TypeRecordLabeled
	};
	Mem = rFork(256);
	mPush(Mem,r); r->Mem = Mem;
	r->ScrTypeBrowser.Static = &BrowserStatic;
	r->ScrTypeStructBrowser.Static = &StructBrowserStatic;
	BinTreeInit(&r->Mapped);
	r->FreeHole.n = ListEnd;
	r->FreeStruct.n = ListEnd;
	r->Result = &ScrMapTypeNull;
	r->Struct.Mem = Mem;
	r->Struct.Public = 0;
	r->Struct.List.n = ListEnd;
	r->Struct.Hole.n = ListEnd;
	r->Struct.b = r->Struct.e = 0;
    r->Struct.Size = 0; 
	r->Struct.Align = 1;
	return r;
}

/*_______________________________
 |
 | Context
 |_______________________________
*/

typedef struct DataCtx {
	ScrDataCtx ScrDataCtx;
	ScrDataCtx *Exec;
	ScrDataCtx **Global;
	int Lvl;
	struct DataCtx *Lexical;
    ScrDataVal *Param,*Fifo;
	int LocalsNb;
	ScrDataVal **Locals;
} tDataCtx;

typedef struct {
	MemStack *Mem;
	ScrDataCtx ScrDataCtx;
	ScrDataCtx *Const;
	TypeRecord *Type;
	ScrVar *Local,*Result;
} IntrnCtx;

static ScrDataCtx *InternalCtxNew(ScrDataCtx *Const,TypeRecord *Types,ScrVar *Result,ScrType *Local);

static ScrDataCtx *IntrnExtrn(ScrDataCtx *this,int ExtrnNum) {
	ThisToThat(IntrnCtx,ScrDataCtx);
	return (Call(that->Const,Extrn,1(ExtrnNum)));
}
static ScrDataCtx *IntrnLexical(ScrDataCtx *this,int Depth) {
	ThisToThat(IntrnCtx,ScrDataCtx);
	return Call(that->Const,Lexical,1(Depth));
}
static ScrDataVal *IntrnParam(ScrDataCtx *this) {
	ThisToThat(IntrnCtx,ScrDataCtx);
	return Call(that->Const,Param,0);
}
static ScrDataVal *IntrnLocal(ScrDataCtx *this,int Idx) {
	ThisToThat(IntrnCtx,ScrDataCtx);
	return Call(that->Const,Local,1(Idx));
}
static ScrDataVal *IntrnFifoParam(ScrDataCtx *this,ScrDataVal *Param) {
	ThisToThat(IntrnCtx,ScrDataCtx);
	return Call(that->Const,FifoParam,1(Param));
}
static ScrDataCtx *IntrnSetLocals(ScrDataCtx *this,int Depth,int LocalsNb,ScrDataExpr **Locals) {
	ThisToThat(IntrnCtx,ScrDataCtx);
	return Call(that->Const,SetLocals,3(Depth,LocalsNb,Locals));
}

static ScrVar *IntrnExecBase(ScrDataCtx *this) {
	ThisToThat(IntrnCtx,ScrDataCtx);
	return that->Local;
}
static ScrVar *IntrnExecResult(ScrDataCtx *this) {
	ThisToThat(IntrnCtx,ScrDataCtx);
	return that->Result;
}
static ScrDataCtx *IntrnExecInvoke(ScrDataCtx *this,int Depth,ScrVar *Result,ScrDataVal *Param,ScrType *Local) {
	ScrDataCtx *Const;
	ThisToThat(IntrnCtx,ScrDataCtx);
	Call(that->Const,FifoParam,1(Param));
	Const = Call(that->Const,SetLocals,3(Depth,0,0));
	return InternalCtxNew(Const,that->Type,Result,Local);
}

static ScrVar *IntrnInstantiate(IntrnCtx *that,ScrMapType *Arr) {
	void *data;
	int siz,alig,dof,db;
	ScrVar *r;
	siz = Call(Arr,Size,1(&alig));
	mnPush(that->Mem,data,siz+alig);
	dof = data-((void *)0); 
	db = (dof+alig-1) &(-alig);
	data = ((void *)0)+db;
	r = Call(Arr,Map,1(data));
	return r;
}

static ScrVar *IntrnExecExpand(ScrDataCtx *this,ScrType *Base,int nb) {
	ScrMapType *Elt,*Arr;
	ThisToThat(IntrnCtx,ScrDataCtx);
	Elt = TrGetType(that->Type,Base);
	mIn(that->Mem,Arr = ScrMapTypeArray(Elt,nb));
	return IntrnInstantiate(that,Arr);
}

static ScrDataCtx *InternalCtxNew(ScrDataCtx *Const,TypeRecord *Types,ScrVar *Result,ScrType *Local) {
	IntrnCtx *r;
	MemStack *Mem;
	static struct ScrDataCtx Static = {
        IntrnExtrn,IntrnLexical,IntrnParam,IntrnLocal,IntrnFifoParam,IntrnSetLocals,
		{IntrnExecBase,IntrnExecResult,IntrnExecInvoke,IntrnExecExpand}
	};
	Mem = rFork(256);
	mPush(Mem,r); r->Mem = Mem; r->ScrDataCtx.Static = &Static;
	r->Const = Const; r->Type = Types; r->Result = Result;
	r->Local = IntrnInstantiate(r,TrGetType(r->Type,Local));
	return &r->ScrDataCtx;
}
static ScrDataCtx *ScrInternalExecCtx(ScrDataCtx *Const) {
	return InternalCtxNew(Const,TypeRecordNew(),&ScrVarNull,&ScrTypeNull);
}

/*______________________________________________
 |
 | Const
 |______________________________________________
*/

static ScrDataCtx *DataCtxExtrn(ScrDataCtx *this,int ExtrnNum) {
	ThisToThat(tDataCtx,ScrDataCtx);
	return that->Global[ExtrnNum];
}
static ScrDataCtx *DataCtxLexical(ScrDataCtx *this,int Depth) {
	tDataCtx *r;
	ThisToThat(tDataCtx,ScrDataCtx);
	r = that;
	while (Depth>0) { r = r->Lexical; Depth--; }
	return &r->ScrDataCtx;
}
static ScrDataVal *DataCtxParam(ScrDataCtx *this) {
	ThisToThat(tDataCtx,ScrDataCtx);
	return that->Param;
}
static ScrDataVal *DataCtxLocal(ScrDataCtx *this,int Idx) {
	ThisToThat(tDataCtx,ScrDataCtx);
	return that->Locals[Idx];
}

static ScrDataVal *DataCtxFifoParam(ScrDataCtx *this,ScrDataVal *Param) {
	ScrDataVal *r;
	ThisToThat(tDataCtx,ScrDataCtx);
	r = that->Fifo; that->Fifo = Param;
	return r;
}
static ScrVar *DataCtxExecBase(ScrDataCtx *this) {
	ThisToThat(tDataCtx,ScrDataCtx);
	return Call(that->Exec,Exec.Base,0);
}
static ScrVar *DataCtxExecResult(ScrDataCtx *this) {
	ThisToThat(tDataCtx,ScrDataCtx);
    return Call(that->Exec,Exec.Result,0);
}
static ScrDataCtx *DataCtxExecInvoke(ScrDataCtx *this,int Depth,ScrVar *Result,ScrDataVal *Param,ScrType *Local) {
	ThisToThat(tDataCtx,ScrDataCtx);
	return Call(that->Exec,Exec.Invoke,4(Depth,Result,Param,Local));
}
static ScrVar *DataCtxExpand(ScrDataCtx *this,ScrType *Base,int nb) {
	ThisToThat(tDataCtx,ScrDataCtx);
	return Call(that->Exec,Exec.Expand,2(Base,nb));
}

static ScrDataCtx *DataCtxSetLocals(ScrDataCtx *this,int Lvl,int LocalsNb,ScrDataExpr **Locals) {
	tDataCtx *r,*p;
	int n;
	static struct ScrDataCtx Static = {
		DataCtxExtrn,DataCtxLexical,DataCtxParam,DataCtxLocal,DataCtxFifoParam,DataCtxSetLocals,
		{DataCtxExecBase,DataCtxExecResult,DataCtxExecInvoke,DataCtxExpand}
	};
	ThisToThat(tDataCtx,ScrDataCtx);
	rPush(r);
    r->ScrDataCtx.Static = &Static;
	r->Global = that->Global; r->Exec = that->Exec;
	r->Param = that->Fifo;
	r->Fifo = that->Fifo = &ScrDataValNull;
	r->Lvl = Lvl;
	p = that; n = that->Lvl-Lvl;
	while (n>=0) { p = p->Lexical; n--;}
	r -> Lexical = p;
	/* Set locals */ {
	    ScrDataVal **p,**e;
	    ScrDataExpr **q;
		ScrDataCtx *n;
	    rnPush(r->Locals,LocalsNb);
		r->LocalsNb = LocalsNb; n = &r->ScrDataCtx;
	    p = r->Locals; e = p+LocalsNb; q = Locals;
	    while(p<e) { *p = ScrLazyData(*q,n); p++; q++; }
	}
	return &r->ScrDataCtx;
}

    /*---------------------*/

static ScrDataCtx *CtxGlobalSetLocals(ScrDataCtx *this,int LexicalLvl,int LocalsNb,ScrDataExpr **Locals) {
	ScrDataCtx *r;
	ThisToThat(tDataCtx,ScrDataCtx);
    r = DataCtxSetLocals(this,LexicalLvl,LocalsNb,Locals);
	if (!that->Locals) {
	    tDataCtx *R;
        R = CastBack(tDataCtx,ScrDataCtx,r);
		that->LocalsNb = R->LocalsNb;
		that->Locals = R->Locals;
	}
	return r;
}

    /*---------------------*/

static ScrVar *ScrCtxGlobalExecBase(ScrDataCtx *this) {
	return &ScrVarNull; /* ToDo */
}
static ScrVar *ScrCtxGlobalExecResult(ScrDataCtx *this) {
	return &ScrVarNull; /* ToDo */
}
static ScrDataCtx *ScrCtxGlobalExecInvoke(ScrDataCtx *this,int lvl,ScrVar *Result,ScrDataVal *Param,ScrType *Local) {
	return this;  /* ToDo: New Thread */
}
static ScrVar *ScrCtxGlobalExpand(ScrDataCtx *this,ScrType *Base,int nb) {
	return &ScrVarNull; /* ToDo */
}

ScrDataCtx *ScrCtxGlobal(ScrDataCtx **Globals,int num) {
	tDataCtx *r;
	static struct ScrDataCtx Static = {
		DataCtxExtrn,DataCtxLexical,DataCtxParam,DataCtxLocal,DataCtxFifoParam,CtxGlobalSetLocals,
		{ScrCtxGlobalExecBase,ScrCtxGlobalExecResult,ScrCtxGlobalExecInvoke,ScrCtxGlobalExpand}
	};
	rPush(r);
    r->ScrDataCtx.Static = &Static;
	r->Lexical = r;
	r->Global = Globals;
	r->Param = &ScrDataValNull;
	r->Fifo = &ScrDataValNull;
	r->Lvl = 0;
	r->Locals = 0;
	r->LocalsNb = 0;
	r->Global[num] = &r->ScrDataCtx;
	r->Exec = ScrInternalExecCtx(&r->ScrDataCtx);
	return  r->Exec;
}


