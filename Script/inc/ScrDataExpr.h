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

#ifndef _ScrDataExpr_h_
#define _ScrDataExpr_h_

#include <ScrType.h>
#include <ScrBuilder.h>

/*--------------------------*/

typedef struct { struct ScrPath *Static;} ScrPath;
extern ScrPath ScrPathNull;

typedef struct {struct ScrExprBrowser *Static;} ScrExprBrowser;
extern ScrExprBrowser ScrExprBrowserNull;

typedef struct {struct ScrDataBrowser *Static;} ScrDataBrowser;
extern ScrDataBrowser ScrDataBrowserNull; 

typedef struct {struct ScrDataCtx *Static; } ScrDataCtx;
extern ScrDataCtx ScrDataCtxNull;

/*__________________________
 |
 | Values
 |__________________________
*/

typedef struct { struct ScrInstruction *Static; } ScrInstruction;
extern ScrInstruction ScrInstructionNull;

typedef struct { struct ScrDataExpr *Static; } ScrDataExpr; // for rValues
extern ScrDataExpr ScrDataExprNull;
typedef struct {struct ScrDataVal *Static; } ScrDataVal;
extern ScrDataVal ScrDataValNull;

typedef struct { struct ScrDataTgt *Static; } ScrDataTgt; // for lValues
extern ScrDataTgt ScrDataTgtNull;
typedef struct { struct ScrVar *Static; } ScrVar;
extern ScrVar ScrVarNull;

/*__________________________
 |
 | Instructions
 |__________________________
*/

struct ScrInstruction {
    ScrInstruction *(*Browse)(ScrInstruction *this,ScrExprBrowser *Tgt);
    void (*Perform)(ScrInstruction *this,ScrDataCtx *Ctx);
};

/*________________________
 |
 | Variables / Path / Tgt
 |________________________
*/

struct ScrVar {
    ScrDataVal *(*Value)(ScrVar *this);
    ScrVar *(*Elt)(ScrVar *this,int idx);
    void (*Address)(ScrVar *this,ScrVar **Base,int *Idx);
    void (*Set)(ScrVar *this,ScrDataVal *Value);
};

struct ScrDataTgt {
    ScrDataTgt *(*Browse)(ScrDataTgt *this,ScrExprBrowser *Tgt);
    ScrType *(*GetType)(ScrDataTgt *this);
    ScrVar *(*Eval)(ScrDataTgt *this,ScrDataCtx *Ctx);
};

/*___________________________
 |
 | Value/Expr
 |___________________________
*/

struct ScrPath {
    ScrVar *(*Follow)(ScrPath *this,ScrVar *Base);
};

struct ScrDataVal {
    ScrDataVal *(*Browse)(ScrDataVal *this,ScrDataBrowser *Tgt);
    ScrDataVal *(*Eval)(
        ScrDataVal *this,ScrDataCtx *Ctx,ScrVar *Result,ScrDataVal *Param
    );
    int (*Int)(ScrDataVal *this);
    ScrString *(*String)(ScrDataVal *this);
    ScrPath *(*Path)(ScrDataVal *this);
    ScrVar *(*PtrTgt)(ScrDataVal *this);
    ScrArray *(*Array)(ScrDataVal *this);
    ScrDataVal *(*GetField)(ScrDataVal *this,int Idx);
};

/*_____________________________
 |
 | Expressions
 |_____________________________
*/

struct ScrDataExpr {
    ScrDataExpr *(*Browse)(ScrDataExpr *this,ScrExprBrowser *Tgt);
    ScrType *(*GetType)(ScrDataExpr *this);
    ScrDataVal *(*Eval)(ScrDataExpr *this,ScrDataCtx *Ctx);
};

typedef struct {
    int Label;
    ScrDataVal *(*op)(ScrDataVal *x);
} ScrDataUOpDesc;
typedef struct {
    int Label;
    ScrDataVal *(*op)(ScrDataVal *x,ScrDataVal *y);
} ScrDataBOpDesc;


int ScrIntExprEvalAttempt(int *value,ScrDataExpr *Expr); // returns success status

    /*--------------*/

struct ScrDataCtx {
    ScrDataCtx *(*Extrn)(ScrDataCtx *this,int ExtrnNum);
    ScrDataCtx *(*Lexical)(ScrDataCtx *this,int Depth);
    ScrDataVal *(*Param)(ScrDataCtx *this);
    ScrDataVal *(*Local)(ScrDataCtx *this,int Idx);
    ScrDataVal *(*FifoParam)(ScrDataCtx *this,ScrDataVal *Param);
    ScrDataCtx *(*SetLocals)(ScrDataCtx *this,int Depth,int LocalsNb,ScrDataExpr **Locals);
    struct {
        ScrVar *(*Base)(ScrDataCtx *this);
        ScrVar *(*Result)(ScrDataCtx *this);
        ScrDataCtx *(*Invoke)(ScrDataCtx *this,
            int Depth,ScrVar *Result,ScrDataVal *Param,ScrType *Local
        );
        ScrVar *(*Expand)(ScrDataCtx *this,ScrType *Base,int nb);
    } Exec;
};

ScrDataCtx *ScrCtxGlobal(ScrDataCtx **Globals,int num);

    /*---------------*/

struct ScrDataBrowser {
    ScrDataVal *(*Int)(ScrDataBrowser *this,int i);
    ScrDataVal *(*String)(ScrDataBrowser *this,ScrString *String);
    ScrDataVal *(*Complex)(ScrDataBrowser *this,
        ScrType *Type,ScrDataVal **Val);
    ScrDataVal *(*Vector)(ScrDataBrowser *this,ScrArray *Content);
    ScrDataVal *(*Ptr)(ScrDataBrowser *this,ScrDataVal *Tgt); // ToDo: give a proper prototype to this.
};

ScrDataVal *SDValInt(int Value);
ScrDataVal *SDValChar(int Value);
ScrDataVal *SDValString(ScrString *Value);
ScrDataVal *SDValArray(ScrArray *Content);
ScrDataVal *ScrValAddress(ScrVar *Var);

ScrDataVal *ScrLazyData(ScrDataExpr *Modele,ScrDataCtx *Ctx);


struct ScrExprBrowser {
    ScrDataExpr *(*Const)(ScrExprBrowser *this,ScrType *Type,ScrDataVal *v);
    ScrDataExpr *(*Cond)(ScrExprBrowser *this,
        ScrDataExpr *c,ScrDataExpr *t,ScrDataExpr *f);

    ScrDataExpr *(*Alter)(ScrExprBrowser *this,
        ScrDataUOpDesc *Desc,ScrDataExpr *x);
    ScrDataExpr *(*Compose)(ScrExprBrowser *this,
        ScrDataBOpDesc *Desc,ScrDataExpr *x,ScrDataExpr *y);
    
    ScrDataExpr *(*Complex)(ScrExprBrowser *this,
        int LLvl,ScrType *Type,ScrDataExpr **Result,int LocalNb,
        ScrDataExpr **Local);
    ScrDataExpr *(*Apply)(ScrExprBrowser *this,ScrDataExpr *Expr,
        ScrDataExpr *Param);
    ScrDataExpr *(*FieldSelect)(ScrExprBrowser *this,ScrDataExpr *Struct,
        int Num);

    ScrDataExpr *(*Vector)(ScrExprBrowser *this,ScrType *Type,ScrArray *Space);
    ScrDataExpr *(*EltSelect)(ScrExprBrowser *this,ScrDataExpr *Space,
        ScrDataExpr *Idx);

    ScrDataExpr *(*Variable)(ScrExprBrowser *this,ScrDataTgt *Var);
    ScrDataExpr *(*Ptr)(ScrExprBrowser *this,ScrDataTgt *Tgt);

    ScrDataExpr *(*Param)(ScrExprBrowser *this,ScrType *Type,int Depth);
    ScrDataExpr *(*Local)(ScrExprBrowser *this,ScrType *Type,int Depth,
        int Idx);
    ScrDataExpr *(*Global)(ScrExprBrowser *this,ScrType *Type,int Module,
        int Idx);

    struct {
        ScrDataTgt *(*Targeted)(ScrExprBrowser *this,ScrDataExpr *Ptr);

        ScrDataTgt *(*EltAccess)(ScrExprBrowser *this,ScrDataTgt *Base,
            ScrDataExpr *Idx);
        ScrDataTgt *(*FieldAccess)(ScrExprBrowser *this,ScrDataTgt *Base,
            int Idx);

        ScrDataTgt *(*Result)(ScrExprBrowser *this,int Depth,ScrType *Type,
            int Idx);
        ScrDataTgt *(*Local)(ScrExprBrowser *this,int Depth,ScrType *Type,
            int Idx);
        ScrDataTgt *(*Extern)(ScrExprBrowser *this,ScrType *Type,int Module,
            int Idx);

        ScrVar *(*Const)(ScrExprBrowser *this,ScrDataVal *Val);
        ScrDataExpr *(*Procedure)(ScrExprBrowser *this,int Depth,
            ScrType *Prototype,ScrType *Local,ScrInstruction *Body);
    } Var;

    struct {
        ScrInstruction *(*Set)(ScrExprBrowser *this,ScrDataTgt *Tgt,
            ScrDataExpr *Val);
        ScrInstruction *(*Sequence)(ScrExprBrowser *this,
            ScrInstruction **Begin,ScrInstruction **End);
        ScrInstruction *(*While)(ScrExprBrowser *this,ScrDataExpr *Cond,
            ScrInstruction *Inst);
        ScrInstruction *(*Until)(ScrExprBrowser *this,ScrInstruction *Inst,
            ScrDataExpr *Cond);
        ScrInstruction *(*Cond)(ScrExprBrowser *this,ScrDataExpr *Cond,
            ScrInstruction *IfTrue);
        ScrInstruction *(*Alternate)(ScrExprBrowser *this,ScrDataExpr *Cond,
            ScrInstruction *ifTrue,ScrInstruction *Else);
        ScrInstruction *(*Select)(ScrExprBrowser *this,ScrDataExpr *Select,
            ScrArray *Choices);
        ScrInstruction *(*Invoke)(ScrExprBrowser *this,ScrDataTgt *Result,
            ScrDataExpr *Param,int Depth,ScrType *Local,ScrInstruction *Block);
        ScrInstruction *(*Extend)(ScrExprBrowser *this,ScrDataTgt *ArrayPtr,
            ScrDataExpr *Nb);
    } Instr;
};

ScrExprBrowser *ScrRawExpr(void);

#endif
