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

#include <DataTgt.Loc.h>

/*________________________________
 |
 | Var <-> Val Bridge
 |________________________________
*/

typedef struct {
	ScrVar ScrVar;
	int Idx;
	ScrVar *Base;
} ConstEltVar;
static ScrDataVal *ConstEltValue(ScrVar *this) {
	ScrDataVal *v;
	ThisToThat(ConstEltVar,ScrVar);
    v = Call(that->Base,Value,0);
	return Call(v,GetField,1(that->Idx));
}
static void ConstEltAddress(ScrVar *this,ScrVar **Base,int *idx) {
	ThisToThat(ConstEltVar,ScrVar);
	*Base = that->Base; *idx = that->Idx;
}
static void ConstEltSet(ScrVar *this,ScrDataVal *Val) { }
static ScrVar *ConstEltElt(ScrVar *this,int idx) {
	ConstEltVar *r;
	static struct ScrVar Static = {
		ConstEltValue,ConstEltElt,ConstEltAddress,ConstEltSet
	};
	rPush(r); r->ScrVar.Static = &Static;
	r->Base = this; r->Idx = idx;
	return &r->ScrVar;
}
static ScrVar *ScrConstElt(ScrVar *this,int idx) {
	return ConstEltElt(this,idx);
}

     /*---------*/

typedef struct {
	ScrVar ScrVar;
	ScrDataVal *Value;
} ConstVar;
static ScrDataVal *ConstVarValue(ScrVar *this) { ThisToThat(ConstVar,ScrVar); return that->Value; }
static ScrVar *ConstVarElt(ScrVar *this,int idx) { return ScrConstElt(this,idx); }
static void ConstVarAddress(ScrVar *this,ScrVar **Base,int *Idx) { *Base = this; *Idx = 0; }
static void ConstVarSet(ScrVar *this,ScrDataVal *Value) {}
ScrVar *ScrConstVar(ScrDataVal *Val) {
	ConstVar *r;
	static struct ScrVar Static = {
		ConstVarValue,ConstVarElt,ConstVarAddress,ConstVarSet
	};
	rPush(r); r->ScrVar.Static = &Static;
	r->Value = Val;
	return &r->ScrVar;
}

/*_________________________________
 |
 | Local/Result/Extern
 |_________________________________
*/

typedef struct {
	ScrDataTgt ScrDataTgt;
	ScrType *Type;
	int Idx,Depth;
} LocalVar;

    /*----- Local -----*/

static ScrDataTgt *LocalVarBrowse(ScrDataTgt *this,ScrExprBrowser *Tgt) {
	ThisToThat(LocalVar,ScrDataTgt);
	return Call(Tgt,Var.Local,3(that->Depth,that->Type,that->Idx));
}
static ScrType *LocalVarGetType(ScrDataTgt *this){
	ThisToThat(LocalVar,ScrDataTgt);
	return that->Type;
}
static ScrVar *LocalVarEval(ScrDataTgt *this,ScrDataCtx *Ctx) {
	ScrVar *Base;
	ThisToThat(LocalVar,ScrDataTgt);
	if (that->Idx>0) { Ctx = Call(Ctx,Lexical,1(that->Depth)); }
    Base = Call(Ctx,Exec.Base,0);
	return Call(Base,Elt,1(that->Idx));
}
ScrDataTgt *ScrTgtLocal(int Depth,ScrType *Type,int Idx) {
	LocalVar *r;
	static struct ScrDataTgt Static = {
		LocalVarBrowse,LocalVarGetType,LocalVarEval
	};
	rPush(r); r->ScrDataTgt.Static = &Static;
	r->Depth = Depth; r->Type = Type; r->Idx = Idx;
	return &r->ScrDataTgt;
}

    /*----- Result -----*/

static ScrDataTgt *TgtResultBrowse(ScrDataTgt *this,ScrExprBrowser *Tgt) {
	ThisToThat(LocalVar,ScrDataTgt);
	return Call(Tgt,Var.Result,3(that->Depth,that->Type,that->Idx));
}
static ScrVar *TgtResultEval(ScrDataTgt *this,ScrDataCtx *Ctx) {
	ScrVar *Result;
	ThisToThat(LocalVar,ScrDataTgt);
	if (that->Idx>0) { Ctx = Call(Ctx,Lexical,1(that->Idx));}
	Result = Call(Ctx,Exec.Result,0);
	return Call(Result,Elt,1(that->Idx));
}
ScrDataTgt *ScrTgtResult(int Depth,ScrType *Type,int Idx) {
	LocalVar *r;
	static struct ScrDataTgt Static = {TgtResultBrowse,LocalVarGetType,TgtResultEval};
	rPush(r); r->ScrDataTgt.Static = &Static;
    r->Depth = Depth; r->Type = Type; r->Idx = Idx;
	return &r->ScrDataTgt;
}

    /*----- Extern -----*/

typedef struct {
	ScrDataTgt ScrDataTgt;
    ScrType *Type;
	int Module,Idx;
} ExtrnTgt;

static ScrDataTgt *ExtrnTgtBrowse(ScrDataTgt *this,ScrExprBrowser *Tgt) {
    ThisToThat(ExtrnTgt,ScrDataTgt);
	return Call(Tgt,Var.Extern,3(that->Type,that->Module,that->Idx));
}
static ScrType *ExtrnTgtGetType(ScrDataTgt *this) {
	ThisToThat(ExtrnTgt,ScrDataTgt);
	return that->Type;
}
static ScrVar *ExtrnTgtEval(ScrDataTgt *this,ScrDataCtx *Ctx) {
	ScrVar *Base;
	ThisToThat(ExtrnTgt,ScrDataTgt);
	Ctx = Call(Ctx,Extrn,1(that->Module));
    Base = Call(Ctx,Exec.Base,0);
	return Call(Base,Elt,1(that->Idx));
}

ScrDataTgt *ScrTgtExtern(ScrType *Type,int Module,int Idx) {
	ExtrnTgt *r;
	static struct ScrDataTgt Static = {
		ExtrnTgtBrowse,ExtrnTgtGetType,ExtrnTgtEval
	};
	rPush(r); r->ScrDataTgt.Static = &Static;
	r->Type = Type; r->Module = Module; r->Idx = Idx;
	return &r->ScrDataTgt;
}

/*___________________________________
 |
 | Field/Elt Access
 |___________________________________
*/

    /*----- Field -----*/

typedef struct {
	ScrDataTgt ScrDataTgt;
	ScrDataTgt *Base;
	int Idx;
} FieldAccess;
static ScrDataTgt *FieldAccessBrowse(ScrDataTgt *this,ScrExprBrowser *Tgt) {
	ThisToThat(FieldAccess,ScrDataTgt);
	return Call(Tgt,Var.FieldAccess,2(that->Base,that->Idx));
}
static ScrType *FieldAccessGetType(ScrDataTgt *this) {
	ScrType *Base;
	char *Dummy;
	ThisToThat(FieldAccess,ScrDataTgt);
    Base = Call(that->Base,GetType,0);
	return Call(Base,Struct.Elt,2(&Dummy,that->Idx));
}
static ScrVar *FieldAccessEval(ScrDataTgt *this,ScrDataCtx *Ctx) {
	ScrVar *Base;
	ThisToThat(FieldAccess,ScrDataTgt);
    Base = Call(that->Base,Eval,1(Ctx));
	return Call(Base,Elt,1(that->Idx));
}
ScrDataTgt *ScrTgtFieldAccess(ScrDataTgt *Base,int Idx) {
	FieldAccess *r;
	static struct ScrDataTgt Static = {
		FieldAccessBrowse,FieldAccessGetType,FieldAccessEval
	};
	rPush(r); r->ScrDataTgt.Static = &Static;
	r->Base = Base; r->Idx = Idx;
	return &r->ScrDataTgt;
}

    /*----- Elt -----*/

typedef struct {
	ScrDataTgt ScrDataTgt;
	ScrDataTgt *Base;
	ScrDataExpr *Idx;
} EltAccess;

static ScrDataTgt *EltAccessBrowse(ScrDataTgt *this,ScrExprBrowser *Tgt) {
	ThisToThat(EltAccess,ScrDataTgt);
	return Call(Tgt,Var.EltAccess,2(that->Base,that->Idx));
}
static ScrType *EltAccessGetType(ScrDataTgt *this) {
	ScrType *Base;
	ThisToThat(EltAccess,ScrDataTgt);
    Base = Call(that->Base,GetType,0);
	return Call(Base,Array.EltType,0);
}
static ScrVar *EltAccessEval(ScrDataTgt *this,ScrDataCtx *Ctx) {
	ScrVar *Base;
	ScrDataVal *Idx;
	int idx;
	ThisToThat(EltAccess,ScrDataTgt);
	Base = Call(that->Base,Eval,1(Ctx));
	Idx = Call(that->Idx,Eval,1(Ctx));
	idx = Call(Idx,Int,0);
    return Call(Base,Elt,1(idx));
}
ScrDataTgt *ScrTgtEltAccess(ScrDataTgt *Base,ScrDataExpr *Idx) {
	EltAccess *r;
	static struct ScrDataTgt Static = {
		EltAccessBrowse,EltAccessGetType,EltAccessEval
	};
	rPush(r); r->ScrDataTgt.Static = &Static;
	r->Base = Base; r->Idx = Idx;
	return &r->ScrDataTgt;
}

/*_______________________
 |
 | Ptr
 |_______________________
*/

typedef struct {
	ScrDataTgt ScrDataTgt;
	ScrDataExpr *Ptr;
} TargetedVar;
static ScrDataTgt *TargetedVarBrowse(ScrDataTgt *this,ScrExprBrowser *Tgt) {
	ThisToThat(TargetedVar,ScrDataTgt);
	return Call(Tgt,Var.Targeted,1(that->Ptr));
}
static ScrType *TargetedVarGetType(ScrDataTgt *this) {
	ScrType *Type;
	ThisToThat(TargetedVar,ScrDataTgt);
    Type = Call(that->Ptr,GetType,0);
	return Call(Type,Ptr.Target,0);
}
static ScrVar *TargetedVarEval(ScrDataTgt *this,ScrDataCtx *Ctx) {
	ScrDataVal *Ptr;
	ThisToThat(TargetedVar,ScrDataTgt);
	Ptr = Call(that->Ptr,Eval,1(Ctx));
	return Call(Ptr,PtrTgt,0);
}
ScrDataTgt *ScrTargetedVar(ScrDataExpr *Ptr) {
	TargetedVar *r;
	static struct ScrDataTgt Static = {
		TargetedVarBrowse,TargetedVarGetType,TargetedVarEval
	};
	rPush(r); r->ScrDataTgt.Static = &Static;
    r->Ptr = Ptr;
	return &r->ScrDataTgt;
}


