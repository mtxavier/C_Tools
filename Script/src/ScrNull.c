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

#include <ScrDataExpr.h>


/*________________________________
 |
 | ScrExprBrowserNull
 |________________________________
*/

static ScrDataExpr *BrowserNullConst(ScrExprBrowser *this,ScrType *Type,ScrDataVal *v) { return &ScrDataExprNull;}
static ScrDataExpr *BrowserNullCond(ScrExprBrowser *this,ScrDataExpr *c,ScrDataExpr *t,ScrDataExpr *f){ return &ScrDataExprNull;}
static ScrDataExpr *BrowserNullAlter(ScrExprBrowser *this,ScrDataUOpDesc *Desc,ScrDataExpr *x) { return &ScrDataExprNull;}
static ScrDataExpr *BrowserNullCompose(ScrExprBrowser *this,ScrDataBOpDesc *Desc,ScrDataExpr *x,ScrDataExpr *y) { return &ScrDataExprNull;}
static ScrDataExpr *BrowserNullComplex(ScrExprBrowser *this,int Llvl,ScrType *Type,ScrDataExpr **Result,
	int LocalNb, ScrDataExpr **Local) {  return &ScrDataExprNull;}
static ScrDataExpr *BrowserNullApply(ScrExprBrowser *this,ScrDataExpr *Expr,ScrDataExpr *Param) {return &ScrDataExprNull;}
static ScrDataExpr *BrowserNullFieldSelect(ScrExprBrowser *this,ScrDataExpr *Struct,int Num) {return &ScrDataExprNull;}
static ScrDataExpr *BrowserNullVector(ScrExprBrowser *this,ScrType *Type,ScrArray *Space) {return &ScrDataExprNull;}
static ScrDataExpr *BrowserNullEltSelect(ScrExprBrowser *this,ScrDataExpr *Space,ScrDataExpr *Idx) { return &ScrDataExprNull;}
static ScrDataExpr *BrowserNullVariable(ScrExprBrowser *this,ScrDataTgt *Var) { return &ScrDataExprNull;}
static ScrDataExpr *BrowserNullAddress(ScrExprBrowser *this,ScrDataTgt *Var) { return &ScrDataExprNull;}
static ScrDataExpr *BrowserNullParam(ScrExprBrowser *this,ScrType *Type,int Depth) { return &ScrDataExprNull;}
static ScrDataExpr *BrowserNullLocal(ScrExprBrowser *this,ScrType *Type,int Depth,int Idx) { return &ScrDataExprNull;}
static ScrDataExpr *BrowserNullGlobal(ScrExprBrowser *this,ScrType *Type,int Module,int Idx) { return &ScrDataExprNull;}

static ScrDataTgt *BrowserNullVarTargeted(ScrExprBrowser *this,ScrDataExpr *Ptr) { return &ScrDataTgtNull;}
static ScrDataTgt *BrowserNullVarEltAccess(ScrExprBrowser *this,ScrDataTgt *Base,ScrDataExpr *Idx) { 
	return &ScrDataTgtNull;
}
static ScrDataTgt *BrowserNullVarFieldAccess(ScrExprBrowser *this,ScrDataTgt *Base,int Idx) { return &ScrDataTgtNull;}
static ScrDataTgt *BrowserNullVarResult(ScrExprBrowser *this,int Depth,ScrType *Type,int Idx) {return &ScrDataTgtNull;}
static ScrDataTgt *BrowserNullVarLocal(ScrExprBrowser *this,int Depth,ScrType *Type,int Idx) {return &ScrDataTgtNull;}
static ScrDataTgt *BrowserNullVarExtern(ScrExprBrowser *this,ScrType *Type,int Module,int Idx) {return &ScrDataTgtNull;}
static ScrVar *BrowserNullVarConst(ScrExprBrowser *this,ScrDataVal *Val) {return &ScrVarNull;}
static ScrDataExpr *BrowserNullVarProcedure(
	ScrExprBrowser *this,int Depth,ScrType *Prototype,ScrType *Local,ScrInstruction *Body
) { return &ScrDataExprNull; }

static ScrInstruction *BrowserNullInstrSet(ScrExprBrowser *this,ScrDataTgt *Tgt,ScrDataExpr *Val) { 
	return &ScrInstructionNull; 
}
static ScrInstruction *BrowserNullInstrSequence(ScrExprBrowser *this,ScrInstruction **Begin,ScrInstruction **End) { 
	return &ScrInstructionNull; 
}
static ScrInstruction *BrowserNullInstrWhile(ScrExprBrowser *this,ScrDataExpr *Cond,ScrInstruction *Instr) { 
	return &ScrInstructionNull; 
}
static ScrInstruction *BrowserNullInstrUntil(ScrExprBrowser *this,ScrInstruction *Instr,ScrDataExpr *Cond) { 
	return &ScrInstructionNull; 
}
static ScrInstruction *BrowserNullInstrCond(ScrExprBrowser *this,ScrDataExpr *Cond,ScrInstruction *t) { 
	return &ScrInstructionNull; 
}
static ScrInstruction *BrowserNullInstrAlternate(ScrExprBrowser *this,ScrDataExpr *C,ScrInstruction *t,ScrInstruction *f) {
	return &ScrInstructionNull; 
}
static ScrInstruction *BrowserNullInstrSelect(ScrExprBrowser *this,ScrDataExpr *Select,ScrArray *Choices) { 
	return &ScrInstructionNull; 
}
static ScrInstruction *BrowserNullInstrInvoke(
	ScrExprBrowser *this,ScrDataTgt *r,ScrDataExpr *p,int lvl,ScrType *Lcl,ScrInstruction *Block
) { 
	return &ScrInstructionNull; 
}
static ScrInstruction *BrowserNullInstrExtend(ScrExprBrowser *this,ScrDataTgt *ArrayPtr,ScrDataExpr *Nb) { 
	return &ScrInstructionNull; 
} 

static struct ScrExprBrowser BrowserNullStatic = {
	BrowserNullConst,BrowserNullCond,BrowserNullAlter,BrowserNullCompose,
	BrowserNullComplex,BrowserNullApply,BrowserNullFieldSelect,
	BrowserNullVector,BrowserNullEltSelect,
	BrowserNullVariable,BrowserNullAddress,
	BrowserNullParam,BrowserNullLocal,BrowserNullGlobal,
	{ 
		BrowserNullVarTargeted,BrowserNullVarEltAccess,BrowserNullVarFieldAccess,
		BrowserNullVarResult,BrowserNullVarLocal,BrowserNullVarExtern,
		BrowserNullVarConst,BrowserNullVarProcedure
	},
	{
		BrowserNullInstrSet,BrowserNullInstrSequence,BrowserNullInstrWhile,BrowserNullInstrUntil,
		BrowserNullInstrCond,BrowserNullInstrAlternate,BrowserNullInstrSelect,BrowserNullInstrInvoke,
		BrowserNullInstrExtend
	}
};
ScrExprBrowser ScrExprBrowserNull = {&BrowserNullStatic};

     /*--------*/

static ScrVar *PathNullFollow(ScrPath *this,ScrVar *Base) { return &ScrVarNull; }
static struct ScrPath PathNullStatic = {PathNullFollow};
ScrPath ScrPathNull = {&PathNullStatic};

/*_________________________________
 |
 | DataExpr
 |_________________________________
*/

static ScrDataExpr *ExprNullBrowse(ScrDataExpr *this,ScrExprBrowser *Tgt) { return &ScrDataExprNull; }
static ScrType *ExprNullGetType(ScrDataExpr *this){return &ScrTypeNull;}
static ScrDataVal *ExprNullEval(ScrDataExpr *this,ScrDataCtx *Ctx){return &ScrDataValNull;}
static struct ScrDataExpr ExprNullStatic = {ExprNullBrowse,ExprNullGetType,ExprNullEval};
ScrDataExpr ScrDataExprNull = {&ExprNullStatic};

/*_______________________
 |
 | TgtNull
 |_______________________
*/

static ScrDataTgt *TgtNullBrowse(ScrDataTgt *this,ScrExprBrowser *Tgt) {return &ScrDataTgtNull;}
static ScrType *TgtNullGetType(ScrDataTgt *this) {return &ScrTypeNull;}
static ScrVar *TgtNullEval(ScrDataTgt *this,ScrDataCtx *Ctx) { return &ScrVarNull; }
static struct ScrDataTgt TgtNullStatic = {TgtNullBrowse,TgtNullGetType,TgtNullEval};
ScrDataTgt ScrDataTgtNull = {&TgtNullStatic};

/*________________________
 |
 |   Var Null;
 |________________________
*/

static ScrDataVal *VarNullValue(ScrVar *this) { return &ScrDataValNull; }
static ScrVar *VarNullElt(ScrVar *this,int idx) { return &ScrVarNull; }
static void VarNullAddress(ScrVar *this,ScrVar **Base,int *Idx) { *Base = &ScrVarNull; *Idx = 0; }
static void VarNullSet(ScrVar *this,ScrDataVal *Value) {}
static struct ScrVar VarNullStatic = {
	VarNullValue,VarNullElt,VarNullAddress,VarNullSet
};
ScrVar ScrVarNull = {&VarNullStatic};

/*____________________________
 |
 | Ctx Null
 |____________________________
*/

static ScrDataCtx *CtxNullExtrn(ScrDataCtx *this,int ExtrnNum) { return &ScrDataCtxNull;}
static ScrDataCtx *CtxNullLexical(ScrDataCtx *this,int Depth){ return &ScrDataCtxNull; }
static ScrDataVal *CtxNullParam(ScrDataCtx *this) { return &ScrDataValNull; }
static ScrDataVal *CtxNullLocal(ScrDataCtx *this,int Idx) { return &ScrDataValNull; }
static ScrDataVal *CtxNullFifoParam(ScrDataCtx *this,ScrDataVal *N) { return &ScrDataValNull;}
static ScrDataCtx *CtxNullSetLocals(ScrDataCtx *this,int LexicalLvl,int LocalsNb,ScrDataExpr **Locals) { 
	return &ScrDataCtxNull; 
}
static struct ScrDataCtx CtxNullStatic = {
	CtxNullExtrn,CtxNullLexical,CtxNullParam,CtxNullLocal,CtxNullFifoParam,CtxNullSetLocals
};
ScrDataCtx ScrDataCtxNull = {&CtxNullStatic};

/*_____________________
 |
 | DataBrowser
 |_____________________
*/

static ScrDataVal *DataBrowserNullInt(ScrDataBrowser *this,int i) { &ScrDataValNull; }
static ScrDataVal *DataBrowserNullString(ScrDataBrowser *this,ScrString *String) { &ScrDataValNull; }
static ScrDataVal *DataBrowserNullComplex(ScrDataBrowser *this,ScrType *Type,ScrDataVal **Val){ &ScrDataValNull; }
static ScrDataVal *DataBrowserNullVector(ScrDataBrowser *this,ScrArray *Content) { &ScrDataValNull; }
static ScrDataVal *DataBrowserNullPtr(ScrDataBrowser *this,ScrDataVal *Tgt) { &ScrDataValNull; }
static struct ScrDataBrowser DataBrowserNullStatic = {
	DataBrowserNullInt,DataBrowserNullString,DataBrowserNullComplex,DataBrowserNullVector, DataBrowserNullPtr
};
ScrDataBrowser ScrDataBrowserNull = {&DataBrowserNullStatic};

