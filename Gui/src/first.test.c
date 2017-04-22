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

#include <Gui.h>

#if (0==0)
// Not needed with optimisation, since the offset is hard coded
	struct C_Gui C_Gui;
	struct C_Img C_Img;
#endif

static int nSin(int n,int a) {
	float prod,alpha,divi,invn,cm;
	alpha = a&0xffff; alpha = alpha/0x10000; alpha = alpha*44./7.;
	invn = 1./n;
	prod = alpha;
	divi = 1.;
	cm = prod;
	alpha = alpha*alpha;
	while (prod>(invn/2)) {
		divi+=2.; prod = (alpha*prod)/(divi*(divi-1.));
		cm -= prod;
		divi+=2.; prod = (alpha*prod)/(divi*(divi-1.));
		cm += prod;
	}
	return (n*cm);
}

static int nCos(int n,int a) {
	float prod,alpha,divi,invn,cm;
	alpha = a&0xffff; alpha = alpha/0x10000; alpha = alpha*44./7.;
	invn = 1./n;
	prod = 1.;
	divi = 0.;
	cm = prod;
	alpha = alpha*alpha;
	while (prod>(invn/2)) {
		divi+=2.; prod = (alpha*prod)/(divi*(divi-1.));
		cm -= prod;
		divi+=2.; prod = (alpha*prod)/(divi*(divi-1.));
		cm += prod;
	}
	return (n*cm);
}

/*-----------------------------------*/

typedef struct CircInCirc {
    int R;
    int dTeta;
    int Teta;
    int ChildrenNb;
    struct {
        int x,y;
        struct CircInCirc *Circ;
    } Children[8];
} CircInCirc;

static CircInCirc *CircInCircNew(int R,int ChildrenNb,int dTeta,int depth) {
    CircInCirc *r;
    rPush(r);
    r->R = R;
    r->Teta = 0;
    r->dTeta = dTeta;
    if (depth) {
        int rc,ddTetac,dTetac,i;
        rc = R/3; ddTetac = dTeta;
        r->ChildrenNb = ChildrenNb;
        for (i=0;i<ChildrenNb;i++) {
             r->Children[i].x = nCos(R-rc,(i*0x10000)/ChildrenNb);
             r->Children[i].y = nSin(R-rc,(i*0x10000)/ChildrenNb);
             r->Children[i].Circ = CircInCircNew(rc,ChildrenNb+1,-ddTetac*2,depth-1);
        }
    } else {
        r->ChildrenNb = 0;
    }
    return r;
}
static void CircInCircUpdate(CircInCirc *Circ,int dt) {
    int i,r;
    Circ->Teta = (Circ->Teta + (Circ->dTeta*dt)) & 0xffff;
    r = (Circ->R*2)/3;
    for (i=0; i<Circ->ChildrenNb; i++) {
        Circ->Children[i].x = nCos(r,Circ->Teta+((i*0x10000)/Circ->ChildrenNb));
        Circ->Children[i].y = nSin(r,Circ->Teta+((i*0x10000)/Circ->ChildrenNb));
        CircInCircUpdate(Circ->Children[i].Circ,dt);
    }
}

static GuiOutput *CircInCircOut(CircInCirc *Circ) {
    GuiOutput *all;
    all = GuiOutCircle(Circ->R,2);
    if (Circ->ChildrenNb>0) {
        int i;
        GuiOutput **Children;
        rnPush(Children,Circ->ChildrenNb+1);
        Children[0] = all;
        for (i=0; i<Circ->ChildrenNb; i++) {
            Children[i+1] = GuiDOutPos2d(&Circ->Children[i].x,&Circ->Children[i].y,CircInCircOut(Circ->Children[i].Circ));
            }
            all = GuiNOutput(Circ->ChildrenNb+1,Children);
	}
    return all;
}

/*-----------------------------------*/

typedef struct {
    GuiInput GuiInput;
    int DateOrg,OldDate,Inited;
    int FrameRate,FrameNum;
    int tim[16],*t;
    CircInCirc *Data;
} sMyInput;

static int waitQuitKeyDown(GuiInput *this,int device,int key) {
    return C_(Gui,Event,Finished);
}
static int waitQuitKeyUp(GuiInput *this,int device,int key) {
    return C_(Gui,Event,Processed);
}
static int waitQuitCursor(GuiInput *this,int device,int x,int y) {
    return C_(Gui,Event,Ignored);
}

static void myUpdateTime(sMyInput *Circ,int dt) {
    int i;
    Circ->t++;
    if (Circ->t>=Circ->tim+16) {
        Circ->t = Circ->tim;
        for (i=0;i<16;i++) { printf(" %d",Circ->tim[i]); }
        printf("\n");
    }
    *Circ->t = dt;
}
static int MyInputmsTime(GuiInput *this,int *next,int current) {
    ThisToThat(sMyInput,GuiInput);
    int base,offset,FrameNum,oFrameNum,nFrameNum,dt;
    if (!that->Inited) { 
        that->Inited = (0==0);
        that->DateOrg = (current/1000)*1000;
        that->OldDate = current;
    }
    that->OldDate = current;
    offset = ((current-that->DateOrg)%1000);
    if (offset>=1000) that->DateOrg += 1000*(offset/1000);
    base = current-offset;
    oFrameNum = that->FrameNum;
    FrameNum = (offset*that->FrameRate)/1000;
    nFrameNum = (FrameNum+1)%that->FrameRate;
    myUpdateTime(that,FrameNum);
    dt = nFrameNum-oFrameNum; if (dt<0) dt+=that->FrameRate;
    CircInCircUpdate(that->Data,dt);
    *next = 1 + base + (((FrameNum+1)*1000)/that->FrameRate);
    that->FrameNum = nFrameNum;
    return C_(Gui,Event,Processed);
}
static int waitQuitSuspend(GuiInput *this,int evt) {
    return C_(Gui,Event,Processed);
}

static GuiInput *MyInput(CircInCirc *Data,int FrameRate) {
    sMyInput *Circ;
    static struct GuiInput Static = {
        waitQuitKeyDown,waitQuitKeyUp,waitQuitCursor,MyInputmsTime,waitQuitSuspend
    };
    rPush(Circ);
    Circ->FrameRate = FrameRate;
    Circ->GuiInput.Static = &Static;
    Circ->Data = Data;
    Circ->Inited = (0!=0);
    Circ->OldDate = 0;
    Circ->FrameNum = 0;
    Circ->t = Circ->tim;
    *Circ->t = 0;
    return &Circ->GuiInput;
}


/*---------------------------------*/

static GuiOutput *SimpleCircle(GuiLibrary *Libr) {
    static char Data[] =
    "      zzzz      "
    "    zzzOOzzz    "
    "   zzOOOOOOzz   "
    "  zzOOOOOOOOzz  "
    " zzOOO1111OOOzz "
    " zOOOOOOOOOOOOz "
    "zzOaOOOffOOOWOzz"
    "zOOaOOffffOOWOOz"
    "zOOaOOffffOOWOOz"
    "zzOaOOOffOOOWOzz"
    " zOOOOOOOOOOOOz "
    " zzOOO4444OOOzz "
    "  zzOOOOOOOOzz  "
    "   zzOOOOOOzz   "
    "    zzzOOzzz    "
    "      zzzz      ";
    ImgDesc Desc;
    int id;
    GuiOutput *r;
    Geo2dRectangle rect;
    Desc.Format.Format = C_(Img,Format.Type,Palette/*Grey*/);
    Desc.Format.Desc.Palette.nb = 256;
    Desc.Format.Desc.Palette.key = -1;
    Desc.Format.Desc.Palette.Colors = GuiFixed256Palette();
    Desc.Dim.w = 16; Desc.Dim.h = 16;
    Desc.pitch = 16;
    Desc.Data = Data;
    id = Call(Libr,GetPageId,0);
    Call(Libr,PageImport,2(id,&Desc));
    rect.Pos.x = rect.Pos.y =0;
    rect.Extent.w = 16; rect.Extent.h = 16;
    r = Call(Libr,Picture,2(id,&rect));
    return r;
}


/*---------------------------------*/

static Geo2dRectangle *Geo2dRectangleFill(Geo2dRectangle *r,int x,int y,int w,int h) {
    r->Pos.x = x;
    r->Pos.y = y;
    r->Extent.w = w;
    r->Extent.h = h;
    return r;
}
typedef struct {
    GuiOutput GuiOutput;
    GuiOutput *Child;
    int Updated;
} myOutput;
static void myOutputDisplay(GuiOutput *this,Gui2dEngine *Engine) {
    ThisToThat(myOutput,GuiOutput);
    Call(that->Child,Display,1(Engine));
}

static GuiOutput *myOutputNew(GuiLibrary *Libr,CircInCirc *Data) {
    myOutput *r;
    static struct GuiOutput Static = {myOutputDisplay};
    static char imgname[] = "data/iso_8859-1.png";
	rPush(r);
	r->GuiOutput.Static = &Static;
	r->Updated = (0==0);
	{
            Geo2dRectangle Rect;
            GuiOutput *outp[8];
            int inum;
            inum = Call(Libr,GetPageId,0);
            Call(Libr,PageLoad,2(inum,imgname));
            outp[0] = Call(Libr,Picture,2(inum,Geo2dRectangleFill(&Rect,0,0,800,600)));
            outp[1] = Call(Libr,Picture,2(inum,Geo2dRectangleFill(&Rect,125,212,64,64)));
            outp[2] = GuiXOutput(
                SimpleCircle(Libr), GuiOutFgColor(127,255,0,GuiRGBAlpha_Opaque,GuiOut2dPos(0,400,
                GuiOutLineWidth(3,GuiOutFilledPoly(C_(Img,Style.Polygon,Outlined),0,GuiXOutput(
                GuiOutCircle(1,2),GuiOut2dPos(9,0,GuiOutCircle(5,2)),GuiOut2dPos(27,0,GuiOutCircle(10,2)),
                GuiOut2dPos(81,0,GuiOutCircle(40,2)),GuiOut2dPos(240,0,GuiOutCircle(80,2)),
                GuiOut2dPos(500,-100,CircInCircOut(Data)),
				0)
                )))),0
            );
            outp[3] = GuiOutFgColor(255,192,127,GuiRGBAlpha_Opaque,GuiOut2dPos(320,200,
                GuiXOutput(
                     GuiOutLine(100,0),GuiOutLine(84,50),GuiOutLine(70,70),GuiOutLine(50,84),
                     GuiOutLine(0,100),GuiOutLine(-50,84),GuiOutLine(-70,70),GuiOutLine(-84,50),
                     GuiOutLine(-100,0),GuiOutLine(-84,-50),GuiOutLine(-70,-70),GuiOutLine(-50,-84),
                     GuiOutLine(0,-100),GuiOutLine(50,-84),GuiOutLine(70,-70),GuiOutLine(84,-50),
			0)
		));
            r->Child = GuiOutPair(GuiOutPair(outp[0],outp[3]),GuiOutPair(GuiOut2dPos(80,60,outp[1]),GuiOut2dPos(20,20,outp[2])));
	}
	return &r->GuiOutput;
}

extern GuiEngine *GuiEngineNew(char *WindowName,GuiLibrary *Library,int w,int h,int *FrameRate);

main() {
	int Continue;
	GuiEngine *engine;
	GuiLibrary *Libr;
	CircInCirc *Data;
	int FrameRate;
	EnvOpen(4096,4096);
	Libr = GuiLibraryNew();
	FrameRate = 25;
	engine = GuiEngineNew("FirstTest",Libr,800,600,&FrameRate);
	Data = CircInCircNew(160,3,0x100,3);
	if (Call(engine,Open,0)) {
            Call(engine,EventLoop,2(myOutputNew(Libr,Data)/*myImg(imgname)*/,MyInput(Data,25)));
            Call(engine,Close,0);
	}
	EnvClose();
}

