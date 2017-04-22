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


/*________________________________
 |
 | Raw Expr
 |________________________________
*/

typedef struct {
    ScrDataBrowser ScrDataBrowser;
	MemStack *Mem;
} RawData;
typedef struct {
	ScrExprBrowser ScrExprBrowser;
	MemStack *Mem;
	RawData *Data;
} RawExpr;

/*----- ExprConst ------*/

typedef struct {
	ScrDataExpr ScrDataExpr;
	ScrType *Type;
	ScrDataVal *v;
} ExprConst;
static ScrDataExpr *ExprConstBrowse(ScrDataExpr *this,ScrExprBrowser *Tgt) {
	ThisToThat(ExprConst,ScrDataExpr);
	return Call(Tgt,Const,2(that->Type,that->v));
}
static ScrType *ExprConstType(ScrDataExpr *this) {
	ThisToThat(ExprConst,ScrDataExpr);
	return that->Type;
}
static ScrDataVal *ExprConstEval(ScrDataExpr *this,ScrDataCtx *Ctx) {
	ThisToThat(ExprConst,ScrDataExpr);
	return that->v;
}
static ScrDataExpr *RawExprConst(ScrExprBrowser *this,ScrType *Type,ScrDataVal *Value) {
	ExprConst *r;
	static struct ScrDataExpr Static = {ExprConstBrowse,ExprConstType,ExprConstEval};
	ThisToThat(RawExpr,ScrExprBrowser);
	mPush(that->Mem,r);
	r->ScrDataExpr.Static = &Static;
	r->v = Call(Value,Browse,1(&that->Data->ScrDataBrowser));
	r->Type = Type;
	return &r->ScrDataExpr;
}

/*----- ExprCond ------*/

typedef struct {
	ScrDataExpr ScrDataExpr;
	ScrDataExpr *c,*t,*f;
} ExprCond;
static ScrDataExpr *ExprCondBrowse(ScrDataExpr *this,ScrExprBrowser *Tgt) {
	ThisToThat(ExprCond,ScrDataExpr);
	return Call(Tgt,Cond,3(that->c,that->t,that->f));
}
static ScrType *ExprCondType(ScrDataExpr *this) {
	ThisToThat(ExprCond,ScrDataExpr);
	return Call(that->t,GetType,0);
}
static ScrDataVal *ExprCondEval(ScrDataExpr *this,ScrDataCtx *Ctx) {
	ScrDataVal *c;
	int cv;
	ThisToThat(ExprCond,ScrDataExpr);
	c = Call(that->c,Eval,1(Ctx));
	cv = Call(c,Int,0);
	return (cv)?Call(that->t,Eval,1(Ctx)):Call(that->f,Eval,1(Ctx));
}
static ScrDataExpr *RawExprCond(ScrExprBrowser *this,ScrDataExpr *c,ScrDataExpr *t,ScrDataExpr *f) {
	ExprCond *r;
	static struct ScrDataExpr Static = {ExprCondBrowse,ExprCondType,ExprCondEval};
	ThisToThat(RawExpr,ScrExprBrowser);
	mPush(that->Mem,r);
	r->ScrDataExpr.Static = &Static;
	r->c = c; r->t = t; r->f = f;
	return &r->ScrDataExpr;
}

/*----- Val Alter -----*/

typedef struct {
	ScrDataExpr ScrDataExpr;
	ScrDataUOpDesc *Desc;
	ScrDataVal *(*op)(ScrDataVal *x);
	ScrDataExpr *x;
} IntAlter;

static ScrDataExpr *ValAlterBrowse(ScrDataExpr *this,ScrExprBrowser *Tgt) {
	ThisToThat(IntAlter,ScrDataExpr);
	return Call(Tgt,Alter,2(that->Desc,that->x));
}
static ScrType *ValAlterGetType(ScrDataExpr *this) {
	// This version of alter is only perform internal transformation.
	// Specific operators (translators) should be used for external transformation.
	ThisToThat(IntAlter,ScrDataExpr);
	return Call(that->x,GetType,0);
}
static ScrDataVal *IntAlterEval(ScrDataExpr *this,ScrDataCtx *Ctx) {
	ScrDataVal *x;
	ThisToThat(IntAlter,ScrDataExpr);
	x = Call(that->x,Eval,1(Ctx));
	return that->op(x);
}
static ScrDataExpr *RawExprIntAlter(ScrExprBrowser *this,ScrDataUOpDesc *Desc,ScrDataExpr *x) {
	IntAlter *r;
	static struct ScrDataExpr Static = { ValAlterBrowse,ValAlterGetType,IntAlterEval};
	ThisToThat(RawExpr,ScrExprBrowser);
	mPush(that->Mem,r);
	r->ScrDataExpr.Static = &Static;
	r->Desc = Desc; r->op = Desc->op; r->x = x;
	return &r->ScrDataExpr;
}

    /*--- Val Compose ---*/

typedef struct {
	ScrDataExpr ScrDataExpr;
    ScrDataBOpDesc *Desc;
	ScrDataVal *(*op)(ScrDataVal *x,ScrDataVal *y);
	ScrDataExpr *x,*y;
} IntCompose;

static ScrDataExpr *ValComposeBrowse(ScrDataExpr *this,ScrExprBrowser *Tgt) {
	ThisToThat(IntCompose,ScrDataExpr);
	return Call(Tgt,Compose,3(that->Desc,that->x,that->y));
}
static ScrType *ValComposeGetType(ScrDataExpr *this) {
	// This is mainly for internal composition, but with this definition,
	// functions of the kind of AxB -> A is also supported.
	ThisToThat(IntCompose,ScrDataExpr);
	return Call(that->x,GetType,0);
}
static ScrDataVal *IntComposeInt(ScrDataExpr *this,ScrDataCtx *Ctx) {
	ScrDataVal *x,*y;
	ThisToThat(IntCompose,ScrDataExpr);
	x = Call(that->x,Eval,1(Ctx));
	y = Call(that->y,Eval,1(Ctx));
	return that->op(x,y);
}
static ScrDataExpr *RawExprIntCompose(ScrExprBrowser *this,ScrDataBOpDesc *op,ScrDataExpr *x,ScrDataExpr *y) {
	IntCompose *r;
	static struct ScrDataExpr Static = { ValComposeBrowse,ValComposeGetType,IntComposeInt};
	ThisToThat(RawExpr,ScrExprBrowser);
	mPush(that->Mem,r);
	r->ScrDataExpr.Static = &Static;
	r->Desc = op; r->op = op->op; r->x = x; r->y = y;
	return &r->ScrDataExpr;
}

/*___________________________________
 |
 | Struct / Fn
 |___________________________________
*/

typedef struct {
	ScrDataExpr ScrDataExpr;
	ScrType *Type;
	int FieldNb;
	ScrDataExpr **Elts;
	int Llvl;
    struct {
	    int LocalsNb;
	    ScrDataExpr **Locals;
    } Declare;
} ExprStruct;

typedef struct {
	ScrDataVal ScrDataVal;
	ScrType *Type;
	ExprStruct *Expr;
	ScrDataVal **Values;
} DataStruct;

static DataStruct *StructEval(ExprStruct *that,ScrDataCtx *Ctx);
     /*---------------------*/

static ScrDataVal *DataStructBrowse(ScrDataVal *this,ScrDataBrowser *Tgt) {
	ThisToThat(DataStruct,ScrDataVal);
	return Call(Tgt,Complex,2(that->Type,that->Values));
}
static ScrDataVal *DataStructEval(ScrDataVal *this,ScrDataCtx *Ctx,ScrVar *Result,ScrDataVal *Param) {
	DataStruct *V;
	ThisToThat(DataStruct,ScrDataVal);
	Call(Ctx,FifoParam,1(Param));
    V = StructEval(that->Expr,Ctx);
	V->Type = Call(V->Type,Fn.Result,0);
	Call(Result,Set,1(&V->ScrDataVal));
	return &V->ScrDataVal;
}
static int DataStructInt(ScrDataVal *this) { return 0;}
static ScrString *DataStructString(ScrDataVal *this) { return &ScrStringNull; }
static ScrPath *DataStructPath(ScrDataVal *this) { return &ScrPathNull; }
static ScrVar *DataStructPtrTgt(ScrDataVal *this) { return &ScrVarNull; }
static ScrArray *DataStructArray(ScrDataVal *this) { return ScrOneEltArray(&ScrDataValNull,this);}
static ScrDataVal *DataStructGetField(ScrDataVal *this,int Idx) {
	ThisToThat(DataStruct,ScrDataVal);
	return that->Values[Idx];
}

        /*------------*/

static DataStruct *StructEval(ExprStruct *that,ScrDataCtx *Ctx) {
	static struct ScrDataVal Static = {
		DataStructBrowse, DataStructEval, DataStructInt,
		DataStructString, DataStructPath, DataStructPtrTgt, DataStructArray, DataStructGetField
	};
	DataStruct *r;
	ScrDataVal **p,**e;
	ScrDataExpr **q;
	ScrDataCtx *nc;
	rPush(r); r->ScrDataVal.Static = &Static;
	rnPush(r->Values,that->FieldNb);
	r->Expr = that;
	r->Type = that->Type;
	nc = Call(Ctx,SetLocals,3(that->Llvl,that->Declare.LocalsNb,that->Declare.Locals));
	p = r->Values; e = p+that->FieldNb;
	q = that->Elts;
	while (p<e) { *p++ = ScrLazyData(*q++,nc); }
	return r;
}

static ScrDataExpr *ExprScopeBrowse(ScrDataExpr *this,ScrExprBrowser *Tgt) {
	ThisToThat(ExprStruct,ScrDataExpr);
	return Call(Tgt,Complex,5(that->Llvl,that->Type,that->Elts,that->Declare.LocalsNb,that->Declare.Locals));
}
static ScrDataVal *ExprStructEval(ScrDataExpr *this,ScrDataCtx *Ctx) {
	ThisToThat(ExprStruct,ScrDataExpr);
	return &StructEval(that,Ctx)->ScrDataVal;
}

static ScrType *ExprStructGetType(ScrDataExpr *this) {
	ThisToThat(ExprStruct,ScrDataExpr);
	return that->Type;
}

static ScrDataExpr *RawExprComplex(ScrExprBrowser *this,int LLvl,ScrType *Type,ScrDataExpr **Result,int LocalNb,ScrDataExpr **Local) {
	ExprStruct *r;
	ScrDataExpr **p,**q,**e;
	static struct ScrDataExpr Static = { ExprScopeBrowse,ExprStructGetType,ExprStructEval};
	ThisToThat(RawExpr,ScrExprBrowser);
	mPush(that->Mem,r) r->ScrDataExpr.Static = &Static;
	r->Type = Type; r->FieldNb = Call(r->Type,Struct.EltNb,0); r->Llvl = LLvl;
	r->Declare.LocalsNb = LocalNb;
	mnPush(that->Mem,r->Elts,r->FieldNb+r->Declare.LocalsNb);
	r->Declare.Locals = r->Elts + r->FieldNb;
	p = r->Elts; e = r->Declare.Locals; q = Result;
	while (p<e) { *p++ = *q++; }
	e = e+LocalNb; q = Local;
	while (p<e) { *p++ = *q++; }
	return &r->ScrDataExpr;
}

/*______________________
 |
 | Field
 |______________________
*/

typedef struct {
	ScrDataExpr ScrDataExpr;
	ScrDataExpr *Struct;
	int Idx;
} ExprField;

static ScrDataExpr *ExprFieldBrowse(ScrDataExpr *this,ScrExprBrowser *Tgt) {
	ThisToThat(ExprField,ScrDataExpr);
	return Call(Tgt,FieldSelect,2(that->Struct,that->Idx));
}
static ScrType *ExprFieldGetType(ScrDataExpr *this) {
	ScrType *t;
	char *dummy;
	ThisToThat(ExprField,ScrDataExpr);
	t = Call(that->Struct,GetType,0);
	return Call(t,Struct.Elt,2(&dummy,that->Idx));
}
static ScrDataVal *ExprFieldEval(ScrDataExpr *this,ScrDataCtx *Ctx) {
	ScrDataVal *s;
	ThisToThat(ExprField,ScrDataExpr);
    s = Call(that->Struct,Eval,1(Ctx));
	return Call(s,GetField,1(that->Idx));
}
static ScrDataExpr *RawExprField(ScrExprBrowser *this,ScrDataExpr *Data,int Idx) {
	ExprField *r;
	static struct ScrDataExpr Static = {
		ExprFieldBrowse,ExprFieldGetType,ExprFieldEval
	};
	ThisToThat(RawExpr,ScrExprBrowser);
	mPush(that->Mem,r);
	r->ScrDataExpr.Static = &Static;
	r->Struct = Data ; r->Idx = Idx;
	return &r->ScrDataExpr;

}

/*______________________________________________
 |
 | Fn invocation
 |______________________________________________
*/

typedef struct {
	ScrDataExpr ScrDataExpr;
	ScrDataExpr *fn,*param;
} ApplyExpr;
static ScrDataExpr *ApplyExprBrowse(ScrDataExpr *this,ScrExprBrowser *Tgt) {
	ThisToThat(ApplyExpr,ScrDataExpr);
    return  Call(Tgt,Apply,2(that->fn,that->param));
}
static ScrType *ApplyExprGetType(ScrDataExpr *this) {
	ScrType *r;
	ThisToThat(ApplyExpr,ScrDataExpr);
	r = Call(that->fn,GetType,0);
	return Call(r,Fn.Result,0);
}
static ScrDataVal *ApplyExprEval(ScrDataExpr *this,ScrDataCtx *Ctx) {
	ScrDataVal *fn,*r,*param;
	ThisToThat(ApplyExpr,ScrDataExpr);
	param = Call(that->param,Eval,1(Ctx));
	fn = Call(that->fn,Eval,1(Ctx));
	r = Call(fn,Eval,3(Ctx,&ScrVarNull,param));
	return r;
}
static ScrDataExpr *RawExprApply(ScrExprBrowser *this,ScrDataExpr *Fn,ScrDataExpr *Param) {
	ApplyExpr *r;
	static struct ScrDataExpr Static = {ApplyExprBrowse,ApplyExprGetType,ApplyExprEval};
	ThisToThat(RawExpr,ScrExprBrowser);
	mPush(that->Mem,r);
	r->ScrDataExpr.Static = &Static;
	r->fn = Fn; r->param = Param;
	return &r->ScrDataExpr;
}

    /*___________________
	 |
	 | Ctx Access
	 |___________________
	*/

typedef struct {
	ScrDataExpr ScrDataExpr;
	ScrType *Type;
} ExprParam;
static ScrDataExpr *ExprParamBrowse(ScrDataExpr *this,ScrExprBrowser *Tgt) { 
	ThisToThat(ExprParam,ScrDataExpr);
	return Call(Tgt,Param,2(that->Type,0)); 
}
static ScrType *ExprParamGetType(ScrDataExpr *this) {
	ThisToThat(ExprParam,ScrDataExpr);
	return that->Type;
}
static ScrDataVal *ExprParamEval(ScrDataExpr *this,ScrDataCtx *Ctx) { return Call(Ctx,Param,0); }
static ScrDataExpr *localParam(RawExpr *that,ScrType *Type) {
	ExprParam *r;
	static struct ScrDataExpr Static = {ExprParamBrowse,ExprParamGetType,ExprParamEval};
	mPush(that->Mem,r);
	r->ScrDataExpr.Static = &Static;
	r->Type = Type;
	return &r->ScrDataExpr;
}
      /*---------*/

typedef struct {
	ExprParam ExprParam;
	int Depth;
} DeepParam;

static ScrDataExpr *DeepParamBrowse(ScrDataExpr *this,ScrExprBrowser *Tgt) { 
	ThisToThat(DeepParam,ExprParam.ScrDataExpr); 
	return Call(Tgt,Param,2(that->ExprParam.Type,that->Depth)); 
}
static ScrDataVal *DeepParamEval(ScrDataExpr *this,ScrDataCtx *Ctx) { 
	ScrDataCtx *deep;
	ThisToThat(DeepParam,ExprParam.ScrDataExpr);
	deep = Call(Ctx,Lexical,1(that->Depth));
	return Call(deep,Param,0); 
}
static ScrDataExpr *RawExprParam(ScrExprBrowser *this,ScrType *Type,int Depth) {
	ScrDataExpr *R;
	ThisToThat(RawExpr,ScrExprBrowser);
	if (Depth) {
	    static struct ScrDataExpr Static = {DeepParamBrowse,ExprParamGetType,DeepParamEval};
	    DeepParam *r;
	    mPush(that->Mem,r);
	    r->ExprParam.ScrDataExpr.Static = &Static;
	    r->ExprParam.Type = Type;
		r->Depth = Depth;
		R = &r->ExprParam.ScrDataExpr;
	} else {
		R = localParam(that,Type);
	}
	return R;
}

      /*---------*/

typedef struct {
	ExprParam ExprParam;
	int Idx;
} ExprLocal;
static ScrDataExpr *ExprLocalBrowse(ScrDataExpr *this,ScrExprBrowser *Tgt) {
	ThisToThat(ExprLocal,ExprParam.ScrDataExpr);
	return Call(Tgt,Local,3(that->ExprParam.Type,0,that->Idx));
}
static ScrDataVal *ExprLocalEval(ScrDataExpr *this,ScrDataCtx *Ctx) {
	ThisToThat(ExprLocal,ExprParam.ScrDataExpr);
	return Call(Ctx,Local,1(that->Idx));
}
static ScrDataExpr *lExprLocal(RawExpr *that,ScrType *Type,int Idx) {
	ExprLocal *r;
	static struct ScrDataExpr Static = { ExprLocalBrowse,ExprParamGetType,ExprLocalEval};
	mPush(that->Mem,r); r->ExprParam.ScrDataExpr.Static = &Static;
	r->ExprParam.Type = Type; r->Idx = Idx;
	return &r->ExprParam.ScrDataExpr;
}
     /*------------*/

typedef struct {
	ExprParam ExprParam;
	int Depth,Num;
} ExprDeep;
static ScrDataExpr *ExprDeepBrowse(ScrDataExpr *this,ScrExprBrowser *Tgt) {
	ThisToThat(ExprDeep,ExprParam);
	return Call(Tgt,Local,3(that->ExprParam.Type,that->Depth,that->Num));
}
static ScrDataVal *ExprDeepEval(ScrDataExpr *this,ScrDataCtx *Ctx) {
	ScrDataCtx *c;
	ScrDataExpr *expr;
	ThisToThat(ExprDeep,ExprParam.ScrDataExpr);
    c = Call(Ctx,Lexical,1(that->Depth));
	return Call(c,Local,1(that->Num));
}
static ScrDataExpr *RawExprLocal(ScrExprBrowser *this,ScrType *Type,int Depth,int Num) {
	ScrDataExpr *R;
	ThisToThat(RawExpr,ScrExprBrowser);
	if (!Depth) {
		R = lExprLocal(that,Type,Num);
	} else {
	    ExprDeep *r;
	    static struct ScrDataExpr Static = { ExprDeepBrowse,ExprParamGetType,ExprDeepEval};
	    mPush(that->Mem,r); r->ExprParam.ScrDataExpr.Static = &Static;
	    r->ExprParam.Type = Type; r->Depth = Depth; r->Num = Num;
	    R = &r->ExprParam.ScrDataExpr;
	}
	return R;
}

     /*------------*/

typedef struct {
	ExprParam ExprParam;
    int Module,Idx;
} ExprGlobal;
static ScrDataExpr *ExprGlobalBrowse(ScrDataExpr *this,ScrExprBrowser *Tgt) {
	ThisToThat(ExprGlobal,ExprParam.ScrDataExpr);
	return Call(Tgt,Global,3(that->ExprParam.Type,that->Module,that->Idx));
}
static ScrDataVal *ExprGlobalEval(ScrDataExpr *this,ScrDataCtx *Ctx) {
	ScrDataCtx *c;
	ThisToThat(ExprGlobal,ExprParam.ScrDataExpr);
	c = Call(Ctx,Extrn,1(that->Module));
	return Call(c,Local,1(that->Idx));
}
static ScrDataExpr *RawExprExtern(ScrExprBrowser *this,ScrType *Type,int Module,int Idx) {
	ExprGlobal *r;
	static struct ScrDataExpr Static = {ExprGlobalBrowse,ExprParamGetType,ExprGlobalEval};
	ThisToThat(RawExpr,ScrExprBrowser);
	mPush(that->Mem,r); r->ExprParam.ScrDataExpr.Static = &Static;
	r->ExprParam.Type = Type;
	r->Module = Module; r->Idx = Idx;
	return &r->ExprParam.ScrDataExpr;
}

/*_______________________________________
 |
 | Arrays
 |_______________________________________
*/

#include <ScrBuilder.h>

typedef struct {
	ScrDataVal ScrDataVal;
	ScrArray *Content;
} ArrayVal;
static ScrDataVal *ArrayValBrowse(ScrDataVal *this,ScrDataBrowser *Tgt) { 
	ThisToThat(ArrayVal,ScrDataVal);
	return Call(Tgt,Vector,1(that->Content));
}
static ScrDataVal *ArrayValEval(ScrDataVal *this,ScrDataCtx *Ctx,ScrVar *Result,ScrDataVal *Param) { 
	Call(Result,Set,1(this));
	return this; 
}
static int ArrayValInt(ScrDataVal *this) {
	int min,max,nb;
	ThisToThat(ArrayVal,ScrDataVal);
    Call(that->Content,Dim,3(&min,&max,&nb));
	return max;
}
static ScrString *ArrayValString(ScrDataVal *this) { return &ScrStringNull; }
static ScrPath *ArrayValPath(ScrDataVal *this) { return &ScrPathNull; }
static ScrVar *ArrayValPtrTgt(ScrDataVal *this) { return &ScrVarNull; }
static ScrArray *ArrayValArray(ScrDataVal *this) {
	ThisToThat(ArrayVal,ScrDataVal);
	return that->Content;
}
static ScrDataVal *ArrayValGetField(ScrDataVal *this,int Idx) {
	ScrCatalogEntry r;
	ThisToThat(ArrayVal,ScrDataVal);
	r = Call(that->Content,GetElt,1(Idx));
	return r.Data;
}
ScrDataVal *SDValArray(ScrArray *Content) {
	ArrayVal *r;
	static struct ScrDataVal Static = {
		ArrayValBrowse,ArrayValEval,ArrayValInt,ArrayValString,
		ArrayValPath,ArrayValPtrTgt,ArrayValArray,ArrayValGetField
	};
	rPush(r); r->ScrDataVal.Static = &Static;
	r->Content = Content;
	return &r->ScrDataVal;
}

    /*----------------*/

typedef struct {
	ScrDataExpr ScrDataExpr;
	ScrType *Type;
	ScrArray *Content;
} ArrayExpr;
static ScrDataExpr *ArrayExprBrowse(ScrDataExpr *this,ScrExprBrowser *Tgt) {
	ThisToThat(ArrayExpr,ScrDataExpr);
    return Call(Tgt,Vector,2(that->Type,that->Content));
}
static ScrType *ArrayExprGetType(ScrDataExpr *this) {
	ThisToThat(ArrayExpr,ScrDataExpr);
	return that->Type;
}
static ScrCatalogEntry ArrayExprTranslate(int num,ScrCatalogEntry Elt,void *Clos) {
	ScrDataCtx *ctx;
	ScrDataExpr *elt;
	ScrCatalogEntry r;
	ctx = Clos; elt = Elt.Data;
	r.Data= ScrLazyData(elt,ctx); /* Should be Lazy eval, if elt are allowed to reference other elts. */
	return r;
}
static ScrDataVal *ArrayExprEval(ScrDataExpr *this,ScrDataCtx *Ctx) {
	ScrArray *val;
	ScrCatalogEntry DefExpr,DefData;
	ScrDataExpr *defExpr;
	ThisToThat(ArrayExpr,ScrDataExpr);
	DefExpr = Call(that->Content,GetDefault,0); defExpr = DefExpr.Data;
	DefData.Data = Call(defExpr,Eval,1(Ctx));
    val = Call(that->Content,Translate,3(DefData,ArrayExprTranslate,Ctx));
	return SDValArray(val);
}

static ScrDataExpr *RawArrayExpr(ScrExprBrowser *this,ScrType *Type,ScrArray *Content){
	ArrayExpr *r;
	static struct ScrDataExpr Static = {
		ArrayExprBrowse,ArrayExprGetType,ArrayExprEval
	};
	ThisToThat(RawExpr,ScrExprBrowser);
	mPush(that->Mem,r); r->ScrDataExpr.Static = &Static;
	/*ToDo: Content should be duplicated */
	r->Type = Type; r->Content = Content;
	return &r->ScrDataExpr;
}

    /*----------------*/

typedef struct {
	ScrDataExpr ScrDataExpr;
	ScrDataExpr *Array;
	ScrDataExpr *Idx;
} ArrayElt;

static ScrDataExpr *ArrayEltBrowse(ScrDataExpr *this,ScrExprBrowser *Tgt) { 
	ThisToThat(ArrayElt,ScrDataExpr);
	return Call(Tgt,EltSelect,2(that->Array,that->Idx));
}
static ScrType *ArrayEltGetType(ScrDataExpr *this){
	ScrType *arrayType;
	ThisToThat(ArrayElt,ScrDataExpr);
    arrayType = Call(that->Array,GetType,0);
	return Call(arrayType,Array.EltType,0);
}
static ScrDataVal *ArrayEltEval(ScrDataExpr *this,ScrDataCtx *Ctx){
	ScrDataVal *r;
	ScrDataVal *Array,*Idx;
	int idx;
	ThisToThat(ArrayElt,ScrDataExpr);
    Array = Call(that->Array,Eval,1(Ctx));
    Idx = Call(that->Idx,Eval,1(Ctx));
	idx = Call(Idx,Int,0);
	return Call(Array,GetField,1(idx));
}
static ScrDataExpr *RawEltSelect(ScrExprBrowser *this,ScrDataExpr *Array,ScrDataExpr *Idx) {
	ArrayElt *r;
	static struct ScrDataExpr Static = {
        ArrayEltBrowse, ArrayEltGetType, ArrayEltEval
	};
	ThisToThat(RawExpr,ScrExprBrowser);
	mPush(that->Mem,r); r->ScrDataExpr.Static = &Static;
	r->Array = Array; r->Idx = Idx;
	return &r->ScrDataExpr;
}

/*__________________________________
 |
 |
 |__________________________________
*/

static ScrDataVal *RawDataInt(ScrDataBrowser *this,int i) {
	ScrDataVal *r;
	ThisToThat(RawData,ScrDataBrowser);
	mIn(that->Mem,r = SDValInt(i));
	return r;
}
static ScrDataVal *RawDataString(ScrDataBrowser *this,ScrString *String) {
	int sz,sa,align;
	ScrDataVal *r;
	char *d;
	ThisToThat(RawData,ScrDataBrowser);
	align = sizeof(void *);
	sz = Call(String,Size,0)+1;
	sa = (sz+align-1) & (-align);
	mnPush(that->Mem,d,sa);
	Call(String,Copy,3(d,0,sz-1));
	d[sz-1]=0;
	mIn(that->Mem,r=SDValString(ScrStringDirect(d)));
	return r;
}
static ScrDataVal *RawDataComplex(ScrDataBrowser *this,ScrType *Type,ScrDataVal **Val) {
	ThisToThat(RawData,ScrDataBrowser);
	/* ToDo, however not required right now */
	return  &ScrDataValNull;
}
static ScrDataVal *RawDataVector(ScrDataBrowser *this,ScrArray *Content) {
	ThisToThat(RawData,ScrDataBrowser);
	/* ToDo, however not required right now */
	return &ScrDataValNull;
}
static ScrDataVal *RawDataPtr(ScrDataBrowser *this,ScrDataVal *Tgt) {
	ThisToThat(RawData,ScrDataBrowser);
	/* ToDo */
	return &ScrDataValNull;
}
static RawData *ScrRawData(MemStack *Mem) {
	RawData *r;
	static struct ScrDataBrowser Static = {
		RawDataInt,RawDataString,RawDataComplex,RawDataVector,RawDataPtr
	};
	mPush(Mem,r); r->Mem = Mem; r->ScrDataBrowser.Static = &Static;
	return r;
}

/*______________________________
 |
 | ScrExprAddress
 |______________________________
*/

typedef struct {
	ScrDataExpr ScrDataExpr;
	ScrType *Type;
    ScrDataTgt *Tgt;
} TgtAddress;
static ScrDataExpr *TgtAddressBrowse(ScrDataExpr *this,ScrExprBrowser *Browser) { 
	ThisToThat(TgtAddress,ScrDataExpr);
	return Call(Browser,Ptr,1(that->Tgt));
}
static ScrType *TgtAddressGetType(ScrDataExpr *this) {
	ThisToThat(TgtAddress,ScrDataExpr);
	return that->Type;
}
static ScrDataVal *TgtAddressEval(ScrDataExpr *this,ScrDataCtx *Ctx) {
	ScrVar *v;
	ThisToThat(TgtAddress,ScrDataExpr);
	v = Call(that->Tgt,Eval,1(Ctx));
    return ScrValAddress(v);
}
static ScrDataExpr *RawAddress(ScrExprBrowser *this,ScrDataTgt *Data) {
	TgtAddress *r;
	ScrType *t;
	static struct ScrDataExpr Static = {
		TgtAddressBrowse,TgtAddressGetType,TgtAddressEval
	};
	ThisToThat(RawExpr,ScrExprBrowser);
	mPush(that->Mem,r); r->ScrDataExpr.Static = &Static;
	r->Tgt=Data; t = Call(Data,GetType,0); r->Type = Call(t,GetPtr,0);
	return &r->ScrDataExpr;
}

/*___________________________
 |
 | Var Value
 |___________________________
*/

typedef struct {
	ScrDataExpr ScrDataExpr;
	ScrDataTgt *Var;
} VarValExpr;
static ScrDataExpr *VarValBrowse(ScrDataExpr *this,ScrExprBrowser *Tgt) {
	ThisToThat(VarValExpr,ScrDataExpr);
	return Call(Tgt,Variable,1(that->Var));
}
static ScrType *VarValGetType(ScrDataExpr *this) {
    ThisToThat(VarValExpr,ScrDataExpr);
    return Call(that->Var,GetType,0);
}
static ScrDataVal *VarValEval(ScrDataExpr *this,ScrDataCtx *Ctx){
	ScrVar *Var;
	ThisToThat(VarValExpr,ScrDataExpr);
    Var = Call(that->Var,Eval,1(Ctx));
	return Call(Var,Value,0);
}
static ScrDataExpr *RawVariable(ScrExprBrowser *this,ScrDataTgt *Var)  {
	VarValExpr *r;
	static struct ScrDataExpr Static = {
		VarValBrowse,VarValGetType,VarValEval
	};
	ThisToThat(RawExpr,ScrExprBrowser);
	mPush(that->Mem,r); r->ScrDataExpr.Static = &Static;
	r->Var = Var;
	return &r->ScrDataExpr;
}

/*__________________________________
 |
 |
 |__________________________________
*/

#include <DataTgt.Loc.h>
#include <DataInstr.Loc.h>

static ScrDataTgt *RawVarTargeted(ScrExprBrowser *this,ScrDataExpr *Ptr) {
	ScrDataTgt *r;
	ThisToThat(RawExpr,ScrExprBrowser);
	mIn(that->Mem,r=ScrTargetedVar(Ptr));
	return r;
}
static ScrDataTgt *RawVarEltAccess(ScrExprBrowser *this,ScrDataTgt *Base,ScrDataExpr *Idx) {
	ScrDataTgt *r;
	ThisToThat(RawExpr,ScrExprBrowser);
	mIn(that->Mem,r=ScrTgtEltAccess(Base,Idx));
	return r;

}
static ScrDataTgt *RawVarFieldAccess(ScrExprBrowser *this,ScrDataTgt *Base,int Idx) {
	ScrDataTgt *r;
	ThisToThat(RawExpr,ScrExprBrowser);
	mIn(that->Mem,r=ScrTgtFieldAccess(Base,Idx));
	return r;
}
static ScrDataTgt *RawVarResult(ScrExprBrowser *this,int Depth,ScrType *Type,int Idx) {
	ScrDataTgt *r;
	ThisToThat(RawExpr,ScrExprBrowser);
	mIn(that->Mem,r=ScrTgtResult(Depth,Type,Idx));
	return r;
}
static ScrDataTgt *RawVarLocal(ScrExprBrowser *this,int Depth,ScrType *Type,int Idx) {
	ScrDataTgt *r;
	ThisToThat(RawExpr,ScrExprBrowser);
	mIn(that->Mem,r=ScrTgtLocal(Depth,Type,Idx));
	return r;
}
static ScrDataTgt *RawVarExtern(ScrExprBrowser *this,ScrType *Type,int Module,int Idx) {
	ScrDataTgt *r;
	ThisToThat(RawExpr,ScrExprBrowser);
	mIn(that->Mem,r=ScrTgtExtern(Type,Module,Idx));
	return r;
}
static ScrVar *RawVarConst(ScrExprBrowser *this,ScrDataVal *Val) {
	ScrVar *r;
	ScrDataVal *v;
	ThisToThat(RawExpr,ScrExprBrowser);
	v = Call(Val,Browse,1(&that->Data->ScrDataBrowser));
	mIn(that->Mem,r=ScrConstVar(v));
	return r;
}
static ScrDataExpr *RawVarProcedure(ScrExprBrowser *this,int Depth,ScrType *Prototype,ScrType *Local,ScrInstruction *Body) {
	ScrDataExpr *r;
	ThisToThat(RawExpr,ScrExprBrowser);
	mIn(that->Mem,r=ScrValProcedure(Depth,Prototype,Local,Body));
	return r;
}

static ScrInstruction *RawInstrSet(ScrExprBrowser *this,ScrDataTgt *tgt,ScrDataExpr *Val) {
	ScrInstruction *r;
	ThisToThat(RawExpr,ScrExprBrowser);
	mIn(that->Mem,r=ScrInstrSet(tgt,Val));
	return r;
}
static ScrInstruction *RawInstrSequence(ScrExprBrowser *this,ScrInstruction **Begin,ScrInstruction **End) {
	ScrInstruction *r;
	ThisToThat(RawExpr,ScrExprBrowser);
	mIn(that->Mem,r=ScrInstrSequence(Begin,End));
	return r;
}
static ScrInstruction *RawInstrWhile(ScrExprBrowser *this,ScrDataExpr *Cond,ScrInstruction *Inst) {
	ScrInstruction *r;
	ThisToThat(RawExpr,ScrExprBrowser);
	mIn(that->Mem,r=ScrInstrWhile(Cond,Inst));
	return r;
}
static ScrInstruction *RawInstrUntil(ScrExprBrowser *this,ScrInstruction *Inst,ScrDataExpr *Cond) {
	ScrInstruction *r;
	ThisToThat(RawExpr,ScrExprBrowser);
	mIn(that->Mem,r=ScrInstrUntil(Inst,Cond));
	return r;
}
static ScrInstruction *RawInstrCond(ScrExprBrowser *this,ScrDataExpr *Cond,ScrInstruction *t) {
	ScrInstruction *r;
	ThisToThat(RawExpr,ScrExprBrowser);
	mIn(that->Mem,r=ScrInstrCond(Cond,t));
	return r;
}
static ScrInstruction *RawInstrAlternate(ScrExprBrowser *this,ScrDataExpr *c,ScrInstruction *t,ScrInstruction *f) {
	ScrInstruction *r;
	ThisToThat(RawExpr,ScrExprBrowser);
	mIn(that->Mem,r=ScrInstrAlternate(c,t,f));
	return r;
}
static ScrInstruction *RawInstrSelect(ScrExprBrowser *this,ScrDataExpr *Select,ScrArray *Choices) {
	ScrInstruction *r;
	ThisToThat(RawExpr,ScrExprBrowser);
	mIn(that->Mem,r=ScrInstrSelect(Select,Choices));
	return r;
}
static ScrInstruction *RawInstrInvoke(ScrExprBrowser *this,ScrDataTgt *Result,ScrDataExpr *Param,int Depth,ScrType *Local,ScrInstruction *Block) {
	ScrInstruction *r;
	ThisToThat(RawExpr,ScrExprBrowser);
	mIn(that->Mem,r=ScrInstrInvoke(Result,Param,Depth,Local,Block));
	return r;
}
static ScrInstruction *RawInstrExtend(ScrExprBrowser *this,ScrDataTgt *ArrayPtr,ScrDataExpr *Nb) {
	ScrInstruction *r;
	ThisToThat(RawExpr,ScrExprBrowser);
	mIn(that->Mem,r=ScrInstrLocalArray(ArrayPtr,Nb));
	return r;
}

ScrExprBrowser *ScrRawExpr(void) {
	MemStack *Mem;
	RawExpr *r;
	static struct ScrExprBrowser Static = {
        RawExprConst,RawExprCond,RawExprIntAlter,RawExprIntCompose,RawExprComplex,
		RawExprApply,RawExprField,RawArrayExpr,RawEltSelect,
		RawVariable,RawAddress,
		RawExprParam,RawExprLocal,RawExprExtern,
		{
			RawVarTargeted,RawVarEltAccess,RawVarFieldAccess,
			RawVarResult,RawVarLocal,RawVarExtern,RawVarConst,RawVarProcedure
		},
		{
			RawInstrSet,RawInstrSequence,RawInstrWhile,RawInstrUntil,RawInstrCond,
			RawInstrAlternate,RawInstrSelect,RawInstrInvoke,RawInstrExtend
		}
	};
	Mem = rFork(4096); mPush(Mem,r); r->Mem = Mem; r->ScrExprBrowser.Static = &Static;
	r->Data = ScrRawData(Mem);
	return &r->ScrExprBrowser;
}

/*_________________________________
 |
 |
 |_________________________________
*/

int ScrIntExprEvalAttempt(int *value,ScrDataExpr *Expr) {
	int Success;
	ScrDataVal *V;
	Success = (0==0);
	V = Call(Expr,Eval,1(&ScrDataCtxNull));
	Success = (V!=&ScrDataValNull);
	if (Success) { *value = Call(V,Int,0);}
	return Success;
}

