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
#include <ScrPathExpr.h>

    /*--------*/

static ScrType *PathNullGetType(ScrPathExpr *this,int *IsLValue) { *IsLValue=(0!=0); return &ScrTypeNull; }
static ScrDataTgt *PathNullLeft(ScrPathExpr *this,int *Success) { *Success = (0!=0); return &ScrDataTgtNull; }
static ScrDataExpr *PathNullRight(ScrPathExpr *this) { return &ScrDataExprNull; }
static ScrPathExpr *PathNullAddress(ScrPathExpr *this) { return &ScrPathExprNull; }
static ScrPathExpr *PathNullIdrct(ScrPathExpr *this) { return &ScrPathExprNull; }
static ScrInstruction *PathNullSet(ScrPathExpr *this,ScrDataTgt *Tgt) { return &ScrInstructionNull; }
static struct ScrPathExpr ScrPathExprNullStatic = {
	PathNullGetType,PathNullLeft, PathNullRight, PathNullAddress, PathNullIdrct, PathNullSet
};
ScrPathExpr ScrPathExprNull = {&ScrPathExprNullStatic};

    /*-----------------*/

static ScrType *ValPathGetType(ScrPathExpr *this,int *lv) {
	ScrDataExpr *Expr;
	*lv = (0!=0);
	Expr = Call(this,Right,0);
	return Call(Expr,GetType,0);
}
static ScrType *VarPathGetType(ScrPathExpr *this,int *lv) {
	ScrDataTgt *Tgt;
	int success;
	*lv = (0==0);
	Tgt = Call(this,Left,1(&success));
	return Call(Tgt,GetType,0);
}
typedef struct {
	ScrPathExpr ScrPathExpr;
	ScrExprBrowser *Hld;
} HldPath;
static ScrPathExpr *PathAddress(ScrPathExpr *this) { 
	ThisToThat(HldPath,ScrPathExpr);
	return ScrPathAddress(that->Hld,this); 
}
static ScrPathExpr *PathIdrct(ScrPathExpr *this) { 
	ThisToThat(HldPath,ScrPathExpr);
	return ScrPathIdrct(that->Hld,this); 
}
static ScrInstruction *PathSet(ScrPathExpr *this,ScrDataTgt *Tgt) { 
	ScrDataExpr *val;
	ThisToThat(HldPath,ScrPathExpr);
	val = Call(this,Right,0);
	return Call(that->Hld,Instr.Set,2(Tgt,val)); 
}

       /*-----*/

typedef struct {
	HldPath HldPath;
	ScrDataTgt *Tgt;
} PathVar;
static ScrDataTgt *PathVarLeft(ScrPathExpr *this,int *Success) { 
	ThisToThat(PathVar,HldPath.ScrPathExpr); 
	*Success = (0==0);
	return that->Tgt;
}
static ScrDataExpr *PathVarRight(ScrPathExpr *this) { 
	ThisToThat(PathVar,HldPath.ScrPathExpr);
    return Call(that->HldPath.Hld,Variable,1(that->Tgt));
}
ScrPathExpr *ScrPathExprVar(ScrExprBrowser *Hld,ScrDataTgt *Tgt) {
	PathVar *r;
	static struct ScrPathExpr Static = { VarPathGetType, PathVarLeft, PathVarRight, PathAddress, PathIdrct, PathSet};
	rPush(r); r->HldPath.ScrPathExpr.Static = &Static; r->Tgt = Tgt; r->HldPath.Hld = Hld;
	return &r->HldPath.ScrPathExpr;
}

       /*-----*/

typedef struct {
	HldPath HldPath;
	ScrDataExpr *Val;
} PathData;
static ScrDataTgt *PathDataLeft(ScrPathExpr *this,int *Success) {
	*Success = (0!=0);
	return &ScrDataTgtNull; 
}
static ScrDataExpr *PathDataRight(ScrPathExpr *this) {
	ThisToThat(PathData,HldPath.ScrPathExpr);
	return that->Val;
}
ScrPathExpr *ScrPathExprData(ScrExprBrowser *Hld,ScrDataExpr *Val) {
	PathData *r;
	static struct ScrPathExpr Static = { ValPathGetType, PathDataLeft, PathDataRight, PathAddress, PathIdrct, PathSet };
	rPush(r); r->HldPath.ScrPathExpr.Static = &Static; r->Val = Val; r->HldPath.Hld = Hld;
	return &r->HldPath.ScrPathExpr;
}

      /*------*/

typedef struct {
	HldPath HldPath;
	ScrDataAccess *Acc;
} PathAccess;
static ScrType *PathAccessGetType(ScrPathExpr *this,int *IsLvalue) {
	ScrDataTgt *Tgt;
	ThisToThat(PathAccess,HldPath.ScrPathExpr);
	pOpen
	    pIn(Tgt = Call(that->Acc,GetVar,0));
	    *IsLvalue = (Tgt!=&ScrDataTgtNull);
	pClose
	return Call(that->Acc,GetType,0);
}
static ScrDataTgt *PathAccessLeft(ScrPathExpr *this,int *Success) {
	ScrDataTgt *r;
	ThisToThat(PathAccess,HldPath.ScrPathExpr);
	r = Call(that->Acc,GetVar,0);
	*Success = (r!=&ScrDataTgtNull);
	return r;
}
static ScrDataExpr *PathAccessRight(ScrPathExpr *this) {
	ThisToThat(PathAccess,HldPath.ScrPathExpr);
	return Call(that->Acc,GetValue,0);
}
ScrPathExpr *ScrPathExprAccess(ScrExprBrowser *Hld,ScrDataAccess *Acc) {
	PathAccess *r;
	static struct ScrPathExpr Static = {
		PathAccessGetType,PathAccessLeft,PathAccessRight,PathAddress,PathIdrct,PathSet
	};
	rPush(r); r->HldPath.ScrPathExpr.Static=&Static; r->Acc=Acc; r->HldPath.Hld = Hld;
	return &r->HldPath.ScrPathExpr;
}


      /*------*/

typedef struct {
	HldPath HldPath;
	ScrPathExpr *Base;
	int Id;
} PathField;
static ScrType *PathFieldGetType(ScrPathExpr *this,int *lv) {
	ScrType *Base;
	char *discard;
	ThisToThat(PathField,HldPath.ScrPathExpr);
	Base = Call(that->Base,GetType,1(lv));
	return Call(Base,Struct.Elt,2(&discard,that->Id));
}
static ScrDataTgt *PathFieldLeft(ScrPathExpr *this,int *Success) {
	ScrDataTgt *Base;
	ThisToThat(PathField,HldPath.ScrPathExpr);
    Base = Call(that->Base,Left,1(Success));
	return (*Success)?Call(that->HldPath.Hld,Var.FieldAccess,2(Base,that->Id)):&ScrDataTgtNull;
}
static ScrDataExpr *PathFieldRight(ScrPathExpr *this) {
	ScrDataExpr *Base;
	ThisToThat(PathField,HldPath.ScrPathExpr);
	Base = Call(that->Base,Right,0);
	return Call(that->HldPath.Hld,FieldSelect,2(Base,that->Id));
}
ScrPathExpr *ScrPathExprField(ScrExprBrowser *Hld,ScrPathExpr *Base,int Id) {
	PathField *r;
	static struct ScrPathExpr Static = {PathFieldGetType,PathFieldLeft,PathFieldRight,PathAddress,PathIdrct,PathSet};
	rPush(r); r->HldPath.ScrPathExpr.Static = &Static; r->Base = Base; r->Id = Id; r->HldPath.Hld = Hld;
	return &r->HldPath.ScrPathExpr;
}

   /*-----------*/

typedef struct {
	HldPath HldPath;
	ScrPathExpr *Base;
	ScrDataExpr *Id;
} PathElt;
static ScrType *PathEltGetType(ScrPathExpr *this,int *lv) {
	ScrType *Base;
	ThisToThat(PathElt,HldPath.ScrPathExpr);
    Base = Call(that->Base,GetType,1(lv));
	return Call(Base,Array.EltType,0);
}
static ScrDataTgt *PathEltLeft(ScrPathExpr *this,int *Success) {
	ScrDataTgt *Base;
	ThisToThat(PathElt,HldPath.ScrPathExpr);
	Base = Call(that->Base,Left,1(Success));
	return (*Success)?Call(that->HldPath.Hld,Var.EltAccess,2(Base,that->Id)):&ScrDataTgtNull;
}
static ScrDataExpr *PathEltRight(ScrPathExpr *this) {
	ScrDataExpr *Base;
    ThisToThat(PathElt,HldPath.ScrPathExpr);
	Base = Call(that->Base,Right,0);
	return Call(that->HldPath.Hld,EltSelect,2(Base,that->Id));
}
ScrPathExpr *ScrPathExprElt(ScrExprBrowser *Hld,ScrPathExpr *Base,ScrDataExpr *Id) {
	PathElt *r;
	static struct ScrPathExpr Static = {PathEltGetType,PathEltLeft,PathEltRight,PathAddress,PathIdrct,PathSet};
	rPush(r); r->HldPath.ScrPathExpr.Static = &Static; r->Base=Base; r->Id=Id; r->HldPath.Hld = Hld;
	return &r->HldPath.ScrPathExpr;
}

   /*-----------*/

typedef struct {
	HldPath HldPath;
	ScrPathExpr *Base;
	ScrDataExpr *Param;
} PathApply;
static ScrType *PathApplyGetType(ScrPathExpr *this,int *lv) {
	ScrType *Base;
	ThisToThat(PathApply,HldPath.ScrPathExpr);
	Base = Call(that->Base,GetType,1(lv)); *lv = (0!=0); 
    return Call(Base,Fn.Result,0);
}
static ScrDataTgt *PathApplyLeft(ScrPathExpr *this,int *Success) { 
	*Success = (0!=0);
	return &ScrDataTgtNull; 
}
static ScrDataExpr *PathApplyRight(ScrPathExpr *this) {
	ScrDataExpr *Base;
	ThisToThat(PathApply,HldPath.ScrPathExpr);
	Base = Call(that->Base,Right,0);
	return Call(that->HldPath.Hld,Apply,2(Base,that->Param));
}
ScrPathExpr *ScrPathExprApply(ScrExprBrowser *Hld,ScrPathExpr *Base,ScrDataExpr *Param) {
	PathApply *r;
	static struct ScrPathExpr Static = {
		PathApplyGetType,PathApplyLeft,PathApplyRight,PathNullAddress,PathIdrct,PathSet
	};
	rPush(r); r->HldPath.ScrPathExpr.Static = &Static; r->Base=Base; r->Param=Param; r->HldPath.Hld = Hld;
	return &r->HldPath.ScrPathExpr;
}

   /*-----------*/

typedef struct {
	HldPath HldPath;
	ScrType *Type;
	ScrPathExpr *Base;
} PathAddr;
static ScrType *PathAddressGetType(ScrPathExpr *this,int *lv) {
	ThisToThat(PathAddr,HldPath.ScrPathExpr);
	*lv = (0!=0);
	return that->Type;
}
static ScrDataTgt *PathAddressLeft(ScrPathExpr *this,int *Success) { 
	*Success = (0!=0);
	return &ScrDataTgtNull; 
}
static ScrDataExpr *PathAddressRight(ScrPathExpr *this) {
	ScrDataTgt *Base;
	int Success;
	ThisToThat(PathAddr,HldPath.ScrPathExpr);
	Base = Call(that->Base,Left,1(&Success));
	return Success ? Call(that->HldPath.Hld,Ptr,1(Base)):&ScrDataExprNull;
}
static ScrPathExpr *PathAddressIdrct(ScrPathExpr *this) {
	ThisToThat(PathAddr,HldPath.ScrPathExpr);
	return that->Base;
}
ScrPathExpr *ScrPathAddress(ScrExprBrowser *Hld,ScrPathExpr *Base) {
	PathAddr *r;
	ScrType *t;
	int discard;
	static struct ScrPathExpr Static = {
		PathAddressGetType,PathAddressLeft,PathAddressRight,PathNullAddress,PathAddressIdrct,PathSet
	};
	t = Call(Base,GetType,1(&discard));
	rPush(r); r->HldPath.ScrPathExpr.Static = &Static; r->Base = Base; r->Type = Call(t,GetPtr,0); r->HldPath.Hld = Hld;
	return &r->HldPath.ScrPathExpr;
}

    /*------------*/

typedef struct {
	HldPath HldPath;
	ScrPathExpr *Ptr;
} tPathIdrct;
static ScrType *PathIdrctType(ScrPathExpr *this,int *lv) {
	ScrType *ptr;
	ThisToThat(tPathIdrct,HldPath.ScrPathExpr);
	ptr = Call(that->Ptr,GetType,1(lv));
	*lv = (0==0); /* ToDo : check for the status of *Ptr instead. */
    return Call(ptr,Ptr.Target,0);
}
static ScrDataTgt *PathIdrctLeft(ScrPathExpr *this,int *Success) {
	ScrDataExpr *Ptr;
	ThisToThat(tPathIdrct,HldPath.ScrPathExpr);
	*Success = (0==0);
	Ptr = Call(that->Ptr,Right,0);
	return Call(that->HldPath.Hld,Var.Targeted,1(Ptr));
}
static ScrDataExpr *PathIdrctRight(ScrPathExpr *this) {
	ScrDataExpr *Ptr;
	ScrDataTgt *Var;
	ThisToThat(tPathIdrct,HldPath.ScrPathExpr);
    Ptr = Call(that->Ptr,Right,0);
	Var = Call(that->HldPath.Hld,Var.Targeted,1(Ptr));
	return Call(that->HldPath.Hld,Variable,1(Var));
}
static ScrPathExpr *PathIdrctAddress(ScrPathExpr *this) {
	ThisToThat(tPathIdrct,HldPath.ScrPathExpr);
	return that->Ptr;
}
ScrPathExpr *ScrPathIdrct(ScrExprBrowser *Hld,ScrPathExpr *Ptr) {
	tPathIdrct *r;
	static struct ScrPathExpr Static = {
		PathIdrctType,PathIdrctLeft,PathIdrctRight,PathIdrctAddress,PathIdrct,PathSet
	};
	rPush(r); r->HldPath.ScrPathExpr.Static = &Static; r->Ptr = Ptr; r->HldPath.Hld = Hld;
	return &r->HldPath.ScrPathExpr;
}

/*-------------------------*/

typedef struct {
	ScrPathExpr ScrPathExpr;
	ScrPathExpr *P,*R;
} PathRorP;
static ScrType *PathRorPGetType(ScrPathExpr *this,int *lv) {
	ScrType *r;
	ThisToThat(PathRorP,ScrPathExpr);
	r = Call(that->P,GetType,1(lv));
	*lv = (0==0);
	return r;
}
static ScrDataTgt *PathRorPLeft(ScrPathExpr *this,int *Success) {
	ThisToThat(PathRorP,ScrPathExpr);
	return Call(that->R,Left,1(Success));
}
static ScrDataExpr *PathRorPRight(ScrPathExpr *this) {
	ThisToThat(PathRorP,ScrPathExpr);
	return Call(that->P,Right,0);
}
static ScrPathExpr *PathRorPAddress(ScrPathExpr *this){
	ThisToThat(PathRorP,ScrPathExpr);
	return Call(that->P,Address,0); // ambigues at best; but R is supposed to be only there to receive result...
}
static ScrPathExpr *PathRorPIdrct(ScrPathExpr *this) {
	ThisToThat(PathRorP,ScrPathExpr);
	return Call(that->P,Idrct,0);
}
static ScrInstruction *PathRorPSet(ScrPathExpr *this,ScrDataTgt *Tgt) {
	ThisToThat(PathRorP,ScrPathExpr);
	return Call(that->P,Set,1(Tgt));
}
ScrPathExpr *ScrPathExprRorP(ScrPathExpr *R,ScrPathExpr *P) {
	PathRorP *r;
	static struct ScrPathExpr Static = {
		PathRorPGetType,PathRorPLeft,PathRorPRight,PathRorPAddress,PathRorPIdrct,PathRorPSet
	};
	rPush(r); r->ScrPathExpr.Static = &Static; r->R = R; r->P = P;
	return &r->ScrPathExpr;
}

/*-------------------------*/

typedef struct {
	HldPath HldPath;
	ScrType *Type;
	ScrPathExpr *Org,*End;
} PathPath;
static ScrType *PathPathGetType(ScrPathExpr *this,int *lv) {
    ThisToThat(PathPath,HldPath.ScrPathExpr);
	*lv = (0!=0);
	return that->Type;
}
static ScrDataExpr *PathPathRight(ScrPathExpr *this) {
	ThisToThat(PathPath,HldPath.ScrPathExpr);
	/* ToDo */
	return &ScrDataExprNull;
}
ScrPathExpr *ScrPathPath(ScrExprBrowser *Hld,ScrType *Type,ScrPathExpr *End,ScrPathExpr *Org) {
	PathPath *r;
	static struct ScrPathExpr Static = {
		PathPathGetType,PathDataLeft,PathDataRight,PathAddress,PathIdrct,PathSet
	};
	rPush(r); r->HldPath.ScrPathExpr.Static = &Static; r->Org = Org; r->End = End; r->Type = Type; r->HldPath.Hld = Hld;
	return &r->HldPath.ScrPathExpr;
}


