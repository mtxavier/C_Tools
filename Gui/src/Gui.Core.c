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
#include <Gui.Core.h>
#include <stdarg.h>

#define l_(a,b) C_(Gui,a,b)

/*------------------------------*/

void OutNullDisplay(GuiOut *this,GuiOutCtx *Ctx) {}
static struct GuiOut GuiOutNullStatic = {&OutNullDisplay};
GuiOut GuiOutNull;

GuiOutCtx GuiOutCtxNull;

/*-----------------------------*/

static int InNullKeyDown(GuiInput *this,int device,int key) { 
    return l_(Event,Ignored); 
}
static int InNullKeyUp(GuiInput *this,int device,int key) {
    return l_(Event,Ignored);
}
static int InNullCursor(GuiInput *this,int device,int x,int y) {
    return l_(Event,Ignored);
}
static int InNullmsTime(GuiInput *this,int *next,int current) {
    return l_(Event,Ignored);
}
static int InNullSuspend(GuiInput *this,int evt) {
    return l_(Event,Processed);
}
static struct GuiInput InNullStatic = {
    InNullKeyDown,InNullKeyUp,InNullCursor,InNullmsTime,InNullSuspend
};
GuiInput GuiInputNull={&InNullStatic};

/*-----------------------------*/

static inline Geo2dRectangle GlobalDomain(GuiOut **p,GuiOut **e) {
    Geo2dRectangle r,d0,d1;
    if (p==e) {
        r.Pos.x = r.Pos.y = r.Extent.w = r.Extent.h = 0;
    } else {
        r = Call(*p,Domain,0);
        p++;
        while (p<e) {
            d0 = r;
            d1 = Call(*p,Domain,0);
            p++;
            Geo2dRectangleExtend(r,d0,d1);
        }
    }
    return r;
}

/*------------------------------*/

typedef struct {
    GuiOut GuiOut;
    GuiOut *Children[2];
} OutPair;
static void OutPairDisplay(GuiOut *this,GuiOutCtx *Ctx) {
    ThisToThat(OutPair,GuiOut);
    Call(that->Children[0],Display,1(Ctx));
    Call(that->Children[1],Display,1(Ctx));
}
static Geo2dRectangle OutPairDomain(GuiOut *this) {
    Geo2dRectangle r,d0,d1;
    ThisToThat(OutPair,GuiOut);
    d0 = Call(that->Children[0],Domain,0);
    d1 = Call(that->Children[1],Domain,0);
    Geo2dRectangleExtend(r,d0,d1);
    return r;
}
GuiOut *GuiOutTrmPair(GuiOut *c0,GuiOut *c1) {
    OutPair *r;
    static struct GuiOut Static = {OutPairDisplay,OutPairDomain};
    rPush(r);
    r->GuiOut.Static = &Static;
    r->Children[0] = c0;
    r->Children[1] = c1;
    return &r->GuiOut;
}

/*------------------------------*/

typedef struct {
    GuiOut GuiOut;
    GuiOut **b,**e;
} OutSequence;
static void OutSequenceDisplay(GuiOut *this,GuiOutCtx *Ctx) {
    GuiOut **p,**e;
    ThisToThat(OutSequence,GuiOut);
    p = that->b; e = that->e;
    while (p<e) {
        Call(*p,Display,1(Ctx)); 
        p++;
    }
}
static Geo2dRectangle OutSequenceDomain(GuiOut *this) {
    GuiOut **p,**e;
    ThisToThat(OutSequence,GuiOut);
    return GlobalDomain(that->b,that->e);
}
GuiOut *GuiOutSequence(GuiOut **b,GuiOut **e) {
    OutSequence *r;
    static struct GuiOut Static= {OutSequenceDisplay,OutSequenceDomain};
    rPush(r); 
    r->GuiOut.Static = &Static;
    r->b = b; 
    r->e = e;
    return &r->GuiOut;
}

GuiOut *GuiOutSeq(GuiOut *fst,...) {
    int nb;
    GuiOut **b,**e,**q,*p;
    va_list vl;
    nb = 0;
    p  = fst;
    va_start(vl,fst);
    while (p!=0) {
        nb++;
        p = va_arg(vl,GuiOut *);
    }
    va_end(vl);
    rnPush(b,nb); 
    e=b+nb;
    q = b; p = fst;
    va_start(vl,fst);
    while (q<e) { 
        *q++=p; 
        p = va_arg(vl,GuiOut *);
    }
    va_end(vl);
    return GuiOutSequence(b,e);
}

/*--------------------------*/

typedef struct {
    GuiOut GuiOut;
    GuiOut **b;
    int *Select,nb;
} OutSelect;
static void OutSelectDisplay(GuiOut *this,GuiOutCtx *Ctx) {
    int sel;
    ThisToThat(OutSelect,GuiOut);
    sel = *that->Select;
    if ((sel>=0)&&(sel<that->nb)) {
        Call(that->b[sel],Display,1(Ctx));
    }
}
static Geo2dRectangle OutSelectDomain(GuiOut *this) {
    ThisToThat(OutSelect,GuiOut);
    return GlobalDomain(that->b,that->b+that->nb);
}
GuiOut *GuiOutSelect(int *Sel,GuiOut **b,GuiOut **e) {
    OutSelect *r;
    static struct GuiOut Static = {OutSelectDisplay,OutSelectDomain};
    rPush(r);
    r->GuiOut.Static = &Static;
    r->b = b;
    r->nb = e-b;
    r->Select = Sel;
    return &r->GuiOut;
}
GuiOut *GuiOutSel(int *Sel,...) {
    va_list vl;
    int nb;
    GuiOut **b,**e,*p,**q;
    nb = 0;
    va_start(vl,Sel);
    p = va_arg(vl,GuiOut *);
    while (p) {
        nb++;
        p = va_arg(vl,GuiOut *);
    }
    va_end(vl);
    rnPush(b,nb);
    e = b+nb; q = b;
    va_start(vl,Sel);
    while (q<e) {
        p = va_arg(vl,GuiOut *);
        *q++=p;
    }
    va_end(vl);
    return GuiOutSelect(Sel,b,e);
}

/*____________________________________________________________________
 |
 |
 |____________________________________________________________________
*/

typedef struct {
    GuiOut GuiOut;
    Geo2dPoint *Org;
    GuiOut *Child;
} OutOrg;
static void OutPosDisplay(GuiOut *this,GuiOutCtx *Ctx) {
    Geo2dPoint o;
    ThisToThat(OutOrg,GuiOut);
    o = Ctx->Pos;
    Ctx->Pos.x += that->Org->x;
    Ctx->Pos.y += that->Org->y;
    Call(that->Child,Display,1(Ctx));
    Ctx->Pos = o;
}
static Geo2dRectangle OutPosDomain(GuiOut *this) {
    Geo2dRectangle r;
    ThisToThat(OutOrg,GuiOut);
    r = Call(that->Child,Domain,0);
    r.Pos.x += that->Org->x;
    r.Pos.y += that->Org->y;
    return r;
}
GuiOut *GuiOutPos(Geo2dPoint *Pos,GuiOut *Child) {
    OutOrg *r;
    static struct GuiOut Static = {OutPosDisplay,OutPosDomain};
    rPush(r);
    r->GuiOut.Static = &Static;
    r->Org = Pos;
    r->Child = Child;
    return &r->GuiOut;
}

/*----------------------------*/

static void OutOrgDisplay(GuiOut *this,GuiOutCtx *Ctx) {
    Geo2dPoint o;
    ThisToThat(OutOrg,GuiOut);
    o = Ctx->Pos;
    Ctx->Pos.x -= that->Org->x;
    Ctx->Pos.y -= that->Org->y;
    Call(that->Child,Display,1(Ctx));
    Ctx->Pos = o;
}
static Geo2dRectangle OutOrgDomain(GuiOut *this) {
    Geo2dRectangle r;
    ThisToThat(OutOrg,GuiOut);
    r = Call(that->Child,Domain,0);
    r.Pos.x -= that->Org->x;
    r.Pos.y -= that->Org->y;
    return r;
}
GuiOut *GuiOutOrg(Geo2dPoint *Org,GuiOut *Child) {
    OutOrg *r;
    static struct GuiOut Static = {OutOrgDisplay,OutOrgDomain};
    rPush(r);
    r->GuiOut.Static = &Static;
    r->Org = Org;
    r->Child = Child;
    return &r->GuiOut;
}

/*______________________________________________
 |
 |
 |______________________________________________
*/

typedef struct {
    GuiOut GuiOut;
    GuiOut *Child;
    Geo2dRectangle *Clip;
} OutClip;

static void OutClipDisplay(GuiOut *this,GuiOutCtx *Ctx) {
    Geo2dRectangle o,n;
    ThisToThat(OutClip,GuiOut);
    o = Ctx->ViewPort;
    n = *that->Clip;
    n.Pos.x += Ctx->Pos.x;
    n.Pos.y += Ctx->Pos.y;
    Geo2dRectangleClip(Ctx->ViewPort,n,o);
    Call(that->Child,Display,1(Ctx));
    Ctx->ViewPort = o;
}
static Geo2dRectangle OutClipDomain(GuiOut *this) {
    ThisToThat(OutClip,GuiOut);
    return *that->Clip;
}
GuiOut *GuiOutClip(Geo2dRectangle *Clip,GuiOut *c) {
    OutClip *r;
    static struct GuiOut Static = {OutClipDisplay,OutClipDomain};
    rPush(r);
    r->GuiOut.Static = &Static;
    r->Clip = Clip;
    r->Child = c;
    return &r->GuiOut;
}

/*---------------*/

static void OutFrameDisplay(GuiOut *this,GuiOutCtx *Ctx) {
    Geo2dRectangle o,n;
    ThisToThat(OutClip,GuiOut);
    o = Ctx->ViewPort;
    n.Extent = that->Clip->Extent;
    n.Pos.x = -that->Clip->Pos.x;
    n.Pos.y = -that->Clip->Pos.y;
    Geo2dRectangleClip(Ctx->ViewPort,n,o);
    Call(that->Child,Display,1(Ctx));
    Ctx->ViewPort = o;
}
static Geo2dRectangle OutFrameDomain(GuiOut *this) {
    Geo2dRectangle r;
    ThisToThat(OutClip,GuiOut);
    r.Extent = that->Clip->Extent;
    r.Pos.x = -that->Clip->Pos.x;
    r.Pos.y = -that->Clip->Pos.y;
    return r;
}
GuiOut *GuiOutPage(Geo2dRectangle *Clip,GuiOut *c) {
    OutClip *r;
    static struct GuiOut Static = {OutFrameDisplay,OutFrameDomain};
    rPush(r);
    r->GuiOut.Static = &Static;
    r->Clip = Clip;
    r->Child = c;
    return &r->GuiOut;
}

