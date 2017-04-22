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
#include <List.h>
#include <C_Lexic.h>
#include <ScrParser.h>
#include <ScrModule.h>
#include <ScrPathExpr.h>
#include <Tools.h>

/*__________________________
 |
 | Interface
 |__________________________
*/


/*_____________________
 |
 | Error Log
 |_____________________
*/

static void ErrorLogNullMsg(ScrErrorLog *this,char *Msg) { }
static struct ScrErrorLog ErrorLogNullStatic = {ErrorLogNullMsg};
ScrErrorLog ScrErrorLogNull = {&ErrorLogNullStatic};

#include <stdio.h>
static void ErrorLogMsg(ScrErrorLog *this,char *Msg) { printf(Msg); printf("\n"); }
ScrErrorLog *ScrErrorDirect(void){
	static struct ScrErrorLog Static = {&ErrorLogMsg};
	static ScrErrorLog Log = {&Static};
	return &Log;
}

typedef struct {
	ScrErrorLog ScrErrorLog;
    CPreprocEnv *Src;
	ScrErrorLog *Dst;
} CPrepErrorLog;
static void CPrepErrorLogMsg(ScrErrorLog *this,char *Msg) {
	ThisToThat(CPrepErrorLog,ScrErrorLog);
	// CPreprocIssueError doesn't work quite well right now...
    /* rOpen {
	    char Msg[256],*p,*q,*eq;
		BuffText *s;
        s = CPreprocIssueError(that->Src,BuffTextNew(Msg));
		q = Msg; eq = Msg+255; *eq=0;
		p = s->p;
		do {
            while ((q<eq)&&(*p)) {*q++=*p++;}
			if (!*p) { p = Call(s,Check0,1(p)); }
			if ((!p)||(q==eq)) {
				Call(that->Dst,Msg,1(Msg));
				q = Msg;
		   	}
		} while(p);
	} rClose */
	Call(that->Dst,Msg,1(Msg));
}
static ScrErrorLog *CPrepErrorLogNew(ScrErrorLog *Dst,CPreprocEnv *Src) {
	CPrepErrorLog *R;
	static struct ScrErrorLog Static = {CPrepErrorLogMsg};
	rPush(R);
    R->ScrErrorLog.Static = &Static;
	R->Dst = Dst;
	R->Src = Src;
	return &R->ScrErrorLog;
}

/*_____________________
 |
 | Lexic
 |_____________________
*/

typedef struct {
	CSynLexemType Type;
	char *Literal;
} LxcItem;
typedef struct {
	MemStack *Mem;
	dList dList;
	LxcItem Item;
} LxcItemList;

static void LxcItemFill(LxcItemList *Dst,CPreprocEnv *Src,ScrVocabulary *Voc) {
    CSynLexem *n;
	char *Literal;
	mLeave(Dst->Mem);
	mEnter(Dst->Mem);
	mIn(Dst->Mem,n = CSynPreprocGetLexem(Src));
	Dst->Item.Type = n->Type;
	mIn(Dst->Mem,Literal=Call(n,Instance,0));
	Dst->Item.Literal = Call(Voc,GetLabel,1(Literal));
}
static LxcItemList *LxcItemNew(CPreprocEnv *Src,ScrVocabulary *Voc) {
	LxcItemList *r;
	MemStack *Mem;
	Mem = rFork(64);
	mPush(Mem,r);
	r->Mem = Mem;
	r->dList.n = r->dList.p = &r->dList;
	mEnter(r->Mem);
	LxcItemFill(r,Src,Voc);
	return r;
}

typedef struct { struct LxcSrc *Static; } LxcSrc;
struct LxcSrc {
	LxcItem *(*Peek)(LxcSrc *this,int Fwd);
	void (*Next)(LxcSrc *this,int Fwd);
	void (*SetVocabulary)(LxcSrc *this,ScrVocabulary *Voc);
	void (*Error)(LxcSrc *this,char *Msg);
};
typedef struct {
	LxcSrc LxcSrc;
	MemStack *Mem;
	ScrVocabulary *Voc;
    CPreprocEnv *Src;
    int FwdNb;
	LxcItemList *Current;
	ScrErrorLog *Error;
} myLxcSrc;

static LxcItem *LxcSrcPeek(LxcSrc *this,int Fwd) {
	dList *p;
	int n,e;
	LxcItemList *r;
	ThisToThat(myLxcSrc,LxcSrc);
    n = 0; 
	e = (Fwd<that->FwdNb)?Fwd:that->FwdNb-1; 
	p = &that->Current->dList;
	while (n<e) { p = p->n; }
	if (n!=Fwd) {
		while (n<Fwd) {
			dList *q;
			mIn(that->Mem,r = LxcItemNew(that->Src,that->Voc));
			q = &r->dList;
			q->p = p; q->n = p->n;
			q->n->p = q; q->p->n = q;
			p = q;
			n++;
		}
	}
	r = CastBack(LxcItemList,dList,p);
	return &r->Item;
}
static void LxcSrcNext(LxcSrc *this,int Fwd) {
	dList *p;
	ThisToThat(myLxcSrc,LxcSrc);
	p = &that->Current->dList;
	while (Fwd>0) {
        LxcItemList *P;
		P = CastBack(LxcItemList,dList,p);
	    LxcItemFill(P,that->Src,that->Voc);
		p = p->n;
		Fwd--;
	}
    that->Current = CastBack(LxcItemList,dList,p);
}
static void LxcSrcSetVocabulary(LxcSrc *this,ScrVocabulary *Voc) {
	ThisToThat(myLxcSrc,LxcSrc);
	that->Voc = Voc;
}
static void LxcSrcError(LxcSrc *this,char *Msg) {
	ThisToThat(myLxcSrc,LxcSrc);
	Call(that->Error,Msg,1(Msg));
}

static LxcSrc *LxcSrcNew(ScrVocabulary *Voc,ScrErrorLog *Err,CPreprocEnv *Src) {
	myLxcSrc *r;
	MemStack *Mem;
	static struct LxcSrc Static = { 
		LxcSrcPeek,LxcSrcNext,LxcSrcSetVocabulary,LxcSrcError
	};
	Mem = rFork(64);
	mPush(Mem,r);
	r->Mem = Mem;
	r->LxcSrc.Static = &Static;
	mIn(r->Mem,r->Error = CPrepErrorLogNew(Err,Src));
	r->Voc = Voc;
	r->Src = Src;
	mIn(r->Mem,r->Current = LxcItemNew(Src,Voc));
	r->FwdNb = 1;
	return &r->LxcSrc;
}


/*_____________________
 |
 | Parser
 |_____________________
*/

typedef struct {
	char **Path;
    ScrErrorLog *Err;
	ScrScope *Dst;
	LxcSrc *Src;
	ScrTypeBrowser *TypeTgt;
	ScrExprBrowser *ExprTgt;
} ParseEnv;

     /*--------------*/

static void SkipAfter(ParseEnv *this,int End);
static void SkipTo(ParseEnv *this,int End) {
	int cont;
	LxcItem *c;
	CSynLexemType t;
	c = Call(this->Src,Peek,1(0));
	t = c->Type;
	cont = (t!=CS_Eof) && (t!=End);
	if (End==';') cont = cont && (t!='}') && (t!=')');
	while (cont) {
		Call(this->Src,Next,1(1));
		switch (t) {
		case '(':
			SkipAfter(this,')');
		break;
		case '{':
		    SkipAfter(this,'}');
		break;
		case '[':
		    SkipAfter(this,']');
		break;
		}
		c = Call(this->Src,Peek,1(0));
		t = c->Type;
	    cont = (t!=CS_Eof) && (t!=End);
	    if (End==';') cont = cont && (t!='}') && (t!=')');
	}
}
static void SkipAfter(ParseEnv *this,int End) {
	LxcItem *c;
	SkipTo(this,End);
	c = Call(this->Src,Peek,1(0));
	if (c->Type = End) {
		Call(this->Src,Next,1(1));
	}
}
static int CheckDiscard(ParseEnv *this,int Symbol,char *Msg) {
	LxcItem *c;
	int Success;
	c = Call(this->Src,Peek,1(0));
	Success = (c->Type==Symbol);
	if (Success) {
		Call(this->Src,Next,1(1));
	} else {
		Call(this->Src,Error,1(Msg));
	}
	return Success;
}
static int ForcedCheckDiscard(ParseEnv *this,int Symbol,char *Msg) {
	int r;
	r = CheckDiscard(this,Symbol,Msg);
	if (!r) { SkipAfter(this,Symbol);}
	return r;
}

/*___________________________
 |
 | Arithmetic Expr
 |___________________________
*/

#define DefUIntOpDesc(Nm,Lbl,Op) \
	static ScrDataVal *Opratr##Nm(ScrDataVal *x){ return SDValInt(Op Call(x,Int,0)); }\
    static ScrDataUOpDesc Operator##Nm = {\
		Lbl,/*&ScrTypeInt,&ScrTypeInt,*/Opratr##Nm\
	}

#define DefBIntOpDesc(Nm,Lbl,Op) \
	static ScrDataVal *Opratr##Nm(ScrDataVal *x,ScrDataVal *y){return SDValInt(Call(x,Int,0) Op Call(y,Int,0));}\
    static ScrDataBOpDesc Operator##Nm = {\
		 Lbl,/*&ScrTypeInt,&ScrTypeInt,&ScrTypeInt,*/Opratr##Nm\
	}

DefUIntOpDesc(Neg,'n',-);
DefUIntOpDesc(Invert,'~',~);
DefUIntOpDesc(Not,'!',!);
DefBIntOpDesc(Prod,'*',*);
DefBIntOpDesc(Quotien,'/',/);
DefBIntOpDesc(Rest,'%',%);
DefBIntOpDesc(Sum,'+',+);
DefBIntOpDesc(Diff,'-',-);
DefBIntOpDesc(BinAnd,'&',&);
DefBIntOpDesc(BinOr,'|',|);
DefBIntOpDesc(Xor,'^',^);
DefBIntOpDesc(Sar,CS_SAR,>>);
DefBIntOpDesc(Sal,CS_SAL,<<);
DefBIntOpDesc(Eq,CS_Eq,==);
DefBIntOpDesc(nEq,CS_NotSet,!=);
DefBIntOpDesc(And,CS_bAnd,&&);
DefBIntOpDesc(Or,CS_bOr,||);
DefBIntOpDesc(InfEq,CS_InfEq,<=);
DefBIntOpDesc(SupEq,CS_SupEq,>=);
DefBIntOpDesc(Inf,'<',<);
DefBIntOpDesc(Sup,'>',>);


     /*--------------------*/

static int ParseArithmeticElt(ParseEnv *this,ScrDataExpr **Res);
static int ParseArithmeticExpr(ParseEnv *this,ScrDataExpr **Res);
static int ParseTypedElt(ParseEnv *this,ScrDataExpr **R);
static int ParseCondExpr(ParseEnv *this,ScrDataExpr **T,ScrType *Expected);

#define ParseComposeBegin(Name,Cmpnt)\
static int Name(ParseEnv *this,ScrDataExpr **R) {\
	int Success,Cont;\
	LxcItem *c;\
	ScrDataExpr *A,*B;\
    ScrDataBOpDesc *Op;\
	Success = Cmpnt(this,&A);\
	Cont = Success;\
	while (Success && Cont) {\
	    c = Call(this->Src,Peek,1(0));\
		Op = 0;\
		switch (c->Type){

#define ParseComposeEnd(Cmpnt)\
		}\
		Cont = (Op!=0);\
		if (Cont) {\
			Call(this->Src,Next,1(1));\
			Success = Cmpnt(this,&B);\
			if (Success) {A=Call(this->ExprTgt,Compose,3(Op,A,B));}\
		}\
	} \
	*R = A;\
	return Success;\
}

ParseComposeBegin(ParseArtProd,ParseArithmeticElt)
	case '*': Op = &OperatorProd; break;
	case '/': Op = &OperatorQuotien; break;
	case '%': Op = &OperatorRest; break;
ParseComposeEnd(ParseArithmeticElt)

ParseComposeBegin(ParseArtSum,ParseArtProd)
	case '+': Op = &OperatorSum; break;
	case '-': Op = &OperatorDiff; break;
ParseComposeEnd(ParseArtProd)

ParseComposeBegin(ParseBinOp,ParseArtSum)
	case '&': Op = &OperatorBinAnd; break;
	case '|': Op = &OperatorBinOr; break;
	case '^': Op = &OperatorXor; break;
ParseComposeEnd(ParseArtSum)

ParseComposeBegin(ParseShiftOp,ParseBinOp)
    case CS_SAL: Op = &OperatorSal; break;
    case CS_SAR: Op = &OperatorSar; break;
ParseComposeEnd(ParseBinOp)

ParseComposeBegin(ParseCompOp,ParseShiftOp)
	case '<': Op = &OperatorInf; break;
	case '>': Op = &OperatorSup; break;
	case CS_SupEq: Op = &OperatorSupEq; break;
	case CS_InfEq: Op = &OperatorInfEq; break;
	case CS_Eq: Op = &OperatorEq; break;
	case CS_NotSet: Op = &OperatornEq; break;
ParseComposeEnd(ParseShiftOp)

ParseComposeBegin(ParseBoolOp,ParseCompOp)
	case CS_bAnd: Op = &OperatorAnd; break;
	case CS_bOr: Op = &OperatorOr; break;
ParseComposeEnd(ParseCompOp)

static int ParseArithmeticExpr(ParseEnv *this,ScrDataExpr **Res) {
	int Success;
	Success = ParseBoolOp(this,Res);
	return Success;
}
static int ParseArithmeticElt(ParseEnv *this,ScrDataExpr **Res) {
	int Success;
	LxcItem *c;
	int t;
	Success = (0==0);
	c = Call(this->Src,Peek,1(0));
	t = c->Type;
	switch (t) {
		case '+':
	        Call(this->Src,Next,1(1));
			Success = ParseArithmeticElt(this,Res);
        break;
		case '-':
	        Call(this->Src,Next,1(1));
			Success = ParseArithmeticElt(this,Res);
			if (Success) {*Res = Call(this->ExprTgt,Alter,2(&OperatorNeg,*Res));}
        break;
		case '~':
	        Call(this->Src,Next,1(1));
			Success = ParseArithmeticElt(this,Res);
			if (Success) {*Res = Call(this->ExprTgt,Alter,2(&OperatorInvert,*Res));}
		break;
		case '!':
		     Call(this->Src,Next,1(1));
			 Success = ParseArithmeticElt(this,Res);
			 if (Success) {*Res = Call(this->ExprTgt,Alter,2(&OperatorNot,*Res));}
		break;
		case '(':
	        Call(this->Src,Next,1(1));
            Success = ParseArithmeticExpr(this,Res);
            Success = Success && ForcedCheckDiscard(this,')',"Expression arithmetique: ')' manquant");
		break;
		case CS_ConstChar:{
			ScrType *tc;
		    tc = Call(this->TypeTgt,Atom,1(ScbChar));
	        *Res = Call(this->ExprTgt,Const,2(tc,SDValInt(c->Literal[0])));
			Call(this->Src,Next,1(1));
		} break;
		case CS_ConstInt:
		case CS_ConstFloat: {
			ScrType *ti;
			ti = Call(this->TypeTgt,Atom,1(ScbInt));
		    *Res = Call(this->ExprTgt,Const,2(ti,SDValInt(TlsStringToInt(c->Literal))));
			Call(this->Src,Next,1(1));
	    } break;
		case '?':  {
			ScrType *ti;
			ti = Call(this->TypeTgt,Atom,1(ScbInt));
		    Success = ParseCondExpr(this,Res,ti); 
		} break;
		case CS_ConstIdentifier: 
		    Success = ParseTypedElt(this,Res); 
		break;
		default: {
			Success = (0!=0);
			*Res = &ScrDataExprNull;
			Call(this->Src,Error,1("Erreur de on attendait un entier."));
		} break;
	}
	return Success;
}

/*___________________________
 |
 | String Expr
 |___________________________
*/

static ScrDataVal *StringConcat(ScrDataVal *A,ScrDataVal *B) {
	return SDValString(ScrStringCat(Call(A,String,0),Call(B,String,0)));
}
static ScrDataBOpDesc StrngCatOperator = {'_',/*&ScrTypeString,&ScrTypeString,&ScrTypeString,*/&StringConcat};

static ScrDataVal *IntToString(ScrDataVal *A) { return SDValString(ScrStringDecimal(Call(A,Int,0)));}
static ScrDataUOpDesc IntToStringOperator = {'$',/*&ScrTypeString,&ScrTypeInt,*/IntToString};

static int ParseStringExpr(ParseEnv *this,ScrDataExpr **Res);
static int ParseStringElt(ParseEnv *this,ScrDataExpr **Res) {
	int Success;
	LxcItem *c;
	c = Call(this->Src,Peek,1(0));
	switch (c->Type) {
	case '!': case '~': case '-': case '+':
	case CS_ConstInt: case CS_ConstChar: case CS_ConstFloat:
		Success = ParseArithmeticExpr(this,Res);
		if (Success) {
			*Res = Call(this->ExprTgt,Alter,2(&IntToStringOperator,*Res));
		}
	break;
	case CS_ConstString: {
		ScrType *str;
		Success = (0==0); str = Call(this->TypeTgt,Atom,1(ScbString));
        *Res = Call(this->ExprTgt,Const,2(str,SDValString(ScrStringDirect(c->Literal))));
		Call(this->Src,Next,1(1));
	} break;
	case '(':
	    Call(this->Src,Next,1(1));
		ParseStringExpr(this,Res);
		ForcedCheckDiscard(this,')',"Expression chaine: ')' manquant.");
	break;
	case CS_ConstIdentifier: 
	    Success = ParseTypedElt(this,Res); 
	break;
	case '?': {
		ScrType *str;
		str = Call(this->TypeTgt,Atom,1(ScbString));
	    Success = ParseCondExpr(this,Res,str); 
    } break;
	default:
        Success = (0!=0);
	    Call(this->Src,Error,1("Expression de type chaine attendue."));
	break;
	}
	return Success;
}
static int ParseStringExpr(ParseEnv *this,ScrDataExpr **Res) {
	int Success;
	ScrDataExpr *A,*B;
	LxcItem *c;
	Success = ParseStringElt(this,&A);
	c = Call(this->Src,Peek,1(0));
	while((Success)&&(c->Type=='~')) {
		Call(this->Src,Next,1(1));
		Success = ParseStringElt(this,&B);
		if (Success) {
			A = Call(this->ExprTgt,Compose,3(&StrngCatOperator,A,B));
		    c = Call(this->Src,Peek,1(0));
		}
	}
	*Res = A;
	return Success;
}

/*______________________________________________________
 |
 | Array expr:
 |    Concat:: expr~expr
 |    Alter::  expr1/expr2   [[ expr1 is altering expr2. ]]
 |  Form1:: {elt,elt,elt...elt} [[ elt nb should match the array size ]]
 |  Form2:: (label0:elt,...,elt; label1,...,labeln:elt,elt,...,elt; default:elt;) [[default may appear anywhere]]
 |
 | Access:
 | In expression context, it is disallowed to set elt value individually.
 | Use an instruction context to do so.
 |______________________________________________________
*/

    /*-----*/

typedef struct { 
	struct ArrayEltParse *Static; 
	int nb,count;
    ScrArray *Tgt;
	void *EltNull;
} ArrayEltParse;
struct ArrayEltParse {
	ScrCatalogEntry (*GetEntry)(ArrayEltParse *this,int *Success,ParseEnv *Src);
};
static int ParseArrayForm1(ParseEnv *this,ArrayEltParse *tgt) {
	int count,Success,cont;
	Success = (0==0);
	count = 0; cont = (tgt->nb!=count);
	while (((tgt->nb<0)||(count!=tgt->nb)) && cont) {
		ScrCatalogEntry D;
		LxcItem *c;
		int success;
		D = Call(tgt,GetEntry,2(&success,this));
		Success = Success && success;
		if (success) Call(tgt->Tgt,SetElt,2(count,D));
		count ++;
		c = Call(this->Src,Peek,1(0));
		cont = (c->Type==',');
		if (cont) { Call(this->Src,Next,1(1)); }
	}
	tgt->count = count;
	if (cont) { Call(this->Src,Error,1("Elements surnumeraires dans une definition de tableau."));}
	Success = Success && (!cont);
	return Success;
}

static int ParseArrayForm2EltList(ParseEnv *this,int idx,ScrCatalogEntry *Elt,ArrayEltParse *Tgt) {
	int Success;
	*Elt = Call(Tgt,GetEntry,2(&Success,this)); // First elt is also the value of the list of elt
	if (Success) {
		LxcItem *c;
		ScrCatalogEntry D;
		D = *Elt; Call(Tgt->Tgt,SetElt,2(idx,D)); 
		c = Call(this->Src,Peek,1(0));
		while (c->Type==',') {
			int success;
		    idx++; Call(this->Src,Next,1(1));
            D = Call(Tgt,GetEntry,2(&success,this));
			if (success) { Call(Tgt->Tgt,SetElt,2(idx,D)); }
			c = Call(this->Src,Peek,1(0));
			Success = Success && success;
		}
	}
	return Success;
}
static int ParseArrayForm2IdxList(ParseEnv *this,ScrCatalogEntry *Elt,ArrayEltParse *Tgt) {
	int Success,idx;
	ScrDataExpr *Idx;
	LxcItem *c;
	Success = ParseArithmeticExpr(this,&Idx);
	Success = Success && ScrIntExprEvalAttempt(&idx,Idx);
    c = Call(this->Src,Peek,1(0));
	if (c->Type==','||c->Type==':') {
		if (c->Type==',') {
			Call(this->Src,Next,1(1));
            Success = Success && ParseArrayForm2IdxList(this,Elt,Tgt); // We're lazy here.
			if (Success) { Call(Tgt->Tgt,SetElt,2(idx,*Elt)); }
		} else {
            Call(this->Src,Next,1(1));
			Success = Success && ParseArrayForm2EltList(this,idx,Elt,Tgt);
		}
	} else {
		Call(this->Src,Error,1("Definition de tableau: ',' ou ':' attendu"));
		Elt->Data = Tgt->EltNull;
		Success = (0!=0);
	}
	return Success;
}

static int ParseArrayForm2(ParseEnv *this,ArrayEltParse *tgt) {
	int Success,cont,success;
	Success = (0==0); cont = (0==0);
	while (cont && Success) {
		LxcItem *c;
		ScrCatalogEntry D;
		c = Call(this->Src,Peek,1(0));
		if (c->Type == CS_Default) {
			Call(this->Src,Next,1(1));
			Success = CheckDiscard(this,':',"Definition de tableau: ':' manquant.");
			D = Call(tgt,GetEntry,2(&success,this));
			Call(tgt->Tgt,SetDefault,1(D));
            Success = Success && success;
		} else {
			Success = ParseArrayForm2IdxList(this,&D,tgt);
		}
		Success = Success && CheckDiscard(this,';',"Definition de tableau: ';' manquant.");
        c = Call(this->Src,Peek,1(0));
		cont = ((c->Type!=')')&&(c->Type!=CS_Eof));
	}
	return Success;
}

    /*-----*/

static int ParseArrayElt(ParseEnv *this,ScrDataExpr **R,ScrType *T);
static ScrDataVal *ArrayConcatOp(ScrDataVal *A,ScrDataVal *B) {
	ScrArray *a,*b,*c;
	int min,max,nb;
	a = Call(A,Array,0); b = Call(B,Array,0);
	c = ScrConcatArray(a,b);
	Call(c,Dim,3(&min,&max,&nb));
	return SDValArray(c);
}
static ScrDataVal *ArrayAlterOp(ScrDataVal *A,ScrDataVal *B) {
	ScrArray *a,*b,*c;
	a = Call(A,Array,0); b = Call(B,Array,0);
	c = ScrAlteredArray(a,b);
	return SDValArray(c);
}

static int ParseArrayAlter(ParseEnv *this,ScrDataExpr **R,ScrType *T) {
	int Success;
	ScrDataExpr *d;
	LxcItem *c;
    Success = ParseArrayElt(this,&d,T);
	c = Call(this->Src,Peek,1(0));
	while (c->Type == '/') {
		ScrDataExpr *n;
        ScrDataBOpDesc *Operation;
		rPush(Operation);
		Operation->Label = '/'+0x80;
		// Operation->x = Call(d,GetType,0);
		Operation->op = ArrayAlterOp;
		Call(this->Src,Next,1(1));
        Success = Success && ParseArrayElt(this,&n,T);
		// Operation->y = Operation->r = Call(n,GetType,0);
		d = Call(this->ExprTgt,Compose,3(Operation,d,n));
		c = Call(this->Src,Peek,1(0));
	}
	*R = d;
	return Success;
}
static int ParseArrayConcat(ParseEnv *this,ScrDataExpr **R,ScrType *T) {
	int Success;
	ScrDataExpr *d;
	LxcItem *c;
	Success = ParseArrayAlter(this,&d,T);
	c = Call(this->Src,Peek,1(0));
	if (c->Type == '~') {
    ScrDataBOpDesc *Operation;
	ScrType *eltT,*uArr;
	eltT = Call(T,Array.EltType,0);
	uArr = Call(this->TypeTgt,Array,2(-1,eltT));
	rPush(Operation);
	Operation->Label = '~'+0x80;
	// Operation->x = Operation->y = Operation->r = uArr;
	Operation->op = ArrayConcatOp;
	while (c->Type == '~') {
		ScrDataExpr *n;
		Call(this->Src,Next,1(1));
		Success = Success && ParseArrayAlter(this,&n,uArr);
		d = Call(this->ExprTgt,Compose,3(Operation,d,n));
	    c = Call(this->Src,Peek,1(0));
	}}
	*R = d;
	return Success;
}

  /*--------*/

static ScrCatalogEntry ExprEltGetEntry(ArrayEltParse *this,int *Success,ParseEnv *Src) {
	ScrDataExpr *r;
	ScrCatalogEntry R;
	*Success = ParseTypedElt(Src,&r);
    R.Data = r;
	return R;
}
static ArrayEltParse *ExprEltParseInit(ArrayEltParse *dst,ScrType *T) {
	static struct ArrayEltParse Static = {ExprEltGetEntry};
	dst->Static = &Static;
	dst->nb = Call(T,Array.EltNb,0);
	if (dst->nb>=0) {
		dst->Tgt = ScrSetSizedArray(dst->nb,&ScrDataExprNull);
	} else {
		dst->Tgt = ScrUnknownSizedArray(&ScrDataExprNull);
	}
	dst->count = 0; dst->EltNull = &ScrDataExprNull;
	return dst;
}

static int ExprParseArrayForm1(ParseEnv *this,ScrDataExpr **R,ScrType *T) {
	int Success;
	ArrayEltParse tgt;
    ExprEltParseInit(&tgt,T);
	Success = ParseArrayForm1(this,&tgt);
	if (tgt.nb<0) { 
		ScrType *EltType;
		EltType = Call(T,Array.EltType,0);
		tgt.nb = tgt.count; 
		T = Call(this->TypeTgt,Array,2(tgt.count,EltType)); 
	}
	if (tgt.count!=tgt.nb) {
		Success = (0!=0);
		Call(this->Src,Error,1("Elements manquant dans une definition de tableau."));
	}
	*R = Call(this->ExprTgt,Vector,2(T,tgt.Tgt));
	return Success;
}

static int ExprParseArrayForm2(ParseEnv *this,ScrDataExpr **R,ScrType *T) {
	int Success;
	ScrType *EltType;
	ArrayEltParse tgt;
    ExprEltParseInit(&tgt,T);
	Success = ParseArrayForm2(this,&tgt);
    *R = Call(this->ExprTgt,Vector,2(T,tgt.Tgt));
	return Success;
}

   /*--------*/

static int ParseArrayElt(ParseEnv *this,ScrDataExpr **R,ScrType *T) {
	int Success;
	LxcItem *c;
	Success = (0==0);
    c = Call(this->Src,Peek,1(0));
	switch (c->Type) {
	case '{': {
		Call(this->Src,Next,1(1));
		Success = ExprParseArrayForm1(this,R,T);
		ForcedCheckDiscard(this,'}',"Definition de tableau: '}' manquant.");
	} break;
	case '(': {
		Call(this->Src,Next,1(1));
		Success = ExprParseArrayForm2(this,R,T);
		ForcedCheckDiscard(this,'}',"Definition de tableau: ')' manquant.");
	} break;
	case CS_ConstIdentifier: {
		Success = ParseTypedElt(this,R);
	} break;
	case '?': {
		Success = ParseCondExpr(this,R,T);
	} break;
    default:
		Call(this->Src,Error,1("Definition de tableau attendue."));
		Success = (0!=0);
	break;
	}
	return Success;
}

static int ParseArrayExpr(ParseEnv *this,ScrDataExpr **R,ScrType *T) {
    return ParseArrayConcat(this,R,T);
}

/*______________________________________________________
 |
 | Struct/Fn Def Expr
 | Form1:: (elt,elt,...,elt)  [[ elt nb should match the size ]]
 | Form2:: { type a=b; x=a+2; ....  z=a-2; } [[ This is generic block, with local scopes/values. ]]
 |
 | Invocation:
 |     Fn Param   [[ The custom is to use the Form1 above to write Param. ]]
 |______________________________________________________
*/

typedef struct { struct InsertAction *Static; } InsertAction; 
struct InsertAction {
     int (*Insert)(InsertAction *this,ParseEnv *tgt,char *Label,ScrType *Type);
};

typedef int ParseTypeMulti(ParseEnv *this,ScrType *Base,InsertAction *Insert);
static ParseTypeMulti ParseTypePrefixList,ParseTypePrefix;
static int ParseTypeElt(ParseEnv *this,ParseTypeMulti *Get,InsertAction *Insert);

static InsertAction ParseInsertType;
static InsertAction ParseInsertData;

static int ParseSetExprList(ParseEnv *this);
static int ParseExprBodyScope(ParseEnv *this);
static int ParseExprParamScope(ParseEnv *this,ScrType *Param);
static int ParseExprParamElt(ParseEnv *this,ScrDataExpr **D,ScrType *T);

static ScrScope *OpenStructScope(ParseEnv *this,ScrType *T) { 
	this->Dst = Call(this->Dst,ExprStructOpen,1(T));
	return this->Dst;
}

static ScrScope *OpenFnScope(ParseEnv *this,ScrType *T) {
	this->Dst = Call(this->Dst,ExprFnOpen,1(T));
	return this->Dst;
}

static int ParseStructExpr(ParseEnv *this,ScrDataExpr **R,ScrType *T,ScrScope *(*scopeOpen)(ParseEnv *this,ScrType *T)) {
	int Success;
	LxcItem *c;
	Success = (0==0);
	c = Call(this->Src,Peek,1(0));
	switch (c->Type) {
	case '{':case'(': {
		ScrScope *o,*n;
		ScrInstruction *dummy;
		o = this->Dst; n = scopeOpen(this,T);
		if (c->Type=='{') {
		    Call(this->Src,Next,1(1));
            Success = ParseExprBodyScope(this);
		    ForcedCheckDiscard(this,'}',"Definition de structure: '}' manquant.");
		} else {
		    Call(this->Src,Next,1(1));
			Success = ParseExprParamScope(this,T);
		    ForcedCheckDiscard(this,')',"Definition de structure: ')' manquant.");
		}
		this->Dst = o;
		*R = Call(n,Close,1(&ScrInstructionNull));
	} break;
	case CS_ConstIdentifier: {
		Success = ParseTypedElt(this,R);
	} break;
	case '?': {
		Success = ParseCondExpr(this,R,T);
	} break;
	default:
		Call(this->Src,Error,1("Definition de structure attendue."));
		Success = (0!=0);
	break;
	}
	return Success;
}

static int ParseExprParamElt(ParseEnv *this,ScrDataExpr **D,ScrType *T) {
	int Success;
	ScrTypeBase eb;
	Success = (0==0);
	eb = ScrTypeActualBase(T);
	switch (eb) {
	case ScbInt: Success = ParseArithmeticExpr(this,D); break;
	case ScbString: Success = ParseStringExpr(this,D); break;
	case ScbStruct: Success = ParseStructExpr(this,D,T,OpenStructScope); break;
	case ScbFn: Success = ParseStructExpr(this,D,T,OpenFnScope); break;
	case ScbArray: Success = ParseArrayExpr(this,D,T); break;
	default:
	break;
	}
	return Success;
}
static int ParseExprParamScope(ParseEnv *this,ScrType *Param) { // Expression parenthesee "(Expr,Expr,Expr...)"
	int Success;
	int i,nb;
	i = 0; nb = Call(Param,Struct.EltNb,0);
	Success = (0==0);
	while ((i<nb)&&(Success)) {
		ScrType *T;
		ScrDataExpr *p;
		char *Label;
		T = Call(Param,Struct.Elt,2(&Label,i));
		Success = ParseExprParamElt(this,&p,T);
		if (Success) {
		    ScrDataAccess *d;
		    d = Call(this->Dst,GetData,1(Label));
		    Call(d,SetValue,1(p));
		}
		i++;
        if (i<nb) { Success = Success && CheckDiscard(this,',',"Expression complexe: ',' manquant.");}
	}
	return Success;
}

static int ParseInstrBlock(ParseEnv *this,ScrInstruction **R);
static int ParseDefineDataInsert(InsertAction *this,ParseEnv *tgt,char *Label,ScrType *Type) {
	int Success;
	LxcItem *c;
	Success = Call(tgt->Dst,AddData,2(Label,Type));
	c = Call(tgt->Src,Peek,1(0));
	if ((c->Type=='=')||(c->Type=='{')) {
		ScrDataExpr *r;
		int set;
		set = (c->Type=='=');
		if (c->Type=='=') {
		    Call(tgt->Src,Next,1(1));
            Success = ParseExprParamElt(tgt,&r,Type);
		} else {
		    ScrTypeBase tb;
		    tb = ScrTypeActualBase(Type);
			set = (tb==ScbFn);
			if (set) {
			    ScrScope *o;
			    ScrInstruction *Instr;
		        Call(tgt->Src,Next,1(1));
			    o = tgt->Dst;
			    tgt->Dst = Call(tgt->Dst,ProcedureOpen,1(Type));
                Success = ParseInstrBlock(tgt,&Instr);
			    r = Call(tgt->Dst,Close,1(Instr));
			    tgt->Dst = o;
				ForcedCheckDiscard(tgt,'}',"Definition de procedure: '}' manquant. ");
			}
		}
		if (Success && set) {
			ScrDataAccess *ac;
			ac = Call(tgt->Dst,GetData,1(Label));
			Call(ac,SetValue,1(r));
		}
	}
	return Success;
}
static struct InsertAction ParseDefineDataStatic = { &ParseDefineDataInsert};
static InsertAction ParseDefineData = {&ParseDefineDataStatic};

static int ParseSetExprList(ParseEnv *this) { // Expression "a=va,b=vb,...;"
	int Success,cont;
	LxcItem *c;
	Success = (0==0); cont = (0==0);
	do {
		c = Call(this->Src,Peek,1(0));
		Success = (c->Type == CS_ConstIdentifier);
		if (Success) {
            ScrDataAccess *ac;
			ac = Call(this->Dst,GetData,1(c->Literal));
			Call(this->Src,Next,1(1));
            Success = CheckDiscard(this,'=',"Signe '=' attendu dans liste de definition");
            if (Success) {
				ScrDataExpr *val;
				Success = ParseExprParamElt(this,&val,Call(ac,GetType,0));
				if (Success) { Call(ac,SetValue,1(val));}
	            c = Call(this->Src,Peek,1(0));
		        cont = (c->Type==',');
		        if (cont) { Call(this->Src,Next,1(1)); }
			}
		} else {
			Call(this->Src,Error,1("Identifiant attendu dans liste de definition."));
		}
	} while((cont)&&(Success));
	return Success;
}
static int ParseExprBodyScope(ParseEnv *this) { // Expression accolee "{declaration,... valeur=Expr,...}"
	int Success,cont;
	LxcItem *c;
	cont = (0==0);
	do {
        c = Call(this->Src,Peek,1(0));
	    switch (c->Type) {
	    case CS_Typedef:
		     Call(this->Src,Next,1(1));
		     Success = ParseTypeElt(this,ParseTypePrefixList,&ParseInsertType);
		     SkipAfter(this,';');
	    break;
		case CS_Char:
		case CS_Int:
			Success = ParseTypeElt(this,ParseTypePrefixList,&ParseDefineData);
			SkipAfter(this,';');
		break;
		case '{': {
			ScrScope *o;
			ScrInstruction *dummy;
			Call(this->Src,Next,1(1));
			o = this->Dst;
			this->Dst = Call(this->Dst,ExtendOpen,0);
			Success = ParseExprBodyScope(this);
			Call(this->Dst,Close,1(&ScrInstructionNull));
			this->Dst = o;
		    Success = Success && ForcedCheckDiscard(this,'}',"Extension de portee: } manquant.");
		} break;
		case CS_ConstIdentifier: {
		    ScrType *t;
		    t = Call(this->Dst,GetType,1(c->Literal));
            if (t==&ScrTypeNull) {
                Success = ParseSetExprList(this);
			} else {
				Success = ParseTypeElt(this,ParseTypePrefixList,&ParseDefineData);
			}
			SkipAfter(this,';');
	    } break;
	    default:
		    cont = (0!=0);
	    break;
	    }
	} while(cont);
	return Success;
}

/*____________________________________
 |
 | Identifiers / Typeless expressions.
 |____________________________________
*/

static int ParseCondExpr(ParseEnv *this,ScrDataExpr **T,ScrType *Expected) {
	int Success;
	Success = CheckDiscard(this,'?',"Expression conditionnelle: '?' manquant.");
	if (Success) {
		ScrDataExpr *cond,*t,*f;
		cond = t = f = &ScrDataExprNull;
		Success = CheckDiscard(this,'(',"Expression conditionnelle: ( manquant.");
		if (Success) { Success = ParseArithmeticExpr(this,&cond);}
		if (Success) { Success = CheckDiscard(this,',',"Expression conditionnelle: , manquant."); }
		if (Success) { Success = ParseExprParamElt(this,&t,Expected); }
		if (Success) { Success = CheckDiscard(this,',',"Expression conditionnelle: , manquant."); }
		if (Success) { Success = ParseExprParamElt(this,&f,Expected); }
		*T = Call(this->ExprTgt,Cond,3(cond,t,f));
		Success = Success && ForcedCheckDiscard(this,')',"Expression conditionnelle: ) manquant.");
	}
	return Success;
}

   /*------------------*/

static int ParseRValue(ParseEnv *this,ScrPathExpr **R);
static int ParseLValue(ParseEnv *this,ScrPathExpr **R);

static int ParsePathBase(ParseEnv *this,int *Atom,ScrPathExpr **R) {
	int Success;
	LxcItem *c;
	ScrTypeBase tb;
	c = Call(this->Src,Peek,1(0));
	*Atom = (0!=0);
	Success = (c->Type==CS_ConstIdentifier);
	if (Success) {
		ScrDataAccess *ac;
		ScrDataTgt *try;
        ac = Call(this->Dst,GetData,1(c->Literal));
		*R = ScrPathExprAccess(this->ExprTgt,ac);
		tb = ScrTypeActualBase(Call(ac,GetType,0));
		switch (tb) {
		case ScbInt: case ScbString: {
			Call(this->Src,Next,1(1));
			*Atom = (0==0);
		} break;
		case ScbArray:
		case ScbStruct:
		case ScbFn:{
			Call(this->Src,Next,1(1));
		} break;
		default: {
			*Atom=(0==0); Success = (0!=0);
			Call(this->Src,Next,1(1));
			Call(this->Src,Error,1("Identifiant inconnu"));
		} break;
	    }
	} else {
		Success = (0!=0);
		*R = &ScrPathExprNull;
		*Atom = (0==0);
		Call(this->Src,Error,1("Identifiant attendu"));
	}
	return Success;
}

static int ParsePathPrefix(ParseEnv *this,ScrPathExpr **R);

static ScrPathExpr *AddAppliedValue(ParseEnv *this,ScrPathExpr *Fn,ScrDataExpr *Param) {
	ScrDataAccess *Private;
	ScrPathExpr *Applied;
	ScrDataExpr *applied;
	ScrType *fnres;
	int discard;
	Applied = ScrPathExprApply(this->ExprTgt,Fn,Param);
	fnres = Call(Applied,GetType,1(&discard));
	Private = Call(this->Dst,AddPrivate,1(fnres));
	applied = Call(Applied,Right,0);
	Call(Private,SetValue,1(applied));
	return Applied;
}
static int ParsePathSuffix(ParseEnv *this,ScrPathExpr *Base,ScrPathExpr **R) {
	int Success,cont;
	ScrTypeBase tb;
	cont = (0==0); Success = (0==0);
	while(cont && Success) {
	    LxcItem *c;
		int discard;
	    c = Call(this->Src,Peek,1(0));
		tb = ScrTypeActualBase(Call(Base,GetType,1(&discard)));
		switch (c->Type) {
		case CS_ConstIdentifier: case '{': case '(': {
			Success = (tb==ScbFn);
			if (Success) {
				ScrType *Bt;
				ScrDataExpr *Param;
				Bt = Call(Base,GetType,1(&discard));
				Success = ParseExprParamElt(this,&Param,Call(Bt,Fn.Param,0));
				if (Success) { Base = AddAppliedValue(this,Base,Param); }
			} else {
				cont = (0!=0);
				Call(this->Src,Error,1("Application de parametre a un type autre qu'une fonction."));
			}
		} break;
		case '[': {
			Success = (tb==ScbArray);
			Call(this->Src,Next,1(1));
			if (Success) {
				ScrDataExpr *Idx;
				Success = ParseArithmeticExpr(this,&Idx);
				Base = ScrPathExprElt(this->ExprTgt,Base,Idx);
			}
			ForcedCheckDiscard(this,']',"Element de tableau: ']' manquant.");
		} break;
		case '+': {
			cont = ((tb==ScbArray)||(tb==ScbPtr));
			if (cont) {
				ScrDataExpr *Idx;
				Call(this->Src,Next,1(1));
				Success = ParseArithmeticExpr(this,&Idx);
				Base = ScrPathExprElt(this->ExprTgt,Base,Idx);
				Base = Call(Base,Address,0);
			}
		} break;
		case '-': {
			cont = ((tb==ScbArray)||(tb==ScbPtr));
			if (cont) {
				ScrType *pt;
				ScrPathExpr *derv;
				Call(this->Src,Next,1(1));
				Success = ParsePathPrefix(this,&derv);
				pt = Call(this->TypeTgt,Atom,1(ScbPath));
				if (Success){ Base = ScrPathPath(this->ExprTgt,pt,derv,Base); }
			}
		} break;
		case '.': case CS_Idrct: {
			Success = (c->Type=='.');
			if (!Success) {
				Success = (tb==ScbPtr);
				if (Success) {
				    Base = ScrPathIdrct(this->ExprTgt,Base);
	                tb =  ScrTypeActualBase(Call(Base,GetType,1(&discard)));
				}
			}
			Success = Success && (tb==ScbStruct);
			Call(this->Src,Next,1(1));
		    if (Success) {
				ScrType *T;
				int Num;
			    c = Call(this->Src,Peek,1(0));
			    Success = (c->Type==CS_ConstIdentifier);
				if (Success) {
				    T = Call(Base,GetType,1(&discard));
				    Num = Call(T,Struct.EltNum,1(c->Literal));
				    Success = (Num>=0);
					if (Success){
						Base = ScrPathExprField(this->ExprTgt,Base,Num);
					} else {
						Call(this->Src,Error,1("Acces a un champs: champs inconnu"));
					}
					Call(this->Src,Next,1(1));
				} else {
					Call(this->Src,Error,1("Acces a un champs: identifiant attendu"));
				}
			} else {
				Call(this->Src,Error,1("Acces a un champs dans autre chose qu'une structure."));
			}
		} break;
        default: cont = (0!=0);
		}
	}
	*R = Base;
	return Success;
}

static int ParsePathPrefix(ParseEnv *this,ScrPathExpr **R) {
	int Success;
	ScrPathExpr *r;
	LxcItem *c;
	r = &ScrPathExprNull;
	Success = (0==0);
	c = Call(this->Src,Peek,1(0));
	switch (c->Type) {
	case '*': {
		ScrPathExpr *Base;
		Success = ParsePathPrefix(this,&Base);
		if (Success) { r = Call(Base,Idrct,0);} 
	} break;
	case '&': {
		ScrPathExpr *Base;
		Success = ParsePathPrefix(this,&Base);
		if (Success) { r = Call(Base,Address,0);}
	} break;
	case '(': {
		ScrPathExpr *Base;
		Call(this->Src,Next,1(1));
	    Success = ParsePathPrefix(this,&Base);
		Success = Success && CheckDiscard(this,')',"')' manquant dans un l'expression d'une valeur.");
		if (Success) { Success = ParsePathSuffix(this,Base,&r); }
	} break;
	default: {
		ScrPathExpr *Base;
		int Atom;
		Success = ParsePathBase(this,&Atom,&Base);
		if (Success) {
			if (Atom) {
				r = Base;
			} else {
			    Success = ParsePathSuffix(this,Base,&r);
			}
		}
	} break;
	}
	*R = r;
	return Success;
}

/*--------------------*/

static int ParseTypedElt(ParseEnv *this,ScrDataExpr **R) {
	ScrPathExpr *Path,*Base;
	ScrDataExpr *r;
	int Success,atom;
	r = &ScrDataExprNull;
	Success = ParsePathBase(this,&atom,&Base) ;
	if (Success) {
	    if (!atom) {
			Success = ParsePathSuffix(this,Base,&Path) ;
		} else {
			Path = Base;
		}
	    r = Call(Path,Right,0);
	}
	*R = r;
	return Success;
}

/*___________________________
 |
 | Instruction Block
 |___________________________
*/


static int ParseTarget(ParseEnv *this,ScrDataTgt **R) {
	int Success;
	LxcItem *c;
	ScrDataTgt *r;
	c = Call(this->Src,Peek,1(0));
	r = &ScrDataTgtNull;
	Success = (c->Type == CS_ConstIdentifier);
	if (Success) {
		char *Label;
		ScrDataAccess *Base;
		ScrDataTgt *base;
		Label = c->Literal;
		Base = Call(this->Dst,GetData,1(Label));
		r = Call(Base,GetVar,0);
	    Call(this->Src,Next,1(1));
		/* ToDo */
	}
	*R = r;
	return Success;
}

/*________________________________
 |
 |
 |________________________________
*/

static int ParseInstruction(ParseEnv *this,ScrInstruction **R);
typedef struct {
	InsertAction InsertAction;
	ScrUnkBound *Instr;
	int cont;
} tParseDefineVar;
static int ParseDefineVarInsert(InsertAction *this,ParseEnv *tgt,char *Label,ScrType *Type) {
	int Success;
	ScrTypeBase tb;
	ThisToThat(tParseDefineVar,InsertAction);
	tb = ScrTypeActualBase(Type);
	if (tb==ScbFn) {
		Success = Call(&ParseDefineData,Insert,3(tgt,Label,Type));
	} else {
	ScrDataAccess *ac;
	ac = Call(tgt->Dst,AddVar,2(Label,Type));
	if (ac!=&ScrDataAccessNull) {
	    LxcItem *c;
        c = Call(tgt->Src,Peek,1(0));
		if (c->Type=='=') {
			ScrInstruction *Instr;
			ScrDataExpr *Expr;
			that->cont = (0!=0);
			Call(tgt->Src,Next,1(1));
			Success = ParseExprParamElt(tgt,&Expr,Type);
			if (Success){
				Instr = Call(ac,SetValue,1(Expr));
				Call(that->Instr,Add,1(Instr));
			}
		}
	} }
	return Success;
}
static tParseDefineVar *ParseDefineVar(void) {
	tParseDefineVar *r;
	static struct InsertAction Static = { ParseDefineVarInsert };
	rPush(r); r->InsertAction.Static = &Static;
	r->cont = (0==0); r->Instr = ScrOneShotUnkBound();
	return r;
}

static int ParseInstrDeclareVar(ParseEnv *this,ScrInstruction **R) {
	int Success;
    ScrInstruction *r;
	tParseDefineVar *insert;
	int InstrNb;
	r = &ScrInstructionNull; Success = (0==0);
	pOpen
	    pIn(insert = ParseDefineVar());
		Success = ParseTypeElt(this,ParseTypePrefixList,&insert->InsertAction);
		InstrNb = Call(insert->Instr,Size,0);
		if (InstrNb>0) {
			if (InstrNb>1) {
			    ScrInstruction **b;
			    b = Call(insert->Instr,Pack,0);
			    r = Call(this->ExprTgt,Instr.Sequence,2(b,b+InstrNb));
			} else {
				r = Call(insert->Instr,Get,1(0));
			}
		}
	pClose
	*R = r;
    return Success;
}

static int ParseInstructionSet(ParseEnv *this,ScrInstruction **R) {
	ScrPathExpr *Tgt,*Data;
	ScrDataTgt *tgt;
	ScrInstruction *r;
	LxcItem *c;
	int Success,success;
	r = &ScrInstructionNull;
	Success = ParsePathPrefix(this,&Tgt);
	c = Call(this->Src,Peek,1(0));
	if (c->Type=='=') {
		Success = Success && ParsePathPrefix(this,&Data);
		tgt = Call(Tgt,Left,1(&success)); Success = Success && success;
		if (Success) {
			r = Call(Data,Set,1(tgt));
		} else {
			Call(this->Src,Error,1("Variable attendue mais valeur obtenue."));
		}
	} else {
		if (Success) {
		    ScrDataExpr *data;
		    ScrDataAccess *n;
		    ScrType *t;
		    data = Call(Tgt,Right,0);
		    t = Call(data,GetType,0);
		    n = Call(this->Dst,AddVar,2(0,t));
		    tgt = Call(n,GetVar,0);
		    r = Call(Tgt,Set,1(tgt));
		}
	}
	*R = r;
	return Success;
}

   /*---- Switch instruction -----*/

static ScrCatalogEntry InstrEltGetEntry(ArrayEltParse *this,int *Success,ParseEnv *Src) {
	ScrInstruction *r;
	ScrCatalogEntry D;
    *Success = ParseInstruction(Src,&r);
	D.Data = r;
	return D;
}
static ArrayEltParse *InstrEltParseInit(ArrayEltParse *dst) {
   static struct ArrayEltParse Static = {InstrEltGetEntry};
   dst->Static = &Static;
   dst->nb = -1; dst->count = 0;
   dst->Tgt = ScrUnknownSizedArray(&ScrInstructionNull);
   dst->EltNull = &ScrInstructionNull;
   return dst;
}
static int ParseInstructionArray(ParseEnv *this,ScrInstruction **R) {
    int Success;
	ScrInstruction *r;
	ScrDataExpr *idx;
	r = &ScrInstructionNull;
	Success = ParseArithmeticExpr(this,&idx);
	if (Success) { Success = CheckDiscard(this,'{',"Instruction switch: '{' manquant."); }
	if (Success) {
	    ArrayEltParse tgt;
	    InstrEltParseInit(&tgt);
		Success = ParseArrayForm2(this,&tgt);
		Success = Success && ForcedCheckDiscard(this,'}',"Instruction switch: '}' manquant.");
		if (Success) { r = Call(this->ExprTgt,Instr.Select,2(idx,tgt.Tgt)); }
	}
	*R = r;
	return Success;
}

    /*----------------*/
   
static int ParseInstruction(ParseEnv *this,ScrInstruction **R) {
	int Success,cont;
	LxcItem *c;
	ScrInstruction *r;
	Success = (0==0); cont = (0!=0); r = &ScrInstructionNull;
	do {
	c = Call(this->Src,Peek,1(0));
	switch (c->Type) {
	case '{': {
		ScrScope *o;
		Call(this->Src,Next,1(1));
		o = this->Dst;
		this->Dst = Call(this->Dst,ExtendOpen,0);
		Success = ParseInstrBlock(this,&r);
		Call(this->Dst,Close,1(&ScrInstructionNull));
		this->Dst = o;
		Success = Success && ForcedCheckDiscard(this,'}',"Bloc d'instruction: '}' manquant. ");
	} break;
	case CS_Switch:{
		Call(this->Src,Next,1(1));
		Success = ParseInstructionArray(this,&r);
	} break;
	case CS_If: {
		ScrDataExpr *cond;
		ScrInstruction *t,*f;
		Call(this->Src,Next,1(1));
		Success = ParseArithmeticExpr(this,&cond);
		if (Success) {
			Success = ParseInstruction(this,&t);
			if (Success) {
                c = Call(this->Src,Peek,1(0));
				if (c->Type==CS_Else) {
					Call(this->Src,Next,1(1));
					r = Call(this->ExprTgt,Instr.Cond,2(cond,t));
				} else {
					Success = ParseInstruction(this,&f);
					if (Success) { r = Call(this->ExprTgt,Instr.Alternate,3(cond,t,f));}
				}
			}
		}
	} break;
	case CS_While: {
        ScrDataExpr *cond;
		ScrInstruction *Ins;
		Call(this->Src,Next,1(1));
		Success = ParseArithmeticExpr(this,&cond);
		if (Success) {
			Success = ParseInstruction(this,&Ins);
			if (Success) {
				r = Call(this->ExprTgt,Instr.While,2(cond,Ins));
			}
		}
	} break;
	case CS_Do: {
		ScrDataExpr *cond;
		ScrInstruction *Ins;
		Call(this->Src,Next,1(1));
	    Success = ParseInstruction(this,&Ins);
		if (Success) {
			Success = ParseArithmeticExpr(this,&cond);
			if (Success) {
				r = Call(this->ExprTgt,Instr.Until,2(Ins,cond));
			}
		}
	} break;
	case CS_Const: {
		Call(this->Src,Next,1(1));
		Success = ParseTypeElt(this,ParseTypePrefixList,&ParseDefineData);
		SkipAfter(this,';');
		cont = (0==0);
	} break;
	case CS_Typedef:{
		Call(this->Src,Next,1(1));
		Success = ParseTypeElt(this,ParseTypePrefixList,&ParseInsertType);
		SkipAfter(this,';');
		cont = (0==0);
	} break;
	case CS_Unsigned: case CS_Signed: case CS_Long: case CS_Short: case CS_Void: case CS_Char:
	case CS_Int: case CS_Double: case CS_Float: case CS_Struct: {
		Success = ParseInstrDeclareVar(this,&r);
		cont = (r==&ScrInstructionNull);
		if (cont) SkipAfter(this,';');
	} break;
	case CS_ConstIdentifier: {
		char *Label;
		ScrType *T;
		Label = c->Literal;
        T = Call(this->Dst,GetType,1(Label));
		if (T!=&ScrTypeNull) {
			Success = ParseInstrDeclareVar(this,&r);
			cont = (r==&ScrInstructionNull);
		} else {
			Success = ParseInstructionSet(this,&r);
		}
		if (cont) SkipAfter(this,';');
	} break;
	case '*': {
		Success = ParseInstructionSet(this,&r);
	} break;
	}
	} while (Success && cont);
	*R = r;
	return Success;
}

static int ParseInstrBlock(ParseEnv *this,ScrInstruction **R) {
	int Success,cont;
	ScrUnkBound *Instr;
	LxcItem *c;
	ScrInstruction *r;
	Success = (0==0); cont = (0==0); r = &ScrInstructionNull;
	pOpen 
	    pIn(Instr=ScrOneShotUnkBound());
	    do {
			ScrInstruction *n;
			Success = ParseInstruction(this,&n);
			cont = (n!=&ScrInstructionNull);
			if (cont) { Call(Instr,Add,1(n)); }
			c = Call(this->Src,Peek,1(0));
			cont = (cont && (c->Type==';'));
			if (c->Type==';') { Call(this->Src,Next,1(1)); }
	    } while (Success && cont);
	    if (Success) {
			int in;
			ScrInstruction **b;
			in = Call(Instr,Size,0);
            b = Call(Instr,Pack,0);
			r = Call(this->ExprTgt,Instr.Sequence,2(b,b+in));
	    }
	pClose
	*R = r;
	return Success;
}

/*___________________________
 |
 | Type
 |___________________________
*/

static int ParseInsertTypeInsert(InsertAction *this,ParseEnv *tgt,char *Label,ScrType *Type) {
	int r;
	ScrType *Labeled;
	r = Call(tgt->Dst,AddType,1(Label));
	Labeled = Call(tgt->TypeTgt,Labeled,2(Label,Type));
	if (r) { r = Call(tgt->Dst,SetType,2(Label,Labeled)); }
	return r;
}
static struct InsertAction ParseInsertTypeStatic = {ParseInsertTypeInsert};
static InsertAction ParseInsertType = {&ParseInsertTypeStatic};

static int ParseInsertDataInsert(InsertAction *this,ParseEnv *tgt,char *Label,ScrType *Type) {
	int r;
	r = Call(tgt->Dst,AddData,2(Label,Type));
	return r;
}
static struct InsertAction ParseInsertDataStatic = {ParseInsertDataInsert};
static InsertAction ParseInsertData = {&ParseInsertDataStatic};

static int ParseTypeStructFields(ParseEnv *this,char *Label,ScrType **Res,int separator);

static int ParseFieldList(ParseEnv *this,int separator,InsertAction *Insert) {
	LxcItem *c;
	int Success,cont;
	ParseTypeMulti *multi;
	Success = (0==0);
	multi = (separator==',')?ParseTypePrefix:ParseTypePrefixList;
	do {
		Success = ParseTypeElt(this,multi,&ParseInsertData);
		c = Call(this->Src,Peek,1(0));
        cont = (c->Type == separator);
		if (cont) { 
			Call(this->Src,Next,1(1));
			c = Call(this->Src,Peek,1(0));
			cont = (c->Type!=')') && (c->Type!='}');
		}
	} while (Success && cont);
	return Success;
}

static int ParseTypeSuffix(ParseEnv *this,ScrType **Deriv,ScrType *Base) {
	LxcItem *c;
	ScrType *Actual;
	int Success;
	Success = (0==0);
	Actual = Base;
	c = Call(this->Src,Peek,1(0));
	switch (c->Type) {
	case '(': {
		Call(this->Src,Next,1(1));
		c = Call(this->Src,Peek,1(0));
		switch (c->Type) {
		case ')': {
			Actual=Call(this->TypeTgt,Fn,2(Base,Base));
		} break;
		case CS_Void: {
			ScrType *tv;
			Call(this->Src,Next,1(1));
			tv = Call(this->TypeTgt,Atom,1(ScbVoid));
			Actual=Call(this->TypeTgt,Fn,2(Base,tv));
	    } break;
		default: {
			ScrType *Param;
            Success = ParseTypeStructFields(this,"",&Param,',');
			if (Success) { Actual=Call(this->TypeTgt,Fn,2(Base,Param));}
		} break;
		}
	    ForcedCheckDiscard(this,')',"Definition de prototype: ')' manquant");
		if (Success) {
		     Success = ParseTypeSuffix(this,Deriv,Actual);
		}
	} break;
	case CS_Etc:{
		Call(this->Src,Next,1(1));
		Actual=Call(this->TypeTgt,List,1(Base));
		Success = ParseTypeSuffix(this,Deriv,Actual);
	} break;
	case '[': {
		int n;
		Call(this->Src,Next,1(1));
		c = Call(this->Src,Peek,1(0));
		n = -1;
		if (c->Type!=']') {
	        ScrDataExpr *Nb;
			if (ParseArithmeticExpr(this,&Nb)) {
				int s;
			    s = ScrIntExprEvalAttempt(&n,Nb);
				if (!s) Call(this->Src,Error,1("Definition de tableau: taille statique attendue."));
			} else {
                Call(this->Src,Error,1("Definition de tableau: expression entiere attendue."));
			}
		}
		Actual=Call(this->TypeTgt,Array,2(n,Base));
		SkipAfter(this,']');
		Success = ParseTypeSuffix(this,Deriv,Actual);
	} break;
	default:
	    *Deriv = Base;
	}
	return Success;
}
static int ParseTypePrefix(ParseEnv *this,ScrType *Base,InsertAction *Insert) { 
	int Success;
	ScrType *Actual;
	LxcItem *c;
	Success = (0==0);
	Actual = Base;
	c = Call(this->Src,Peek,1(0));
	switch (c->Type) {
	case '*': {
		Actual = Call(this->TypeTgt,Ptr,1(Base));
		Call(this->Src,Next,1(1));
		Success = ParseTypePrefix(this,Actual,Insert);
	} break;
	case CS_ConstIdentifier: {
		char *Label;
		Label = c->Literal;
		Call(this->Src,Next,1(1));
		Success = ParseTypeSuffix(this,&Actual,Base);
		if (Success) { Call(Insert,Insert,3(this,Label,Actual)); }
	} break;
	case '(': {
		ScrTypeThrou *Bbis;
		Bbis = Call(this->TypeTgt,Throu,0);
		Call(this->Src,Next,1(1));
		Success = ParseTypePrefix(this,&Bbis->ScrType,Insert);
		if (Success) {
			ForcedCheckDiscard(this,')',"Definition de type: ')' manquant");
            Success = ParseTypeSuffix(this,&Actual,Base);
			Bbis->Child = Actual;
		} else {
			SkipAfter(this,')');
		}
	} break;
	default: {
	    Success = ParseTypeSuffix(this,&Actual,Base);
	} break;
	}
	return Success;
}
static int ParseTypePrefixList(ParseEnv *this,ScrType *Base,InsertAction *Insert) {
	int Success,cont;
	do {
	    LxcItem *c;
	    Success = ParseTypePrefix(this,Base,Insert);
	    c = Call(this->Src,Peek,1(0));
		cont = (c->Type == ',');
		if (cont) { Call(this->Src,Next,1(1)); }
	} while (Success && cont);
	return Success;
}
static int ParseTypeStructFields(ParseEnv *this,char *Label,ScrType **Res,int separator) {
	int Success;
	ScrScope *pScope;
	pScope = this->Dst;
	this->Dst = Call(pScope,StructTypedefOpen,0);
	Success = ParseFieldList(this,separator,&ParseInsertData);
	*Res = Call(this->Dst,TypeClose,1(Label));
	this->Dst = pScope;
	return Success;
}
static int ParseTypeStruct(ParseEnv *this,ScrType **Res) {
	int Success;
	LxcItem *c;
	char *Label;
	Success = (0==0);
	c = Call(this->Src,Peek,1(0));
	if (c->Type==CS_ConstIdentifier) {
		Label = c->Literal;
		Call(this->Src,Next,1(1));
		c = Call(this->Src,Peek,1(0));
	} else { Label = ""; }
	Success = (c->Type=='{');
	if (Success) {
	    Call(this->Src,Next,1(1));
		Success = ParseTypeStructFields(this,Label,Res,';');
		SkipAfter(this,'}');
	} else {
        Call(this->Src,Error,1("Definition de structure manquante."));
	}
	if (!Success) { *Res = &ScrTypeNull; }
	return Success;
}
static int ParseTypeElt(ParseEnv *this,ParseTypeMulti *Get,InsertAction *Insert) {
	int Success;
	LxcItem *c;
	ScrType *t;
	Success = (0==0);
	c = Call(this->Src,Peek,1(0));
	switch (c->Type) {
	case CS_Int:
		Call(this->Src,Next,1(1));
		t = Call(this->TypeTgt,Atom,1(ScbInt));
		Success = Get(this,t,Insert);
    break;
	case CS_Char:
        Call(this->Src,Next,1(1));
		t = Call(this->TypeTgt,Atom,1(ScbChar));
		Success = Get(this,t,Insert);
	break;
	case CS_Void:
	    t = Call(this->TypeTgt,Atom,1(ScbVoid));
	    Success = Get(this,t,Insert);
	break;
	case CS_Struct: {
        Call(this->Src,Next,1(1));
		Success = ParseTypeStruct(this,&t);
		if (Success) { Success = Get(this,t,Insert); }
    } break;
	case CS_ConstIdentifier: {
		char *Label;
		Label = c->Literal;
		t = Call(this->Dst,GetType,1(Label));
		Success = (t!=&ScrTypeNull);
		if (Success) {
			Call(this->Src,Next,1(1));
			Success = Get(this,t,Insert);
		} else {
			char *s;
			sprintf(s,"Definition de type: Type %s inconnu.",Label);
			Call(this->Src,Error,1(s));
		}
    } break;
	default:
	    Call(this->Src,Error,1("Definition de type: Base inconnue."));
		Success = (0!=0);
	break;
	}
	return Success;
}


/*____________________________
 |
 | Module
 |____________________________
*/

static void ParseModule(ParseEnv *this) {
	LxcItem *c;
	int Success;
	c = Call(this->Src,Peek,1(0));
	while (c->Type!=CS_Eof) {
        ParseExprBodyScope(this);
		c = Call(this->Src,Peek,1(0));
		if (c->Type!=CS_Eof) {
			Call(this->Src,Error,1("Dans module: expression inconnue.\n"));
			SkipAfter(this,';');
			c = Call(this->Src,Peek,1(0));
		}
	}
}

/*
static int ParseEnvParseString(ScrParseEnv *this,char *Data) {
	int r;
	ThisToThat(ParseEnv,ScrParseEnv);
	r=ParseEnvParse(that,CPreprocEnvStringOpen(that->Path,Data));
	return r;
}*/

/*----------------------*/

typedef struct {
	ScrModuleParser ScrModuleParser;
	ParseEnv Envr;
} tModuleParser;
typedef struct {
	ScrModuleDesc ScrModuleDesc;
	tModuleParser *Parser;
	char *Name;
} FileModule;

static void FileModuleOpen(ScrModuleDesc *this,ScrScope *Base) {
	ScrVocabulary *Voc;
	ParseEnv *Envr;
	CPreprocEnv *Src;
	ScrErrorLog *OldErr;
	ScrInstruction *Proc;
	ThisToThat(FileModule,ScrModuleDesc);
	Envr = &that->Parser->Envr;
	Voc = Call(Base,GetVocabulary,0);
	Src = CPreprocEnvOpen(Envr->Path,that->Name);
	OldErr = Envr->Err;
	Envr->Err = CPrepErrorLogNew(OldErr,Src);
	Envr->TypeTgt = Call(Base,TypeTgt,0);
	Envr->ExprTgt = Call(Base,ExprTgt,0);
	Envr->Dst = Base;
	Envr->Src = LxcSrcNew(Voc,Envr->Err,Src);
    ParseModule(Envr);
	Envr->Err = OldErr;
	Envr->Src = 0;
	Call(Envr->Dst,Close,1(&ScrInstructionNull));
	Envr->Dst = 0;
	Envr->ExprTgt = 0;
	Envr->TypeTgt = 0;
}
static void FileModuleClose(ScrModuleDesc *this,ScrScope *Base) {
	ThisToThat(FileModule,ScrModuleDesc);
}

static ScrModuleDesc *ModuleParserSource(ScrModuleParser *this,char *Name) {
	FileModule *r;
	static struct ScrModuleDesc Static = { FileModuleOpen,FileModuleClose };
	ThisToThat(tModuleParser,ScrModuleParser);
    rPush(r); r->ScrModuleDesc.Static = &Static;
	r->Parser = that; r->Name = Name;
	return &r->ScrModuleDesc;
}
ScrModuleParser *ScrSetParserPath(char **Path,int dumpLog){
	tModuleParser *r;
	static struct ScrModuleParser Static = { ModuleParserSource };
	rPush(r); r->ScrModuleParser.Static = &Static;
	r->Envr.Path = Path;
	r->Envr.Err = dumpLog ? &ScrErrorLogNull:ScrErrorDirect();
	r->Envr.Dst = 0;
	r->Envr.Src = 0;
	r->Envr.TypeTgt = 0;
	r->Envr.ExprTgt = 0;
	return &r->ScrModuleParser;
}

