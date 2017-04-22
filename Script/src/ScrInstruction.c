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

#include <DataInstr.Loc.h>

/*___________________________________________________
 |
 |
 |___________________________________________________
*/

/*______________________
 |
 | Instruction null
 |______________________
*/


static ScrInstruction *InstructionNullBrowse(ScrInstruction *this,ScrExprBrowser *Tgt) { return &ScrInstructionNull; }
static void InstructionNullPerform(ScrInstruction *this,ScrDataCtx *Ctx) {}
static struct ScrInstruction InstructionNullStatic = {&InstructionNullBrowse,&InstructionNullPerform};
ScrInstruction ScrInstructionNull = {&InstructionNullStatic};

/*______________
 |
 | Affectation
 |______________
*/

typedef struct {
	ScrInstruction ScrInstruction;
	ScrDataTgt *Tgt;
	ScrDataExpr *Val;
} Affectation;
static ScrInstruction *AffectationBrowse(ScrInstruction *this,ScrExprBrowser *Tgt) {
	ThisToThat(Affectation,ScrInstruction);
	return Call(Tgt,Instr.Set,2(that->Tgt,that->Val));
}
static void AffectationPerform(ScrInstruction *this,ScrDataCtx *Ctx) {
	ScrDataVal *Val;
	ScrVar *Var;
	ThisToThat(Affectation,ScrInstruction);
    Var = Call(that->Tgt,Eval,1(Ctx));
    Val = Call(that->Val,Eval,1(Ctx));
	Call(Var,Set,1(Val));
}
ScrInstruction *ScrInstrSet(ScrDataTgt *Tgt,ScrDataExpr *Val) {
	Affectation *r;
	static struct ScrInstruction Static = { AffectationBrowse,AffectationPerform };
	rPush(r); r->ScrInstruction.Static = &Static;
	r->Val = Val; r->Tgt = Tgt;
	return &r->ScrInstruction;
}

/*_______________________
 |
 | Sequence
 |_______________________
*/

typedef struct {
	ScrInstruction ScrInstruction;
	ScrInstruction **b,**e;
} InstructionSequence;
static ScrInstruction *SequenceBrowse(ScrInstruction *this,ScrExprBrowser *Tgt) {
	ThisToThat(InstructionSequence,ScrInstruction);
	return Call(Tgt,Instr.Sequence,2(that->b,that->e));
}
static void SequencePerform(ScrInstruction *this,ScrDataCtx *Ctx) {
	ScrInstruction **p,**e;
	ThisToThat(InstructionSequence,ScrInstruction);
	p = that->b; e = that->e;
	while (p<e) {
		Call(*p,Perform,1(Ctx));
		p++;
	}
}
ScrInstruction *ScrInstrSequence(ScrInstruction **b,ScrInstruction **e) {
	InstructionSequence *r;
	static struct ScrInstruction Static = {SequenceBrowse,SequencePerform};
	rPush(r); r->ScrInstruction.Static = &Static;
	r->b = b; r->e = e;
	return &r->ScrInstruction;
}

/*_______________
 |
 | Alternative
 |_______________
*/

typedef struct {
	ScrInstruction ScrInstruction;
	ScrDataExpr *Cond;
	ScrInstruction *t,*f;
} Alternative;
static ScrInstruction *AlternativeBrowse(ScrInstruction *this,ScrExprBrowser *Tgt) {
	ThisToThat(Alternative,ScrInstruction);
	return Call(Tgt,Instr.Alternate,3(that->Cond,that->t,that->f));
}
static void AlternativePerform(ScrInstruction *this,ScrDataCtx *Ctx) {
	int cond;
	ScrDataVal *Cond;
	ThisToThat(Alternative,ScrInstruction);
    Cond = Call(that->Cond,Eval,1(Ctx));
	cond = Call(Cond,Int,0);
	if (cond) {
		Call(that->t,Perform,1(Ctx));
	} else {
		Call(that->f,Perform,1(Ctx));
	}
}
ScrInstruction *ScrInstrAlternate(ScrDataExpr *Cond,ScrInstruction *t,ScrInstruction *f) {
	Alternative *r;
	static struct ScrInstruction Static = {AlternativeBrowse,AlternativePerform};
	rPush(r); r->ScrInstruction.Static = &Static;
	r->Cond = Cond; r->t = t; r->f = f;
	return &r->ScrInstruction;
}

/*_________________________
 |
 | Conditional Instruction
 |_________________________
*/

typedef struct {
	ScrInstruction ScrInstruction;
	ScrDataExpr *Cond;
	ScrInstruction *t;
} Condition;
static ScrInstruction *ConditionBrowse(ScrInstruction *this,ScrExprBrowser *Tgt) {
	ThisToThat(Condition,ScrInstruction);
	return Call(Tgt,Instr.Cond,2(that->Cond,that->t));
}
static void ConditionPerform(ScrInstruction *this,ScrDataCtx *Ctx) {
	int cond;
	ScrDataVal *Cond;
	ThisToThat(Condition,ScrInstruction);
	Cond = Call(that->Cond,Eval,1(Ctx));
	cond = Call(Cond,Int,0);
	if (cond) { Call(that->t,Perform,1(Ctx)); }
}
ScrInstruction *ScrInstrCond(ScrDataExpr *Cond,ScrInstruction *t) {
	Condition *r;
	static struct ScrInstruction Static = {ConditionBrowse,ConditionPerform};
	rPush(r); r->ScrInstruction.Static = &Static;
	r->Cond = Cond; r->t = t;
	return &r->ScrInstruction;
}

/*_____________________
 |
 | Repeat Instruction
 |_____________________
*/

/*---- while(cond) Inst; ----*/

static ScrInstruction *WhileBrowse(ScrInstruction *this,ScrExprBrowser *Tgt) {
	ThisToThat(Condition,ScrInstruction);
	return Call(Tgt,Instr.While,2(that->Cond,that->t));
}
static void WhilePerform(ScrInstruction *this,ScrDataCtx *Ctx) {
	int cond;
	ScrDataVal *Cond;
	ThisToThat(Condition,ScrInstruction);
	Cond = Call(that->Cond,Eval,1(Ctx));
	cond = Call(Cond,Int,0);
	while (cond) {
		Call(that->t,Perform,1(Ctx));
		Cond = Call(that->Cond,Eval,1(Ctx));
		cond = Call(Cond,Int,0);
	}
}
ScrInstruction *ScrInstrWhile(ScrDataExpr *Cond,ScrInstruction *Inst) {
	Condition *r;
	static struct ScrInstruction Static = {WhileBrowse,WhilePerform};
	rPush(r); r->ScrInstruction.Static = &Static;
	r->Cond = Cond; r->t = Inst;
	return &r->ScrInstruction;
} 

/*---- do inst while(cond); ----*/

static ScrInstruction *DoWhileBrowse(ScrInstruction *this,ScrExprBrowser *Tgt) {
	ThisToThat(Condition,ScrInstruction);
	return Call(Tgt,Instr.Until,2(that->t,that->Cond));
}
static void DoWhilePerform(ScrInstruction *this,ScrDataCtx *Ctx) {
	int cond;
	ScrDataVal *Cond;
	ThisToThat(Condition,ScrInstruction);
    do {
		Call(that->t,Perform,1(Ctx));
	    Cond = Call(that->Cond,Eval,1(Ctx));
	    cond = Call(Cond,Int,0);
	} while (cond);
}
ScrInstruction *ScrInstrUntil(ScrInstruction *Inst,ScrDataExpr *Cond) {
	Condition *r;
	static struct ScrInstruction Static = {DoWhileBrowse,DoWhilePerform};
	rPush(r); r->ScrInstruction.Static = &Static;
	r->Cond = Cond; r->t = Inst;
	return &r->ScrInstruction;
}

/*_________________________
 |
 | Switch
 |_________________________
*/

typedef struct {
	ScrInstruction ScrInstruction;
	ScrDataExpr *Select;
	ScrArray *Choices;
} InstructionSwitch;
static ScrInstruction *SwitchBrowse(ScrInstruction *this,ScrExprBrowser *Tgt) {
	ThisToThat(InstructionSwitch,ScrInstruction);
	return Call(Tgt,Instr.Select,2(that->Select,that->Choices));
}
static void SwitchPerform(ScrInstruction *this,ScrDataCtx *Ctx) {
	ScrDataVal *Select;
	int select;
	ScrCatalogEntry Ch;
	ScrInstruction *ch;
	ThisToThat(InstructionSwitch,ScrInstruction);
	Select = Call(that->Select,Eval,1(Ctx));
	select = Call(Select,Int,0);
    Ch = Call(that->Choices,GetElt,1(select));
	ch = Ch.Data;
	Call(ch,Perform,1(Ctx));
}
ScrInstruction *ScrInstrSelect(ScrDataExpr *Select,ScrArray *Choices){
    InstructionSwitch *r;
	static struct ScrInstruction Static = {SwitchBrowse,SwitchPerform};
	rPush(r); r->ScrInstruction.Static = &Static;
	r->Select = Select; r->Choices = Choices;
	return &r->ScrInstruction;
}


/*___________________________
 |
 |
 |___________________________
*/

typedef struct {
	ScrInstruction ScrInstruction;
	ScrType *Base;
	ScrDataExpr *Nb;
	ScrDataTgt *Ptr;
} CtxExpand;
static ScrInstruction *CtxExpandBrowse(ScrInstruction *this,ScrExprBrowser *Tgt) {
	ThisToThat(CtxExpand,ScrInstruction);
	return Call(Tgt,Instr.Extend,2(that->Ptr,that->Nb));
}
static void CtxExpandPerform(ScrInstruction *this,ScrDataCtx *Ctx) {
	ScrVar *Ptr,*Data;
	ScrDataVal *Nb;
	int nb;
	ThisToThat(CtxExpand,ScrInstruction);
    Ptr = Call(that->Ptr,Eval,1(Ctx));
	Nb = Call(that->Nb,Eval,1(Ctx));
	nb = Call(Nb,Int,0);
    Data = Call(Ctx,Exec.Expand,2(that->Base,nb));
	Call(Ptr,Set,1(ScrValAddress(Data)));
}
ScrInstruction *ScrInstrLocalArray(ScrDataTgt *ArrayPtr,ScrDataExpr *Size) {
	CtxExpand *r;
	ScrType *tgt,*elt;
	static struct ScrInstruction Static = { CtxExpandBrowse,CtxExpandPerform };
	rPush(r); r->ScrInstruction.Static = &Static;
	tgt = Call(ArrayPtr,GetType,0);
	tgt = Call(tgt,Ptr.Target,0);
	elt = Call(tgt,Array.EltType,0);
	r->Nb = Size; r->Ptr = ArrayPtr; r->Base = elt;
	return &r->ScrInstruction;
}

    /*______________________
	 |
	 | Procedure invocation
	 |______________________
	*/

typedef struct {
	ScrInstruction ScrInstruction;
	int Depth;
	ScrDataExpr *Param;
	ScrDataTgt *Result;
	ScrInstruction *Body;
	ScrType *Local;
} ProcedureCall;
static ScrInstruction *ProcedureCallBrowse(ScrInstruction *this,ScrExprBrowser *Tgt) {
	ThisToThat(ProcedureCall,ScrInstruction);
	return Call(Tgt,Instr.Invoke,5(that->Result,that->Param,that->Depth,that->Local,that->Body));
}
static void ProcedureCallPerform(ScrInstruction *this,ScrDataCtx *Ctx) {
	ScrDataVal *Param;
	ScrVar *Result;
	ScrDataCtx *ctx;
	ThisToThat(ProcedureCall,ScrInstruction);
	rOpen
        Param = Call(that->Param,Eval,1(Ctx));
	    Result = Call(that->Result,Eval,1(Ctx));
        ctx = Call(Ctx,Exec.Invoke,4(that->Depth,Result,Param,that->Local));
	    Call(that->Body,Perform,1(ctx));
	rClose
}
ScrInstruction *ScrInstrInvoke(ScrDataTgt *Result,ScrDataExpr *Param,int Depth,ScrType *Local,ScrInstruction *Body) {
	ProcedureCall *r;
	static struct ScrInstruction Static = {ProcedureCallBrowse,ProcedureCallPerform};
	rPush(r); r->ScrInstruction.Static = &Static;
	r->Result = Result; r->Param = Param; r->Local = Local; r->Body = Body; r->Depth = Depth;
	return &r->ScrInstruction;
}



