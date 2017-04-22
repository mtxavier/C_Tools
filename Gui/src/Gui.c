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
#include <Gui.local.h>
#include <stdarg.h>

/*----------------------------*/

#define rgbset(A,rc,gc,bc,ac) (A).r = rc; (A).g = gc; (A).b = bc; (A).a = ac;
void Gui2dEngineDefaultInit(Render2dEngine *n,int w,int h) {
    n->Cursor.x = 0;
    n->Cursor.y = 0;
    n->Clip.Pos.x = 0;
    n->Clip.Pos.y = 0;
    n->Clip.Extent.w = w;
    n->Clip.Extent.h = h;
    rgbset(n->Front,255,255,255,GuiRGBAlpha_Opaque);
    rgbset(n->Back,0,0,0,GuiRGBAlpha_Opaque);
    rgbset(n->ColorKey,16,0,0,GuiRGBAlpha_Transparent);
    n->iColorKey = 256;
    n->AlphaEnabled = (0!=0);
    n->ColorKeyEnabled = (0!=0);
    n->LineWidth = 1;
    n->JoinStyle = C_(Img,Style.Join,None);
    n->FillType = C_(Img,Style.Polygon,Filled);
    n->FillPattern = 0;
    Call(n,ColorUpdateFront,0);
    Call(n,ColorUpdateBack,0);
    Call(n,ColorUpdateKey,0);
    Call(n,AlphaKeyEnabledUpdate,0);
    Call(n,iColorUpdateKey,0);
    Call(n,UpdateClip,0);
    Call(n,SetStyle,4(n->LineWidth,n->JoinStyle,n->FillType,n->FillPattern));
}

/*----------------------------*/

static int inNullKeyDown(GuiInput *this,int device,int key) { return C_(Gui,Event,Finished); }
static int inNullKeyUp(GuiInput *this,int device,int key) { return C_(Gui,Event,Finished); }
static int inNullCursor(GuiInput *this,int device,int x,int y) { return C_(Gui,Event,Finished); }
static int inNullmsTime(GuiInput *this,int *next,int current) { *next = current+60000; return C_(Gui,Event,Finished); }
static int inNullSuspend(GuiInput *this,int evt) { return C_(Gui,Event,Finished);}
static struct GuiInput inNullStatic = {
    inNullKeyDown,inNullKeyUp,inNullCursor,inNullmsTime,inNullSuspend
};
GuiInput GuiInputNull = {&inNullStatic};

/*----------------------------*/

static void GuiOutputNullDisplay(GuiOutput *this,Render2dEngine *Engine) {};
static struct GuiOutput GuiOutputNullStatic = {GuiOutputNullDisplay};
GuiOutput GuiOutputNull = {&GuiOutputNullStatic};

/*----------------------------*/

static void Img2dLibNullInfo(Img2dLib *this,int *w,int *h,ImgFormat *format){
    if (w) *w=0;
    if (h) *h=0;
    if (format) {
        format->Format = C_(Img,Format.Type,Unknown);
    }
}
static void Img2dLibNullDisplay(Img2dLib *this,Geo2dRectangle *picture) { }
static int Img2dLibNullOpen(Img2dLib *this) {return (0==0);}
static void Img2dLibNullClose(Img2dLib *this) {}
static void Img2dLibNullFree(Img2dLib *this) {}
static struct Img2dLib Img2dLibNullStatic = {
    &Img2dLibNullInfo,&Img2dLibNullDisplay,
    &Img2dLibNullOpen,&Img2dLibNullClose,&Img2dLibNullFree
};
Img2dLib Img2dLibNull = {&Img2dLibNullStatic};

/*-------------------------------*/

typedef struct {
    GuiOutput GuiOutput;
    Img2dLib *Lib;
    Geo2dRectangle img;
} img2d;
static void img2dDisplay(GuiOutput *this,Render2dEngine *Engine) {
    ThisToThat(img2d,GuiOutput);
    Call(that->Lib,Display,1(&that->img));
}
GuiOutput *GuiOut2dImg(Img2dLib *Lib,int x,int y,int w,int h) {
    img2d *R;
    static struct GuiOutput Static = {img2dDisplay};
    rPush(R);
    R->GuiOutput.Static = &Static;
    R->Lib = Lib;
    R->img.Pos.x = x; R->img.Pos.y = y;
    R->img.Extent.w = w; R->img.Extent.h = h;
    return &R->GuiOutput;
}

/*-------------------------------*/

typedef struct {
    GuiOutput GuiOutput;
    GuiOutput *Child;
    Geo2dExtent Offset;
} outVector;

static void posDisplay(GuiOutput *this,Render2dEngine *Engine) {
    Geo2dPoint pos;
    ThisToThat(outVector,GuiOutput);
    pos = Engine->Cursor;
    Engine->Cursor.x += that->Offset.w;
    Engine->Cursor.y += that->Offset.h;
    Call(that->Child,Display,1(Engine));
    Engine->Cursor = pos;
}
GuiOutput *GuiOut2dPos(int x,int y,GuiOutput *Child) {
    outVector *r;
    static struct GuiOutput Static = {posDisplay};
    rPush(r);
    r->GuiOutput.Static = &Static;
    r->Offset.w = x;
    r->Offset.h = y;
    r->Child = Child;
    return &r->GuiOutput;
}

/*--------------------------*/

static void attachDisplay(GuiOutput *this,Render2dEngine *Engine) {
    Geo2dPoint pos;
    ThisToThat(outVector,GuiOutput);
    pos = Engine->Cursor;
    Engine->Cursor.x -= that->Offset.w;
    Engine->Cursor.y -= that->Offset.h;
    Call(that->Child,Display,1(Engine));
    Engine->Cursor = pos;
}
GuiOutput *GuiOut2dAttach(int x,int y,GuiOutput *Child) {
    outVector *r;
    static struct GuiOutput Static = {attachDisplay};
    rPush(r);
    r->GuiOutput.Static = &Static;
    r->Offset.w = x;
    r->Offset.h = y;
    r->Child = Child;
    return &r->GuiOutput;
}

/*--------------------------*/

typedef struct {
    GuiOutput GuiOutput;
    GuiOutput *Child;
    Geo2dRectangle Rect;
} outClip;

static void outRelClipDisplay(GuiOutput *this,Render2dEngine *Engine) {
    Geo2dRectangle Rect;
    ThisToThat(outClip,GuiOutput);
    Rect = Engine->Clip;
    {   int x0,y0,x1,y1;
        x0 = Engine->Cursor.x + that->Rect.Pos.x;
        x1 = x0 + that->Rect.Extent.w;
        if (x0<Rect.Pos.x) x0 = Rect.Pos.x;
        if (x1>Rect.Pos.x+Rect.Extent.w) x1 = Rect.Pos.x+Rect.Extent.w;
        if (x1<x0) x1 = x0;
        y0 = Engine->Cursor.y + that->Rect.Pos.y;
        y1 = y0 + that->Rect.Extent.h;
        if (y0<Rect.Pos.y) y0 = Rect.Pos.y;
        if (y1>Rect.Pos.y+Rect.Extent.h) y1 = Rect.Pos.y+Rect.Extent.h;
        if (y1<y0) y1 = y0;
        Engine->Clip.Pos.x = x0;
        Engine->Clip.Pos.y = y0;
        Engine->Clip.Extent.w = x1-x0;
        Engine->Clip.Extent.h = y1-y0;
    }
    Call(Engine,UpdateClip,0);
    Call(that->Child,Display,1(Engine));
    Engine->Clip = Rect;
    Call(Engine,UpdateClip,0);
}

GuiOutput *GuiOutRelClip(int x,int y,int w,int h,GuiOutput *child) {
    static struct GuiOutput Static = {
        outRelClipDisplay
    };
    outClip *r;
    rPush(r);
    r->GuiOutput.Static = &Static;
    r->Rect.Pos.x = x;
    r->Rect.Pos.y = y;
    r->Rect.Extent.w = w;
    r->Rect.Extent.h = h;
    r->Child = child;
    return &r->GuiOutput;
}

/*--------------------------*/

static void screenClearDisplay(GuiOutput *this,Render2dEngine *Engine) {
    Call(Engine,ScreenClear,0);
}
static struct GuiOutput clearedScreenStatic = {&screenClearDisplay};
GuiOutput GuiOutClearedScreen = {&clearedScreenStatic};

/*--------------------------*/

typedef struct {
    GuiOutput GuiOutput;
    GuiOutput *Child;
    GuiRGBA Color;
} outColor;

static void outColorFgDisplay(GuiOutput *this,Render2dEngine *Engine) {
    GuiRGBA color;
    ThisToThat(outColor,GuiOutput);
    color = Engine->Front;
    Engine->Front = that->Color;
    Call(Engine,ColorUpdateFront,0);
    Call(that->Child,Display,1(Engine));
    Engine->Front = color;
    Call(Engine,ColorUpdateFront,0);
}
GuiOutput *GuiOutFgColor(unsigned char r,unsigned char g,unsigned char b,unsigned char a,GuiOutput *Child) {
    outColor *R;
    static struct GuiOutput Static = { outColorFgDisplay };
    rPush(R);
    R->GuiOutput.Static = &Static;
    R->Color.r = r;
    R->Color.g = g;
    R->Color.b = b;
    R->Color.a = a;
    R->Child = Child;
    return &R->GuiOutput;
}

static void outColorBgDisplay(GuiOutput *this,Render2dEngine *Engine) {
    GuiRGBA color;
    ThisToThat(outColor,GuiOutput);
    color = Engine->Back;
    Engine->Back = that->Color;
    Call(Engine,ColorUpdateBack,0);
    Call(that->Child,Display,1(Engine));
    Engine->Back = color;
    Call(Engine,ColorUpdateBack,0);
}
GuiOutput *GuiOutBgColor(unsigned char r,unsigned char g,unsigned char b,unsigned char a,GuiOutput *Child) {
    outColor *R;
    static struct GuiOutput Static = {
        outColorBgDisplay
    };
    rPush(R);
    R->GuiOutput.Static = &Static;
    R->Color.r = r;
    R->Color.g = g;
    R->Color.b = b;
    R->Color.a = a;
    R->Child = Child;
    return &R->GuiOutput;
}

/*--------------------------*/

typedef struct {
    GuiOutput GuiOutput;
    GuiOutput *Child;
    int w;
} goutLineWidth;
static void lineWidthDisplay(GuiOutput *this,Render2dEngine *Engine) {
    int ow;
    ThisToThat(goutLineWidth,GuiOutput);
    ow = Engine->LineWidth;
    if (ow!=that->w) {
        Call(Engine,SetStyle,4(that->w,Engine->JoinStyle,Engine->FillType,Engine->FillPattern));
        Call(that->Child,Display,1(Engine));
        Call(Engine,SetStyle,4(ow,Engine->JoinStyle,Engine->FillType,Engine->FillPattern));
    } else {
        Call(that->Child,Display,1(Engine));
    }
}
GuiOutput *GuiOutLineWidth(int w,GuiOutput *Child) {
    goutLineWidth *R;
    static struct GuiOutput Static = {lineWidthDisplay};
    rPush(R);
    R->GuiOutput.Static = &Static;
    R->Child = Child;
    R->w = w;
    return &R->GuiOutput;
}

/*----------------------------*/

typedef struct {
    GuiOutput GuiOutput;
    GuiOutput *Child;
    int ftype;
    void *Pattern;
} goutFilledPoly;
static void filledPolyDisplay(GuiOutput *this,Render2dEngine *Engine) {
    int otype;
    void *opatt;
    ThisToThat(goutFilledPoly,GuiOutput);
    otype = Engine->FillType;
    opatt = Engine->FillPattern;
    if ((otype!=that->ftype)||(opatt!=that->Pattern)) {
        Call(Engine,SetStyle,4(Engine->LineWidth,Engine->JoinStyle,that->ftype,that->Pattern));
        Call(that->Child,Display,1(Engine));
        Call(Engine,SetStyle,4(Engine->LineWidth,Engine->JoinStyle,otype,opatt));
    } else {
        Call(that->Child,Display,1(Engine));
    }
}
GuiOutput *GuiOutFilledPoly(int ftype,void *FillPattern,GuiOutput *Child) {
    goutFilledPoly *R;
    static struct GuiOutput Static = {filledPolyDisplay};
    rPush(R);
    R->GuiOutput.Static = &Static;
    R->Child = Child;
    R->ftype = ftype;
    R->Pattern = FillPattern;
    return &R->GuiOutput;
}


/*--------------------------*/

typedef struct {
    GuiOutput GuiOutput;
    GuiOutput *Children[2];
} output2;

static void output2Display(GuiOutput *this,Render2dEngine *Engine) {
    ThisToThat(output2,GuiOutput);
    Call(that->Children[0],Display,1(Engine));
    Call(that->Children[1],Display,1(Engine));
}

GuiOutput *GuiOutPair(GuiOutput *A,GuiOutput *B) {
    output2 *r;
    static struct GuiOutput Static = {
        output2Display
    };
    rPush(r);
    r->GuiOutput.Static = &Static;
    r->Children[0] = A;
    r->Children[1] = B;
    return &r->GuiOutput;
}

/*--------------------------*/

typedef struct {
    GuiOutput GuiOutput;
    GuiOutput **Children,**end;
} outputn;

static void outputnDisplay(GuiOutput *this,Render2dEngine *Engine) {
    GuiOutput **p,**e;
    ThisToThat(outputn,GuiOutput);
    p = that->Children;
    e = that->end;
    while (p!=e) {
        Call(*p,Display,1(Engine));
        p++;
    }
}

GuiOutput *GuiNOutput(int n,GuiOutput **Children) {
    outputn *r;
    static struct GuiOutput Static = {
        outputnDisplay
    };
    rPush(r);
    r->GuiOutput.Static = &Static;
    rnPush(r->Children,n);
    r->end = r->Children + n;
    { GuiOutput **p,**q,**e;
        p = Children; e = p+n;
        q = r->Children;
        while (p!=e) *q++ = *p++;
    }
    return &r->GuiOutput;
}

GuiOutput *GuiXOutput(GuiOutput *c0,...) {
    outputn *r;
    int n;
    static struct GuiOutput Static = {
        outputnDisplay
    };
    rPush(r);
    r->GuiOutput.Static = &Static;
    {
        va_list l;	
        GuiOutput *p;
        n = 0;
        p = c0;
        va_start(l,c0);
        while (p) { 
            n++;
            p = va_arg(l,GuiOutput *);
        }
        va_end(l);
    }
    rnPush(r->Children,n);
    r->end = r->Children+n;
    {
        va_list l;
        GuiOutput *q,**p,**e;
        p = r->Children;
        e = r->end;
        va_start(l,c0);
        q = c0;
        while (p!=e) {
            *p++ = q;
            q = va_arg(l,GuiOutput *);
        }
        va_end(l);
    }
    return &r->GuiOutput;
}

/*----------------------------------------*/

static void outBackDisplay(GuiOutput *this,Render2dEngine *Engine) {
    GuiRGBA oFg;
    Geo2dPoint oCurs;
    oCurs = Engine->Cursor;
    oFg = Engine->Front;
    Engine->Front = Engine->Back;
    Engine->Cursor.x = Engine->Cursor.y = 0;
    /* ToDo: should also test Fill setting. */
    Call(Engine,ColorUpdateFront,0);
    Call(Engine,Box,2(Engine->Clip.Extent.w,Engine->Clip.Extent.h));
    Engine->Front = oFg;
    Engine->Cursor = oCurs;
    Call(Engine,ColorUpdateFront,0);
}

static struct GuiOutput clearBackStatic = {outBackDisplay};
GuiOutput GuiOutClearBack = {&clearBackStatic};

/*----------------------------------------*/

typedef struct {
    GuiOutput GuiOutput;
    Geo2dExtent Extent;
} outputRectangle;

static void outRectDisplay(GuiOutput *this,Render2dEngine *Engine) {
    ThisToThat(outputRectangle,GuiOutput);
    Call(Engine,Box,2(that->Extent.w,that->Extent.h));
}
GuiOutput *GuiOutRectangle(int w,int h) {
    static struct GuiOutput Static = {
        outRectDisplay
    };
    outputRectangle *r;
    rPush(r);
    r->GuiOutput.Static = &Static;
    r->Extent.w = w;
    r->Extent.h = h;
    return &r->GuiOutput;
}

/*--------------------------------*/

static void outLineDisplay(GuiOutput *this,Render2dEngine *Engine) {
    ThisToThat(outputRectangle,GuiOutput);
    Call(Engine,Line,2(that->Extent.w,that->Extent.h));
}
GuiOutput *GuiOutLine(int dx,int dy) {
    static struct GuiOutput Static = {outLineDisplay};
    outputRectangle *r;
    rPush(r);
    r->GuiOutput.Static = &Static;
    r->Extent.w = dx;
    r->Extent.h = dy;
    return &r->GuiOutput;
}

/*--------------------------------*/

static void outPointDisplay(GuiOutput *this,Render2dEngine *Engine) {
    Call(Engine,Point,0);
}
GuiOutput *GuiOutPoint(void) {
    static struct GuiOutput Static = {outPointDisplay};
    static GuiOutput R = {&Static};
    return &R;
}

/*--------------------------------*/

typedef struct {
    GuiOutput GuiOutput;
    int r,w;
} outputCircle;
static Geo2dPoint guiOrigin = {0,0};
static void outCircleDisplay(GuiOutput *this,Render2dEngine *Engine) {
    Geo2dShape *Shape;
    ThisToThat(outputCircle,GuiOutput);
    rOpen
        Shape = Geo2dShapeCircle(&guiOrigin,that->r,that->w);
        Call(Engine,Shape,1(Shape));
    rClose
}
GuiOutput *GuiOutCircle(int r,int w) {
    outputCircle *R;
    static struct GuiOutput Static = {outCircleDisplay};
    rPush(R);
    R->GuiOutput.Static = &Static;
    R->r = r; R->w = w;
    return &R->GuiOutput;
}

/*--------------------*/

typedef struct {
    GuiOutput GuiOutput;
    int r;
} outputDisk;
static void outDiskDisplay(GuiOutput *this,Render2dEngine *Engine) {
    Geo2dShape *Shape;
    ThisToThat(outputDisk,GuiOutput);
    rOpen
        Shape = Geo2dShapeDisk(&guiOrigin,that->r);
        Call(Engine,Shape,1(Shape));
    rClose
}
GuiOutput *GuiOutDisk(int r) {
    outputDisk *R;
    static struct GuiOutput Static = {outDiskDisplay};
    rPush(R);
    R->GuiOutput.Static = &Static;
    R->r = r;
    return &R->GuiOutput;
}

/*--------------------------------*/

typedef struct {
    GuiOutput GuiOutput;
    Geo2dShape *Shape;
} outputShape;
static void outShapeDisplay(GuiOutput *this,Render2dEngine *Engine) {
    ThisToThat(outputShape,GuiOutput);
    Call(Engine,Shape,1(that->Shape));
}
GuiOutput *GuiOutShape(Geo2dShape *Shape) {
    outputShape *R;
    static struct GuiOutput Static = {outShapeDisplay};
    rPush(R);
    R->GuiOutput.Static = &Static;
    R->Shape = Shape;
    return &R->GuiOutput;
}

/*--------------------------------*/

typedef struct {
    GuiOutput GuiOutput;
    GuiOutput **Child;
} outDContainer;
static void outDContainerDisplay(GuiOutput *this,Render2dEngine *Engine) {
    ThisToThat(outDContainer,GuiOutput);
    Call(*that->Child,Display,1(Engine));
}
GuiOutput *GuiDOutContainer(GuiOutput **Child) {
    outDContainer *R;
    static struct GuiOutput Static = {outDContainerDisplay};
    rPush(R);
    R->GuiOutput.Static = &Static;
    R->Child = Child;
    return  &R->GuiOutput;
}

/*---------------------------------*/

typedef struct {
    GuiOutput GuiOutput;
    GuiOutput  *Child;
    int *x,*y;
} outDPos2d;

static void outDPos2dDisplay(GuiOutput *this,Render2dEngine *Engine) {
    Geo2dPoint oCurs;
    ThisToThat(outDPos2d,GuiOutput);
    oCurs = Engine->Cursor;
    Engine->Cursor.x += *that->x; Engine->Cursor.y += *that->y;
    Call(that->Child,Display,1(Engine));
    Engine->Cursor = oCurs;
}
GuiOutput *GuiDOutPos2d(int *x,int *y,GuiOutput *Child) {
    outDPos2d *R;
    static struct GuiOutput Static = {outDPos2dDisplay};
    rPush(R);
    R->GuiOutput.Static = &Static;
    R->Child = Child;
    R->x = x;
    R->y = y;
    return &R->GuiOutput;
}

/*----------------------------------*/

typedef struct {
    GuiOutput GuiOutput;
    int lw,w,h;
} guiOutFrame;

static void outFrameDraw(GuiOutput *this,Render2dEngine *eng) {
    ThisToThat(guiOutFrame,GuiOutput);
    rOpen {
        int x,y,dx,dy,i,w;
        GuiOutput *out,*hl,*wl;
        dx = that->w; dy = that->h;
        w = that->lw;
        hl = GuiOutLine(dx+(w<<1),0);
        wl = GuiOutLine(0,dy);
        dy--; dx--;
        for (i=-w;i<0;i++) {
            out = GuiOut2dPos(-w,i,hl);
            Call(out,Display,1(eng));
            out = GuiOut2dPos(-w,dy-i,hl);
            Call(out,Display,1(eng));
            out = GuiOut2dPos(i,0,wl);
            Call(out,Display,1(eng));
            out = GuiOut2dPos(dx-i,0,wl);
            Call(out,Display,1(eng));
        }
    } rClose
}
GuiOutput *GuiOutFrame(int w,int h,int wdth) {
    guiOutFrame *r;
    static struct GuiOutput Static = {&outFrameDraw};
    rPush(r);
    r->GuiOutput.Static = &Static;
    r->lw = wdth; r->w = w; r->h = h;
    return &r->GuiOutput;
}

