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
#include <Tools.h>
#include <Gui.Term.h>

/*_____________________________________________________________________
 |
 |
 |_____________________________________________________________________
*/

typedef struct {
    GuiTermEngine GuiTermEngine;
    GuiOutCtx Ctx;
    GuiTermCtrl *TermCtrl;
    GuiTerm *Term;
    GuiTermInfo *TermInfo;
    GuiColorPair Color;
    int fgnb;
} TermEngine;

/*_____________________________________________________________________
 |
 |
 |_____________________________________________________________________
*/

typedef struct {
    GuiOut GuiOut;
    TermEngine *E;
    GuiOut *Child;
} UnclipOut;
static void UnclipOutDisplay(GuiOut *this,GuiOutCtx *Ctx) {
    Geo2dRectangle *Ar,o;
    ThisToThat(UnclipOut,GuiOut);
    Ar = &that->E->Ctx.ViewPort;
    Ar->Pos.x = Ar->Pos.y = 0;
    Ar->Extent = that->E->TermInfo->Dim;
    Call(that->Child,Display,1(&that->E->Ctx));
}
static Geo2dRectangle UnclipOutDomain(GuiOut *this) {
    ThisToThat(UnclipOut,GuiOut);
    return that->E->Ctx.ViewPort;
}
static GuiOut *TermViewPort(GuiTermEngine *this,GuiOut *c) {
    UnclipOut *R;
    static struct GuiOut UnclipStatic = {UnclipOutDisplay,UnclipOutDomain};
    ThisToThat(TermEngine,GuiTermEngine);
    rPush(R);
    R->GuiOut.Static = &UnclipStatic;
    R->E = that;
    R->Child = c;
    return &R->GuiOut;
}


/*-----*/

typedef struct {
    GuiOut GuiOut;
    TermEngine *E;
    Geo2dRectangle Area;
    GuiOut *Child;
} ClipOut;
static void ClipOutDisplay(GuiOut *this,GuiOutCtx *Ctx) {
    Geo2dRectangle o;
    TermEngine *E;
    ThisToThat(ClipOut,GuiOut);
    E = that->E;
    { 
       Geo2dRectangle *Ar,c;
       Ar = &E->Ctx.ViewPort;
       o = *Ar; c = that->Area;
       c.Pos.x+=E->Ctx.Pos.x;
       c.Pos.y+=E->Ctx.Pos.y;
       Geo2dRectangleClip((*Ar),o,c);
    }
    Call(that->Child,Display,1(Ctx));
    that->E->Ctx.ViewPort = o;
}
static Geo2dRectangle ClipOutDomain(GuiOut *this) {
    Geo2dRectangle r,a;
    ThisToThat(ClipOut,GuiOut);
    a = Call(that->Child,Domain,0);
    Geo2dRectangleClip(r,a,that->Area);
    return r;
}
static GuiOut *TermClip(GuiTermEngine *this,Geo2dRectangle *rect,
    GuiOut *c
) {
    GuiOut *r;
    static struct GuiOut Static = {ClipOutDisplay,ClipOutDomain};
    if (!rect) {
        r = TermViewPort(this,c);
    } else {
        ClipOut *R;
        ThisToThat(TermEngine,GuiTermEngine);
        rPush(R);
        R->GuiOut.Static = &Static;
        R->E = that;
        R->Area = *rect;
        R->Child = c;
        r = &R->GuiOut;
    }
    return r;
}

/*--------------------*/

typedef struct {
    GuiOut GuiOut;
    GuiColorPair Color,*v;
    TermEngine *E;
    GuiOut *Child;
} ColorOut;
static void ColorOutDisplay(GuiOut *this,GuiOutCtx *Ctx) {
    GuiColorPair o,n,*c;
    ThisToThat(ColorOut,GuiOut);
    n = that->Color;
    if ((that->v->fg!=n.fg)||(that->v->bg!=n.bg)){
        n = *that->v;
        if (n.bg>0) {
            n.bg = n.bg*that->E->fgnb;
        }
        that->Color=n;
    }
    c = &that->E->Color;
    o = *c;
    if (n.fg==-1) { n.fg = o.fg; }
    if (n.bg==-1) { n.bg = o.bg; }
    *c = n;
    Call(that->E->Term,ColorSet,1(n.fg+n.bg));
    Call(that->Child,Display,1(Ctx));
    Call(that->E->Term,ColorSet,1(o.fg+o.bg));
    *c = o;
}
static Geo2dRectangle ColorOutDomain(GuiOut *this) {
    ThisToThat(ColorOut,GuiOut);
    return Call(that->Child,Domain,0);
}
static GuiOut *TermColor(GuiTermEngine *this,GuiColorPair *clr,GuiOut *c) {
    ColorOut *r;
    static struct GuiOut Static = {ColorOutDisplay,ColorOutDomain};
    ThisToThat(TermEngine,GuiTermEngine);
    rPush(r);
    r->GuiOut.Static = &Static;
    r->E = that;
    r->v = clr;
    r->Color = *clr;
    if (clr->bg>0) {
        r->Color.bg = clr->bg*that->fgnb;
    }
    r->Child = c;
    return &r->GuiOut;
}

/*--------------------*/

typedef struct {
    GuiOut GuiOut;
    TermEngine *Engine;
    TlsCharString *String;
} OutString;
static void OutStringDisplay(GuiOut *this,GuiOutCtx *Ctx)  {
    int y;
    TermEngine *E;
    ThisToThat(OutString,GuiOut);
    E = that->Engine;
    y = E->Ctx.Pos.y;
    if ((y>=E->Ctx.ViewPort.Pos.y)&&(y<E->Ctx.ViewPort.Pos.y+E->Ctx.ViewPort.Extent.h)) {
        unsigned char *b,*e,*ce,hack;
        int x0,x1,bx,ex;
        b = that->String->b;
        ce = e = that->String->e;
        x0 = E->Ctx.Pos.x; x1 = x0+e-b;
        bx = E->Ctx.ViewPort.Pos.x; ex = bx+E->Ctx.ViewPort.Extent.w;
        if (bx>x0) { b+=(bx-x0); x0 = bx;}
        if (ex<x1) { ce-=(x1-ex); }
        if (b<ce) {
            hack = *ce; *ce=0;
            Call(E->Term,AddStr,3(x0,y,b));
            *ce = hack;
        }
    }
}
static Geo2dRectangle OutStringDomain(GuiOut *this) {
    Geo2dRectangle r;
    ThisToThat(OutString,GuiOut);
    r.Pos.x=r.Pos.y=0;
    r.Extent.h = 1;
    r.Extent.w = that->String->e-that->String->b;
    return r;
}
static GuiOut *TermAscii(GuiTermEngine *this,TlsCharString *String) {
    OutString *r;
    static struct GuiOut Static = {OutStringDisplay,OutStringDomain};
    ThisToThat(TermEngine,GuiTermEngine);
    rPush(r);
    r->GuiOut.Static = &Static;
    r->Engine = that;
    r->String = String;
    return &r->GuiOut;
}

/*-----------------*/

typedef struct {
    GuiOut GuiOut;
    TermEngine *Engine;
    GuiOut *Child;
} OutClear;

static void ClearDisplay(GuiOut *this,GuiOutCtx *Ctx) {
    TermEngine *E;
    int w;
    ThisToThat(OutClear,GuiOut);
    E = that->Engine;
    w = E->Ctx.ViewPort.Extent.w;
    if (w>=1) {
        char *wh,*p,*e;
        int rw,h,x,y,ye;
        GuiTerm *t;
        rw = TlsRoundUp(int,(w+1));
        t = that->Engine->Term;
        rOpen
            rnPush(wh,rw);
            p = wh; e = p+w;
            while (p<e) { *p++ = ' '; }
            *p = 0;
            x = E->Ctx.ViewPort.Pos.x;
            y = E->Ctx.ViewPort.Pos.y;
            ye = y+E->Ctx.ViewPort.Extent.h;
            while (y<ye) {
                Call(t,AddStr,3(x,y,wh));
                y++;
            }
        rClose
        Call(that->Child,Display,1(Ctx));
    }
}
static Geo2dRectangle ClearDomain(GuiOut *this) {
    Geo2dRectangle r;
    r.Extent.w = r.Extent.h = TlsMaxSignedVal(int);
    r.Pos.x = r.Pos.y = (r.Extent.w>>1)-(r.Extent.w);
    return r;
}
static GuiOut *TermClear(GuiTermEngine *this,GuiOut *c) {
    OutClear *r;
    static struct GuiOut Static = {ClearDisplay,ClearDomain};
    ThisToThat(TermEngine,GuiTermEngine);
    rPush(r);
    r->GuiOut.Static = &Static;
    r->Engine = that;
    r->Child = c;
    return &r->GuiOut;
}

/*----------------*/

GuiTermEngine *GuiTermEngineNew(GuiTermCtrl *Display,int fgnb) {
    TermEngine *r;
    static struct GuiTermEngine Static = {
        TermViewPort,TermColor,TermAscii,TermClear
    };
    rPush(r);
    r->GuiTermEngine.Static = &Static;
    r->TermCtrl = Display;
    r->Term = Call(Display,GetViewPort,0);
    r->TermInfo = Call(Display,GetInfo,0);
    r->Ctx.ViewPort.Pos.x = r->Ctx.ViewPort.Pos.y = 0;
    r->Ctx.ViewPort.Extent = r->TermInfo->Dim;
    r->Ctx.Pos.x = r->Ctx.Pos.y = 0;
    r->fgnb = fgnb; r->Color.fg = 1; r->Color.bg = 0;
    return &r->GuiTermEngine;
}


