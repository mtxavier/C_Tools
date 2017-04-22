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
#include <ScrDataExpr.h>
#include <ScrBuilder.h>
#include <Builder.Loc.h>
#include <Patricia.h>
#include <List.h>

/*___________________________
 |
 | Type Builder
 |___________________________
*/

typedef struct {
	PatriciaNod/*<StructBuilderElt.Label>*/ Label;
	BinTree/*<StructBuilderElt.Idx>*/ Idx;
	ScrType *EltType;
} StructBuilderElt;
typedef struct {
	ScrType ScrType;
	ScrTypeStructBuilder ScrTypeStructBuilder;
	MemStack *Mem;
	List/*<StructBuilder.Base>*/ Base;
	PatriciaNod/*<StructBuilderElt.Label>*/ Elt;
	BinTree/*<StructBuilderElt.Idx>*/ Num;
	ScrTypeBase Type;
	int EltNum;
} StructBuilder;

     /*-----  Type part -------*/

static ScrTypeBase StructBuilderBase(ScrType *this) { 
	ThisToThat(StructBuilder,ScrType); 
	return that->Type;
}
static int StructBuilderStructEltNb(ScrType *this) {
	ThisToThat(StructBuilder,ScrType);
	return that->EltNum; // incomplete too.
}
static ScrType *StructBuilderStructElt(ScrType *this,char **Label,int Idx) {
	ScrType *r;
	BinTree *P;
	ThisToThat(StructBuilder,ScrType);
	r = &ScrTypeNull; *Label = 0;
	P = BinTreeSeek(&that->Num,Idx);
	if (P!=BinTreeEnd) {
		StructBuilderElt *p;
        p = CastBack(StructBuilderElt,Idx,P);
		*Label = p->Label.Key;
		r = p->EltType;
	}
	return r;
}
static int StructBuilderStructEltNum(ScrType *this,char *Label) {
	int r;
	PatriciaNod *P;
    ThisToThat(StructBuilder,ScrType);
	r = -1;
    P = PatriciaSeek(&that->Elt,Label);
	if (P!=&that->Elt) {
		StructBuilderElt *p;
		p = CastBack(StructBuilderElt,Label,P);
		r = p->Idx.Id;
	}
	return r;
}
static ScrType *StructBuilderLabelBase(ScrType *this) {
	ScrType *r;
	ThisToThat(StructBuilder,ScrType);
	if (that->Base.n==ListEnd) {
		/* !!!! Maybe we should create a new base here, (depending on usage)  !!!! */
		r = &ScrTypeNull;
	} else {
		StructBuilder *Base;
		Base = CastBack(StructBuilder,Base,that->Base.n);
		r = &Base->ScrType;
	}
	return r;
}

     /*------- Builder part -----*/

static StructBuilder *TypeStructMap(ScrTypeBase Type, int Size);
static ScrType *StructBuilderPartial(ScrTypeStructBuilder *this) {
	ThisToThat(StructBuilder,ScrTypeStructBuilder);
	return &that->ScrType;
}

static int StructBuilderEndAddField(ScrTypeStructBuilder *this,int Depth,char *Label,ScrType *EltType) {
	int r;
	PatriciaNod *p;
	ThisToThat(StructBuilder,ScrTypeStructBuilder);
	p = PatriciaSeek(&that->Elt,Label);
	if (p==&that->Elt) {
		StructBuilderElt *n;
		mPush(that->Mem,n);
		n->EltType = EltType;
		BinTreeSeekOrInsert(&that->Num,that->EltNum,&n->Idx);
		PatriciaSeekOrInsert(&that->Elt,Label,&n->Label);
	    r = that->EltNum++;
	} else {
		r = -1;
	}
	return r;
}
static int StructBuilderAddField(ScrTypeStructBuilder *this,int Depth,char *Label,ScrType *EltType) {
	int r;
	if (Depth>0) {
		StructBuilder *Base;
		ThisToThat(StructBuilder,ScrTypeStructBuilder);
		if (that->Base.n==ListEnd) {
            mIn(that->Mem,Base = TypeStructMap(ScbStruct,0));
			that->Base.n = &Base->Base;
		} else {
			Base = CastBack(StructBuilder,Base,that->Base.n);
		}
		r = Call(&Base->ScrTypeStructBuilder,AddField,3(Depth-1,Label,EltType));
	} else {
		r =StructBuilderEndAddField(this,Depth,Label,EltType); 
	}
    return r;
}
static PatriciaNod *StructBuilderCpyElt(PatriciaNod *P,void *Closure) {
	ScrTypeStructBrowser *clos;
	StructBuilderElt *p;
	clos = Closure;
	p = CastBack(StructBuilderElt,Label,P);
	Call(clos,Add,3(p->Idx.Id,P->Key,p->EltType));
	return 0;
}
static ScrType *StructBuilderClose(ScrTypeStructBuilder *this,ScrTypeBrowser *Dst,char *Label) {
	ScrType *base;
	ScrTypeStructBrowser *dst;
	ThisToThat(StructBuilder,ScrTypeStructBuilder);
	if (that->Base.n==ListEnd) {
	    base = &ScrTypeNull;
	} else {
		StructBuilder *Base;
		Base = CastBack(StructBuilder,Base,that->Base.n);
		base = Call(&Base->ScrTypeStructBuilder,Close,2(Dst,""));
	}
	dst = Call(Dst,Struct,3(Label,base,that->EltNum));
	PatriciaForEach(&that->Elt,StructBuilderCpyElt,dst);
	return Call(dst,End,0);

}
static void TypeNullBrowse(ScrType *this,ScrTypeBrowser *Tgt) {}
static ScrType *TypeAtomGetPtr(ScrType *this) { return &ScrTypeNull; }
static ScrType *TypeNullPtrTarget(ScrType *this) { return &ScrTypeNull; }
static ScrType *TypeAtomFnResult(ScrType *this) { return &ScrTypeNull; }
static ScrType *TypeAtomFnParam(ScrType *this) { return &ScrTypeNull; }
static int TypeAtomArrayEltNb(ScrType *this) { return 0;}
static ScrType *TypeAtomArrayEltType(ScrType *this) { return &ScrTypeNull;}
static ScrType *TypeAtomListEltType(ScrType *this) { return &ScrTypeNull; }
static char *TypeNullLabelLabel(ScrType *this) { return ""; }

static StructBuilder *TypeStructMap(ScrTypeBase Type, int Size) {
	StructBuilder *r;
	MemStack *Mem;
	static struct ScrType TypeStatic = {
		TypeNullBrowse, StructBuilderBase, TypeAtomGetPtr, /* ToDo : the hook should provides its own GetPtr method. */
		{TypeNullPtrTarget},
	    {StructBuilderStructEltNb,StructBuilderStructElt,StructBuilderStructEltNum},
	    {TypeAtomFnResult,TypeAtomFnParam},
	    {TypeAtomArrayEltNb,TypeAtomArrayEltType},
	    {TypeAtomListEltType},
	    {TypeNullLabelLabel,StructBuilderLabelBase}
	};
	static struct ScrTypeStructBuilder Static = {
		StructBuilderPartial,StructBuilderAddField,StructBuilderClose
	};
	Mem = rFork(256);
	mPush(Mem,r);
	r->Mem = Mem;
	r->ScrType.Static = &TypeStatic;
	r->ScrTypeStructBuilder.Static = &Static;
	PatriciaInit(&r->Elt);
	BinTreeInit(&r->Num);
	r->Base.n = ListEnd;
	r->EltNum = 0; r->Type = Type;
	return r;
}

ScrTypeStructBuilder *ScrTypeStructNew(void) { return &TypeStructMap(ScbStruct,0)->ScrTypeStructBuilder; }
ScrTypeStructBuilder *ScrTypeScopeNew(void) { return &TypeStructMap(ScbScope,0)->ScrTypeStructBuilder; }

/*___________________________
 |
 | ExprScopeBuilder
 |___________________________
*/

typedef struct {
    ScrType *Type;
	ScrDataExpr *Val;
} SBPrivate;
typedef struct {
	MemStack *Mem;
	ScrExprScopeBuilder ScrExprScopeBuilder;
	ScrType *Type; 
	int Llvl;
	int Overlay;
	ScrCatalog *Fields;
	ScrCatalog *Privates;
}tExprStructBuilder;
static ScrType *ExprStructBuilderGetType(ScrExprScopeBuilder *this) {
	ThisToThat(tExprStructBuilder,ScrExprScopeBuilder);
	return that->Type;
}
static int ExprStructBuilderSetField(ScrExprScopeBuilder *this,int Idx,ScrDataExpr *Expr) {
	int r; char *Label;
	ThisToThat(tExprStructBuilder,ScrExprScopeBuilder);
	Label = Call(that->Fields,GetLabel,1(Idx));
	r = (Label!=0); if (r) {r=(*Label!=0);}
	if (!r) {
		int n;
	    Call(that->Type,Struct.Elt,2(&Label,Idx));
	    r = (Label!=0); if (r) {r=(*Label!=0);}
		if (r) {
			Call(that->Fields,SetLabel,2(Idx,Label));
			n = Call(that->Fields,GetNum,1(Label));
			r = (n==Idx);
		}
	}
	if (r) { 
	    ScrCatalogEntry data; 
		data.Data = Expr;
		Call(that->Fields,SetData,2(Idx,data)); 
	}
	return r;
}
static int ExprStructBuilderAddPrivate(ScrExprScopeBuilder *this,ScrType *Type,char *Label) {
	int r,add;
	ThisToThat(tExprStructBuilder,ScrExprScopeBuilder);
	add = !Label;
	if (add) { r = -1; } else {
	    Call(that->Privates,SetLabel,2(-1,Label));
	    r = Call(that->Privates,GetNum,1(Label));
		add = (r>=0);
	}
	if (add) {
		SBPrivate *n;
		ScrCatalogEntry N;
		mPush(that->Mem,n);
		n->Type = Type; n->Val = &ScrDataExprNull;
		N.Data = n;
		r = Call(that->Privates,SetData,2(r,N));
	}
	return r;
}
static ScrType *ExprStructBuilderGetPrivate(ScrExprScopeBuilder *this,int *num,char *Label) {
	ScrType *r;
	SBPrivate *P;
	int n;
	ThisToThat(tExprStructBuilder,ScrExprScopeBuilder);
	if (Label) {
	    n = Call(that->Privates,GetNum,1(Label));
	} else {
	    n = *num;
	}
    P = Call(that->Privates,GetData,1(n)).Data;
	*num = n;
	return P->Type;
}
static int ExprStructBuilderSetPrivate(ScrExprScopeBuilder *this,int Num,ScrDataExpr *Expr) {
	int r;
	ScrCatalogEntry Data;
	SBPrivate *P;
	ThisToThat(tExprStructBuilder,ScrExprScopeBuilder);
	r = Num;
	if (Num>=0) {
		Data = Call(that->Privates,GetData,1(Num));
		P = Data.Data;
		P->Val = Expr;
		P->Type = Call(Expr,GetType,0);
	} else {
	    mPush(that->Mem,P);
	    P->Type = Call(Expr,GetType,0);
	    P->Val = Expr;
	    Data.Data = P;
	    r = Call(that->Privates,SetData,2(-1,Data));
	}
	return r;
}
static ScrDataExpr *ExprStructBuilderClose(ScrExprScopeBuilder *this,ScrExprBrowser *Tgt) {
	ScrDataExpr **p,**e,**result,**local,*r;
	int localnb,fieldnb;
	ThisToThat(tExprStructBuilder,ScrExprScopeBuilder);
	fieldnb = Call(that->Type,Struct.EltNb,0);
	localnb = Call(that->Privates,EltNb,0);
	rOpen {
	rnPush(result,fieldnb+localnb);
	p = result; e = result+fieldnb;
	/* Fields */{
		int i;
		ScrDataExpr *prm,*cp;
		i= 0; prm = 0;
		while (p<e) {
			ScrCatalogEntry data;
			data = Call(that->Fields,GetData,1(i));
			cp = data.Data;
			if ((cp==&ScrDataExprNull)&&(that->Overlay)){
                if (!prm) {
					ScrType *tp;
					tp = Call(that->Type,Fn.Param,0);
					prm = Call(Tgt,Param,2(tp,0));
				}
				cp = Call(Tgt,FieldSelect,2(prm,i));
			}
			*p++ = cp; i++;
		} 
	}
	local = p; e = local + localnb;
	/* Privates */{
		int q; q = 0;
        while (p<e) {
			SBPrivate *P;
			P = Call(that->Privates,GetData,1(q)).Data;
			*p = P->Val; p++; q++;
		}
	}
	Call(that->Privates,Close,0);
	r = Call(Tgt,Complex,5(that->Llvl,that->Type,result,localnb,local));
	} rClose
	return r;
}
static ScrExprScopeBuilder *iExprScopeBuild(
		struct ScrExprScopeBuilder *Static,ScrType *Type,int Llvl,int Overlay
) {
	MemStack *Mem;
	tExprStructBuilder *r;
	ScrDataExpr **p,**e;
	static SBPrivate SBPrivateNull = {&ScrTypeNull,&ScrDataExprNull};
	Mem = rFork(256);
	mPush(Mem,r);
	r->Mem = Mem;
	r->ScrExprScopeBuilder.Static = Static;
	r->Type = Type; r->Llvl = Llvl;
	r->Privates = ScrCatalogNew(&SBPrivateNull);
	r->Fields = ScrCatalogNew(&ScrDataExprNull);
	r->Overlay = Overlay;
	return &r->ScrExprScopeBuilder;
}


ScrExprScopeBuilder *ScrExprStructBuild(ScrType *Type,int Llvl) {
	static struct ScrExprScopeBuilder Static = {
		ExprStructBuilderGetType,ExprStructBuilderSetField,
		ExprStructBuilderAddPrivate,ExprStructBuilderGetPrivate,ExprStructBuilderSetPrivate,
        ExprStructBuilderClose
	};
	return iExprScopeBuild(&Static,Type,Llvl,(0!=0));
}

ScrExprScopeBuilder *ScrExprScopeBuild(ScrType *Type) {
	return ScrExprStructBuild(Type,0);
}

ScrExprScopeBuilder *ScrExprFunctionBuild(ScrType *Type,int LLvl) {
	static struct ScrExprScopeBuilder Static = {
		ExprStructBuilderGetType,ExprStructBuilderSetField,
		ExprStructBuilderAddPrivate,ExprStructBuilderGetPrivate,ExprStructBuilderSetPrivate,
        ExprStructBuilderClose
	};
	return iExprScopeBuild(&Static,Type,LLvl,(0==0));
}

/*____________________
 |
 |
 |____________________
*/

typedef struct {
	ScrExprScopeBuilder ScrExprScopeBuilder;
	ScrExprScopeBuilder *Actual;
	ScrTypeStructBuilder *TypeBuild;
	ScrTypeBrowser *TypeDst;
	ScrExprBrowser *ExprTgt;
	ScrTypeThrou *Type;
	ScrVocabulary *Voc;
	int ModuleNum;
	int Anonymous;
} ScopeScopeBuilder;

static ScrType *SSBGetType(ScrExprScopeBuilder *this){
	ThisToThat(ScopeScopeBuilder,ScrExprScopeBuilder);
	return &that->Type->ScrType;
}
static int SSBSetField(ScrExprScopeBuilder *this,int Idx,ScrDataExpr *Data){
	ThisToThat(ScopeScopeBuilder,ScrExprScopeBuilder);
	return Call(that->Actual,SetField,2(Idx,Data));
}
static int SSBAddPrivate(ScrExprScopeBuilder *this,ScrType *Type,char *Label){
	int nolabel,r;
	ThisToThat(ScopeScopeBuilder,ScrExprScopeBuilder);
	nolabel = (Label==0); if (!nolabel) {nolabel = (*Label==0);}
	if (nolabel) {
		Label = Call(that->Voc,InternalLabel,1(that->Anonymous));
		that->Anonymous++;
		r = Call(that->Actual,AddPrivate,2(Type,Label));
	} else {
		int nf;
		ScrDataExpr *xt;
		r = Call(that->Actual,AddPrivate,2(Type,Label));
        Call(that->TypeBuild,AddField,3(0,Label,Type));
		nf = Call(&that->Type->ScrType,Struct.EltNum,1(Label));
		xt = Call(that->ExprTgt,Global,3(Type,that->ModuleNum,r));
		Call(that->Actual,SetField,2(nf,xt));
	}
	return r;
}
static ScrType *SSBGetPrivate(ScrExprScopeBuilder *this,int *num,char *Label) {
	ThisToThat(ScopeScopeBuilder,ScrExprScopeBuilder);
	return Call(that->Actual,GetPrivate,2(num,Label));
}
static int SSBSetPrivate(ScrExprScopeBuilder *this,int num,ScrDataExpr *Data) {
	ThisToThat(ScopeScopeBuilder,ScrExprScopeBuilder);
	return Call(that->Actual,SetPrivate,2(num,Data));
}
static ScrDataExpr *SSBClose(ScrExprScopeBuilder *this,ScrExprBrowser *Tgt){
	ScrDataExpr *r;
	ThisToThat(ScopeScopeBuilder,ScrExprScopeBuilder);
	that->Type->Child = Call(that->TypeBuild,Close,2(that->TypeDst,""));
	r = Call(that->Actual,Close,1(Tgt));
	return r;
}
ScrExprScopeBuilder *ScrHiddenExprScopeBuilder(
			int ModuleNum,ScrTypeBrowser *TypeDst,ScrExprBrowser *ExprTgt,ScrTypeThrou *Type,ScrVocabulary *Voc) {
	ScopeScopeBuilder *r;
	static struct ScrExprScopeBuilder Static = {
        SSBGetType,SSBSetField,SSBAddPrivate,SSBGetPrivate,SSBSetPrivate,SSBClose
	};
	rPush(r); r->ScrExprScopeBuilder.Static = &Static;
	r->Type = Type; r->Voc = Voc;
	r->TypeBuild = ScrTypeScopeNew();
	r->Type->Child = Call(r->TypeBuild,Partial,0);
	r->Actual = ScrExprStructBuild(&Type->ScrType,0);
	r->ModuleNum = ModuleNum; r->Anonymous = 0;
	r->TypeDst = TypeDst; r->ExprTgt = ExprTgt;
	return &r->ScrExprScopeBuilder;
}

