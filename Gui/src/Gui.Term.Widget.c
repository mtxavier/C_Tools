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
#include <GuiWidget.h>
#include <Gui.Term.h>
#include <Geo.h>

#define l_(a,b) C_(Gui,a,b)
#define w_(a,b) C_(Widget,a,b)

/*____________________________________________________________________________
 |
 |____________________________________________________________________________
*/

typedef struct { struct GuiOutConstr *Static; } GuiOutConstr;
struct GuiOutConstr {
    GuiOut *(*Look)(GuiOutConstr *this,int tId,char *ctId,GuiWidget *Widg);
};

/*____________________________________________________________________________
 |
 |____________________________________________________________________________
*/

static void IntIgnore(GuiWidgetOut *this,int id,char *cid,TlsIntVal *Val) {}
static void StringIgnore(GuiWidgetOut *this,int id,char *cid,TlsStringVal *Val){}
static void WidgetIgnore(GuiWidgetOut *this,int id,char *cid,GuiWidget *Val){}
static void ArrayIgnore(
    GuiWidgetOut *this,int id,char *cid,TlsIntVal *Nb,GuiWidget *Vals
) {}

/*____________________________________________________________________________
 |
 |____________________________________________________________________________
*/

typedef struct {
    GuiOut GuiOut;
    GuiWidgetStyle *Style;
    int *tm,otm;
    TlsCharString *C;
    GuiOut *oC,*oE;
} oCursor;
static void oCursorDisplay(GuiOut *this,GuiOutCtx *Ctx) {
    int dt,tm;
    GuiWidgetStyle *st;
    ThisToThat(oCursor,GuiOut);
    st = that->Style;
    tm = *that->tm; // Be careful here since *that->tm is potentially volatil
    dt = tm-that->otm;
    if (dt>st->AsciiTextField.CursorBlink.t2) {
        dt = dt%st->AsciiTextField.CursorBlink.t2;
        that->otm = tm-dt;
    }
    if (dt<st->AsciiTextField.CursorBlink.t1) {
        if (that->C->e>that->C->b) {
            Call(that->oC,Display,1(Ctx));
        } else {
            Call(that->oE,Display,1(Ctx));
        }
    }
}
GuiOut *GuiOutTermCursor(GuiTermEngine *E,GuiWidgetStyle *St,Geo2dPoint *O,TlsCharString *C,int *ts) {
    oCursor *r;
    GuiOut *t;
    static struct GuiOut Static = {oCursorDisplay};
    static unsigned char EmptyVal[]="   ";
    static TlsCharString Empty = {EmptyVal+0,EmptyVal+1};
    rPush(r);
    r->GuiOut .Static = &Static;
    r->tm = ts;
    r->otm = *ts;
    r->C = C;
    r->Style = St;
    t = Call(E,Ascii,1(C));
    t = GuiOutPos(O,t);
    r->oC = Call(E,Color,2(&St->AsciiTextField.Cursor0,t));
    t = Call(E,Ascii,1(&Empty));
    t = GuiOutPos(O,t);
    r->oE = Call(E,Color,2(&St->AsciiTextField.Cursor1,t));
    return &r->GuiOut;
}

/*____________________________________________________________________
 |
 |
 |____________________________________________________________________
*/

static void UninitedDisplay(GuiOut *this,GuiOutCtx *Ctx) {}
static struct GuiOut UninitedStatic = {&UninitedDisplay};

typedef struct {
    GuiWidgetOut GuiWidgetOut,Array;
    GuiOut GuiOut;
    Geo2dPoint Org,cOrg;
    Geo2dExtent Clip;
    TlsIntVal *Nb;
    GuiOut **b,**e,**cursOut;
    TlsIntVal *Cursor,*Focus,*cFocus,*Height;
} OutvList;

static void ArrvListWidget(GuiWidgetOut *this,int id,char *cid,GuiWidget *Val){
    ThisToThat(OutvList,Array);

}
static void OutvListDisplay(GuiOut *this,GuiOutCtx *Ctx) {
    int y,h,foc,curs;
    GuiOut **p,**e;
    ThisToThat(OutvList,GuiOut);
    y = 0; 
    h = Call(that->Height,Val,0);
    p = that->b; e=that->e;
    while (p<e) {
        that->Org.y = y;
        Call(*p,Display,1(Ctx));
        y+=h; p++;
    }
    foc = Call(that->Focus,Val,0);
    if (foc==l_(Reflex.Focus,Selected)) {
        foc = Call(that->cFocus,Val,0);
        if  (foc==l_(Reflex.Focus,Selected)) {
            curs = Call(that->Cursor,Val,0);
            that->cOrg.y = h*curs;
            Call(*that->cursOut,Display,1(Ctx));
        }
    }
}
static void OutvListOpen(GuiWidgetOut *this,int id,char *cid) { }
static void OutvListInt(GuiWidgetOut *this,int id,char *cid,TlsIntVal *Val) {
    ThisToThat(OutvList,GuiWidgetOut);
    switch (id) {
        case w_(vList,Cursor): that->Cursor=Val; break;
        case w_(vList,Focus): that->Focus=Val; break;
        case w_(vList,cFocus): that->cFocus=Val; break;
    }
}
static void OutvListArray(
    GuiWidgetOut *this,int id,char *cid,TlsIntVal *Nb,GuiWidget *Vals
) {
    int nb;
    ThisToThat(OutvList,GuiWidgetOut);
    if (id==w_(vList,Val)) {
        that->Nb = Nb;
        rnPush(that->b,nb);
        that->e=that->b+nb;
        that->cursOut=that->b;
        Call(Vals,OutputEnum,1(this));
    }
}
static void OutvListClose(GuiWidgetOut *this) {}


/*-----------------------------------*/

typedef struct {
    GuiOut GuiOut;
} OutStringEdit;

/*-----------------------------------*/

typedef struct {
    GuiOut GuiOut;
} OutIntEdit;

/*____________________________________________________________________
 |
 |
 |____________________________________________________________________
*/

typedef struct {
    GuiWidgetOut GuiWidgetOut;
    GuiOut GuiOut;
    struct {
        GuiWidgetOut StringEdit,IntEdit,vList;
    } Ctx;
    GuiWidgetOut *c;
    GuiTermEngine *E;
} TermOutCtx;

