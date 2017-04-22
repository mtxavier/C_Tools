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
#include <List.h>
#include <ScrModule.h>
#include <Builder.Loc.h>


/*___________________________________
 |
 | 
 |___________________________________
*/

static ScrType *AccessNullGetType(ScrDataAccess *this) { return &ScrTypeNull; }
static ScrInstruction *AccessNullSetValue(ScrDataAccess *this,ScrDataExpr *Val) { &ScrInstructionNull;}
static ScrDataExpr *AccessNullGetValue(ScrDataAccess *this) { return &ScrDataExprNull; }
static ScrDataTgt *AccessNullGetVar(ScrDataAccess *this) { return &ScrDataTgtNull; }
static struct ScrDataAccess AccessNullStatic = {
	AccessNullGetType, AccessNullSetValue, AccessNullGetValue,AccessNullGetVar
};
ScrDataAccess ScrDataAccessNull = {&AccessNullStatic};


/*________________________________________
 |
 | Scopes
 |________________________________________
*/

static ScrVocabulary *ScopeNullGetVocabulary(ScrScope *this) { return &ScrVocabularyNull; }
static ScrTypeBrowser *ScopeNullTypeTgt(ScrScope *this) { return &ScrTypeBrowserNull; }
static ScrExprBrowser *ScopeNullExprTgt(ScrScope *this) { return &ScrExprBrowserNull; }
static int ScopeNullAddType(ScrScope *this,char *Label) { return (0!=0); }
static int ScopeNullSetType(ScrScope *this,char *Label,ScrType *Type) { return (0!=0); }
static ScrType *ScopeNullGetType(ScrScope *this,char *Label) { return &ScrTypeNull; }

static int ScopeNullAddData(ScrScope *this,char *Label,ScrType *Type) { return (0!=0); }
static ScrDataAccess *ScopeNullAddVar(ScrScope *this,char *Label,ScrType *Type) { return &ScrDataAccessNull; }
static ScrDataAccess *ScopeNullGetData(ScrScope *this, char *Label) { return &ScrDataAccessNull; }
static ScrDataAccess *ScopeNullAddPrivate(ScrScope *this, ScrType *Type) { return &ScrDataAccessNull;}

static ScrScope *ScopeNullTypedefOpen(ScrScope *this) { return &ScrScopeNull; }
static ScrScope *ScopeNullExtendOpen(ScrScope *this) {return &ScrScopeNull; }
static ScrScope *ScopeNullExprStructOpen(ScrScope *this,ScrType *Type) { return &ScrScopeNull; }
static ScrScope *ScopeNullExprFnOpen(ScrScope *this,ScrType *Type) { return &ScrScopeNull; }
static ScrScope *ScopeNullProcOpen(ScrScope *this,ScrType *Proto) { return &ScrScopeNull; }
static ScrDataExpr *ScopeNullClose(ScrScope *this,ScrInstruction *Block) { 
	return &ScrDataExprNull; 
}
static ScrType *ScopeNullTypeClose(ScrScope *this,char *Label) { return &ScrTypeNull; }

static struct ScrScope ScrScopeNullStatic = {
	ScopeNullGetVocabulary, ScopeNullTypeTgt, ScopeNullExprTgt,
	ScopeNullAddType, ScopeNullSetType, ScopeNullGetType,
	ScopeNullAddData, ScopeNullAddVar, ScopeNullGetData, ScopeNullAddPrivate,
	ScopeNullTypedefOpen, ScopeNullExtendOpen, 
	ScopeNullExprStructOpen, ScopeNullExprFnOpen, ScopeNullProcOpen,
	ScopeNullClose, ScopeNullTypeClose
};
ScrScope ScrScopeNull = {&ScrScopeNullStatic};

/*----------------*/

typedef struct ScopeThrou {
	ScrScope ScrScope;
	struct ScrScopeThrou *Static;
	struct ScopeThrou *Child;
} ScopeThrou;
struct ScrScopeThrou {
	ScrDataExpr *(*Expr)(ScopeThrou *this,ScrType *Type,int Depth,int num);
	ScrDataTgt *(*Tgt)(ScopeThrou *this,ScrType *Type,int Depth,int num);
	ScrDataAccess *(*LexicalData)(ScopeThrou *this,int OrgDepth,char *Label);
};

static ScrVocabulary *ScopeThrouGetVocabulary(ScrScope *this) {
	ThisToThat(ScopeThrou,ScrScope);
	return Call(&that->Child->ScrScope,GetVocabulary,0);
}
static ScrTypeBrowser *ScopeThrouTypeTgt(ScrScope *this) {
	ThisToThat(ScopeThrou,ScrScope);
	return Call(&that->Child->ScrScope,TypeTgt,0);
}
static ScrExprBrowser *ScopeThrouExprTgt(ScrScope *this) {
	ThisToThat(ScopeThrou,ScrScope);
	return Call(&that->Child->ScrScope,ExprTgt,0);
}
static int ScopeThrouAddType(ScrScope *this,char *Label) {
	ThisToThat(ScopeThrou,ScrScope);
	return Call(&that->Child->ScrScope,AddType,1(Label));
}
static int ScopeThrouSetType(ScrScope *this,char *Label,ScrType *Type) {
	ThisToThat(ScopeThrou,ScrScope);
	return Call(&that->Child->ScrScope,SetType,2(Label,Type));
}
static ScrType *ScopeThrouGetType(ScrScope *this,char *Label) {
	ThisToThat(ScopeThrou,ScrScope);
	return Call(&that->Child->ScrScope,GetType,1(Label));
}

static int ScopeThrouAddData(ScrScope *this,char *Label,ScrType *Type) {
	ThisToThat(ScopeThrou,ScrScope);
	return Call(&that->Child->ScrScope,AddData,2(Label,Type));
}
static ScrDataAccess *ScopeThrouAddVar(ScrScope *this,char *Label,ScrType *Type) {
	ThisToThat(ScopeThrou,ScrScope);
	return Call(&that->Child->ScrScope,AddVar,2(Label,Type));
}
static ScrDataAccess *ScopeThrouGetData(ScrScope *this,char *Label) {
	ThisToThat(ScopeThrou,ScrScope);
	return Call(&that->Child->ScrScope,GetData,1(Label));
}
static ScrDataAccess *ScopeThrouAddPrivate(ScrScope *this,ScrType *Type){
	ThisToThat(ScopeThrou,ScrScope);
	return Call(&that->Child->ScrScope,AddPrivate,1(Type));
}

static ScrScope *ScopeThrouStructTypedefOpen(ScrScope *this) {
	ThisToThat(ScopeThrou,ScrScope);
	return Call(&that->Child->ScrScope,StructTypedefOpen,0);
}
static ScrScope *ScopeThrouExtendOpen(ScrScope *this) {
	ThisToThat(ScopeThrou,ScrScope);
	return Call(&that->Child->ScrScope,ExtendOpen,0);
}
static ScrScope *ScopeThrouExprStructOpen(ScrScope *this,ScrType *Type) {
	ThisToThat(ScopeThrou,ScrScope);
	return Call(&that->Child->ScrScope,ExprStructOpen,1(Type));
}
static ScrScope *ScopeThrouExprFnOpen(ScrScope *this,ScrType *Type) {
	ThisToThat(ScopeThrou,ScrScope);
	return Call(&that->Child->ScrScope,ExprFnOpen,1(Type));
}
static ScrScope *ScopeThrouProcOpen(ScrScope *this,ScrType *Type) {
	ThisToThat(ScopeThrou,ScrScope);
	return Call(&that->Child->ScrScope,ProcedureOpen,1(Type));
}

static ScrDataExpr *ScopeThrouClose(ScrScope *this,ScrInstruction *I) {
	ThisToThat(ScopeThrou,ScrScope);
	return Call(&that->Child->ScrScope,Close,1(I));
}
static ScrType *ScopeThrouTypeClose(ScrScope *this,char *Label) {
	ThisToThat(ScopeThrou,ScrScope);
	return Call(&that->Child->ScrScope,TypeClose,1(Label));
}


static ScrDataExpr *ScopeThrouExpr(ScopeThrou *this,ScrType *Type,int Depth,int num) {
	return Call(this->Child,Expr,3(Type,Depth,num));
}
static ScrDataTgt *ScopeThrouTgt(ScopeThrou *this,ScrType *Type,int Depth,int num) {
	return Call(this->Child,Tgt,3(Type,Depth,num));
}
static ScrDataAccess *ScopeThrouLexicalData(ScopeThrou *this,int depth,char *Label) {
	return Call(this->Child,LexicalData,2(depth,Label));
}

    /*---------------*/

typedef struct {
	ScopeThrou ScopeThrou;
	ScrVocabulary *Voc;
	ScrTypeStructBuilder *Dst;
} ScopeTypeStruct;

static ScrVocabulary *ScopeTypeStructGetVocabulary(ScrScope *this) {
	ThisToThat(ScopeTypeStruct,ScopeThrou.ScrScope);
	return that->Voc;
}
static int ScopeTypeStructAddData(ScrScope *this,char *Label,ScrType *Type) {
	int idx,r;
	ThisToThat(ScopeTypeStruct,ScopeThrou.ScrScope);
	idx = Call(that->Dst,AddField,3(0,Label,Type));
	r = (idx>=0);
	if (!r) { r = Call(&that->ScopeThrou.Child->ScrScope,AddData,2(Label,Type)); }
    return r;
}
static ScrDataExpr *ScopeTypeStructClose(ScrScope *this,ScrInstruction *Block) { 
	return &ScrDataExprNull; 
}
static ScrType *ScopeTypeTypeClose(ScrScope *this,char *Label) {
	ScrTypeBrowser *Tgt;
    ThisToThat(ScopeTypeStruct,ScopeThrou.ScrScope);
	Tgt = Call(&that->ScopeThrou.Child->ScrScope,TypeTgt,0);
    return Call(that->Dst,Close,2(Tgt,Label));
}
static ScopeThrou *ScrScopeTypeStruct(ScopeThrou *Parent,ScrTypeStructBuilder *Bld) {
	ScopeTypeStruct *R;
	static struct ScrScopeThrou ThrouStatic = { ScopeThrouExpr, ScopeThrouTgt, ScopeThrouLexicalData};
	static struct ScrScope Static = {
        ScopeTypeStructGetVocabulary, ScopeThrouTypeTgt, ScopeThrouExprTgt,
		ScopeThrouAddType,ScopeThrouSetType,ScopeThrouGetType,
        ScopeTypeStructAddData,ScopeThrouAddVar,ScopeThrouGetData,ScopeThrouAddPrivate,
		ScopeThrouStructTypedefOpen,ScopeThrouExtendOpen,
		ScopeThrouExprStructOpen, ScopeThrouExprFnOpen, ScopeThrouProcOpen,
		ScopeTypeStructClose, ScopeTypeTypeClose
	};
	rPush(R);
	R->ScopeThrou.ScrScope.Static = &Static;
	R->ScopeThrou.Static = &ThrouStatic;
	R->ScopeThrou.Child = Parent;
	R->Voc = Call(&Parent->ScrScope,GetVocabulary,0);
	R->Dst = ScrTypeStructNew();
	return &R->ScopeThrou;
}

     /*---------*/

#include <Patricia.h>

/*__________________________
 |
 | ExprStruct
 |__________________________
*/

typedef struct xExprStruct xExprStruct;
typedef struct tExprStruct tExprStruct;
struct tExprStruct {
	ScopeThrou ScopeThrou;
	ScrDataAccess *(*ResultAccess)(struct tExprStruct *this,int nb);
	ScrType *Result,*Param;
    ScrExprScopeBuilder *Build;
	ScrTypeStructBuilder *VarBuild;
	ScrTypeBrowser *TypeTgt;
	ScrExprBrowser *ExprTgt;
	ScrTypeThrou *VarEnv;
	int AnonymousVar;
	int Lvl;
	struct xExprStruct *Local;
};
struct xExprStruct {
	ScopeThrou ScopeThrou;
	tExprStruct *Base; // can be many levels below.
	ScrCatalog *Local;
	ScrCatalog *Var;
};

typedef struct { 
	// For those case where result and params share symbols. 
	// Result is strictly an lvalue while param is strictly an rvalue
	ScrDataAccess ScrDataAccess;
	tExprStruct *Expr;
	ScrDataAccess *R,*P;
} ExprPorRData;

typedef struct {
	ScrDataAccess ScrDataAccess;
	tExprStruct *Expr;
	int Depth,Num;
} tExprStructData;
typedef struct {
	ScrDataAccess ScrDataAccess;
	xExprStruct *Ctx;
	ScrType *Type;
	int Depth,Num;
} xExprStructData;

   /*-------*/

static ScrType *ExprStructGetFieldType(tExprStruct *this,int Num) {
	char *dummy;
	return Call(this->Result,Struct.Elt,2(&dummy,Num));
}
static ScrType *ExprStructGetParamType(tExprStruct *this,int Num) {
	char *dummy;
	return Call(this->Param,Struct.Elt,2(&dummy,Num));
}
static ScrType *ExprStructGetPrivateType(tExprStruct *this,int Num) {
	return Call(this->Build,GetPrivate,2(&Num,0));
}

    /*------ Field/Result ------*/

static ScrType *ExprStructFieldGetType(ScrDataAccess *this) {
	char *dummy;
	ThisToThat(tExprStructData,ScrDataAccess);
	return Call(that->Expr->Result,Struct.Elt,2(&dummy,that->Num));
}
static ScrInstruction *ExprStructFieldSetValue(ScrDataAccess *this,ScrDataExpr *Val) {
	ThisToThat(tExprStructData,ScrDataAccess);
	Call(that->Expr->Build,SetField,2(that->Num,Val));
	return &ScrInstructionNull;
}
static ScrDataExpr *ExprStructFieldGetValue(ScrDataAccess *this) {
	// shouldn't happen since result is a lvalue
	return &ScrDataExprNull;
}
static ScrDataTgt *ExprStructFieldGetVar(ScrDataAccess *this) {
	// Todo; but for functionals, the result isn't stored in a variable
	return &ScrDataTgtNull;
}
static ScrDataAccess *ExprStructField(tExprStruct *t,int n) {
	tExprStructData *r;
	static struct ScrDataAccess Static= {
		ExprStructFieldGetType,ExprStructFieldSetValue,ExprStructFieldGetValue,ExprStructFieldGetVar
	};
	rPush(r);
	r->ScrDataAccess.Static = &Static;
	r->Expr = t; r->Depth = 0; r->Num = n;
	return &r->ScrDataAccess;
}

    /*------ Params ------*/

static ScrType *ExprStructParamGetType(ScrDataAccess *this) {
	ThisToThat(tExprStructData,ScrDataAccess);
	return ExprStructGetParamType(that->Expr,that->Num);
}
static ScrInstruction *ExprStructParamSetValue(ScrDataAccess *this,ScrDataExpr *Val) {
	return &ScrInstructionNull;
}
static ScrDataExpr *ExprStructParamGetValue(ScrDataAccess *this) {
	ScrDataExpr *ep;
	ThisToThat(tExprStructData,ScrDataAccess);
	ep = Call(that->Expr->ExprTgt,Param,2(that->Expr->Param,that->Depth));
    return Call(that->Expr->ExprTgt,FieldSelect,2(ep,that->Num));
}
static ScrDataTgt *ExprStructParamGetVar(ScrDataAccess *this) {
	return &ScrDataTgtNull;
}
static ScrDataAccess *ExprStructParam(tExprStruct *t,int Depth,int idx) {
	tExprStructData *r;
	static struct ScrDataAccess Static = {
		ExprStructParamGetType,ExprStructParamSetValue,ExprStructParamGetValue,ExprStructParamGetVar
	};
	rPush(r); r->ScrDataAccess.Static = &Static;
	r->Expr = t; r->Depth = Depth; r->Num = idx;
	return &r->ScrDataAccess;
}

    /*------ Params/result -------*/

static ScrType *PorRGetType(ScrDataAccess *this) {
	// We have a problem here, is it the type of the result or that of the param?
	// For now, we expect that whenever result and param share a label, the associated
	// symbols have to be of the same type.
	ThisToThat(ExprPorRData,ScrDataAccess);
	return Call(that->R,GetType,0);
}
static ScrInstruction *PorRSetValue(ScrDataAccess *this,ScrDataExpr *Val) {
	ThisToThat(ExprPorRData,ScrDataAccess);
	return Call(that->R,SetValue,1(Val));
}
static ScrDataExpr *PorRGetValue(ScrDataAccess *this) {
	ThisToThat(ExprPorRData,ScrDataAccess);
	return Call(that->P,GetValue,0);
}
static ScrDataTgt *PorRGetVar(ScrDataAccess *this) { 
	ThisToThat(ExprPorRData,ScrDataAccess);
	return Call(that->R,GetVar,0);
}
static ScrDataAccess *ExprStructPorR(ScrDataAccess *R,ScrDataAccess *P) {
	ExprPorRData *r;
	static struct ScrDataAccess Static = { PorRGetType,PorRSetValue,PorRGetValue,PorRGetVar};
    rPush(r); r->ScrDataAccess.Static = &Static;
	r->R = R; r->P = P;
	return &r->ScrDataAccess;
}

    /*------ Privates/Locals ------*/
    /*---- extended struct/fn scope ----*/

         /*---- in extended expression context, all data are private ----*/

static ScrType *xStructDataGetType(ScrDataAccess *this) {
	ThisToThat(xExprStructData,ScrDataAccess);
    return that->Type;
}
static ScrInstruction *xStructDataSetValue(ScrDataAccess *this,ScrDataExpr *Val) {
	ThisToThat(xExprStructData,ScrDataAccess);
	Call(that->Ctx->Base->Build,SetPrivate,2(that->Num,Val)); 
	return &ScrInstructionNull;
}
static ScrDataExpr *xStructDataGetValue(ScrDataAccess *this) {
	ThisToThat(xExprStructData,ScrDataAccess);
	return Call(&that->Ctx->Base->ScopeThrou,Expr,3(that->Type,that->Depth,that->Num));
}
static ScrDataTgt *xStructDataGetVar(ScrDataAccess *this) { return &ScrDataTgtNull; }

static xExprStructData *xStructPrivateData(xExprStruct *Ctx,ScrType *Type,int Depth,int Num) {
	xExprStructData *r;
	static struct ScrDataAccess Static = { xStructDataGetType,xStructDataSetValue,xStructDataGetValue,xStructDataGetVar };
	rPush(r);
	r->ScrDataAccess.Static = &Static;
	r->Type = Type; r->Ctx = Ctx; r->Depth = Depth; r->Num = Num;
	return r;
}

/*__________________________
 |
 | Variables
 |__________________________
*/

static ScrInstruction *VarResultSetValue(ScrDataAccess *this,ScrDataExpr *Val) { 
	ScrDataTgt *Tgt;
	ThisToThat(tExprStructData,ScrDataAccess);
	Tgt = Call(this,GetVar,0);
	return Call(that->Expr->ExprTgt,Instr.Set,2(Tgt,Val));
}
static ScrDataExpr *VarResultGetValue(ScrDataAccess *this) { return &ScrDataExprNull; }
static ScrDataTgt *VarResultGetVar(ScrDataAccess *this) {
	ThisToThat(tExprStructData,ScrDataAccess);
    return Call(that->Expr->ExprTgt,Var.Result,3(that->Depth,that->Expr->Result,that->Num));
}
static ScrDataAccess *VarResult(tExprStruct *t,int Depth,int idx) {
	tExprStructData *r;
	static struct ScrDataAccess Static = {
		ExprStructFieldGetType,VarResultSetValue,VarResultGetValue,VarResultGetVar
	};
	rPush(r); r->ScrDataAccess.Static = &Static;
	r->Expr = t; r->Depth = Depth; r->Num = idx;
	return &r->ScrDataAccess;
}
static ScrDataAccess *ShallowVarResult(tExprStruct *t,int idx) { return VarResult(t,0,idx); }

   /*-----------*/

static ScrInstruction *VarLocalSetValue(ScrDataAccess *this,ScrDataExpr *Val) {
	ScrDataTgt *Tgt;
	ThisToThat(xExprStructData,ScrDataAccess);
	Tgt = Call(this,GetVar,0);
	return Call(that->Ctx->Base->ExprTgt,Instr.Set,2(Tgt,Val));
}
static ScrDataExpr *VarLocalGetValue(ScrDataAccess *this) {
	ScrDataTgt *Var;
    ThisToThat(xExprStructData,ScrDataAccess);
	Var = Call(this,GetVar,0);
	return Call(that->Ctx->Base->ExprTgt,Variable,1(Var));
}
static ScrDataTgt *VarLocalGetVar(ScrDataAccess *this) {
	ThisToThat(xExprStructData,ScrDataAccess);
	return Call(&that->Ctx->Base->ScopeThrou,Tgt,3(that->Type,that->Depth,that->Num));
}
static xExprStructData *VarLocal(xExprStruct *t,ScrType *Type,int Depth,int idx) {
	xExprStructData *r;
	static struct ScrDataAccess Static = {
		xStructDataGetType,VarLocalSetValue,VarLocalGetValue,VarLocalGetVar
	};
	rPush(r); r->ScrDataAccess.Static = &Static;
    r->Type = Type; r->Ctx = t; r->Depth = Depth; r->Num = idx;
	return r;
}

        /*----- -----*/

static xExprStruct *ExtendedExprStruct(ScopeThrou *parent,tExprStruct *base);
static tExprStruct *ComplexExpr(int Lvl,ScopeThrou *Parent,ScrType *Result,ScrType *Param);
static tExprStruct *ProcedureScope(int Lvl,ScopeThrou *Parent,ScrType *Result,ScrType *Param);

static ScrDataExpr *xExprStructExpr(ScopeThrou *this,ScrType *Type,int Depth,int num) {
	ThisToThat(xExprStruct,ScopeThrou);
	return Call(&that->Base->ScopeThrou,Expr,3(Type,Depth,num));
}
static ScrDataAccess *xExprStructLexicalData(ScopeThrou *this,int OrgDepth,char *Label) {
	ScrDataAccess *r;
	int nf;
	ThisToThat(xExprStruct,ScopeThrou);
	nf = Call(that->Local,GetNum,1(Label));
	if (nf>=0) {
		xExprStructData *data;
        data = Call(that->Local,GetData,1(nf)).Data;
        if (OrgDepth!=that->Base->Lvl) {
		    data = xStructPrivateData(data->Ctx,data->Type,OrgDepth-that->Base->Lvl,data->Num);
		}
		r = &data->ScrDataAccess;
	} else {
		nf = Call(that->Var,GetNum,1(Label));
		if (nf>=0) {
			xExprStructData *data;
			data = Call(that->Var,GetData,1(nf)).Data;
			if (OrgDepth!=that->Base->Lvl) {
				data = VarLocal(data->Ctx,data->Type,OrgDepth-that->Base->Lvl,data->Num);
			}
			r = &data->ScrDataAccess;
		} else {
		    r = Call(this->Child,LexicalData,2(OrgDepth,Label));
		}
	}
	return r;
}
static ScrTypeBrowser *xScopeTypeTgt(ScrScope *this) {
	ThisToThat(xExprStruct,ScopeThrou.ScrScope);
	return that->Base->TypeTgt;
}
static ScrExprBrowser *xScopeExprTgt(ScrScope *this) {
	ThisToThat(xExprStruct,ScopeThrou.ScrScope);
	return that->Base->ExprTgt;
}

static ScrDataAccess *ProcedureAddLocalVar(ScrScope *this,char *Public,char *Label,ScrType *Type) {
	int r,idx;
	ScrDataAccess *R;
	ThisToThat(xExprStruct,ScopeThrou.ScrScope);
	r = Call(that->Var,SetLabel,2(-1,Label));
	r = Call(that->Var,GetNum,1(Label));
	if (r>=0) {
		Call(that->Base->VarBuild,AddField,3(0,Public,Type));
		idx = Call(&that->Base->VarEnv->ScrType,Struct.EltNum,1(Public));
		R = &VarLocal(that,Type,0,idx)->ScrDataAccess;
	}
	return R;
}
static ScrDataAccess *xProcedureAddLocalVar(ScrScope *this,char *Label,ScrType *Type) {
	ScrVocabulary *voc;
	char *Public;
	ThisToThat(xExprStruct,ScopeThrou.ScrScope);
	voc = Call(this,GetVocabulary,0);
	Public = Call(voc,InternalLabel,1(that->Base->AnonymousVar));
	that->Base->AnonymousVar++;
	if (!Label) { Label = Public; } else { if (*Label==0) {Label=Public;}}
	return ProcedureAddLocalVar(this,Public,Label,Type);
}
static ScrDataAccess *rootProcedureAddLocalVar(ScrScope *this,char *Label,ScrType *Type) {
	ScrDataAccess *r;
	if (Label) {
		if (*Label) {
	        r = ProcedureAddLocalVar(this,Label,Label,Type);
		} else {
            r = xProcedureAddLocalVar(this,Label,Type);
		}
	} else {
        r = xProcedureAddLocalVar(this,Label,Type);
	}
	return r;
}

static int ExprStructAddLocalData(ScrScope *this,char *Public,char *Label,ScrType *Type) {
    int r,idx;
	ThisToThat(xExprStruct,ScopeThrou.ScrScope);
	r = Call(that->Local,SetLabel,2(-1,Label));
	r = Call(that->Local,GetNum,1(Label));
	if (r>=0) {
        ScrCatalogEntry Data;
		idx = Call(that->Base->Build,AddPrivate,2(Type,Public));
		Data.Data = xStructPrivateData(that,Type,0,idx);
		Call(that->Local,SetData,2(r,Data));
	}
	return (r>=0);
}
static int xExprStructAddData(ScrScope *this,char *Label,ScrType *Type) {
	return ExprStructAddLocalData(this,0,Label,Type);
}
static int rootExprStructAddData(ScrScope *this,char *Label,ScrType *Type) {
	return ExprStructAddLocalData(this,Label,Label,Type);
}
static ScrDataAccess *xExprStructGetData(ScrScope *this,char *Label) {
	ThisToThat(xExprStruct,ScopeThrou.ScrScope);
	return Call(&that->ScopeThrou,LexicalData,2(that->Base->Lvl,Label));
}
static ScrDataAccess *xExprStructAddPrivate(ScrScope *this,ScrType *Type) {
	xExprStructData *data;
	int idx;
	ThisToThat(xExprStruct,ScopeThrou.ScrScope);
    idx = Call(that->Base->Build,AddPrivate,2(Type,0));
	data = xStructPrivateData(that,Type,0,idx);
    return &data->ScrDataAccess;
}
static ScrScope *xExprStructExtendOpen(ScrScope *this) {
	int Elts;
	xExprStruct *r;
	ThisToThat(xExprStruct,ScopeThrou.ScrScope);
	Elts = Call(that->Local,EltNb,0);
	r = ExtendedExprStruct(&that->ScopeThrou,that->Base);
    return  &r->ScopeThrou.ScrScope;
}
static ScrScope *xExprStructExprStructOpen(ScrScope *this,ScrType *Type) {
	tExprStruct *ts;
	ThisToThat(xExprStruct,ScopeThrou.ScrScope);
	ts = ComplexExpr(that->Base->Lvl+1,&that->ScopeThrou,Type,0);
	return &ts->ScopeThrou.ScrScope;
}
static ScrScope *xExprStructExprFnOpen(ScrScope *this,ScrType *Type) {
	tExprStruct *ts;
	ScrType *Param;
	ThisToThat(xExprStruct,ScopeThrou.ScrScope);
	Param = Call(Type,Fn.Param,0);
	ts = ComplexExpr(that->Base->Lvl+1,&that->ScopeThrou,Type,Param);
	return &ts->ScopeThrou.ScrScope;
}
static ScrScope *xExprStructProcOpen(ScrScope *this,ScrType *Type) {
	tExprStruct *ts;
	ScrType *Param,*Result;
	ThisToThat(xExprStruct,ScopeThrou.ScrScope);
	Param = Call(Type,Fn.Param,0);
	Result = Call(Type,Fn.Result,0);
	ts = ProcedureScope(that->Base->Lvl+1,&that->ScopeThrou,Result,Param);
	return &ts->ScopeThrou.ScrScope;
}
static ScrDataExpr *xExprStructClose(ScrScope *this,ScrInstruction *Block) {
	ThisToThat(xExprStruct,ScopeThrou.ScrScope);
	Call(that->Var,Close,0);
	Call(that->Local,Close,0);
	return &ScrDataExprNull;
}

static xExprStruct *rorxExprStruct(struct ScrScope *Static,ScopeThrou *parent,tExprStruct *base) {
	xExprStruct *r;
	static struct ScrScopeThrou StaticThrou = { xExprStructExpr,ScopeThrouTgt,xExprStructLexicalData };
	rPush(r);
	r->ScopeThrou.Static = &StaticThrou;
	r->ScopeThrou.ScrScope.Static = Static;
	r->ScopeThrou.Child = parent;
	r->Base = base;
	r->Local = ScrCatalogNew(&ScrDataAccessNull);
	r->Var = ScrCatalogNew(&ScrDataAccessNull);
	return r;
}
static xExprStruct *ExtendedExprStruct(ScopeThrou *parent,tExprStruct *base) {
	static struct ScrScope Static = {
		ScopeThrouGetVocabulary, xScopeTypeTgt, xScopeExprTgt,
		ScopeThrouAddType, ScopeThrouSetType, ScopeThrouGetType,
		xExprStructAddData, xProcedureAddLocalVar, xExprStructGetData, xExprStructAddPrivate,
		ScopeThrouStructTypedefOpen, xExprStructExtendOpen, 
		xExprStructExprStructOpen, xExprStructExprFnOpen, xExprStructProcOpen,
		xExprStructClose, ScopeThrouTypeClose
	};
	return rorxExprStruct(&Static,parent,base);
}
static xExprStruct *RootExprStruct(ScopeThrou *parent,tExprStruct *base) {
	xExprStruct *r;
	static struct ScrScope Static = {
		ScopeThrouGetVocabulary, xScopeTypeTgt, xScopeExprTgt,
		ScopeThrouAddType, ScopeThrouSetType, ScopeThrouGetType,
		rootExprStructAddData, rootProcedureAddLocalVar, xExprStructGetData, xExprStructAddPrivate,
		ScopeThrouStructTypedefOpen, xExprStructExtendOpen, 
		xExprStructExprStructOpen, xExprStructExprFnOpen, xExprStructProcOpen,
		xExprStructClose, ScopeThrouTypeClose
	};
    r =	rorxExprStruct(&Static,parent,base);
	return r;
}

    /*---- struct/fn scope---*/

static ScrDataExpr *ExprStructExpr(ScopeThrou *this,ScrType *Type,int depth,int num) {
	ThisToThat(tExprStruct,ScopeThrou);
	return Call(that->ExprTgt,Local,3(Type,depth,num));
}
static ScrDataTgt *ExprStructTgt(ScopeThrou *this,ScrType *Type,int depth,int num) {
	ThisToThat(tExprStruct,ScopeThrou);
	return Call(that->ExprTgt,Var.Local,3(depth,Type,num));
}
static ScrDataAccess *ExprStructLexicalData(ScopeThrou *this,int OrgDepth,char *Label) {
	ScrDataAccess *r;
	int nf;
	ThisToThat(tExprStruct,ScopeThrou);
	nf = Call(that->Local->Local,GetNum,1(Label));
	if (nf<0) { nf = Call(that->Local->Var,GetNum,1(Label)); }
	if (nf>=0) {
		r = Call(&that->Local->ScopeThrou,LexicalData,2(OrgDepth,Label));
	} else {
		int rn,pn;
		rn = (OrgDepth!=that->Lvl) ? -1 : Call(that->Result,Struct.EltNum,1(Label));
		pn = Call(that->Param,Struct.EltNum,1(Label));
	    if ((rn>=0)||(pn>=0)) {
			if ((rn>=0)&&(pn>=0)) {
				ScrDataAccess *R,*P;
				R = that->ResultAccess(that,rn); P = ExprStructParam(that,OrgDepth-that->Lvl,pn);
				r = ExprStructPorR(R,P);
			} else {
				if (rn>=0) {
					ScrDataAccess *R,*P;
					R = that->ResultAccess(that,rn); P = Call(this->Child,LexicalData,2(OrgDepth,Label));
					r = ExprStructPorR(R,P);
				} else { 
					r = ExprStructParam(that,OrgDepth-that->Lvl,pn); 
				}
			}
	    } else {
			r = Call(this->Child,LexicalData,2(OrgDepth,Label));
	    }
	}
	return r;
}
static ScrTypeBrowser *ExprStructTypeTgt(ScrScope *this) {
	ThisToThat(tExprStruct,ScopeThrou.ScrScope);
	return that->TypeTgt;
}
static ScrExprBrowser *ExprStructExprTgt(ScrScope *this) {
	ThisToThat(tExprStruct,ScopeThrou.ScrScope);
	return that->ExprTgt;
}
static int ExprStructAddData(ScrScope *this,char *Label,ScrType *Type) {
	ThisToThat(tExprStruct,ScopeThrou.ScrScope);
	return Call(&that->Local->ScopeThrou.ScrScope,AddData,2(Label,Type));
}
static ScrDataAccess *ExprStructAddVar(ScrScope *this,char *Label,ScrType *Type) {
	ThisToThat(tExprStruct,ScopeThrou.ScrScope);
	return Call(&that->Local->ScopeThrou.ScrScope,AddVar,2(Label,Type));
}
static ScrDataAccess *ExprStructGetData(ScrScope *this,char *Label) {
	ThisToThat(tExprStruct,ScopeThrou.ScrScope);
	return Call(&that->ScopeThrou,LexicalData,2(that->Lvl,Label));
}
static ScrDataAccess *ExprStructAddPrivate(ScrScope *this,ScrType *Type){
	ThisToThat(tExprStruct,ScopeThrou.ScrScope);
	return Call(&that->Local->ScopeThrou.ScrScope,AddPrivate,1(Type));
}
static ScrScope *ExprStructExtendOpen(ScrScope *this) {
	ThisToThat(tExprStruct,ScopeThrou.ScrScope);
	return Call(&that->Local->ScopeThrou.ScrScope,ExtendOpen,0);
}
static ScrScope *ExprStructExprStructOpen(ScrScope *this,ScrType *Type) {
	ThisToThat(tExprStruct,ScopeThrou.ScrScope);
	return Call(&that->Local->ScopeThrou.ScrScope,ExprStructOpen,1(Type));
}
static ScrScope *ExprStructExprFnOpen(ScrScope *this,ScrType *Proto) {
	ThisToThat(tExprStruct,ScopeThrou.ScrScope);
	return Call(&that->Local->ScopeThrou.ScrScope,ExprFnOpen,1(Proto));
}
static ScrScope *ExprStructProcOpen(ScrScope *this,ScrType *Proto) {
	ThisToThat(tExprStruct,ScopeThrou.ScrScope);
	return Call(&that->Local->ScopeThrou.ScrScope,ProcedureOpen,1(Proto));
}
static ScrDataExpr *ExprStructClose(ScrScope *this,ScrInstruction *Block) {
	ThisToThat(tExprStruct,ScopeThrou.ScrScope);
	Call(&that->Local->ScopeThrou.ScrScope,Close,1(&ScrInstructionNull));
	that->VarEnv->Child = Call(that->VarBuild,Close,2(that->TypeTgt,""));
	return Call(that->Build,Close,1(that->ExprTgt));
}
static ScrDataExpr *ExprProcClose(ScrScope *this,ScrInstruction *Block) {
	ScrType *Local,*Prototype;
	ThisToThat(tExprStruct,ScopeThrou.ScrScope);
    Call(&that->Local->ScopeThrou.ScrScope,Close,1(&ScrInstructionNull));
	that->VarEnv->Child = Local = Call(that->VarBuild,Close,2(that->TypeTgt,""));
	Call(that->Build,Close,1(that->ExprTgt));
	Prototype = Call(that->TypeTgt,Fn,2(that->Result,that->Param));
	return  Call(that->ExprTgt,Var.Procedure,4(that->Lvl,Prototype,Local,Block));
}
static tExprStruct *FuncScope(
		struct ScrScope *Static, int Lvl,ScopeThrou *Parent,ScrType *Result,ScrType *Param,
		ScrDataAccess *(*ResultAccess)(tExprStruct *that,int idx)
) {
	tExprStruct *r;
	static struct ScrScopeThrou ThrouStatic = { ExprStructExpr, ExprStructTgt, ExprStructLexicalData};
	rPush(r);
	r->ScopeThrou.Static = &ThrouStatic;
	r->ScopeThrou.ScrScope.Static = Static;
	r->ScopeThrou.Child = Parent;
    r->TypeTgt = Call(&Parent->ScrScope,TypeTgt,0);
	r->ExprTgt = Call(&Parent->ScrScope,ExprTgt,0);
	r->Lvl = Lvl;
	if (Param) {
        r->Result = Call(Result,Fn.Result,0); r->Param = Param;
	    r->Build = ScrExprFunctionBuild(Result,Lvl);
	} else {
        r->Result = Result; r->Param = &ScrTypeNull;
	    r->Build = ScrExprStructBuild(Result,Lvl);
	}
	r->ResultAccess = ResultAccess;
	r->VarBuild = ScrTypeStructNew();
	r->VarEnv = Call(r->TypeTgt,Throu,0);
	r->VarEnv->Child = Call(r->VarBuild,Partial,0);
	r->AnonymousVar = 0;
	r->Local = RootExprStruct(&r->ScopeThrou,r);
	return r;
}

static tExprStruct *ComplexExpr(int Lvl,ScopeThrou *Parent,ScrType *Result,ScrType *Param) {
	static struct ScrScope Static = {
        ScopeThrouGetVocabulary, ExprStructTypeTgt, ExprStructExprTgt,
		ScopeThrouAddType, ScopeThrouSetType, ScopeThrouGetType,
		ExprStructAddData, ExprStructAddVar, ExprStructGetData, ExprStructAddPrivate,
		ScopeThrouStructTypedefOpen, ExprStructExtendOpen,
		ExprStructExprStructOpen, ExprStructExprFnOpen, ExprStructProcOpen,
		ExprStructClose, ScopeThrouTypeClose
	};
	return FuncScope(&Static,Lvl,Parent,Result,Param,ExprStructField);
}
static tExprStruct *ProcedureScope(int Lvl,ScopeThrou *Parent,ScrType *Result,ScrType *Param) {
	static struct ScrScope Static = {
        ScopeThrouGetVocabulary, ExprStructTypeTgt, ExprStructExprTgt,
		ScopeThrouAddType, ScopeThrouSetType, ScopeThrouGetType,
		ExprStructAddData, ExprStructAddVar, ExprStructGetData, ExprStructAddPrivate,
		ScopeThrouStructTypedefOpen, ExprStructExtendOpen,
		ExprStructExprStructOpen, ExprStructExprFnOpen, ExprStructProcOpen,
		ExprProcClose, ScopeThrouTypeClose
	};
	return FuncScope(&Static,Lvl,Parent,Result,Param,ShallowVarResult);
}


/*__________________________
 |
 | Global
 |__________________________
*/

/*_____________________
 |
 | Module data
 |_____________________
*/

typedef struct {
	PatriciaNod/*<ModuleType.Label>*/ Label;
	ScrTypeThrou *Type;
} ModuleType;

typedef struct {
	tExprStruct tExprStruct;
	ScrVocabulary *Voc;
	ScrDataExpr *Closed;
	ScrType *GlobalEnv;
	int Idx;
	int TypeNb;
	PatriciaNod/*<ModuleType.Label>*/ Types;
} ScopeModule;

static int ScopeModuleGetDataId(ScopeModule *this,char *Label) {
	int r;
	Call(this->tExprStruct.Build,GetPrivate,2(&r,Label));
	return r;
}
static ScrDataExpr *GlobalModuleExpr(ScopeThrou *this,ScrType *Type,int Depth,int num) {
	ThisToThat(ScopeModule,tExprStruct.ScopeThrou.ScrScope);
	return Call(that->tExprStruct.ExprTgt,Global,3(Type,that->Idx,num));
}
static ScrDataTgt *GlobalModuleTgt(ScopeThrou *this,ScrType *Type,int Depth,int num) {
	ThisToThat(ScopeModule,tExprStruct.ScopeThrou.ScrScope);
	return Call(that->tExprStruct.ExprTgt,Var.Extern,3(Type,that->Idx,num));
}

static ScrVocabulary *ScopeModuleGetVocabulary(ScrScope *this) {
	ThisToThat(ScopeModule,tExprStruct.ScopeThrou.ScrScope);
	return that->Voc;
}
static int ScopeModuleAddType(ScrScope *this,char *Label) {
	int r;
	PatriciaNod *p;
	ThisToThat(ScopeModule,tExprStruct.ScopeThrou.ScrScope);
	p = PatriciaSeek(&that->Types,Label);
	r = (p==&that->Types);
	if (r) {
		ModuleType *n;
		rPush(n);
		n->Type = Call(that->tExprStruct.TypeTgt,Throu,0);
		PatriciaSeekOrInsert(&that->Types,Label,&n->Label);
		that->TypeNb++;
	}
	return r;
}
static int ScopeModuleSetType(ScrScope *this,char *Label,ScrType *Type) {
	int r;
	PatriciaNod *p;
	ThisToThat(ScopeModule,tExprStruct.ScopeThrou.ScrScope);
	p = PatriciaSeek(&that->Types,Label);
	r = (p!=&that->Types);
    if (r) {
		ModuleType *n;
		n = CastBack(ModuleType,Label,p);
        n->Type->Child = Type;
	} else {
		r = Call(&that->tExprStruct.ScopeThrou.Child->ScrScope,SetType,2(Label,Type));
	}
	return r;
}
static ScrType *ScopeModuleGetType(ScrScope *this,char *Label) {
	ScrType *r;
	PatriciaNod *p;
	ThisToThat(ScopeModule,tExprStruct.ScopeThrou.ScrScope);
    p = PatriciaSeek(&that->Types,Label);
	if (p==&that->Types) {
		r = Call(&that->tExprStruct.ScopeThrou.Child->ScrScope,GetType,1(Label));
	} else {
		ModuleType *n;
		n = CastBack(ModuleType,Label,p);
		r = n->Type->Child;
		if (r==&ScrTypeNull) { r = &n->Type->ScrType; }
	}
	return r;
}
static ScrScope *GlobalStructTypedefOpen(ScrScope *this) {
    ThisToThat(ScopeThrou,ScrScope);
    return &ScrScopeTypeStruct(that,ScrTypeStructNew())->ScrScope;
}
static ScrDataExpr *GlobalExprFnClose(ScrScope *this,ScrInstruction *Block) { 
	ThisToThat(ScopeModule,tExprStruct.ScopeThrou.ScrScope);
	Call(&that->tExprStruct.Local->ScopeThrou.ScrScope,Close,1(Block));
	that->Closed = Call(that->tExprStruct.Build,Close,1(that->tExprStruct.ExprTgt));
	that->GlobalEnv = Call(that->tExprStruct.VarBuild,Close,2(that->tExprStruct.TypeTgt,""));
	return that->Closed; 
}
static ScopeModule *ScrScopeModule(int Idx,ScrVocabulary *Voc,ScopeThrou *Parent) {
	ScopeModule *r;
	ScrTypeThrou *Result;
	ScrTypeBrowser *TypeTgt;
	ScrExprBrowser *ExprTgt;
	static struct ScrScopeThrou ThrouStatic = { GlobalModuleExpr, GlobalModuleTgt, ExprStructLexicalData};
	static struct ScrScope Static = {
		ScopeModuleGetVocabulary, ExprStructTypeTgt, ExprStructExprTgt,
		ScopeModuleAddType,ScopeModuleSetType,ScopeModuleGetType,
		ExprStructAddData,ExprStructAddVar,ExprStructGetData,ExprStructAddPrivate,
		GlobalStructTypedefOpen,ExprStructExtendOpen,
		ExprStructExprStructOpen,ExprStructExprFnOpen,ExprStructProcOpen,
		GlobalExprFnClose, ScopeNullTypeClose
	};
	TypeTgt = ScrRawType();
	ExprTgt = ScrRawExpr();
	rPush(r);
	r->tExprStruct.ScopeThrou.ScrScope.Static = &Static;
	r->tExprStruct.ScopeThrou.Static = &ThrouStatic;
	r->tExprStruct.Lvl = 0;
	r->tExprStruct.ScopeThrou.Child = Parent;
	r->tExprStruct.TypeTgt = TypeTgt;
	r->tExprStruct.ExprTgt = ExprTgt;
	Result = Call(TypeTgt,Throu,0);
	r->tExprStruct.Param = Call(TypeTgt,Atom,1(ScbVoid));
    r->tExprStruct.Result = &Result->ScrType;
	r->tExprStruct.Build = ScrHiddenExprScopeBuilder(Idx,TypeTgt,ExprTgt,Result,Voc);
	r->tExprStruct.Local = RootExprStruct(Parent,&r->tExprStruct);
	r->tExprStruct.ResultAccess = ExprStructField; // Should be a function that return access null instead.
	r->tExprStruct.VarBuild = ScrTypeStructNew();
	r->tExprStruct.VarEnv = Call(TypeTgt,Throu,0);
	r->tExprStruct.VarEnv->Child = Call(r->tExprStruct.VarBuild,Partial,0);
	r->GlobalEnv = &r->tExprStruct.VarEnv->ScrType;
	r->Closed = &ScrDataExprNull;
	r->Voc = Voc;
	r->TypeNb = 0;
	r->Idx = Idx;
	PatriciaInit(&r->Types);
	return r;
}

    /*------------------*/

typedef struct {
	ScrDataAccess ScrDataAccess;
	ScrExprBrowser *ExprTgt;
	ScrType *Type;
	int Module,Id;
} tExternalAccess;

static ScrType *XtrnAccessGetType(ScrDataAccess *this) {
	ThisToThat(tExternalAccess,ScrDataAccess);
	return that->Type;
}
static ScrInstruction *XtrnAccessSetValue(ScrDataAccess *this,ScrDataExpr *Val) {
     // Not allowed for now. You should use the accessors of the module.
	 return &ScrInstructionNull;
}
static ScrDataExpr *XtrnAccessGetValue(ScrDataAccess *this) {
	ThisToThat(tExternalAccess,ScrDataAccess);
	return Call(that->ExprTgt,Global,3(that->Type,that->Module,that->Id));
}
static ScrDataTgt *XtrnAccessGetVar(ScrDataAccess *this) {
	ThisToThat(tExternalAccess,ScrDataAccess);
	return Call(that->ExprTgt,Var.Extern,3(that->Type,that->Module,that->Id));
}
static ScrDataAccess *ExternalDataAccess(ScrExprBrowser *ExprTgt,ScrType *Type,int Module,int Id) {
	tExternalAccess *r;
	static struct ScrDataAccess Static = {XtrnAccessGetType,XtrnAccessSetValue,XtrnAccessGetValue,XtrnAccessGetVar};
    rPush(r);
	r->ScrDataAccess.Static = &Static;
	r->Type = Type; r->Module = Module; r->Id = Id; r->ExprTgt = ExprTgt;
	return &r->ScrDataAccess;
}

/*______________________________
 |
 |
 |______________________________
*/

typedef struct {
	ScopeThrou ScopeThrou;
	PatriciaNod/*<LinkedModule.Label>*/ Label;
	BinTree/*<LinkedModule.Num>*/ Num;
	ScrVocabulary *Voc;
	int Busy;
} LinkScope;
typedef struct {
	MemStack *Mem;
	PatriciaNod/*<LinkedModule.Label>*/ Label;
	BinTree/*<LinkedModule.Num>*/ Num;
	ScrModuleDesc *Desc;
	char **Dependencies;
	int DepCount,Busy;
	ScopeModule *Module;
} LinkedModule; 

static ScrDataExpr *LinkScopeExpr(ScopeThrou *this,ScrType *Type,int Depth,int num) { return &ScrDataExprNull; }
static ScrDataTgt *LinkScopeTgt(ScopeThrou *this,ScrType *Type,int Depth,int num) { return &ScrDataTgtNull;}

struct rLSGetData { 
	char *Label;
    int Module,Id;
	ScrExprBrowser *ExprTgt;
	ScrType *Type;
};
static int rLSGetData(BinTree *B,void *Clos) {
	LinkedModule *b;
	struct rLSGetData *clos;
	b = CastBack(LinkedModule,Num,B);
	clos = Clos;
	clos->Module = B->Id;
	clos->Id = ScopeModuleGetDataId(b->Module,clos->Label);
	if (clos->Id>=0) {
	    ScrDataAccess *a;
        a = Call(&b->Module->tExprStruct.ScopeThrou,LexicalData,2(0,clos->Label));
		clos->Type = Call(a,GetType,0);
	    clos->ExprTgt = b->Module->tExprStruct.ExprTgt;
	}
	return (clos->Id>=0);
}
static ScrDataAccess *LinkScopeLexicalData(ScopeThrou *this,int OrgDepth,char *Label) {
	ScrDataAccess *R;
	ThisToThat(LinkScope,ScopeThrou.ScrScope);
	R = &ScrDataAccessNull;
	if (!that->Busy) {
	    struct rLSGetData r;
		that->Busy = (0==0);
	    r.Label = Label; r.Type = &ScrTypeNull; r.Id = -1; r.Module = -1;
		BinTreeForEach(&that->Num,rLSGetData,&r);
		if (r.Id>=0) {
			R = ExternalDataAccess(r.ExprTgt,r.Type,r.Module,r.Id); 
		}
		that->Busy = (0!=0);
	}
	return R;
}

static ScrVocabulary *LinkScopeGetVocabulary(ScrScope *this) {
	ThisToThat(LinkScope,ScopeThrou.ScrScope);
	return that->Voc;
}

struct rLSGetType { char *Label; ScrType *Found; };
static int rLSGetType(BinTree *B,void *Clos) {
	LinkedModule *b;
	struct rLSGetType *clos;
	b = CastBack(LinkedModule,Num,B);
	clos = Clos;
    clos->Found = Call(&b->Module->tExprStruct.ScopeThrou.ScrScope,GetType,1(clos->Label));
	return (clos->Found!=&ScrTypeNull);
}
static ScrType *LinkScopeGetType(ScrScope *this,char *Label) {
	struct rLSGetType r;
	ThisToThat(LinkScope,ScopeThrou.ScrScope);
	r.Found = &ScrTypeNull;
	if (!that->Busy) {
		that->Busy = (0==0);
		r.Label = Label;
		BinTreeForEach(&that->Num,rLSGetType,&r);
		that->Busy = (0!=0);
	}
	return r.Found;
}

static ScrDataAccess *LinkScopeGetData(ScrScope *this,char *Label) {
	ThisToThat(ScopeThrou,ScrScope);
	return Call(that,LexicalData,2(0,Label));
}

/*_____________________________
 |
 |
 |_____________________________
*/


typedef struct {
	ScrDataVal *Val;
} ItfModDesc;
typedef struct {
	List/*<LinkInstance.List>*/ List;
	int ModuleNb;
	ScrDataCtx **Modules;
	ItfModDesc *Desc;
} LinkInstance;

typedef struct {
	MemStack *Mem;
	ScrLinkScope ScrLinkScope;
	LinkScope LinkScope;
	List/*<LinkInstance.List>*/ Instance;
	int LinkNb;
	int LinkFork; // Hack
	ScrScope *Core;
} LinkScopeItf;

static LinkInstance *LinkScopeLastInstance(LinkScopeItf *this) {
	int LastSize;
	LinkInstance *Last;
	Last = CastBack(LinkInstance,List,this->Instance.n);
	if (Last->ModuleNb<this->LinkNb) {
		LinkInstance *New;
		int n;
		ScrDataCtx **qm,**qe;
		ItfModDesc *qv,*p,*pe;
		mPush(this->Mem,New);
		New->ModuleNb = this->LinkNb;
		mnPush(this->Mem,New->Modules,New->ModuleNb);
		mnPush(this->Mem,New->Desc,New->ModuleNb);
		qm = New->Modules; qe = qm+New->ModuleNb;
		while (qm<qe) { *qm++ = &ScrDataCtxNull; }
		qm = New->Modules; n = 0;
		while (qm<qe) { mIn(this->Mem,*qm++ = ScrCtxGlobal(New->Modules,n)); n++;}
		p = Last->Desc; pe = p+Last->ModuleNb; qv = New->Desc;
		while (p<pe) { *qv++ = *p++; }
		pe = New->Desc+New->ModuleNb; n = Last->ModuleNb;
		while (qv<pe) {
			qv->Val = &ScrDataValNull;
            qv++; n++;
		}
		New->List.n = &Last->List;
		this->Instance.n = &New->List;
		Last = New;
	}
    return Last;
}

static ScopeModule *LSGetModule(LinkScope *Root,char *Label);
static ScrDataVal *LSGetData(LinkScopeItf *Root,char *Label) {
	PatriciaNod *P;
	ScrDataVal *r;
	r = &ScrDataValNull;
	P = PatriciaSeek(&Root->LinkScope.Label,Label);
	if (P!=&Root->LinkScope.Label) {
	    ScopeModule *Mod;
        LinkInstance *Instance;
		LinkedModule *p;
		int num;
		p = CastBack(LinkedModule,Label,P);
	    Mod = LSGetModule(&Root->LinkScope,Label);
	    Instance = LinkScopeLastInstance(Root);
		num = p->Num.Id;
		r = Instance->Desc[num].Val;
		if (r==&ScrDataValNull) {
			LinkInstance *l;
			List *L;
			r = Call(Mod->Closed,Eval,1(Instance->Modules[num]));
			l = Instance; L = &Instance->List;
			while (l->ModuleNb>num) {
				l->Desc[num].Val = r;
				L = L->n;
				l = CastBack(LinkInstance,List,L);
			}
		}
	}
	return r;
}

static int LSReleaseModule(LinkScope *Root,char *Label);
static int LSReleaseData(LinkScopeItf *Root,char *Label) {
	int r;
	r = LSReleaseModule(&Root->LinkScope,Label);
	if (r) {
		PatriciaNod *P;
		LinkedModule *p;
		int num;
		LinkInstance *l;
		List *L;
	    P = PatriciaSeek(&Root->LinkScope.Label,Label);
		p = CastBack(LinkedModule,Label,P); num = p->Num.Id;
		l = LinkScopeLastInstance(Root); L = &l->List;
		while (l->ModuleNb>num) {
			l->Desc[num].Val = &ScrDataValNull;
			L = L->n;
			l = CastBack(LinkInstance,List,L);
		}
	}
	return r;
}

static ScopeModule *LSGetModule(LinkScope *Root,char *Label) {
	PatriciaNod *P;
	ScopeModule *r;
	r = 0;
	P = PatriciaSeek(&Root->Label,Label);
	if (P!=&Root->Label) {
		LinkedModule *p;
		p = CastBack(LinkedModule,Label,P);
		if (!p->Busy) {
		    char **d;
			p->Busy = (0==0);
			if (!p->DepCount) {
			    p->DepCount++;
				mEnter(p->Mem);
				mIn(p->Mem,p->Module = ScrScopeModule(p->Num.Id,Root->Voc,&Root->ScopeThrou));
			    d = p->Dependencies;
			    while (*d) { LSGetModule(Root,*d); d++; }
				mIn(p->Mem,Call(p->Desc,Open,1(&p->Module->tExprStruct.ScopeThrou.ScrScope)));
			} else {
				p->DepCount++;
			}
			p->Busy = (0!=0);
		}
		r = p->Module;// ->tExprStruct.ScopeThrou.ScrScope;
	}
	return r;
}

static int LSReleaseModule(LinkScope *Root,char *Label) {
	PatriciaNod *P;
    int r;
	P = PatriciaSeek(&Root->Label,Label);
	r = (0!=0);
	if (P!=&Root->Label) {
		LinkedModule *p;
		p = CastBack(LinkedModule,Label,P);
		if (!p->Busy) {
			p->Busy = (0==0);
			p->DepCount--;
			r = (!p->DepCount);
			if (r) {
				char **d;
				Call(p->Desc,Close,1(&p->Module->tExprStruct.ScopeThrou.ScrScope));
				p->Module->Closed = &ScrDataExprNull;
				p->Module = 0;
				mLeave(p->Mem);
                d = p->Dependencies;
				while (*d) { LSReleaseModule(Root,*d); d++;}
			}
			p->Busy = (0!=0);
		}
	}
	return r;
}

static int ItfInsertModule(ScrLinkScope *this,char *Label,ScrModuleDesc *Desc,char **Dependencies) {
	PatriciaNod *p;
    LinkedModule *P; 
	ThisToThat(LinkScopeItf,ScrLinkScope);
    p = PatriciaSeek(&that->LinkScope.Label,Label);
	if (p==&that->LinkScope.Label) {
		MemStack *Mem;
		if (that->LinkFork) { Mem = MemStackFork(that->Mem,4096); } else { Mem = that->Mem; }
		mPush(that->Mem,P);
		P->Label.Key = Call(that->LinkScope.Voc,GetLabel,1(Label));
		P->Num.Id = that->LinkNb++;
		P->Mem = Mem; P->Module = 0; P->Desc = Desc; P->Dependencies = Dependencies;
		P->DepCount = 0; P->Busy = (0!=0);
		PatriciaSeekOrInsert(&that->LinkScope.Label,P->Label.Key,&P->Label);
		BinTreeSeekOrInsert(&that->LinkScope.Num,P->Num.Id,&P->Num);
	} else {
		P = CastBack(LinkedModule,Label,p);
	}
	return P->Num.Id;
}
static int ItfModuleNum(ScrLinkScope *this,char *Label) {
	PatriciaNod *p;
	int r;
	ThisToThat(LinkScopeItf,ScrLinkScope);
	r = -1;
	p = PatriciaSeek(&that->LinkScope.Label,Label);
	if (p!=&that->LinkScope.Label) {
	    LinkedModule *P;
		P = CastBack(LinkedModule,Label,p);
		r = P->Num.Id;
	}
	return r;
}
static ScrDataVal *ItfGetModule(ScrLinkScope *this,int num) {
	ScrDataVal *r;
	ThisToThat(LinkScopeItf,ScrLinkScope);
	r = &ScrDataValNull;
	if (num>=0 && num<that->LinkNb) {
		LinkedModule *P;
	    BinTree *p;
        p = BinTreeSeek(&that->LinkScope.Num,num);
		P = CastBack(LinkedModule,Num,p);
        r = LSGetData(that,P->Label.Key);
	}
	return r;
}
static void ItfReleaseModule(ScrLinkScope *this,int num) {
	ThisToThat(LinkScopeItf,ScrLinkScope);
	if (num>=0 && num<that->LinkNb) {
		LinkedModule *P;
		BinTree *p;
		p = BinTreeSeek(&that->LinkScope.Num,num);
		P = CastBack(LinkedModule,Num,p);
		LSReleaseData(that,P->Label.Key);
	}
}

struct ItfTypeBrowse {
	ScrScopeBrowser *Tgt;
	int num;
};
static PatriciaNod *ItfTypeBrowse(PatriciaNod *P,void *Closure) {
	struct ItfTypeBrowse *c;
    ModuleType *p;
	ScrTypeBrowser *br;
	c = Closure;
	p = CastBack(ModuleType,Label,P);
	Call(c->Tgt,AddType,3(c->num,P->Key,&p->Type->ScrType));
	c->num++;
	return 0;
}
static void ItfBrowse(ScrLinkScope *this,int num,ScrScopeBrowser *Tgt) {
	ScrScope *r;
	ThisToThat(LinkScopeItf,ScrLinkScope);
	if (num>=0 && num<that->LinkNb) {
		LinkedModule *p;
		BinTree *P;
		struct ItfTypeBrowse c;
		P = BinTreeSeek(&that->LinkScope.Num,num);
		p = CastBack(LinkedModule,Num,P);
		c.Tgt = Tgt; c.num  = 0;
		Call(Tgt,Open,1(p->Module->TypeNb));
		PatriciaForEach(&p->Module->Types,ItfTypeBrowse,&c);
        Call(Tgt,AddExpr,1(p->Module->Closed));
		Call(Tgt,Close,0);
	}
}

static void CoreDescOpen(ScrModuleDesc *this,ScrScope *Base){
	ScrTypeBrowser *tb;
	ScrType *t;
	tb = Call(Base,TypeTgt,0);
	t = Call(tb,Atom,1(ScbString));
	Call(Base,AddType,1("string"));
	Call(Base,SetType,2("string",t));
	Call(Base,Close,1(&ScrInstructionNull));
}
static void CoreDescClose(ScrModuleDesc *this,ScrScope *Base) {}
static ScrScope *LSSetCoreModule(LinkScopeItf *lnk) {
	LinkedModule *r;
	PatriciaNod *P;
	ScrLinkScope *Lnk;
	int ModNum;
	static char *Dependencies[1] = {0};
	static char *Name = ":Core";
	static struct ScrModuleDesc CoreStatic = {CoreDescOpen,CoreDescClose};
	static ScrModuleDesc CoreDesc = {&CoreStatic};
	Lnk = &lnk->ScrLinkScope;
	ModNum = Call(Lnk,InsertModule,3(Name,&CoreDesc,Dependencies));
    Call(Lnk,GetModule,1(ModNum));
	P = PatriciaSeek(&lnk->LinkScope.Label,Name);
	r = CastBack(LinkedModule,Label,P);
	return &r->Module->tExprStruct.ScopeThrou.ScrScope;
}

static ScrTypeBrowser *LSTypeTgt(ScrScope *this) {
	ThisToThat(LinkScopeItf,LinkScope.ScopeThrou.ScrScope);
	return Call(that->Core,TypeTgt,0);
}
static ScrExprBrowser *LSExprTgt(ScrScope *this) {
	ThisToThat(LinkScopeItf,LinkScope.ScopeThrou.ScrScope);
	return Call(that->Core,ExprTgt,0);
}

ScrLinkScope *ScrLinkScopeNew(ScrVocabulary *Voc) {
	MemStack *Mem;
	LinkScopeItf *r;
	static struct ScrLinkScope Static = {ItfInsertModule,ItfModuleNum,ItfGetModule,ItfReleaseModule,ItfBrowse};
	static struct ScrScopeThrou ThrouStatic = { LinkScopeExpr, LinkScopeTgt, LinkScopeLexicalData };
	static struct ScrScope LinkScopeStatic = {
		LinkScopeGetVocabulary, LSTypeTgt, LSExprTgt,
		ScopeNullAddType, ScopeNullSetType, LinkScopeGetType,
		ScopeNullAddData, ScopeNullAddVar, LinkScopeGetData, ScopeNullAddPrivate,
		ScopeNullTypedefOpen, ScopeNullExtendOpen, 
		ScopeNullExprStructOpen, ScopeNullExprFnOpen, ScopeNullProcOpen,
		ScopeNullClose, ScopeNullTypeClose
	};
	Mem = rFork(1024);
	mPush(Mem,r); r->Mem = Mem;
	r->ScrLinkScope.Static = &Static;
	r->LinkScope.ScopeThrou.Static = &ThrouStatic;
	r->LinkScope.ScopeThrou.ScrScope.Static = &LinkScopeStatic;
	r->LinkScope.ScopeThrou.Child = &r->LinkScope.ScopeThrou;
	r->LinkNb = 0; 
	r->LinkScope.Voc = Voc;
	r->LinkScope.Busy = (0!=0);
	PatriciaInit(&r->LinkScope.Label);
	BinTreeInit(&r->LinkScope.Num);
	/*Instance*/{ 
		LinkInstance *clear;
		mPush(r->Mem,clear);
		clear->List.n = ListEnd;
		clear->ModuleNb = 0;
		clear->Modules = 0;
		clear->Desc = 0;
		r->Instance.n = &clear->List;
	}
	r->LinkFork = (0!=0); // Hack to include the Core Module in the same stack as the scope
	r->Core = LSSetCoreModule(r);
	r->LinkFork = (0==0);
	return &r->ScrLinkScope;
}

/*----- ScrScopeBrowserNull ------*/

static void ScopeBrowserNullOpen(ScrScopeBrowser *this,int TypeNb) {}
static void ScopeBrowserNullAddType(ScrScopeBrowser *this,int num,char *Label,ScrType *Type) {}
static void ScopeBrowserNullAddExpr(ScrScopeBrowser *this,ScrDataExpr *Expr) {}
static void ScopeBrowserNullClose(ScrScopeBrowser *this) {}
static struct ScrScopeBrowser ScopeBrowserNullStatic = {
	ScopeBrowserNullOpen, ScopeBrowserNullAddType, ScopeBrowserNullAddExpr, ScopeBrowserNullClose
};
ScrScopeBrowser ScrScopeBrowserNull = {&ScopeBrowserNullStatic};

