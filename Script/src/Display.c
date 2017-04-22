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

#include <stdio.h>
#include <StackEnv.h>
#include <Classes.h>
#include <List.h>
#include <ScrModule.h>

/*-------------------------*/

typedef struct {struct Output *Static;} Output;
struct Output {
	void (*Add)(Output *this,char *s);
	void (*Open)(Output *this,char *s);
	void (*Close)(Output *this);
};
typedef struct {
	Output Output;
	int Indent;
} myOutput;
static void PrintIndent(int n) {
	while(n) { printf("    "); n--; }
}
static void myOutputAdd(Output *this,char *s) {
	char *q,*e,*p;
	char Buffer[64];
	ThisToThat(myOutput,Output);
	q = Buffer; e = q+63; *e = 0;
	p = s;
	while (*p) {
		if (*p=='\n') {
			*q++=*p++; *q=0; q = Buffer;
			printf(Buffer);
			PrintIndent(that->Indent);
		} else {
			*q++ = *p++;
			if (q>=e) { printf(Buffer); q = Buffer;}
		}
	}
	*q = 0; printf(Buffer);
}
static void myOutputOpen(Output *this,char *s) {
	ThisToThat(myOutput,Output);
	printf(s);
	printf("{\n");
	that->Indent++;
	PrintIndent(that->Indent);
}
static void myOutputClose(Output *this) {
	ThisToThat(myOutput,Output);
	printf("\n");
	that->Indent--;
	PrintIndent(that->Indent);
	printf("}");
}
static Output *OutputNew(void) {
	myOutput *r;
	static struct Output Static = {myOutputAdd,myOutputOpen,myOutputClose};
	rPush(r); r->Output.Static = &Static;
	r->Indent = 0;
	return &r->Output;
}

static void OutputString(Output *Out,ScrString *st) {
	rOpen {
		int sz,p,e;
		char Buffer[72];
		Buffer[64] = 0;
		sz = Call(st,Size,0);
		Call(Out,Add,1("\""));
		p = 0;
		while (p<sz) {
		    e = p+64; if(e>=sz) {e = sz; Buffer[e-p]=0;}
			Call(st,Copy,3(Buffer,p,e));
			Call(Out,Add,1(Buffer));
			p = e;
		}
		Call(Out,Add,1("\""));
	} rClose
}
/*------------------*/

typedef struct { struct AlreadyKnown *Static; } AlreadyKnown;
struct AlreadyKnown {
	void (*Open)(AlreadyKnown *this);
	int (*IsKnown)(AlreadyKnown *this,char *Label);
	int (*IsNew)(AlreadyKnown *this,char *Label); // Try to add the label if it is not already there
	void (*Close)(AlreadyKnown *this);
};
static void VoidOpen(AlreadyKnown *thid) {}
static int VoidIsKnown(AlreadyKnown *this,char *Label) { return (0!=0); }
static int VoidIsNew(AlreadyKnown *this,char *Label) { return (0==0); }
static void VoidClose(AlreadyKnown *this) {}
static struct AlreadyKnown VoidAlreadyKnownStatic = {VoidOpen,VoidIsKnown,VoidIsNew,VoidClose};
static AlreadyKnown VoidAlreadyKnown = {&VoidAlreadyKnownStatic};

    /*   */

#include <Patricia.h>
typedef struct {
	MemStack *Nods;
	AlreadyKnown AlreadyKnown;
	PatriciaNod Known;
} RecordKnown;
static void RecordKnownOpen(AlreadyKnown *this){
	ThisToThat(RecordKnown,AlreadyKnown);
	mEnter(that->Nods);
}
static void RecordKnownClose(AlreadyKnown *this) {
	ThisToThat(RecordKnown,AlreadyKnown);
	PatriciaInit(&that->Known);
	mLeave(that->Nods);
}
static int RecordKnownIsKnown(AlreadyKnown *this,char *Label) {
	PatriciaNod *p;
	ThisToThat(RecordKnown,AlreadyKnown);
    p = PatriciaSeek(&that->Known,Label);
	return (p!=&that->Known);
}
static int RecordKnownIsNew(AlreadyKnown *this,char *Label) {
	PatriciaNod *p;
	int r;
	ThisToThat(RecordKnown,AlreadyKnown);
    p = PatriciaSeek(&that->Known,Label);
	r = (p == &that->Known);
	if (r) { 
		mPush(that->Nods,p);
		PatriciaSeekOrInsert(&that->Known,Label,p);
	}
	return r;
}
static AlreadyKnown *RecordKnownNew(void) {
	RecordKnown *r;
	static struct AlreadyKnown Static = {
		RecordKnownOpen,RecordKnownIsKnown,RecordKnownIsNew,RecordKnownClose
	};
    rPush(r); r->AlreadyKnown.Static = &Static;
	r->Nods = rFork(256);
	PatriciaInit(&r->Known);
	return &r->AlreadyKnown;
}


/*------------------*/

typedef struct { struct ExprKnown *Static; } ExprKnown;
struct ExprKnown {
	void (*Open)(ExprKnown *this);
    char *(*IsKnown)(ExprKnown *this,void *Symbol);
	int (*IsNew)(ExprKnown *this,void *Symbol,char *Label);
	void (*Close)(ExprKnown *this);
};

typedef struct {
	BinTree/*<ExprKnownNod.Expr>*/ Expr;
	char *Label;
} ExprKnownNod;
typedef struct {
	MemStack *Mem;
	ExprKnown ExprKnown;
    BinTree/*<ExprKnownNod.Expr>*/ Known;
	ScrVocabulary *Voc;
} tExprKnown;
static void ExprKnownOpen(ExprKnown *this) {
	ThisToThat(tExprKnown,ExprKnown);
	mEnter(that->Mem);
	BinTreeInit(&that->Known);
}
static char *ExprKnownIsKnown(ExprKnown *this,void *Symbol) {
	BinTree *P;
	int key;
	char *r;
	ThisToThat(tExprKnown,ExprKnown);
    key = (int)(Symbol);
    P = BinTreeSeek(&that->Known,key);
	if (P!=BinTreeEnd) {
		ExprKnownNod *p;
		p = CastBack(ExprKnownNod,Expr,P);
		r = p->Label;
	} else {
		r = 0;
	}
	return r;
}
static int ExprKnownIsNew(ExprKnown *this,void *Symbol,char *Label) {
	BinTree *P; 
	int key;
	int r;
	ThisToThat(tExprKnown,ExprKnown);
	key = (int)(Symbol);
	P = BinTreeSeek(&that->Known,key);
	r = (P==BinTreeEnd);
	if (r) {
        ExprKnownNod *p;
		mPush(that->Mem,p);
		p->Label = Call(that->Voc,GetLabel,1(Label));
		BinTreeSeekOrInsert(&that->Known,key,&p->Expr);
	}
	return r;
}
static void ExprKnownClose(ExprKnown *this) {
	ThisToThat(tExprKnown,ExprKnown);
	BinTreeInit(&that->Known);
	mLeave(that->Mem);
}

static ExprKnown *ExprKnownNew(void) {
	tExprKnown *r;
	MemStack *Mem;
	static struct ExprKnown Static = {
		ExprKnownOpen,ExprKnownIsKnown,ExprKnownIsNew,ExprKnownClose
	};
	Mem = rFork(256);
	mPush(Mem,r); r->ExprKnown.Static = &Static; r->Mem = Mem;
	mIn(Mem,r->Voc = ScrVocabularyNew());
	BinTreeInit(&r->Known);
	return &r->ExprKnown;
}

/*------------------*/


typedef struct {
	MemStack *Nods;
	ScrTypeBrowser ScrTypeBrowser;
	ScrTypeStructBrowser ScrTypeStructBrowser;
	AlreadyKnown *Label,*Struct;
	Output *Out;
} TypeDisplay;

static void StructDisplayAdd(ScrTypeStructBrowser *this,int num,char *Label,ScrType *Type) {
	ThisToThat(TypeDisplay,ScrTypeStructBrowser);
	Call(Type,Browse,1(&that->ScrTypeBrowser));
	Call(that->Out,Add,1(" "));
	Call(that->Out,Add,1(Label));
	Call(that->Out,Add,1(";\n"));
}
static ScrType *StructDisplayEnd(ScrTypeStructBrowser *this) {
	ThisToThat(TypeDisplay,ScrTypeStructBrowser);
	Call(that->Out,Close,0);
	return &ScrTypeNull;
}
static void StructDisplayMap(ScrTypeStructBrowser *this,int offset,int num,char *Label,ScrType *Type) { }
static ScrType *StructDisplayMapEnd(ScrTypeStructBrowser *this,char *Label,int align,int Size) { return &ScrTypeNull; }

static ScrType *TypeDisplayAtom(ScrTypeBrowser *this,ScrTypeBase Atom) {
	ThisToThat(TypeDisplay,ScrTypeBrowser);
	switch(Atom) {
    case ScbInt: Call(that->Out,Add,1("int")); break;
	case ScbChar: Call(that->Out,Add,1("char")); break;
	case ScbVoid: Call(that->Out,Add,1("void")); break;
	case ScbString: Call(that->Out,Add,1("string")); break;
	default: Call(that->Out,Add,1("Error"));
	}
	return &ScrTypeNull;
}
static ScrTypeStructBrowser *TypeDisplayStruct(ScrTypeBrowser *this,char *Label,ScrType *Base,int FieldNb) {
	ScrTypeStructBrowser *r;
	ThisToThat(TypeDisplay,ScrTypeBrowser);
	Call(that->Out,Add,1("struct "));
	Call(that->Out,Open,1(Label));
	r = &that->ScrTypeStructBrowser;
	if (*Label){
		if (!Call(that->Struct,IsNew,1(Label))) { r = &ScrTypeStructBrowserNull; }
	}
	return r;
}
static ScrType *TypeDisplayFn(ScrTypeBrowser *this,ScrType *Result,ScrType *Params) {
	ThisToThat(TypeDisplay,ScrTypeBrowser);
	Call(that->Out,Add,1("-<"));
	Call(Result,Browse,1(this));
	Call(that->Out,Add,1(","));
	Call(Params,Browse,1(this));
	return &ScrTypeNull;
}
static ScrType *TypeDisplayPtr(ScrTypeBrowser *this,ScrType *On) {
	ThisToThat(TypeDisplay,ScrTypeBrowser);
	Call(that->Out,Add,1("*"));
	Call(On,Browse,1(this));
	return &ScrTypeNull;
}
static ScrType *TypeDisplayArray(ScrTypeBrowser *this,int EltNb,ScrType *Of) {
	char r[32];
	ThisToThat(TypeDisplay,ScrTypeBrowser);
	sprintf(r,"[%d]",EltNb);
	Call(that->Out,Add,1(r));
	Call(Of,Browse,1(this));
	return &ScrTypeNull;
}
static ScrType *TypeDisplayList(ScrTypeBrowser *this,ScrType *Of){
	ThisToThat(TypeDisplay,ScrTypeBrowser);
	Call(that->Out,Add,1("list"));
	Call(Of,Browse,1(this));
	return &ScrTypeNull;
}
static ScrType *TypeDisplayLabel(ScrTypeBrowser *this,char *Label,ScrType *Actual) {
	ThisToThat(TypeDisplay,ScrTypeBrowser);
	Call(that->Out,Add,1("'"));
	Call(that->Out,Add,1(Label));
	if (Call(that->Label,IsNew,1(Label))) { 
		Call(that->Out,Add,1(":"));
		Call(Actual,Browse,1(this)); 
	}
	return &ScrTypeNull;
}

static ScrTypeBrowser *OutTypeBrowserDisplay(Output *Out,AlreadyKnown *Type,AlreadyKnown *Struct) {
	TypeDisplay *r;
	static struct ScrTypeStructBrowser StructStatic = {
		StructDisplayAdd, StructDisplayEnd, StructDisplayMap, StructDisplayMapEnd
	};
	static struct ScrTypeBrowser Static = {
		TypeDisplayAtom,TypeDisplayStruct,TypeDisplayFn,TypeDisplayPtr,
		TypeDisplayArray,TypeDisplayList,TypeDisplayLabel
	};
	rPush(r);
	r->ScrTypeBrowser.Static = &Static;
	r->ScrTypeStructBrowser.Static = &StructStatic;
	r->Out = Out; r->Label = Type; r->Struct = Struct;
	return &r->ScrTypeBrowser;
}
ScrTypeBrowser *ScrTypeBrowserDisplay(void) {
	return OutTypeBrowserDisplay(OutputNew(),&VoidAlreadyKnown,&VoidAlreadyKnown);
}

/*___________________________________
 |
 | ScrExprDisplay
 |___________________________________
*/

typedef struct {
	ScrExprBrowser ScrExprBrowser;
	ScrTypeBrowser *TypeBrowser;
	ExprKnown *Known;
	Output *Out;
} ExprDisplay;

static ScrDataExpr *ExprDisplayConst(ScrExprBrowser *this,ScrType *Type,ScrDataVal *v) {
	ThisToThat(ExprDisplay,ScrExprBrowser);
	ScrString *st;
	st = Call(v,String,0);
	OutputString(that->Out,st);
	return &ScrDataExprNull;
}

static void KnownExprDisplay(ExprDisplay *that,ScrDataExpr *expr) {
	char *Label;
	Label = Call(that->Known,IsKnown,1(expr));
	if (!Label) {
		Call(expr,Browse,1(&that->ScrExprBrowser));
	} else {
		Call(that->Out,Add,1(Label));
	}
}
static void KnownExprDefine(ExprDisplay *that,ScrDataExpr *expr,char *Label) {
	int isnew;
	isnew = Call(that->Known,IsNew,2(expr,Label));
	if (isnew) {
		Call(expr,Browse,1(&that->ScrExprBrowser));
	} else {
		char *L;
		L = Call(that->Known,IsKnown,1(expr));
		Call(that->Out,Add,1(L));
	}
}

static ScrDataExpr *ExprDisplayCond(ScrExprBrowser *this,ScrDataExpr *c,ScrDataExpr *t,ScrDataExpr *f) {
	ThisToThat(ExprDisplay,ScrExprBrowser);
	Call(that->Out,Add,1("?("));
	KnownExprDisplay(that,c);
	Call(that->Out,Add,1(","));
	KnownExprDisplay(that,t);
	Call(that->Out,Add,1(","));
	KnownExprDisplay(that,f);
	Call(that->Out,Add,1(")"));
	return &ScrDataExprNull;
}
static ScrDataExpr *ExprDisplayAlter(ScrExprBrowser *this,ScrDataUOpDesc *Desc,ScrDataExpr *x) {
	char op[8];
	ThisToThat(ExprDisplay,ScrExprBrowser);
    op[0] = Desc->Label; op[1] = 0;
	Call(that->Out,Add,1(op));
	KnownExprDisplay(that,x);
	return &ScrDataExprNull;
}
static ScrDataExpr *ExprDisplayCompose(ScrExprBrowser *this,ScrDataBOpDesc *Desc,ScrDataExpr *x,ScrDataExpr *y) {
	char op[8];
	ThisToThat(ExprDisplay,ScrExprBrowser);
	op[0] = Desc->Label; op[1] = 0;
	Call(that->Out,Add,1("("));
	KnownExprDisplay(that,x);
	Call(that->Out,Add,1(op));
	KnownExprDisplay(that,y);
	Call(that->Out,Add,1(")"));
	return &ScrDataExprNull;
}
static ScrDataExpr *ExprDisplayComplex(ScrExprBrowser *this,
			int Llvl,ScrType *Type,ScrDataExpr **Result,int LocalNb,ScrDataExpr **Local) {
	char sLvl[4*sizeof(int)];
	char sLNb[4*sizeof(int)];
	ScrDataExpr **p,**e;
	ScrTypeBase tb;
	int n;
	ThisToThat(ExprDisplay,ScrExprBrowser);
	tb = ScrTypeActualBase(Type);
	sprintf(sLvl,"%d",Llvl);
	sprintf(sLNb,"%d",LocalNb);
	Call(that->Out,Add,1("At["));
	Call(that->Out,Add,1(sLvl));
	Call(that->Out,Add,1(":"));
	Call(that->Out,Add,1(sLNb));
	Call(that->Out,Add,1(":"));
	Call(Type,Browse,1(that->TypeBrowser));
	Call(that->Out,Open,1("]"));
	p = Local; e = p+LocalNb; n = 0; sLNb[0]='$';
	while(p<e) {
        sprintf(sLNb+1,"%d",n);
		Call(that->Out,Add,1(sLNb));
		Call(that->Out,Add,1("="));
		KnownExprDefine(that,*p,sLNb);
		Call(that->Out,Add,1(";\n"));
		p++; n++;
	}
	Call(that->Out,Add,1(":\n"));
	p = Result; e = p + Call(Type,Struct.EltNb,0); n = 0;
	while (p<e) {
		char *Label;
		Call(Type,Struct.Elt,2(&Label,n));
		Call(that->Out,Add,1(Label));
		Call(that->Out,Add,1("="));
		KnownExprDefine(that,*p,Label);
		Call(that->Out,Add,1(";\n"));
		p++; n++;
	}
	Call(that->Out,Close,0);
	return &ScrDataExprNull;
}
static ScrDataExpr *ExprDisplayApply(ScrExprBrowser *this,ScrDataExpr *Expr,ScrDataExpr *Param) {
	ThisToThat(ExprDisplay,ScrExprBrowser);
	Call(that->Out,Add,1("-<->"));
	KnownExprDisplay(that,Expr);
	Call(that->Out,Add,1(":"));
	KnownExprDisplay(that,Param);
	return &ScrDataExprNull;
}
static ScrDataExpr *ExprDisplayFieldSelect(ScrExprBrowser *this,ScrDataExpr *Struct,int Num) {
	ScrType *t;
	char *Label;
	ThisToThat(ExprDisplay,ScrExprBrowser);
	t = Call(Struct,GetType,0);
	Call(t,Struct.Elt,2(&Label,Num));
	KnownExprDisplay(that,Struct);
	Call(that->Out,Add,1("."));
	Call(that->Out,Add,1(Label));
	return &ScrDataExprNull;
}
static int rExprDisplayVector(int num,ScrCatalogEntry Elt,void *Ctx) {
	ExprDisplay *Tgt;
	ScrDataExpr *Expr;
	char aff[4*sizeof(int)];
	Tgt = Ctx; Expr = Elt.Data;
	sprintf(aff,"%d:",num);
	Call(Tgt->Out,Add,1(aff));
	Call(Expr,Browse,1(&Tgt->ScrExprBrowser));
	Call(Tgt->Out,Add,1(","));
	return (0!=0);
}
static ScrDataExpr *ExprDisplayVector(ScrExprBrowser *this,ScrType *Type,ScrArray *Space) { 
	int min,max,nb;
	char aff[(3*4*sizeof(int))+32];
	ThisToThat(ExprDisplay,ScrExprBrowser);
	Call(Space,Dim,3(&min,&max,&nb));
	sprintf(aff,"array[%d..%d..%d]",min,nb,max);
	Call(that->Out,Open,1(aff));
	Call(Space,ForEach,2(rExprDisplayVector,that));
	Call(that->Out,Close,0);
	return &ScrDataExprNull;
}
static ScrDataExpr *ExprDisplayEltSelect(ScrExprBrowser *this,ScrDataExpr *Space,ScrDataExpr *Idx) {
	ThisToThat(ExprDisplay,ScrExprBrowser);
	KnownExprDisplay(that,Space);
	Call(that->Out,Add,1("["));
	Call(Idx,Browse,1(this));
	Call(that->Out,Add,1("]"));
	return &ScrDataExprNull;
}

static ScrDataExpr *ExprDisplayVariable(ScrExprBrowser *this,ScrDataTgt *Tgt) {
	ThisToThat(ExprDisplay,ScrExprBrowser);
	Call(that->Out,Add,1("$"));
	Call(Tgt,Browse,1(this));
	return &ScrDataExprNull;
}
static ScrDataExpr *ExprDisplayPtr(ScrExprBrowser *this,ScrDataTgt *Tgt) {
	ThisToThat(ExprDisplay,ScrExprBrowser);
	Call(that->Out,Add,1("*"));
	Call(Tgt,Browse,1(this));
	return &ScrDataExprNull;
}

static ScrDataExpr *ExprDisplayParam(ScrExprBrowser *this,ScrType *Type,int Depth){
	char sDepth[4*sizeof(int)];
	ThisToThat(ExprDisplay,ScrExprBrowser);
	sprintf(sDepth,"%d",Depth);
	Call(that->Out,Add,1(":<"));
	Call(that->Out,Add,1(sDepth));
	return &ScrDataExprNull;
}
static ScrDataExpr *ExprDisplayLocal(ScrExprBrowser *this,ScrType *Type,int Depth,int Idx) {
	char sDepth[4*sizeof(int)];
	char sIdx[4*sizeof(int)];
	ThisToThat(ExprDisplay,ScrExprBrowser);
	sprintf(sDepth,"%d",Depth);
	sprintf(sIdx,"%d",Idx);
	Call(that->Out,Add,1(":_"));
	Call(that->Out,Add,1(sDepth));
	Call(that->Out,Add,1("_"));
	Call(that->Out,Add,1(sIdx));
	return &ScrDataExprNull;
}
static ScrDataExpr *ExprDisplayGlobal(ScrExprBrowser *this,ScrType *Type,int Module,int Idx) {
	char sMod[4*sizeof(int)];
	char sIdx[4*sizeof(int)];
	ThisToThat(ExprDisplay,ScrExprBrowser);
	sprintf(sMod,"%d",Module);
	sprintf(sIdx,"%d",Idx);
	Call(that->Out,Add,1(":|"));
	Call(that->Out,Add,1(sMod));
	Call(that->Out,Add,1("_"));
	Call(that->Out,Add,1(sIdx));
	return &ScrDataExprNull;
}

static ScrDataTgt *VarDisplayTargeted(ScrExprBrowser *this,ScrDataExpr *Ptr) { 
	return &ScrDataTgtNull; 
}
static ScrDataTgt *VarDisplayEltAccess(ScrExprBrowser *this,ScrDataTgt *Base,ScrDataExpr *Idx) { 
	return &ScrDataTgtNull; 
}
static ScrDataTgt *VarDisplayFieldAccess(ScrExprBrowser *this,ScrDataTgt *Base,int Idx) { 
	return &ScrDataTgtNull; 
}
static ScrDataTgt *VarDisplayResult(ScrExprBrowser *this,int Depth,ScrType *Type,int Idx) { 
	return &ScrDataTgtNull; 
}
static ScrDataTgt *VarDisplayLocal(ScrExprBrowser *this,int Depth,ScrType *Type,int Idx) { 
	return &ScrDataTgtNull; 
}
static ScrDataTgt *VarDisplayExtern(ScrExprBrowser *this,ScrType *Type,int Module,int Idx) { 
	return &ScrDataTgtNull; 
}
static ScrVar *VarDisplayConst(ScrExprBrowser *this,ScrDataVal *Val){ 
	return &ScrVarNull; 
}
static ScrDataExpr *VarDisplayProcedure(ScrExprBrowser *this,int Depth,ScrType *Prototype,ScrType *Local,ScrInstruction *Body) { 
	return &ScrDataExprNull; 
}

static ScrInstruction *InstrDisplaySet(ScrExprBrowser *this,ScrDataTgt *Tgt,ScrDataExpr *Val) {
	return &ScrInstructionNull;
}
static ScrInstruction *InstrDisplaySequence(ScrExprBrowser *this,ScrInstruction **Begin,ScrInstruction **End) {
	return &ScrInstructionNull;
}
static ScrInstruction *InstrDisplayWhile(ScrExprBrowser *this,ScrDataExpr *Cond,ScrInstruction *Instr) {
	return &ScrInstructionNull;
}
static ScrInstruction *InstrDisplayUntil(ScrExprBrowser *this,ScrInstruction *Inst,ScrDataExpr *Cond) {
	return &ScrInstructionNull;
}
static ScrInstruction *InstrDisplayCond(ScrExprBrowser *this,ScrDataExpr *c,ScrInstruction *t) {
	return &ScrInstructionNull;
}
static ScrInstruction *InstrDisplayAlternate(ScrExprBrowser *this,ScrDataExpr *c,ScrInstruction *t,ScrInstruction *f) {
	return &ScrInstructionNull;
}
static ScrInstruction *InstrDisplaySelect(ScrExprBrowser *this,ScrDataExpr *Sel,ScrArray *Choices) {
	return &ScrInstructionNull;
}
static ScrInstruction *InstrDisplayInvoke(ScrExprBrowser *this,ScrDataTgt *r,ScrDataExpr *p,int depth,ScrType *Lcl,ScrInstruction *blc) {
	return &ScrInstructionNull;
}
static ScrInstruction *InstrDisplayExtend(ScrExprBrowser *this,ScrDataTgt *Arrayptr,ScrDataExpr *nb) {
	return &ScrInstructionNull;
}

static ScrExprBrowser *myExprDisplay(Output *Out,ScrTypeBrowser *T,ExprKnown *Known) {
	ExprDisplay *r;
	static struct ScrExprBrowser Static = {
		ExprDisplayConst,ExprDisplayCond,ExprDisplayAlter,ExprDisplayCompose,ExprDisplayComplex,
		ExprDisplayApply,ExprDisplayFieldSelect,ExprDisplayVector,ExprDisplayEltSelect,
		ExprDisplayVariable, ExprDisplayPtr, ExprDisplayParam,ExprDisplayLocal,ExprDisplayGlobal,
		{
			VarDisplayTargeted,VarDisplayEltAccess,VarDisplayFieldAccess,VarDisplayResult,VarDisplayLocal,
			VarDisplayExtern,VarDisplayConst,VarDisplayProcedure
		},
		{
			InstrDisplaySet,InstrDisplaySequence,InstrDisplayWhile,InstrDisplayUntil,InstrDisplayCond,
			InstrDisplayAlternate,InstrDisplaySelect,InstrDisplayInvoke,InstrDisplayExtend
		}
	};
	rPush(r); r->ScrExprBrowser.Static = &Static;
	r->Out = Out;
	r->TypeBrowser = T;
	r->Known = Known;
	return &r->ScrExprBrowser;
}

/*___________________________________
 |
 | ScrDataDisplay
 |___________________________________
*/

typedef struct {
	ScrDataBrowser ScrDataBrowser;
	Output *Out;
} DataBrowser;

static ScrDataVal *ValueInt(ScrDataBrowser *this,int i) {
	char Val[4*sizeof(int)];
	ThisToThat(DataBrowser,ScrDataBrowser);
	sprintf(Val,"%d",i);
	Call(that->Out,Add,1(Val));
	return &ScrDataValNull;
}
static ScrDataVal *ValueString(ScrDataBrowser *this,ScrString *String) {
	ThisToThat(DataBrowser,ScrDataBrowser);
	OutputString(that->Out,String);
	return &ScrDataValNull;
}
static ScrDataVal *ValueComplex(ScrDataBrowser *this,ScrType *Type,ScrDataVal **Val) {
	int nb;
	ScrTypeBase tb;
	ThisToThat(DataBrowser,ScrDataBrowser);
	tb = ScrTypeActualBase(Type);
	if (tb==ScbStruct||tb==ScbScope) { // Functions aren't closed and shouldn't be evaluated.
		char *Label;
		int i;
	    nb = Call(Type,Struct.EltNb,0);
        i = 0;
		Call(that->Out,Open,1(""));
		while (i<nb) {
			Call(Type,Struct.Elt,2(&Label,i));
			if (Label) { Call(that->Out,Add,1(Label)); }
			Call(that->Out,Add,1("="));
			Call(Val[i],Browse,1(this));
			Call(that->Out,Add,1(";\n"));
			i++;
		}
		Call(that->Out,Close,0);
	}
	return &ScrDataValNull;
}
static int rValueVector(int nb,ScrCatalogEntry elt,void *Clos) {
	DataBrowser *Tgt;
	ScrDataVal *Val;
	char Aff[4*sizeof(int)+16];
	Val = elt.Data; Tgt = Clos;
	sprintf(Aff,"%d:",nb);
	Call(Tgt->Out,Add,1(Aff));
	Call(Val,Browse,1(&Tgt->ScrDataBrowser));
	Call(Tgt->Out,Add,1(","));
}
static ScrDataVal *ValueVector(ScrDataBrowser *this,ScrArray *Content) { 
	int min,max,nb;
	char Aff[3*4*sizeof(int)+32];
	ThisToThat(DataBrowser,ScrDataBrowser);
    Call(Content,Dim,3(&min,&max,&nb));
	sprintf(Aff,"array[%d,%d,%d]",min,nb,max);
	Call(that->Out,Open,1(Aff));
	Call(Content,ForEach,2(rValueVector,that));
    Call(that->Out,Close,0);
	return &ScrDataValNull;
}
static ScrDataVal *ValuePtr(ScrDataBrowser *this,ScrDataVal *Tgt) {
	ThisToThat(DataBrowser,ScrDataBrowser);
	Call(that->Out,Add,1("*"));
    Call(Tgt,Browse,1(this));
	return &ScrDataValNull;
}
ScrDataBrowser *ScrDataDisplay(void) {
	DataBrowser *r;
	static struct ScrDataBrowser Static = {
		ValueInt, ValueString, ValueComplex, ValueVector, ValuePtr
	};
	rPush(r); r->ScrDataBrowser.Static = &Static;
	r->Out = OutputNew();
	return &r->ScrDataBrowser;
}


/*___________________________________
 |
 | ScrScopeDisplay
 |___________________________________
*/

typedef struct {
	ScrScopeBrowser ScrScopeBrowser;
	Output *Out;
	AlreadyKnown *KnownType,*KnownStruct;
	ExprKnown *KnownExpr;
	ScrTypeBrowser *Type;
	ScrExprBrowser *Expr;
} ScopeDisplay;
static void ScopeDisplayOpen(ScrScopeBrowser *this,int TypeNb) { 
	ThisToThat(ScopeDisplay,ScrScopeBrowser);
	Call(that->KnownType,Open,0);
	Call(that->KnownStruct,Open,0);
	Call(that->KnownExpr,Open,0);
	Call(that->Out,Open,1("Module "));
}
static void ScopeDisplayAddType(ScrScopeBrowser *this,int num,char *Label,ScrType *Type) {
	ThisToThat(ScopeDisplay,ScrScopeBrowser);
	Call(that->Out,Add,1("typedef "));
	Call(that->Out,Add,1(Label));
	Call(that->Out,Add,1("="));
	Call(Type,Browse,1(that->Type));
	Call(that->Out,Add,1(";\n"));
}
static void ScopeDisplayAddExpr(ScrScopeBrowser *this,ScrDataExpr *Expr) {
	ScrType *t;
	ThisToThat(ScopeDisplay,ScrScopeBrowser);
	Call(that->Out,Open,1("/* Expressions: */"));
	Call(Expr,Browse,1(that->Expr));
	Call(that->Out,Close,0);
}
static void ScopeDisplayClose(ScrScopeBrowser *this) { 
	ThisToThat(ScopeDisplay,ScrScopeBrowser);
	Call(that->Out,Close,0);
	Call(that->Out,Add,1("\n"));
	Call(that->KnownExpr,Close,0);
	Call(that->KnownStruct,Close,0);
	Call(that->KnownType,Close,0);
}
ScrScopeBrowser *ScrScopeBrowserDisplay(void) {
	ScopeDisplay *r;
	static struct ScrScopeBrowser Static = {
		ScopeDisplayOpen,ScopeDisplayAddType,ScopeDisplayAddExpr,ScopeDisplayClose
	};
	rPush(r); r->ScrScopeBrowser.Static = &Static;
	r->Out = OutputNew();
	r->KnownType = RecordKnownNew();
	r->KnownStruct = RecordKnownNew();
	r->KnownExpr = ExprKnownNew();
	r->Type = OutTypeBrowserDisplay(r->Out,r->KnownType,r->KnownStruct);
    r->Expr = myExprDisplay(r->Out,r->Type,r->KnownExpr);
	return &r->ScrScopeBrowser;
}


