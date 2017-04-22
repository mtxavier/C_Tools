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
#include <GuiWidget.h>
#include <Gui/GuiKeyMap.h>
#include <Geo.h>


#define l_(a,b) C_(Gui,a,b)
#define w_(a,b) C_(Widget,a,b)

static int TriggerNullTrigger(GuiTrigger *this) { return l_(Event,Ignored); }
static struct GuiTrigger TriggerNullStatic = {&TriggerNullTrigger};
GuiTrigger GuiTriggerNull = {&TriggerNullStatic};

static int IntTriggerNullTrigger(GuiIntTrigger *this,int v) {
   return l_(Event,Ignored);
}
static struct GuiIntTrigger IntTriggerNullStatic = {&IntTriggerNullTrigger};
GuiIntTrigger GuiIntTriggerNull = {&IntTriggerNullStatic};

static int StreamTriggerNullTrigger(GuiStreamTrigger *this,void *b,void *e) {
    return l_(Event,Ignored);
}
static struct GuiStreamTrigger StreamTriggerNullStatic =
    {&StreamTriggerNullTrigger};
GuiStreamTrigger GuiStreamTriggerNull = {&StreamTriggerNullStatic};

/*_______________________________________________________________________
 |
 |
 |_______________________________________________________________________
*/

typedef struct {
    List/*<PrevTrigger.List>*/ List;
    int Id;
    union {
        GuiTrigger *Trigger;
        GuiIntTrigger *IntTrigger;
        GuiStreamTrigger *StreamTrigger;
    } Old;
} PrevTrigger;
typedef struct {
    List/*<PrevLvl.List>*/ List;
    List/*<PrevTrigger.List>*/ Trigger,IntTrigger,StreamTrigger;
} PrevLvl;

/*-------------------------------*/

typedef struct {
    GuiWidgetCtx GuiWidgetCtx;
    GuiWidgetIn GuiWidgetIn;
    TlsMemPool *LvlPool,*TrgPool;
    PrevLvl *Lvl;
    GuiTrigger **Triggers;
    GuiIntTrigger *AddChar;
    int *FnKeyBind;
} WidgetIO;

static void PrevLvlInit(PrevLvl *l) {
    l->List.n = &l->List;
    l->Trigger.n = &l->Trigger;
    l->IntTrigger.n = &l->IntTrigger;
    l->StreamTrigger.n = &l->StreamTrigger;
}

static int WidgetIOKeyDown(GuiInput *this,int dvc,int key) {
    int r,sel;
    ThisToThat(WidgetIO,GuiWidgetIn.GuiInput);
    if ((dvc>=l_(Device,Keyboard[0]))&&(dvc<=l_(Device,Keyboard[3]))) {
        if (GuiKeyTyp(key)==GuiKeyTypAscii) {
            r = Call(that->AddChar,Trigger,1(key&0xff));
        } else {
            if (GuiKeyTyp(key)==GuiKeyTypFn) {
                sel = that->FnKeyBind[(key&0xff)-C_(Key,Scan,Fn)];
                r = Call(that->Triggers[sel],Trigger,0);
            }
        }
    } else {
       r = l_(Event,Ignored);
    }
    return r;
}
static int WidgetIOKeyUp(GuiInput *this,int device,int key){
    ThisToThat(WidgetIO,GuiWidgetIn.GuiInput);
    return l_(Event,Ignored);
}
static int WidgetIOCursor(GuiInput *this,int device,int x,int y){
    ThisToThat(WidgetIO,GuiWidgetIn.GuiInput);
    return l_(Event,Ignored);
}
static int WidgetIOmsTime(GuiInput *this,int *next,int current){
    ThisToThat(WidgetIO,GuiWidgetIn.GuiInput);
    *next = current+50;
    return l_(Event,Processed);
}
static int WidgetIOSuspend(GuiInput *this,int evt){
    ThisToThat(WidgetIO,GuiWidgetIn.GuiInput);
    return l_(Event,Processed);
}

static void WidgetIOBegin(GuiWidgetIn *this,int TypeId,char *Type) {
    ThisToThat(WidgetIO,GuiWidgetIn);
}
static GuiTrigger *WidgetIOTrigger(
    GuiWidgetIn *this,int num,char *cId,GuiTrigger *Add
) {
    GuiTrigger *r;
    ThisToThat(WidgetIO,GuiWidgetIn);
    r = &GuiTriggerNull;
    if ((num>=0)&&(num<l_(Reflex,nb))) {
        PrevTrigger *n;
        n = Call(that->TrgPool,Alloc,0);
        n->Id = num;
        n->Old.Trigger = that->Triggers[num];
        n->List.n = that->Lvl->Trigger.n;
        that->Lvl->Trigger.n = &n->List;
        r = that->Triggers[num];
        that->Triggers[num] = Add;
    }
    return r;
}
static GuiIntTrigger *WidgetIOIntTrigger(
    GuiWidgetIn *this,int num,char *cId,GuiIntTrigger *Add
) {
    GuiIntTrigger *r;
    ThisToThat(WidgetIO,GuiWidgetIn);
    r = &GuiIntTriggerNull;
    if (num==l_(Reflex.Int,AddChar)) {
        PrevTrigger *n;
        n = Call(that->TrgPool,Alloc,0);
        n->Id = num;
        n->Old.IntTrigger = that->AddChar;
        n->List.n = that->Lvl->IntTrigger.n;
        that->Lvl->IntTrigger.n = &n->List;
        r = that->AddChar;
        that->AddChar = Add;
    }
    return r;
}
static GuiStreamTrigger *WidgetIOStreamTrigger(
    GuiWidgetIn *this,int num,char *cId,GuiStreamTrigger *Add
) {
    ThisToThat(WidgetIO,GuiWidgetIn);
    return &GuiStreamTriggerNull;
}
static void WidgetIOEnd(GuiWidgetIn *this) {
    ThisToThat(WidgetIO,GuiWidgetIn);
}

static GuiWidgetIn *WidgetIOGetIO(GuiWidgetCtx *this) {
    ThisToThat(WidgetIO,GuiWidgetCtx);
    return &that->GuiWidgetIn;
}
static void WidgetIOEnter(GuiWidgetCtx *this) {
    PrevLvl *Lvl;
    List *p,*n;
    ThisToThat(WidgetIO,GuiWidgetCtx);
    Lvl = Call(that->LvlPool,Alloc,0);
    p = &Lvl->List; n =&that->Lvl->List;
    p->n = n;
    PrevLvlInit(Lvl);
    that->Lvl = Lvl;
}
static void WidgetIOLeave(GuiWidgetCtx *this) {
    PrevLvl *Lvl;
    List *p,*n,*e;
    TlsMemPool *Trg;
    PrevTrigger *T;
    ThisToThat(WidgetIO,GuiWidgetCtx);
    Trg = that->TrgPool;
    Lvl = that->Lvl;
    p = &Lvl->List; n = p->n;
    that->Lvl = CastBack(PrevLvl,List,n);
    e = &Lvl->Trigger;
    p = e->n;
    while (p!=e) {
        T = CastBack(PrevTrigger,List,p);
        p=p->n;
        that->Triggers[T->Id]=T->Old.Trigger;
        Call(Trg,Free,1(T));
    }
    e = &Lvl->IntTrigger;
    p = e->n;
    while (p!=e) {
        T = CastBack(PrevTrigger,List,p);
        p=p->n;
        that->AddChar=T->Old.IntTrigger;
        Call(Trg,Free,1(T));
    }
    e = &Lvl->StreamTrigger;
    p = e->n;
    while (p!=e) {
        T = CastBack(PrevTrigger,List,p);
        p = p->n;
        Call(Trg,Free,1(T));
    }
    Call(that->LvlPool,Free,1(Lvl));
}

GuiWidgetCtx *GuiBoxWidgetNew(TlsMemPool *Pool) {
    static struct GuiWidgetCtx CtxStatic = {
        WidgetIOGetIO,WidgetIOEnter,WidgetIOLeave
    };
    static struct GuiInput InStatic = {
        WidgetIOKeyDown,WidgetIOKeyUp,WidgetIOCursor,WidgetIOmsTime,
        WidgetIOSuspend
    };
    static struct GuiWidgetIn Static = {
        WidgetIOBegin,WidgetIOTrigger,WidgetIOIntTrigger,
        WidgetIOStreamTrigger,WidgetIOEnd
    };
    int *p,*e,nb;
    GuiTrigger **tp,**te;
    WidgetIO *r;
    rPush(r);
    r->GuiWidgetCtx.Static = &CtxStatic;
    r->GuiWidgetIn.Static = &Static;
    r->GuiWidgetIn.GuiInput.Static = &InStatic;
    if (!Pool) {
        Pool = TlsMemPoolNew(1024);
    }
    r->LvlPool = Call(Pool,SizeSelect,1(TlsAtomSize(PrevLvl)));
    r->TrgPool = Call(Pool,SizeSelect,1(TlsAtomSize(PrevTrigger)));
    rPush(r->Lvl);
    PrevLvlInit(r->Lvl);
    r->AddChar = &GuiIntTriggerNull;
    nb = l_(Reflex,nb);
    rnPush(r->Triggers,nb);
    tp = r->Triggers; te = tp+nb;
    while (tp<te) { *tp++=&GuiTriggerNull; }
    nb = C_(Key,Scan.Fn,nb);
    rnPush(r->FnKeyBind,nb);
    p = r->FnKeyBind; e = p+nb;
    while (p<e) { *p++=l_(Reflex,_); }
    p = r->FnKeyBind;
#define dKey(a) C_(Key,Scan.Fn,a)
    p[dKey(Up)]=l_(Reflex,Up);
    p[dKey(Down)]=l_(Reflex,Down);
    p[dKey(Left)]=l_(Reflex,Left);
    p[dKey(Right)]=l_(Reflex,Right);
    p[dKey(Begin)]=l_(Reflex,Home);
    p[dKey(End)]=l_(Reflex,End);
    p[dKey(PgUp)]=l_(Reflex,PgUp);
    p[dKey(PgDown)]=l_(Reflex,PgDown);
    p[dKey(Enter)]=l_(Reflex,Enter);
    p[dKey(Esc)]=l_(Reflex,Escape);
    p[dKey(Suppr)]=l_(Reflex,Del);
    p[dKey(Insert)]=l_(Reflex,Insert);
    p[dKey(BackSpace)]=l_(Reflex,Backspace);
    p[dKey(Tab)]=l_(Reflex,Next);
    p[dKey(xLeft)]=l_(Reflex,Left);
    p[dKey(xRight)]=l_(Reflex,Right);
    p[dKey(xDown)]=l_(Reflex,Down);
    p[dKey(xUp)]=l_(Reflex,Up);
    p[dKey(xPgUp)]=l_(Reflex,PgUp);
    p[dKey(xPgDown)]=l_(Reflex,PgDown);
    p[dKey(xBegin)]=l_(Reflex,Home);
    p[dKey(xEnd)]=l_(Reflex,End);
#undef dKey
    return &r->GuiWidgetCtx;
}

/*____________________________________________________________________________
 |
 | Widget array
 |____________________________________________________________________________
*/

typedef struct {
    GuiWidget GuiWidget;
    GuiWidget **b,**e;
} WidgetArray;
static int wArrFocus(GuiWidget *this,int On) {}
static void wArrInputEnum(GuiWidget *this,GuiWidgetIn *Tgt) {
    GuiWidget **p,**e;
    ThisToThat(WidgetArray,GuiWidget);
    Call(Tgt,Begin,2(w_(Field,Array),"Array"));
    p = that->b; e = that->e;
    while (p<e) { 
        Call(*p,InputEnum,1(Tgt));
        p++;
    }
    Call(Tgt,End,0);
}
static void wArrOutputEnum(GuiWidget *this,GuiWidgetOut *Tgt) {
    GuiWidget **p,**e;
    int i;
    ThisToThat(WidgetArray,GuiWidget);
    Call(Tgt,Begin,2(w_(Field,Array),"Array"));
    p = that->b; e = that->e; i=0;
    while (p<e) {
        Call(Tgt,Widget,3(i,"",*p));
        p++; i++;
    }
    Call(Tgt,End,0);
}
static WidgetArray *wArrInit(WidgetArray *r,GuiWidget **b,GuiWidget **e) {
    static struct GuiWidget Static = {
        wArrFocus,wArrInputEnum,wArrOutputEnum
    };
    r->GuiWidget.Static = &Static;
    r->b = b; r->e = e;
    return r;
}


/*____________________________________________________________________________
 |
 | Composite Widgets
 |____________________________________________________________________________
*/

typedef struct {
    GuiWidget GuiWidget;
    GuiWidgetIn *IO,GuiWidgetIn;
    WidgetArray Vals;
    GuiWidget **c;
    GuiWidgetCtx *Ctx;
    int Hanged,ChildFocus;
    GuiTrigger *ChildEnter,*ChildEsc;
    struct {
        GuiOut **b,**e,*Cursor;
        Geo2dRectangle *Clip;
        Geo2dPoint *Org,fOrg,cOrg;
        int *Height,Focused;
    } Out;
    struct WidgListTrigger {
        GuiTrigger Previous,Next,Home,End,Esc,Enter,Validate,Cancel;
    } Triggers;
    struct WidgListField {
        TlsIntVal WidgNb,WidgCurs,Focused,cFocused;
    } Fields;
} WidgList;

static void wlIOBegin(GuiWidgetIn *this,int TypeId,char *Type) {
    ThisToThat(WidgList,GuiWidgetIn);
    Call(that->IO,Begin,2(TypeId,Type));
}
static GuiTrigger *wlIOTrigger(
    GuiWidgetIn *this,int num,char *cId,GuiTrigger *val
) {
    GuiTrigger *r;
    ThisToThat(WidgList,GuiWidgetIn);
    if (num==l_(Reflex,Enter)||num==(l_(Reflex,Escape))) {
        if (num==l_(Reflex,Enter)) {
            that->ChildEnter = val;
            r = &that->Triggers.Enter;
        } else {
            that->ChildEsc = val;
            r = &that->Triggers.Esc;
        }
    } else {
        r = Call(that->IO,Trigger,3(num,cId,val));
    }
    return r;
}
static GuiIntTrigger *wlIOIntTrigger(
    GuiWidgetIn *this,int num,char *cId,GuiIntTrigger *val
) {
    ThisToThat(WidgList,GuiWidgetIn);
    return Call(that->IO,IntTrigger,3(num,cId,val));
}
static GuiStreamTrigger *wlIOStreamTrigger(
    GuiWidgetIn *this,int num,char *cId,GuiStreamTrigger *val
){
    ThisToThat(WidgList,GuiWidgetIn);
    return Call(that->IO,StreamTrigger,3(num,cId,val));
}
static void wlIOEnd(GuiWidgetIn *this) {
    ThisToThat(WidgList,GuiWidgetIn);
    Call(that->IO,End,0);
}

inline void WidgListUnfocusChild(WidgList *L) {
    if (L->ChildFocus) {
        Call(*L->c,Focus,1(l_(Reflex.Focus,Unselected)));
        Call(L->Ctx,Leave,0);
        L->ChildEnter = L->ChildEsc = &GuiTriggerNull;
        L->ChildFocus = (0!=0);
    }
}
inline void WidgListFocusChild(WidgList *L) {
    if ((!L->ChildFocus)&&(L->c!=L->Vals.e)) {
        GuiWidget *c;
        c = *L->c;
        Call(L->Ctx,Enter,0);
        Call(c,InputEnum,1(&L->GuiWidgetIn));
        Call(c,Focus,1(l_(Reflex.Focus,Selected)));
        L->ChildFocus = (0==0);
    }
}
static void WidgListChangeChild(WidgList *L,GuiWidget **c) {
    if ((c!=L->c)&&(c>=L->Vals.b)&&(c<L->Vals.e)) {
        if (L->ChildFocus) {
            WidgListUnfocusChild(L);
            L->c = c;
            WidgListFocusChild(L);
        } else {
            L->c = c;
        }
    }
}

static int wlPreviousTrigger(GuiTrigger *this) {
    ThisToThat(WidgList,Triggers.Previous);
    WidgListChangeChild(that,that->c-1);
    return l_(Event,Processed);
}
static int wlNextTrigger(GuiTrigger *this) {
    ThisToThat(WidgList,Triggers.Next);
    WidgListChangeChild(that,that->c+1);
    return l_(Event,Processed);
}
static int wlHomeTrigger(GuiTrigger *this) {
    ThisToThat(WidgList,Triggers.Home);
    WidgListChangeChild(that,that->Vals.b);
    return l_(Event,Processed);
}
static int wlEndTrigger(GuiTrigger *this) {
    ThisToThat(WidgList,Triggers.End);
    WidgListChangeChild(that,that->Vals.e-1);
    return l_(Event,Processed);
}
static int wlEscTrigger(GuiTrigger *this) {
    int r;
    ThisToThat(WidgList,Triggers.Esc);
    r = l_(Event,Processed);
    if (that->ChildFocus) {
        r = Call(that->ChildEsc,Trigger,0);
        if (r!=l_(Event,Processed)) {
            WidgListUnfocusChild(that);
            r = l_(Event,Processed);
        }
    } else {
        r = l_(Event,Ignored);
    }
    return r;
}
static int wlEnterTrigger(GuiTrigger *this) {
    int r;
    ThisToThat(WidgList,Triggers.Enter);
    r = l_(Event,Processed);
    if (!that->ChildFocus) {
        WidgListFocusChild(that);
    } else {
        r = Call(that->ChildEnter,Trigger,0);
    }
    return r;
}
static int wlValidateTrigger(GuiTrigger *this) {
    ThisToThat(WidgList,Triggers.Validate);
    return l_(Event,Processed);
}
static int wlCancelTrigger(GuiTrigger *this) {
    ThisToThat(WidgList,Triggers.Cancel);
    return l_(Event,Processed);
}
static int wlWidgNbVal(TlsIntVal *this) {
    ThisToThat(WidgList,Fields.WidgNb);
    return that->Vals.e-that->Vals.b;
}
static int wlWidgCursVal(TlsIntVal *this) {
    ThisToThat(WidgList,Fields.WidgCurs);
    return that->c-that->Vals.b;
}
static int wlFocusedVal(TlsIntVal *this) {
    int r;
    ThisToThat(WidgList,Fields.Focused);
    r = (that->Hanged)?l_(Reflex.Focus,Unselected):l_(Reflex.Focus,Selected);
    return r;
}
static int wlcFocusedVal(TlsIntVal *this) {
    int r;
    ThisToThat(WidgList,Fields.cFocused);
    if (that->Hanged) {
        r = l_(Reflex.Focus,Unselected);
    } else {
        r = (that->ChildFocus)?
            l_(Reflex.Focus,Unselected):l_(Reflex.Focus,Selected);
    }
    return r;
}
static void wlVertInputEnum(GuiWidget *this,GuiWidgetIn *IO) {
    ThisToThat(WidgList,GuiWidget);
    Call(IO,Begin,2(w_(Field,vList),"vList"));
    Call(IO,Trigger,3(l_(Reflex,Previous),"Previous",&that->Triggers.Previous));
    Call(IO,Trigger,3(l_(Reflex,Next),"Next",&that->Triggers.Esc));
    Call(IO,Trigger,3(l_(Reflex,Up),"Up",&that->Triggers.Previous));
    Call(IO,Trigger,3(l_(Reflex,Down),"Down",&that->Triggers.Next));
    Call(IO,Trigger,3(l_(Reflex,Home),"Home",&that->Triggers.Home));
    Call(IO,Trigger,3(l_(Reflex,End),"End",&that->Triggers.End));
    Call(IO,Trigger,3(l_(Reflex,Escape),"Esc",&that->Triggers.Esc));
    Call(IO,Trigger,3(l_(Reflex,Enter),"Enter",&that->Triggers.Enter));
    Call(IO,Trigger,3(l_(Reflex,Validate),"Validate",&that->Triggers.Validate));
    Call(IO,Trigger,3(l_(Reflex,Cancel),"Cancel",&that->Triggers.Cancel));
    Call(IO,End,0);
}
static void wlVertOutputEnum(GuiWidget *this,GuiWidgetOut *Tgt) {
    ThisToThat(WidgList,GuiWidget);
    Call(Tgt,Begin,2(w_(Field,vList),"vList"));
    Call(Tgt,Array,4(
        w_(vList,Val),"Val",&that->Fields.WidgNb,&that->Vals.GuiWidget
    ));
    Call(Tgt,Int,3(w_(vList,Cursor),"Cursor",&that->Fields.WidgCurs));
    Call(Tgt,Int,3(w_(vList,Focus),"Focus",&that->Fields.Focused));
    Call(Tgt,Int,3(w_(vList,cFocus),"cFocus",&that->Fields.cFocused));
    Call(Tgt,End,0);
}
static int wlFocus(GuiWidget *this,int On) {
    ThisToThat(WidgList,GuiWidget);
    if (On==l_(Reflex.Focus,Selected)) {
        if (that->Hanged) {
            if (that->ChildFocus) {
                that->ChildFocus = (0!=0);
                WidgListFocusChild(that);
            }
            that->Hanged = (0!=0);
        }
    } else {
        if (!that->Hanged) {
            int ocf;
            ocf = that->ChildFocus;
            WidgListUnfocusChild(that);
            that->ChildFocus = ocf;
            that->Hanged = (0==0);
        }
    }
    return l_(Event,Processed);
}

#include <Gui.Term.h>

typedef struct {
    GuiOut GuiOut;
    GuiOut *Clear,*Displ;
    int sel,t0,tmax,tdiv,*focused,*t;
    Geo2dPoint o,*c;
} wlOutCursor;
static void wlOutCursorDisplay(GuiOut *this,GuiOutCtx *Ctx) {
    int t,dt,dvg;
    ThisToThat(wlOutCursor,GuiOut);
    t = *that->t;
    dt = (t-that->t0)%that->tmax;
    if ((t-that->t0)>that->tmax) {
        that->t0 = t-dt;
    }
    dvg = ((that->c->x!=that->o.x)||(that->c->y!=that->o.y));
    if (dvg||(!(*that->focused))) {
        Call(that->Clear,Display,1(Ctx));
        that->o = *that->c;
    }
    if (*that->focused) {
        that->sel = dt/that->tdiv;
        Call(that->Displ,Display,1(Ctx));
    }
}
static GuiOut *wlOutCursorNew(
    GuiTermEngine *Outp,int *t,int tmax,Geo2dPoint *pos,int *focused
) {
    wlOutCursor *r;
    static unsigned char Cursor[]= " # ";
    static TlsCharString
        sBlank = {Cursor+0,Cursor+1}, sCurs = {Cursor+1,Cursor+2};
    static struct GuiOut Static = {wlOutCursorDisplay};
    static GuiColorPair cCycle[6]={
        {1,-1},{3,-1},{2,-1},{6,-1},{4,-1},{5,-1}
    };
    GuiColorPair *cp;
    GuiOut *o,**b,**e,**p;
    rPush(r);
    r->GuiOut.Static = &Static;
    r->t = t; r->t0 = *t;
    r->tmax = tmax;
    r->tdiv = (tmax+5)/6;
    r->focused = focused;
    r->c = pos;
    r->o = *pos;
    o = Call(Outp,Ascii,1(&sBlank));
    r->Clear = GuiOutPos(&r->o,o);
    o = Call(Outp,Ascii,1(&sCurs));
    o = GuiOutPos(r->c,o);
    rnPush(b,6); e = b+6;
    p = b; cp = cCycle;
    r->sel = 0;
    while (p<e) {
        *p++ = Call(Outp,Color,2(cp,o));;
        cp++;
    }
    r->Displ = GuiOutSelect(&r->sel,b,e);
    return &r->GuiOut;
}

static void wlOutDisplay(GuiOut *this,GuiOutCtx *Ctx) {
    GuiOut **p,**e;
    int y,h,n;
    ThisToThat(WidgList,GuiWidget.GuiOut);
    p = that->Out.b; e = that->Out.e;
    y = 0;
    h = *(that->Out.Height);
    while (p<e) {
        that->Out.fOrg.y = y;
        Call(*p,Display,1(Ctx));
        y+=h; p++;
    }
    n = that->c-that->Vals.b;
    that->Out.cOrg.y = n*h;
    that->Out.Focused = (!that->Hanged)&&(!that->ChildFocus);
    Call(that->Out.Cursor,Display,1(Ctx));
}

GuiWidget *GuiVertWidgetsNew(
    GuiWidgetCtx *Ctx,GuiWidget **b,GuiWidget **e,
    GuiTermEngine *Outp,int *Height,int *time
) {
    WidgList *r;
    static struct GuiTrigger 
        Previous = {&wlPreviousTrigger}, Next = {&wlNextTrigger},
        Home = {&wlHomeTrigger}, End = {&wlEndTrigger}, 
        Esc = {&wlEscTrigger}, Enter = {&wlEnterTrigger}, 
        Validate = {&wlValidateTrigger}, Cancel = {&wlCancelTrigger};
    static struct WidgListTrigger TriggerStatic = {
        &Previous,&Next,&Home,&End,&Esc,&Enter,&Validate,&Cancel
    };
    static struct TlsIntVal WidgNb={&wlWidgNbVal}, WidgCurs={&wlWidgCursVal},
        Focused={&wlFocusedVal}, cFocused={&wlcFocusedVal};
    static struct WidgListField FieldStatic = {
        &WidgNb,&WidgCurs,&Focused,&cFocused
    };
    static struct GuiWidget Static = {wlFocus,wlVertInputEnum,wlVertOutputEnum};
    static struct GuiWidgetIn IOStatic = {
        wlIOBegin,wlIOTrigger,wlIOIntTrigger,wlIOStreamTrigger,wlIOEnd
    };
    static struct GuiOut OutStatic = {wlOutDisplay};
    rPush(r);
    r->GuiWidget.Static = &Static;
    r->GuiWidget.GuiOut.Static = &OutStatic;
    r->GuiWidgetIn.Static = &IOStatic;
    r->Triggers = TriggerStatic;
    r->Fields = FieldStatic;
    r->ChildEnter = &GuiTriggerNull;
    r->ChildEsc = &GuiTriggerNull;
    wArrInit(&r->Vals,b,e);
    r->c = r->Vals.b;
    r->Ctx = Ctx;
    r->IO = Call(Ctx,GetWidgetIn,0);
    r->Hanged = (0==0);
    r->ChildFocus = (0!=0);
    {
        int nb;
        GuiOut **o;
        GuiWidget **p;
        nb = e-b;
        r->Out.Height = Height;
        r->Out.fOrg.x = 1;
        r->Out.fOrg.y = 0;
        rnPush(r->Out.b,nb);
        r->Out.e = r->Out.b+nb;
        p = b; o = r->Out.b;
        while (p<e) {
            *o = GuiOutPos(&r->Out.fOrg,&(*p)->GuiOut);
            o++; p++;
        }
        r->Out.Focused = (0!=0);
        r->Out.cOrg.x=0; r->Out.cOrg.y=0;
        r->Out.Cursor = wlOutCursorNew( 
            Outp,time,1200,&r->Out.cOrg,&r->Out.Focused
        );
    }
    return &r->GuiWidget;
}

/*____________________________________________________________________________
 |
 |
 |____________________________________________________________________________
*/

GuiOut *GuiOutTermCursor(GuiTermEngine *E,GuiWidgetStyle *St,Geo2dPoint *O,TlsCharString *C,int *ts);

/*--------------------------------------*/

typedef struct {
    GuiWidget GuiWidget;
    GuiTermEngine *Out;
    GuiOut *oText,*oWnd,*uoWnd,*oCurs;
    Geo2dPoint Org,cOrg;
    Geo2dRectangle Clip;
    GuiWidgetStyle *Style;
    int *WndWdth,Hanged;
    TlsCharString Val,cVal;
    unsigned char *e,*c;
    struct faiWidget {
        GuiTrigger Left,Right,Home,End,Del,Bckspc;
        GuiIntTrigger Insert;
    } Trg;
    struct faiFields {
        TlsStringVal Val;
        TlsIntVal Cursor,Focus,MaxLength;
    } Fields;
} faiWidget;

static TlsCharString faiValVal(TlsStringVal *this) {
    ThisToThat(faiWidget,Fields.Val);
    return that->Val;
}
static int faiCursorVal(TlsIntVal *this) {
    ThisToThat(faiWidget,Fields.Cursor);
    return that->c-that->Val.b;
}
static int faiFocusVal(TlsIntVal *this) {
    ThisToThat(faiWidget,Fields.Focus);
    return (that->Hanged)?l_(Reflex.Focus,Unselected):l_(Reflex.Focus,Selected);
}
static int faiMaxLengthVal(TlsIntVal *this) {
    ThisToThat(faiWidget,Fields.MaxLength);
    return that->e-that->Val.b;
}

static int faiLeftTrigger(GuiTrigger *this) {
    int r;
    ThisToThat(faiWidget,Trg.Left);
    r = l_(Event,Ignored);
    if (that->c>that->Val.b) {
        that->c--;
        r = l_(Event,Processed);
    }
    return r;
}
static int faiRightTrigger(GuiTrigger *this) {
    int r;
    ThisToThat(faiWidget,Trg.Right);
    r = C_(Gui,Event,Ignored);
    if ((that->c<that->e)&&(that->c<that->Val.e)){
        that->c++;
        r = C_(Gui,Event,Processed);
    }
    return r;
}
static int faiHomeTrigger(GuiTrigger *this) {
    ThisToThat(faiWidget,Trg.Home);
    that->c = that->Val.b;
    return C_(Gui,Event,Processed);
}
static int faiEndTrigger(GuiTrigger *this) {
    ThisToThat(faiWidget,Trg.End);
    that->c = that->Val.e;
    return C_(Gui,Event,Processed);
}
static int faiDelTrigger(GuiTrigger *this) {
    int r;
    ThisToThat(faiWidget,Trg.Del);
    r = C_(Gui,Event,Ignored);
    if (that->c<that->Val.e) {
        char *p,*e;
        p = that->c;
        e = that->Val.e-1;
        while (p<e) { *p=p[1]; p++;}
        *p=0;
        that->Val.e--;
        r = C_(Gui,Event,Processed);
    }
    return r;
}
static int faiBckspcTrigger(GuiTrigger *this){
    int r;
    ThisToThat(faiWidget,Trg.Bckspc);
    r = C_(Gui,Event,Ignored);
    if (that->c>that->Val.b) {
        that->c--;
        r = faiDelTrigger(&that->Trg.Del);
    }
    return r;
}
static int faiInsertTrigger(GuiIntTrigger *this,int val) {
    int r;
    ThisToThat(faiWidget,Trg.Insert);
    r = C_(Gui,Event,Ignored);
    if ((that->Val.e<that->e)&&(val>=0x20)) {
        char *p,*e;
        e = that->c;
        p = that->Val.e;
        while (p>e) { *p=p[-1]; p--;} 
        *that->c = val;
        that->Val.e++; that->c++;
        if (that->Val.e<that->e) that->Val.e[0] = 0;
        r = l_(Event,Processed);
    } else {
        if ((val>=0x20)&&(that->c<that->e)) {
            *that->c = val;
            if ((that->c+1)<that->e) { that->c++; }
            r = l_(Event,Processed);
        }
    }
    return r;
}

static void faiDisplay(GuiOut *this,GuiOutCtx *Ctx) {
    int n,wdth;
    ThisToThat(faiWidget,GuiWidget.GuiOut);
    n = that->c - that->Val.b;
    wdth = *that->WndWdth;
    that->Clip.Extent.w = wdth;
    if (n>wdth) {
        that->Org.x = (wdth-(n+1));
    } else {
        that->Org.x = 0;
    }
    if (!that->Hanged) {
        Call(that->oWnd,Display,1(Ctx));
        if (that->c==that->Val.e) {
            that->cVal.e = that->cVal.b;
        } else {
            that->cVal.b = that->c+0;
            that->cVal.e = that->c+1;
        }
        that->cOrg.x = n;
        Call(that->oCurs,Display,1(Ctx));
    } else {
        Call(that->uoWnd,Display,1(Ctx));
    }
}
static int faiFocus(GuiWidget *this,int On) {
    ThisToThat(faiWidget,GuiWidget);
    that->Hanged = (On!=l_(Reflex.Focus,Selected));
    return l_(Event,Processed);
}
static void faiInputEnum(GuiWidget *this,GuiWidgetIn *Tgt) {
    ThisToThat(faiWidget,GuiWidget);
    Call(Tgt,Begin,2(w_(Field,StringEdit),"StringEdit"));
    Call(Tgt,Trigger,3(l_(Reflex,Left),"Left",&that->Trg.Left));
    Call(Tgt,Trigger,3(l_(Reflex,Right),"Right",&that->Trg.Right));
    Call(Tgt,Trigger,3(l_(Reflex,Home),"Home",&that->Trg.Home));
    Call(Tgt,Trigger,3(l_(Reflex,End),"End",&that->Trg.End));
    Call(Tgt,Trigger,3(l_(Reflex,Del),"Del",&that->Trg.Del));
    Call(Tgt,Trigger,3(l_(Reflex,Backspace),"Backspace",&that->Trg.Bckspc));
    Call(Tgt,IntTrigger,3(l_(Reflex.Int,AddChar),"AddChar",&that->Trg.Insert));
    Call(Tgt,End,0);
}
static void faiOutputEnum(GuiWidget *this,GuiWidgetOut *Tgt) {
    ThisToThat(faiWidget,GuiWidget);
    Call(Tgt,Begin,2(w_(Field,StringEdit),"StringEdit"));
    Call(Tgt,String,3(w_(StringEdit,Val),"Val",&that->Fields.Val));
    Call(Tgt,Int,3(w_(StringEdit,Cursor),"Cursor",&that->Fields.Cursor));
    Call(Tgt,Int,3(w_(StringEdit,Focus),"Focus",&that->Fields.Focus));
    Call(Tgt,Int,3(w_(StringEdit,MaxLength),"MaxLength",&that->Fields.MaxLength));
    Call(Tgt,End,0);
}
GuiWidget *GuiWidgetFixedAsciiInput(char *bTgt,char *eTgt,int *WndWidth,GuiTermEngine *Out,int *tms,GuiWidgetStyle *St) {
    faiWidget *r;
    static struct GuiTrigger 
        Left={faiLeftTrigger}, Right={faiRightTrigger},
        Home={faiHomeTrigger}, End={faiEndTrigger},
        Del={faiDelTrigger}, Bckspc={faiBckspcTrigger};
    static struct GuiIntTrigger Insert = {faiInsertTrigger};
    static struct faiWidget Trg = {
        {&Left},{&Right},{&Home},{&End},{&Del},{&Bckspc},{&Insert}
    };
    static struct TlsIntVal 
        CursorVal={faiCursorVal},FocusVal={faiFocusVal},
        MaxLengthVal={faiMaxLengthVal};
    static struct TlsStringVal ValVal={faiValVal};
    static struct faiFields Fields = {
        {&ValVal},{&CursorVal},{&FocusVal},{&MaxLengthVal}
    };
    static struct GuiWidget Widget = {faiFocus,faiInputEnum,faiOutputEnum};
    static struct GuiOut OutStatic = {faiDisplay};
    static struct {
        unsigned char Val[4];
        TlsCharString Str;
    } Blank;
    char *p;
    GuiOut *o;
    rPush(r);
    r->GuiWidget.Static = &Widget;
    r->GuiWidget.GuiOut.Static = &OutStatic;
    r->Trg = Trg;
    r->Fields = Fields;
    r->Val.b = bTgt;
    r->e = eTgt;
    p = bTgt; while ((*p)&&(p<eTgt)) { p++; }
    r->c = r->Val.e = p;
    r->WndWdth = WndWidth;
    r->Org.x = r->Org.y = 0;
    r->Clip.Pos = r->Org;
    r->Clip.Extent.h = 1; 
    r->Clip.Extent.w = *WndWidth;
    o = Call(Out,Ascii,1(&r->Val));
    o = Call(Out,Clear,1(o));
    o = GuiOutPos(&r->Org,o);
    r->oText = o = GuiOutClip(&r->Clip,o);
    r->oWnd = Call(Out,Color,2(&St->AsciiTextField.Selected,o));
    r->uoWnd = Call(Out,Color,2(&St->AsciiTextField.Unselected,o));
    r->cOrg.x = r->c-r->Val.b;
    r->cOrg.y = 0;
    r->cVal.b = r->cVal.e = r->c;
    r->Style = St;
    r->oCurs = GuiOutTermCursor(Out,St,&r->cOrg,&r->cVal,tms);
    r->Hanged=(0==0);
    return &r->GuiWidget;
}

/*__________________________________________________________________________
 |
 |__________________________________________________________________________
*/

typedef struct {
    GuiWidget GuiWidget;
    int *Val,Min,Max,mInc,Inc;
    struct {
        TlsCharString Displ,Curs;
        GuiOut *O,*sO,*Cursor;
        Geo2dPoint CursPos;
        Geo2dRectangle Clip;
    } Out;
    int Hanged;
    struct fnumWidget {
        GuiTrigger Left,Right,Up,Down,Home,End,Del,Bckspc;
        GuiIntTrigger Insert;
    } Trg;
    struct fnumFields {
        TlsStringVal Display;
        TlsIntVal Val,Min,Max,Cursor,Focus;
    } Fields;
} fnumWidget;

static TlsCharString fnumDisplayVal(TlsStringVal *this) {
    ThisToThat(fnumWidget,Fields.Display);
    return that->Out.Displ;
}
static int fnumValVal(TlsIntVal *this) {
    ThisToThat(fnumWidget,Fields.Val);
    return *(that->Val);
}
static int fnumMinVal(TlsIntVal *this) {
    ThisToThat(fnumWidget,Fields.Min);
    return that->Min;
}
static int fnumMaxVal(TlsIntVal *this) {
    ThisToThat(fnumWidget,Fields.Max);
    return that->Max;
}
static int fnumCursorVal(TlsIntVal *this) {
    ThisToThat(fnumWidget,Fields.Cursor);
    return that->Out.CursPos.x;
}
static int fnumFocusVal(TlsIntVal *this) {
    ThisToThat(fnumWidget,Fields.Focus);
    return (that->Hanged)?l_(Reflex.Focus,Unselected):l_(Reflex.Focus,Selected);
}

static inline int fnWUpdateVal(fnumWidget *w,int v) {
    int r;
    if (v<w->Min) v = w->Min;
    if (v>w->Max) v = w->Max;
    r = l_(Event,Ignored);
    if (v!=*(w->Val)) {
        *(w->Val) = v;
        r = l_(Event,Processed);
    }
    return r;
}

static int fnumLeftTrigger(GuiTrigger *this) {
    int r;
    ThisToThat(fnumWidget,Trg.Left);
    r = l_(Event,Ignored);
    if (that->Inc<that->mInc) {
        that->Inc = that->Inc*10;
        r = l_(Event,Processed);
    }
    return r;
}
static int fnumRightTrigger(GuiTrigger *this) {
    int r;
    ThisToThat(fnumWidget,Trg.Right);
    if (that->Inc>1) {
        that->Inc = that->Inc/10;
        r = l_(Event,Processed);
    } else {
        r = fnWUpdateVal(that,(*that->Val)*10);
    }
    return r;
}
static int fnumUpTrigger(GuiTrigger *this) {
    ThisToThat(fnumWidget,Trg.Up);
    return fnWUpdateVal(that,*that->Val+that->Inc);
}
static int fnumDownTrigger(GuiTrigger *this) {
    ThisToThat(fnumWidget,Trg.Down);
    return fnWUpdateVal(that,*that->Val-that->Inc);
}
static int fnumHomeTrigger(GuiTrigger *this) {
    int r,v,av,i,ni,mInc;
    ThisToThat(fnumWidget,Trg.Home);
    r = l_(Event,Ignored);
    v = *that->Val;
    av = (v>=0)?v:-v;
    mInc = that->mInc;
    i = 1; ni = 10;
    while ((ni<=av)&&(i<mInc)) {
        i = ni; 
        ni = ni*10;
    }
    if (i!=that->Inc) {
        that->Inc = i;
        r = l_(Event,Processed);
    }
    return r;
}
static int fnumEndTrigger(GuiTrigger *this) {
    int r;
    ThisToThat(fnumWidget,Trg.End);
    r = l_(Event,Ignored);
    if  (that->Inc!=1) {
        that->Inc=1;
        r = l_(Event,Processed);
    }
    return r;
}
static int fnumDelTrigger(GuiTrigger *this) {
    int r,v,va,inc;
    ThisToThat(fnumWidget,Trg.Del);
    r = l_(Event,Ignored);
    v = *that->Val; va = (v>=0)?v:-v;
    inc = that->Inc;
    if (inc>va) {
        if (inc>1) {
            that->Inc = inc/10;
            r = l_(Event,Processed);
        }
    } else {
        int h,l;
        l = va%inc;
        h = (va/(inc*10))*inc;
        va = l+h;
        v = (v>=0)?va:-va;
        r = fnWUpdateVal(that,v);
        if (r==l_(Event,Processed)) {
            if (inc>1) that->Inc=inc/10;
        }
    }
    return r;
}
static int fnumBckspcTrigger(GuiTrigger *this) {
    int r,inc,v,av;
    ThisToThat(fnumWidget,Trg.Bckspc);
    r = l_(Event,Ignored);
    v = *that->Val; av=(v>=0)?v:-v;
    inc = that->Inc;
    if (inc<=(av/10)) {
        that->Inc = inc*10;
        r = Call(&that->Trg.Del,Trigger,0);
    }
    return r;
}
static int fnumInsertTrigger(GuiIntTrigger *this,int c) {
    int r;
    ThisToThat(fnumWidget,Trg.Insert);
    r = l_(Event,Ignored);
    if (c=='-') {
        r = fnWUpdateVal(that,-(*that->Val));
    } else {
        if ((c>='0')&&(c<='9')) {
            int v,va,h,l,m,inc;
            inc = that->Inc;
            v = *that->Val;
            va = (v>=0)?v:-v;
            h = va - (va%(inc*10));
            m = inc*(c-'0');
            l = va%inc;
            va = h+l+m;
            v = (v>=0)?va:-va;
            r = fnWUpdateVal(that,v);
            if ((r==l_(Event,Processed))&&(inc>1)) { that->Inc=inc/10; }
        }
    }
    return r;
}

static int fnumFocus(GuiWidget *this,int On) {
    ThisToThat(fnumWidget,GuiWidget);
    that->Hanged = (On!=l_(Reflex.Focus,Selected));
    return l_(Event,Processed);
}
static void fnumInputEnum(GuiWidget *this,GuiWidgetIn *Tgt) {
    ThisToThat(fnumWidget,GuiWidget);
    Call(Tgt,Begin,2(w_(Field,IntEdit),"IntEdit"));
    Call(Tgt,Trigger,3(l_(Reflex,Left),"Left",&that->Trg.Left));
    Call(Tgt,Trigger,3(l_(Reflex,Right),"Right",&that->Trg.Right));
    Call(Tgt,Trigger,3(l_(Reflex,Up),"Up",&that->Trg.Up));
    Call(Tgt,Trigger,3(l_(Reflex,Down),"Down",&that->Trg.Down));
    Call(Tgt,Trigger,3(l_(Reflex,Home),"Home",&that->Trg.Home));
    Call(Tgt,Trigger,3(l_(Reflex,End),"End",&that->Trg.End));
    Call(Tgt,Trigger,3(l_(Reflex,Del),"Del",&that->Trg.Del));
    Call(Tgt,Trigger,3(l_(Reflex,Backspace),"Backspace",&that->Trg.Bckspc));
    Call(Tgt,IntTrigger,3(l_(Reflex.Int,AddChar),"AddChar",&that->Trg.Insert));
    Call(Tgt,End,0);
}
static void fnumOutputEnum(GuiWidget *this,GuiWidgetOut *Tgt) {
    ThisToThat(fnumWidget,GuiWidget);
    Call(Tgt,Begin,2(w_(Field,IntEdit),"IntEdit"));
    Call(Tgt,String,3(w_(IntEdit,Display),"Display",&that->Fields.Display));
    Call(Tgt,Int,3(w_(IntEdit,Val),"Val",&that->Fields.Val));
    Call(Tgt,Int,3(w_(IntEdit,Min),"Min",&that->Fields.Min));
    Call(Tgt,Int,3(w_(IntEdit,Max),"Max",&that->Fields.Max));
    Call(Tgt,Int,3(w_(IntEdit,Cursor),"Cursor",&that->Fields.Cursor));
    Call(Tgt,Int,3(w_(IntEdit,Focus),"Focus",&that->Fields.Focus));
    Call(Tgt,End,0);
}
static void fnumWidgetDisplay(GuiOut *this,GuiOutCtx *Ctx) {
    unsigned char *p,*q,*e,*bp;
    int v,d,inc,i,c;
    ThisToThat(fnumWidget,GuiWidget.GuiOut);
    v = *that->Val;
    p = that->Out.Displ.b;
    inc = that->Inc;
    if (v<0) { *p++='-'; v=-v; }
    bp = p;
    d = v%10; v=v/10;
    *p++ = '0'+d; i=1; c=0;
    while (v) {
        i=i*10;
        if (i==inc) c=(p-bp);
        d = v%10; v=v/10; *p++='0'+d;
    }
    while (i<inc) { 
        i=i*10; if (i==inc) c=(p-bp);
        *p++='0'; 
    }
    e = p; *e = 0;
    p--; q=bp; while (q<p) { d=*q; *q=*p; *p=d; q++; p--; }
    that->Out.Displ.e = e;
    that->Out.Curs.b = (e-1)-c;
    that->Out.Curs.e = (e-c);
    that->Out.CursPos.x = ((e-1)-that->Out.Displ.b)-c ;
    if (that->Hanged) {
        Call(that->Out.O,Display,1(Ctx));
    } else {
        Call(that->Out.sO,Display,1(Ctx));
        Call(that->Out.Cursor,Display,1(Ctx));
    }
}

GuiWidget *GuiNumFieldNew(
    int *Val,int Min,int Max,GuiTermEngine *Outp,int *tms,GuiWidgetStyle *St
) {
    static struct GuiWidget Static = {fnumFocus,fnumInputEnum,fnumOutputEnum};
    static struct GuiOut OutStatic = {fnumWidgetDisplay};
    static struct GuiTrigger Left={fnumLeftTrigger}, Right={fnumRightTrigger},
        Up={fnumUpTrigger}, Down={fnumDownTrigger}, Home={fnumHomeTrigger},
        End={fnumEndTrigger}, Del={fnumDelTrigger}, Bckspc={fnumBckspcTrigger};
    static struct GuiIntTrigger Insert = {fnumInsertTrigger};
    static struct fnumWidget TrgStatic = {
        &Left,&Right,&Up,&Down,&Home,&End,&Del,&Bckspc,
        &Insert
    };
    static struct TlsIntVal ValVal={fnumValVal},MinVal={fnumMinVal},
        MaxVal={fnumMaxVal},CursorVal={fnumCursorVal},FocusVal={fnumFocusVal};
    static struct TlsStringVal DisplayVal={fnumDisplayVal};
    static struct fnumFields Fields = {
        {&DisplayVal},{&ValVal},{&MinVal},{&MaxVal},{&CursorVal},{&FocusVal}
    };
    int v,v0,l,dv;
    char *cDisp;
    fnumWidget *r;
    rPush(r);
    r->GuiWidget.Static = &Static;
    r->GuiWidget.GuiOut.Static = &OutStatic;
    r->Trg = TrgStatic;
    r->Fields = Fields;
    r->Hanged = (0==0);
    r->Val=Val; r->Min=Min; r->Max=Max;
    v = (Min<0)?-Min:Min;
    v0 = (Max<0)?-Max:Max;
    v = (v>v0)?v:v0;
    l = 1; dv=1; v0=v/10;
    while (dv<=v0) { l++; dv=dv*10; }
    r->Inc=1; r->mInc=dv;
    l += (Min<0)?2:1;
    /* Output */ {
        GuiOut *o,*t;
        rnPush(cDisp,TlsRoundUp(int,l));
        cDisp[0] = 0;
        r->Out.CursPos.x = r->Out.CursPos.y = 0;
        r->Out.Clip.Pos.x = r->Out.Clip.Pos.y = 0;
        r->Out.Clip.Extent.h = 1; r->Out.Clip.Extent.w = l-1;
        r->Out.Displ.b = cDisp; r->Out.Displ.e = cDisp+1;
        r->Out.Curs.b = r->Out.Curs.e = cDisp;
        t = Call(Outp,Ascii,1(&r->Out.Displ));
        t = Call(Outp,Clear,1(t));
        t = GuiOutClip(&r->Out.Clip,t);
        r->Out.sO = Call(Outp,Color,2(&St->AsciiTextField.Selected,t));
        r->Out.O = Call(Outp,Color,2(&St->AsciiTextField.Unselected,t));
        r->Out.Cursor = GuiOutTermCursor(Outp,St,&r->Out.CursPos,&r->Out.Curs,
            tms);
    }
    return &r->GuiWidget;
}
