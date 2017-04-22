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

/*______________________________
 |
 | Null
 |______________________________
*/

static ScrDataVal *ValNullBrowse(ScrDataVal *this,ScrDataBrowser *Tgt) {return &ScrDataValNull;}
// static ScrType *ValNullGetType(ScrDataVal *this) { return &ScrTypeNull; }
static ScrDataVal *ValNullEval(ScrDataVal *this,ScrDataCtx *Ctx,ScrVar *Result,ScrDataVal *Param) { 
	return &ScrDataValNull; 
}
static int ValNullInt(ScrDataVal *this) { return 0; }
static ScrString *ValNullString(ScrDataVal *this) { return &ScrStringNull;}
static ScrPath *ValNullPath(ScrDataVal *this) { return &ScrPathNull; }
static ScrVar *ValNullPtrTgt(ScrDataVal *this) { return &ScrVarNull; }
static ScrArray *ValNullArray(ScrDataVal *this) { return ScrArrayNull(&ScrDataValNull); }
static ScrDataVal *ValNullGetField(ScrDataVal *this,int Idx) { return &ScrDataValNull; }

static struct ScrDataVal ScrDataValNullStatic = {
	ValNullBrowse, /*ValNullGetType,*/ ValNullEval,
	ValNullInt,ValNullString,ValNullPath,ValNullPtrTgt,ValNullArray,
	ValNullGetField
};
ScrDataVal ScrDataValNull = {&ScrDataValNullStatic};


/*___________________________________
 |
 | Atom
 |___________________________________
*/

static ScrDataVal *AtomGetField(ScrDataVal *this,int Idx) { return this; }
static ScrDataVal *AtomEval(ScrDataVal *this,ScrDataCtx *Ctx,ScrVar *Result,ScrDataVal *Param) { 
	Call(Result,Set,1(this));
	return this; 
}
static ScrPath *AtomPath(ScrDataVal *this) { return &ScrPathNull; }
static ScrVar *AtomPtrTgt(ScrDataVal *this) { return &ScrVarNull; } 
static ScrArray *AtomArray(ScrDataVal *this) { return ScrOneEltArray(&ScrDataValNull,this);}

/*___________________________________
 |
 | Lazy Data
 |___________________________________
*/

typedef struct {
	ScrDataVal ScrDataVal;
    ScrDataExpr *Modele;
	ScrDataCtx *Ctx;
	ScrDataVal *Value;
} LazyData;
static ScrDataVal *LazyDataBrowse(ScrDataVal *this,ScrDataBrowser *Tgt) {
	ThisToThat(LazyData,ScrDataVal);
	if (!that->Value) { that->Value = Call(that->Modele,Eval,1(that->Ctx));}
	return Call(that->Value,Browse,1(Tgt));
}
static ScrDataVal *LazyDataEval(ScrDataVal *this,ScrDataCtx *Ctx,ScrVar *Result,ScrDataVal *Param) {
	ThisToThat(LazyData,ScrDataVal);
	if (!that->Value) { that->Value = Call(that->Modele,Eval,1(that->Ctx)); }
	return Call(that->Value,Eval,3(Ctx,Result,Param));
}
static int LazyDataInt(ScrDataVal *this) {
	ThisToThat(LazyData,ScrDataVal);
	if (!that->Value) { that->Value = Call(that->Modele,Eval,1(that->Ctx)); }
	return Call(that->Value,Int,0);
}
static ScrString *LazyDataString(ScrDataVal *this) {
	ThisToThat(LazyData,ScrDataVal);
	if (!that->Value) { that->Value = Call(that->Modele,Eval,1(that->Ctx)); }
	return Call(that->Value,String,0);
}
static ScrPath *LazyDataPath(ScrDataVal *this) {
	ThisToThat(LazyData,ScrDataVal);
	if (!that->Value) { that->Value = Call(that->Modele,Eval,1(that->Ctx));}
	return Call(that->Value,Path,0);
}
static ScrVar *LazyDataPtrTgt(ScrDataVal *this) {
	ThisToThat(LazyData,ScrDataVal);
	if (!that->Value) { that->Value = Call(that->Modele,Eval,1(that->Ctx)); }
	return Call(that->Value,PtrTgt,0);
}
static ScrArray *LazyDataArray(ScrDataVal *this) {
	ThisToThat(LazyData,ScrDataVal);
	if (!that->Value) { that->Value = Call(that->Modele,Eval,1(that->Ctx));}
	return Call(that->Value,Array,0);
}
static ScrDataVal *LazyDataGetField(ScrDataVal *this,int Idx) {
	ThisToThat(LazyData,ScrDataVal);
	if (!that->Value) { that->Value = Call(that->Modele,Eval,1(that->Ctx)); } 
	return Call(that->Value,GetField,1(Idx));
}
ScrDataVal *ScrLazyData(ScrDataExpr *Modele,ScrDataCtx *Ctx) {
	LazyData *r;
	static struct ScrDataVal Static = {
		LazyDataBrowse,LazyDataEval,LazyDataInt,LazyDataString,
		LazyDataPath,LazyDataPtrTgt,LazyDataArray,LazyDataGetField
	};
	rPush(r);
	r->ScrDataVal.Static = &Static;
	r->Modele = Modele; r->Ctx = Ctx; r->Value = 0;
	return &r->ScrDataVal;
}

/*_________________________________
 |
 | Int
 |_________________________________
*/

typedef struct {
	ScrDataVal ScrDataVal;
	int Value;
} ConstInt;
static ScrDataVal *ConstIntBrowse(ScrDataVal *this,ScrDataBrowser *Tgt) {
	ThisToThat(ConstInt,ScrDataVal);
	return Call(Tgt,Int,1(that->Value));
}
static int ConstIntInt(ScrDataVal *this) { ThisToThat(ConstInt,ScrDataVal); return that->Value; }
static ScrString *ConstIntString(ScrDataVal *this) {
	int val;
	val = Call(this,Int,0);
	return ScrStringDecimal(val); 
}
ScrDataVal *SDValInt(int Value) {
	ConstInt *r;
	static struct ScrDataVal Static = {
		ConstIntBrowse,AtomEval,ConstIntInt,
		ConstIntString,AtomPath,AtomPtrTgt,AtomArray,AtomGetField
	};
	rPush(r);
	r->ScrDataVal.Static = &Static;
	r->Value = Value;
    return &r->ScrDataVal;
}
   
   /*----- Char -----*/

ScrDataVal *SValChar(int Value) {
	ConstInt *r;
	static struct ScrDataVal Static = {
		ConstIntBrowse,AtomEval,ConstIntInt,
		ConstIntString,AtomPath,AtomPtrTgt,AtomArray,AtomGetField
	};
	rPush(r);
	r->ScrDataVal.Static = &Static;
	r->Value = Value;
    return &r->ScrDataVal;
}

    /*------- String -------*/

typedef struct {
	ScrDataVal ScrDataVal;
	ScrString *Value;
} ValString;
static ScrDataVal *ValStringBrowse(ScrDataVal *this,ScrDataBrowser *Tgt) {
	ThisToThat(ValString,ScrDataVal);
	return Call(Tgt,String,1(that->Value));
}
static ScrString *ValStringGetString(ScrDataVal *this) { ThisToThat(ValString,ScrDataVal); return that->Value; }
ScrDataVal *SDValString(ScrString *Value) {
	ValString *r;
	static struct ScrDataVal Static = {
		ValStringBrowse,AtomEval,ValNullInt, 
		ValStringGetString,AtomPath,AtomPtrTgt,AtomArray,AtomGetField
	};
	rPush(r);
	r->ScrDataVal.Static = &Static;
	r->Value = Value;
	return &r->ScrDataVal;
}

/*______________________________
 |
 | ScrExprAddress
 |______________________________
*/
typedef struct {
	ScrDataVal ScrDataVal;
	ScrVar *Tgt;
} TgtAddressVal;
static ScrDataVal *TgtAddrValBrowse(ScrDataVal *this,ScrDataBrowser *Tgt) { return &ScrDataValNull; }
static ScrVar *TgtAddrValPtrTgt(ScrDataVal *this) {
	ThisToThat(TgtAddressVal,ScrDataVal);
	return that->Tgt;
}
static ScrArray *TgtAddrValArray(ScrDataVal *this) {
	ThisToThat(TgtAddressVal,ScrDataVal);
	return ScrOneEltArray(&ScrVarNull,that->Tgt);
}
ScrDataVal *ScrValAddress(ScrVar *Tgt) {
    TgtAddressVal *r;
	static struct ScrDataVal Static = {
        TgtAddrValBrowse,AtomEval,ValNullInt,ValNullString,
		ValNullPath,TgtAddrValPtrTgt,TgtAddrValArray,AtomGetField
	};
	rPush(r); r->ScrDataVal.Static = &Static;
	r->Tgt = Tgt;
	return &r->ScrDataVal;
}

/*____________________________
 |
 | Procedure
 |____________________________
*/

typedef struct {
	ScrDataVal ScrDataVal;
	ScrType *Type;
	ScrType *Local;
	ScrInstruction *Body;
	int Lvl;
} ProcVal;
static ScrDataVal *ProcValBrowse(ScrDataVal *this,ScrDataBrowser *Tgt) { return this; }
static ScrDataVal *ProcValEval(ScrDataVal *this,ScrDataCtx *Ctx,ScrVar *Result,ScrDataVal *Param) {
    ScrDataCtx *ctx;
	ScrType *tr;
	ThisToThat(ProcVal,ScrDataVal);
	tr = Call(that->Type,Fn.Result,0);
	ctx = Call(Ctx,Exec.Invoke,4(that->Lvl,Result,Param,that->Local));
	Call(that->Body,Perform,1(ctx));
	return Call(Result,Value,0);
}
ScrDataVal *ScrValProcedure(int Depth,ScrType *Prototype,ScrType *Local,ScrInstruction *Body) {
	ProcVal *r;
	static struct ScrDataVal Static = {
        ProcValBrowse,ProcValEval,ValNullInt,ValNullString,
		ValNullPath,ValNullPtrTgt,ValNullArray,ValNullGetField
	};
	rPush(r); r->ScrDataVal.Static = &Static;
	r->Lvl = Depth; r->Type = Prototype; r->Local = Local; r->Body = Body;
	return &r->ScrDataVal;
}

