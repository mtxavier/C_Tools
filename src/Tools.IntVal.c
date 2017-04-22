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
#include <Tools.h>
#include <List.h>
#include <stdarg.h>
#include <string.h>

/*____________________________________________________________________________
 |
 | Int Operators;
 |____________________________________________________________________________
*/

int TlsIntTransNeg(int a) { return -a;}
int TlsIntTransNot(int a) { return (a==(0!=0)); }
int TlsIntTransBool(int a) { return (a!=(0!=0)); }
int TlsIntTransCmpl0(int a) { return ~a; }
int TlsIntTransAbs(int a)  { return (a<0)?-a:a;}

int TlsIntOpSum(int a,int b) { return a+b; }
int TlsIntOpProd(int a,int b) { return a*b; }
int TlsIntOpDiff(int a,int b) {return a-b;}
int TlsIntOpMin(int a,int b) { return (a<b)?a:b; }
int TlsIntOpMax(int a,int b) { return (a>b)?a:b; }
int TlsIntOpDiv(int a,int b) { return a/b; }
int TlsIntOpMod(int a,int b) { return a%b; }
int TlsIntOpAnd(int a,int b) { return a&b; }
int TlsIntOpOr(int a,int b) { return a|b; }
int TlsIntOpXor(int a,int b) { return a^b; }
int TlsBoolOpAnd(int a,int b) { return a&&b; }
int TlsBoolOpOr(int a,int b) { return a||b; }
int TlsBoolOpXor(int a,int b) { return a!=b; }
int TlsBoolOpEq(int a,int b) { return a==b; }
int TlsIntOpSar(int a,int b) { return a>>b; }
int TlsIntOpSal(int a,int b) { return a<<b; }
int TlsIntOpSlr(int a,int b) { unsigned int A; A=a; return (A>>b);}
int TlsIntOpInf(int a,int b) { return a<b; }
int TlsIntOpInfEq(int a,int b) { return a<=b; }
int TlsIntOpSup(int a,int b) { return a>b; }
int TlsIntOpSupEq(int a,int b) { return a>=b; }

/*____________________________________________________________________________
 |
 | TlsIntVal
 |____________________________________________________________________________
*/

static int IntValNullVal(TlsIntVal *this) {return 0;}
static struct TlsIntVal IntValNullStatic = {IntValNullVal};
TlsIntVal TlsIntValNull = {&IntValNullStatic};

    /*------------------*/

typedef struct {
    TlsIntVal TlsIntVal;
    int n;
} ConstIntVal;
static int ConstIntValVal(TlsIntVal *this) {
    ThisToThat(ConstIntVal,TlsIntVal);
    return that->n;
}
TlsIntVal *TlsConstIntVal(int n) {
    ConstIntVal *r;
    static struct TlsIntVal Static = {ConstIntValVal};
    rPush(r);
    r->TlsIntVal.Static = &Static;
    r->n = n;
    return &r->TlsIntVal;
}

    /*------------------*/

typedef struct {
    TlsIntVal TlsIntVal;
    TlsIntTranslator *Op;
    TlsIntVal *x;
} IntValTrans;
static int IntValTransVal(TlsIntVal *this) {
    int x;
    ThisToThat(IntValTrans,TlsIntVal);
    x = Call(that->x,Val,0);
    return that->Op(x);
}
TlsIntVal *TlsIntValTrans(TlsIntTranslator *op,TlsIntVal *x) {
    IntValTrans *r;
    static struct TlsIntVal Static = {IntValTransVal};
    rPush(r);
    r->TlsIntVal.Static = &Static;
    r->Op=op; r->x=x;
    return &r->TlsIntVal;
}

    /*------------------*/

typedef struct {
    TlsIntVal TlsIntVal;
    int *n;
} DirectIntVal;
static int DirectIntValVal(TlsIntVal *this) {
    ThisToThat(DirectIntVal,TlsIntVal);
    return *that->n;
}
TlsIntVal *TlsDirecIntVal(int *n) {
    DirectIntVal *r;
    static struct TlsIntVal Static = {DirectIntValVal};
    rPush(r);
    r->TlsIntVal.Static = &Static;
    r->n = n;
    return &r->TlsIntVal;
}

/*----------------*/

typedef struct {
    TlsIntVal TlsIntVal;
    TlsIntOperator *Op;
    TlsIntVal **b,**e;
} OprVecVal;

typedef struct {
    TlsIntVal TlsIntVal;
    TlsIntVal **b,**e;
} VecVal;
#define VecValGetSize(Tp,n,a) { \
    Tp *p; \
    va_list l; \
    n=0; p=a; \
    va_start(l,a); \
    while (p!=0) { \
        p=va_arg(l,Tp *); \
        n++; \
    } \
    va_end(l); \
}
#define VecValCopy(Tp,b,a) {\
    Tp *q,**p;\
    va_list l;\
    q=a; p=b;\
    va_start(l,a);\
    while (q!=0) {\
        *p=q; p++;\
        q=va_arg(l,Tp *);\
    }\
    va_end(l);\
}

TlsVoidVec *TlsVoidVecNew(TlsVoidVec *r,void *a0,...) {
    int n;
    void **b;
    VecValGetSize(void,n,a0);
    rnPush(b,n);
    r->b=b; r->e=b+n;
    VecValCopy(void,b,a0);
    return r;
}

TlsIntValVec *TlsIntValVecNew(TlsIntValVec *r,TlsIntVal *a,...){
    int n;
    TlsIntVal **b;
    VecValGetSize(TlsIntVal,n,a);
    rnPush(b,n);
    r->b=b; r->e=b+n;
    VecValCopy(TlsIntVal,b,a);
    return r;
}

    /*-----*/

static int ValVecRCmbndVal(TlsIntVal *this) {
    int r,a;
    TlsIntVal **b,**p;
    ThisToThat(OprVecVal,TlsIntVal);
    b=that->b; p=that->e-1;
    if (p>=b) {
        TlsIntOperator *op;
        op = that->Op;
        r=Call(*p,Val,0); p--;
        while (p>=b) {
            a = Call(*p,Val,0); p--;
            r = op(a,r);
        }
    } else {
        r=0;
    }
    return r;
}
TlsIntVal *TlsIntValVecRCmbnd(TlsIntOperator *x,TlsIntValVec *v) {
    OprVecVal *r;
    static struct TlsIntVal Static={ValVecRCmbndVal};
    int n;
    rPush(r);
    r->TlsIntVal.Static=&Static;
    r->Op=x; r->b=v->b; r->e=v->e;
    return &r->TlsIntVal;
}

static int ValVecLCmbndVal(TlsIntVal *this) {
    int r,a;
    TlsIntVal **p,**e;
    ThisToThat(OprVecVal,TlsIntVal);
    p=that->b; e=that->e;
    if (p<e) {
        TlsIntOperator *op;
        op=that->Op;
        r=Call(*p,Val,0); p++;
        while (p<e) {
            a=Call(*p,Val,0); p++;
            r=op(r,a);
        }
    } else {
        r=0;
    }
    return r;
}
TlsIntVal *TlsIntValVecLCmbnd(TlsIntOperator *x,TlsIntValVec *v) {
    OprVecVal *r;
    static struct TlsIntVal Static={ValVecLCmbndVal};
    rPush(r);
    r->TlsIntVal.Static=&Static;
    r->Op=x; r->b=v->b; r->e=v->e;
    return &r->TlsIntVal;
}
    /*-----*/

static int ValVecSumVal(TlsIntVal *this) {
    int r,a;
    TlsIntVal **p,**e;
    ThisToThat(VecVal,TlsIntVal);
    r=0; p=that->b; e=that->e;
    while (p<e) {
        a = Call(*p,Val,0);
        r+=a;
        p++;
    }
    return r;
}
TlsIntVal *TlsIntValVecSum(TlsIntValVec *v) {
    VecVal *r;
    static struct TlsIntVal Static={ValVecSumVal};
    rPush(r);
    r->TlsIntVal.Static=&Static;
    r->b=v->b; r->e=v->e;
    return &r->TlsIntVal;
}

     /*------*/

static int ValMinVal(TlsIntVal *this) {
    int r,a;
    TlsIntVal **p,**e;
    ThisToThat(VecVal,TlsIntVal);
    r=0; p=that->b; e=that->e;
    if (p!=e) {
        r = Call(*p,Val,0);
        p++;
    }
    while (p!=e) {
        a = Call(*p,Val,0);
        if (a<r) r=a;
        p++;
    }
    return r;
}
TlsIntVal *TlsIntValVecMin(TlsIntValVec *v){
    VecVal *r;
    static struct TlsIntVal Static={ValMinVal};
    rPush(r);
    r->TlsIntVal.Static=&Static;
    r->b=v->b; r->e=v->e;
    return &r->TlsIntVal;
}

     /*------*/

static int ValMaxVal(TlsIntVal *this) {
    int r,a;
    TlsIntVal **p,**e;
    ThisToThat(VecVal,TlsIntVal);
    r=0; p=that->b; e=that->e;
    if (p!=e) {
        r = Call(*p,Val,0);
        p++;
    }
    while (p!=e) {
        a = Call(*p,Val,0);
        if (a>r) r=a;
        p++;
    }
    return r;
}
TlsIntVal *TlsIntValVecMax(TlsIntValVec *v){
    static struct TlsIntVal Static={ValMaxVal};
    VecVal *r;
    rPush(r);
    r->TlsIntVal.Static = &Static;
    r->b=v->b; r->e=v->e;
    return &r->TlsIntVal;
}

     /*------*/

static int ValVecProductVal(TlsIntVal *this) {
    int r,a;
    TlsIntVal **p,**e;
    ThisToThat(VecVal,TlsIntVal);
    r=1; p=that->b; e=that->e;
    while ((p<e)&&(r)) {
        a=Call(*p,Val,0);
        r=r*a;
        p++;
    }
    return r;
}
TlsIntVal *TlsIntValVecProduct(TlsIntValVec *v) {
    VecVal *r;
    static struct TlsIntVal Static={ValVecProductVal};
    rPush(r);
    r->TlsIntVal.Static = &Static;
    r->b=v->b; r->e=v->e;
    return &r->TlsIntVal;
}

     /*------*/

typedef struct {
    TlsIntVal TlsIntVal;
    TlsIntOperator *Op;
    TlsIntVal *a,*b;
} CmbndValPair;
static int CmbndValPairVal(TlsIntVal *this) {
    int a,b;
    ThisToThat(CmbndValPair,TlsIntVal);
    a=Call(that->a,Val,0);
    b=Call(that->b,Val,0);
    return that->Op(a,b);
}
TlsIntVal *TlsIntValCmbnd(TlsIntOperator *Op,TlsIntVal *a,TlsIntVal *b){
    CmbndValPair *r;
    static struct TlsIntVal Static = {CmbndValPairVal};
    rPush(r);
    r->TlsIntVal.Static=&Static;
    r->Op=Op; r->a=a; r->b=b;
    return &r->TlsIntVal;
}

     /*------*/

typedef struct {
    TlsIntVal TlsIntVal;
    TlsIntVal *a,*b;
} PairVal;
static int ValDiffVal(TlsIntVal *this) {
    int a,b;
    ThisToThat(PairVal,TlsIntVal);
    a = Call(that->a,Val,0);
    b = Call(that->b,Val,0);
    return a-b;
}
TlsIntVal *TlsIntValDiff(TlsIntVal *a,TlsIntVal *b) {
    PairVal *r;
    static struct TlsIntVal Static = {ValDiffVal};
    rPush(r);
    r->TlsIntVal.Static = &Static;
    r->a=a; r->b=b;
    return &r->TlsIntVal;
}

     /*------*/

static int ValSumVal(TlsIntVal *this) {
    int a,b;
    ThisToThat(PairVal,TlsIntVal);
    a=Call(that->a,Val,0);
    b=Call(that->b,Val,0);
    return a+b;
}
TlsIntVal *TlsIntValSum(TlsIntVal *a,TlsIntVal *b) {
    PairVal *r;
    static struct TlsIntVal Static = {ValSumVal};
    rPush(r);
    r->TlsIntVal.Static = &Static;
    r->a=a; r->b=b;
    return &r->TlsIntVal;
}

     /*------*/

static int ValProductVal(TlsIntVal *this) {
    int a,b;
    ThisToThat(PairVal,TlsIntVal);
    a=Call(that->a,Val,0);
    if (a) { b=Call(that->b,Val,0); } else b=0;
    return a*b;
}
TlsIntVal *TlsIntValProduct(TlsIntVal *a,TlsIntVal *b) {
    PairVal *r;
    static struct TlsIntVal Static={ValProductVal};
    rPush(r);
    r->TlsIntVal.Static=&Static;
    r->a=a; r->b=b;
    return &r->TlsIntVal;
}

    /*-------*/

typedef struct {
    TlsIntVal TlsIntVal;
    TlsIntOperator *Op;
    TlsDyndList *v;
} IntValdList;
struct dListCombine {
    int a;
    int first;
    TlsIntOperator *op;
};
static int dListRCombine(void *B,void *Cls) {
    struct dListCombine *cls;
    TlsIntVal *vb;
    int b;
    cls = Cls; vb=B; b=Call(vb,Val,0);
    if (cls->first) {
        cls->first=(0!=0);
        cls->a=b;
    } else {
        cls->a=cls->op(b,cls->a);
    }
    return (0!=0);
}
static int IntValdListRVal(TlsIntVal *this) {
    struct dListCombine Cls;
    ThisToThat(IntValdList,TlsIntVal);
    Cls.a=0; Cls.first=(0==0);
    Cls.op=that->Op;
    TlsDynRvsdListForEach(that->v,dListRCombine,&Cls);
    return Cls.a;
}
TlsIntVal *TlsIntValdListRCmbnd(TlsIntOperator *op,TlsDyndList *v) {
    IntValdList *r;
    static struct TlsIntVal Static = {IntValdListRVal};
    rPush(r);
    r->TlsIntVal.Static = &Static;
    r->Op=op; r->v=v;
    return &r->TlsIntVal;
}

   /*-------*/

static int dListLCombine(void *B,void *Cls) {
    struct dListCombine *cls;
    int b;
    TlsIntVal *bf;
    cls=Cls; bf=B; b=Call(bf,Val,0);
    if (cls->first) {
        cls->first=(0!=0);
        cls->a=b;
    } else {
        cls->a=cls->op(cls->a,b);
    }
    return (0!=0);
}
static int IntValdListLVal(TlsIntVal *this) {
    struct dListCombine cls;
    ThisToThat(IntValdList,TlsIntVal);
    cls.op=that->Op;
    cls.first=(0==0);
    cls.a=0;
    TlsDyndListForEach(that->v,dListLCombine,&cls);
    return cls.a;
}
TlsIntVal *TlsIntValdListLCmbnd(TlsIntOperator *op,TlsDyndList *v) {
    IntValdList *r;
    static struct TlsIntVal Static = {IntValdListLVal};
    rPush(r);
    r->TlsIntVal.Static = &Static;
    r->Op=op; r->v=v;
    return &r->TlsIntVal;
}

/*_______________________________________________
 |
 | TlsIntFn
 |_______________________________________________
*/

typedef struct {
    TlsIntFn TlsIntFn;
    TlsIntVal *A;
} ValModPair;
static int ValModBonusVal(TlsIntFn *this,int org) {
    int a;
    ThisToThat(ValModPair,TlsIntFn);
    a = Call(that->A,Val,0);
    return org+a;
}
TlsIntFn *TlsIntFnAdd(TlsIntVal *A) {
    static struct TlsIntFn Static={ValModBonusVal};
    ValModPair *r;
    rPush(r);
    r->TlsIntFn.Static = &Static;
    r->A=A;
    return &r->TlsIntFn;
}

    /*------*/

static int ValModFlooredVal(TlsIntFn *this,int org) {
    int a;
    ThisToThat(ValModPair,TlsIntFn);
    a = Call(that->A,Val,0);
    return (a>org)?a:org;
}
TlsIntFn *TlsIntFnMax(TlsIntVal *A){
    static struct TlsIntFn Static = {ValModFlooredVal};
    ValModPair *r;
    rPush(r);
    r->TlsIntFn.Static = &Static;
    r->A=A;
    return &r->TlsIntFn;
}

    /*------*/

static int ValModCappedVal(TlsIntFn *this,int org) {
    int a;
    ThisToThat(ValModPair,TlsIntFn);
    a = Call(that->A,Val,0);
    return (a<org)?a:org;
}
TlsIntFn *TlsIntFnMin(TlsIntVal *A){
    static struct TlsIntFn Static = {ValModCappedVal};
    ValModPair *r;
    rPush(r);
    r->TlsIntFn.Static = &Static;
    r->A=A;
    return &r->TlsIntFn;
}

    /*------*/

typedef struct {
    TlsIntVal TlsIntVal;
    TlsIntVal *x;
    TlsIntFn *f;
} ClosedMod;
static int ClosedModVal(TlsIntVal *this) {
    int x,r;
    ThisToThat(ClosedMod,TlsIntVal);
    x = Call(that->x,Val,0);
    r = Call(that->f,Val,1(x));
    return r;
}
TlsIntVal *TlsIntClosedFn(TlsIntFn *f,TlsIntVal *x) {
    ClosedMod *r;
    static struct TlsIntVal Static = {ClosedModVal};
    rPush(r);
    r->TlsIntVal.Static = &Static;
    r->x=x; r->f=f;
    return &r->TlsIntVal;
}

/*---------------------*/

typedef struct {
    TlsIntFn TlsIntFn;
    TlsIntFn *f,*g;
} ModCmps;

static int ModCmpsVal(TlsIntFn *this,int org) {
    int x;
    ThisToThat(ModCmps,TlsIntFn);
    x = Call(that->g,Val,1(org));
    x = Call(that->f,Val,1(org));
    return x;
}
TlsIntFn *TlsIntFnCmpsd(TlsIntFn *f,TlsIntFn *g) {
    static struct TlsIntFn Static = {ModCmpsVal};
    ModCmps *r;
    rPush(r);
    r->TlsIntFn.Static = &Static;
    r->f = f; r->g = g;
    return &r->TlsIntFn;
}

/*---------------------*/

TlsIntFnVec *TlsIntFnVecNew(TlsIntFnVec *r,TlsIntFn *a0,...) {
    int n;
    TlsIntFn **b;
    VecValGetSize(TlsIntFn,n,a0);
    rnPush(b,n);
    r->b=b; r->e=b+n;
    VecValCopy(TlsIntFn,b,a0);
    return r;
}

/*--------------------*/

typedef struct {
    TlsIntFn TlsIntFn;
    TlsIntFnVec Vec;
} FnVec;
static int FnVecRCmpsdVal(TlsIntFn *this,int x) {
    TlsIntFn **p,**b;
    int y;
    ThisToThat(FnVec,TlsIntFn);
    y=x; b=that->Vec.b; p=that->Vec.e-1;
    while (p>=b) {
        y=Call(*p,Val,1(y));
        p--;
    }
    return y;
}
TlsIntFn *TlsIntFnVecRCmpsd(TlsIntFnVec *vec) {
    FnVec *r;
    static struct TlsIntFn Static = {FnVecRCmpsdVal};
    rPush(r);
    r->TlsIntFn.Static = &Static;
    r->Vec = *vec;
    return &r->TlsIntFn;
}

static int FnVecLCmpsdVal(TlsIntFn *this,int x) {
    TlsIntFn **p,**e;
    int y;
    ThisToThat(FnVec,TlsIntFn);
    y=x; p=that->Vec.b; e=that->Vec.e;
    while (p<e) {
        y=Call(*p,Val,1(y));
        p++;
    }
    return y;
}
TlsIntFn *TlsIntFnVecLCmpsd(TlsIntFnVec *vec) {
    FnVec *r;
    static struct TlsIntFn Static = {FnVecLCmpsdVal};
    rPush(r);
    r->TlsIntFn.Static = &Static;
    r->Vec = *vec;
    return &r->TlsIntFn;
}

     /*-------------*/

typedef struct {
    TlsIntFn TlsIntFn;
    TlsDyndList/*<TlsIntFn *>*/ *Vec;
} IntFndList;
typedef struct {
    int x;
} IntFndListClos;
static int IntFndListCompute(void *B,void *Clos) {
    IntFndListClos *clos;
    TlsIntFn *b;
    clos=Clos; b=B;
    clos->x=Call(b,Val,1(clos->x));
    return (0!=0);
}
static int IntFndListRCmpsdVal(TlsIntFn *this,int x) {
    IntFndListClos cl;
    ThisToThat(IntFndList,TlsIntFn);
    cl.x=x;
    TlsDynRvsdListForEach(that->Vec,IntFndListCompute,&cl);
    return cl.x;
}
TlsIntFn *TlsIntFndListRCmpsd(TlsDyndList/*<TlsIntFn *>*/ *Vec) {
    IntFndList *r;
    static struct TlsIntFn Static = {IntFndListRCmpsdVal};
    rPush(r);
    r->TlsIntFn.Static = &Static;
    r->Vec=Vec;
    return &r->TlsIntFn;
}

static int IntFndListLCmpsdVal(TlsIntFn *this,int x) {
    IntFndListClos cl;
    ThisToThat(IntFndList,TlsIntFn);
    cl.x=x;
    TlsDyndListForEach(that->Vec,IntFndListCompute,&cl);
    return cl.x;
}
TlsIntFn *TlsIntFndListLCmpsd(TlsDyndList/*<TlsIntFn *>*/ *Vec) {
    IntFndList *r;
    static struct TlsIntFn Static = {IntFndListLCmpsdVal};
    rPush(r);
    r->TlsIntFn.Static = &Static;
    r->Vec=Vec;
    return &r->TlsIntFn;
}

/*__________________________________________________________
 |
 | TlsEvaluator
 |__________________________________________________________
*/

static void EvaluatorNullEval(TlsEvaluator *this,void *Ctx) {}
static struct TlsEvaluator EvalNullStatic = {EvaluatorNullEval};
TlsEvaluator TlsEvaluatorNull = {&EvalNullStatic};

/*-------*/

typedef struct {
    TlsEvaluator TlsEvaluator;
    int oFld,Val;
    TlsEvaluator *Child;
} SetIntConst;
static void SetIntConstEval(TlsEvaluator *this,void *Ctx) {
    int *fld,old;
    ThisToThat(SetIntConst,TlsEvaluator);
    fld = Ctx+that->oFld;
    old = *fld;
    *fld = that->Val;
    Call(that->Child,Eval,1(Ctx));
    *fld = old;
}
TlsEvaluator *TlsEvalWithSetIntConst(int oFld,int Val,TlsEvaluator *Perform) {
    static struct TlsEvaluator Static = {SetIntConstEval};
    SetIntConst *R;
    rPush(R);
    R->TlsEvaluator.Static = &Static;
    R->oFld = oFld; R->Val = Val; R->Child = Perform;
    return &R->TlsEvaluator;
}

/*---------*/

typedef struct {
    TlsEvaluator TlsEvaluator;
    int oFld,*Val;
    TlsEvaluator *Child;
} SetInt;
static void SetIntEval(TlsEvaluator *this,void *Ctx) {
    int old,*fld;
    ThisToThat(SetInt,TlsEvaluator);
    fld = Ctx+that->oFld;
    old = *fld;
    *fld = *that->Val;
    Call(that->Child,Eval,1(Ctx));
    *fld = old;
}
TlsEvaluator *TlsEvalWithSetInt(int oFld,int *Val,TlsEvaluator *Perform) {
    SetInt *R;
    static struct TlsEvaluator Static = {SetIntEval};
    rPush(R);
    R->TlsEvaluator.Static = &Static;
    R->oFld = oFld; R->Val = Val; R->Child = Perform;
    return &R->TlsEvaluator;
}

/*----------*/

typedef struct {
    TlsEvaluator TlsEvaluator;
    int oFld;
    TlsIntVal *Val;
    TlsEvaluator *Child;
} SetIntVal;
static void SetIntValEval(TlsEvaluator *this,void *Ctx) {
    int old,*fld;
    ThisToThat(SetIntVal,TlsEvaluator);
    fld = Ctx+that->oFld;
    old = *fld;
    *fld = Call(that->Val,Val,0);
    Call(that->Child,Eval,1(Ctx));
    *fld = old;
}
TlsEvaluator *TlsEvalWithIntVal(int oFld,TlsIntVal *Val,TlsEvaluator *Perform) {
    SetIntVal *R;
    static struct TlsEvaluator Static = {SetIntValEval};
    rPush(R);
    R->TlsEvaluator.Static = &Static;
    R->oFld = oFld; R->Val = Val; R->Child = Perform;
    return &R->TlsEvaluator;
}

/*--------------------*/

typedef struct {
    TlsEvaluator TlsEvaluator;
    size_t sz;
    int oFld;
    void *Val;
    TlsEvaluator *Child;
} SetBlock;
static void SetBlockEval(TlsEvaluator *this,void *Ctx) {
    void *old,*fld;
    ThisToThat(SetBlock,TlsEvaluator);
    pOpen
        pnPush(old,TlsRoundUp(int,(that->sz)));
        fld = Ctx+that->oFld;
        memcpy(old,fld,that->sz);
        memcpy(fld,that->Val,that->sz);
        Call(that->Child,Eval,1(Ctx));
        memcpy(fld,old,that->sz);
    pClose
}
TlsEvaluator *TlsEvalWithSetBlock(int bFld,int eFld,void *Val,TlsEvaluator *Perform) {
    SetBlock *R;
    static struct TlsEvaluator Static = {SetBlockEval};
    rPush(R);
    R->TlsEvaluator.Static = &Static;
    R->oFld = bFld;
    R->sz =  eFld-bFld;
    R->Val = Val;
    R->Child = Perform;
    return &R->TlsEvaluator;
}
