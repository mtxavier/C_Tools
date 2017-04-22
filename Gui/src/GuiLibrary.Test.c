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
#include <Gui.local.h>
#include <stdio.h>

#if (0==0) 
    struct C_Gui C_Gui;
	struct C_Img C_Img;
#endif

/*---------------------*/

typedef struct {
	Render2dEngine Render2dEngine;
	char *Name;
	int LibNb;
} NamedRender2dEngine;
typedef  struct {
	Img2dLib Img2dLib;
	int Num;
	NamedRender2dEngine *Engine;
	int Xtrn;
	ImgDesc Desc;
} NamedImg2dLib;

static void NamedImg2dLibInfo(Img2dLib *this,int *w,int *h,ImgFormat *format) {
	ThisToThat(NamedImg2dLib,Img2dLib);
	printf("\nImgPage::%s::%d::Info",that->Engine->Name,that->Num);
}
static void NamedImg2dLibDisplay(Img2dLib *this,Geo2dRectangle *img) {
	ThisToThat(NamedImg2dLib,Img2dLib);
	printf("\nImgPage::%s::%d::Display((%d,%d),(%d,%d)))",
		that->Engine->Name,that->Num,img->Pos.x,img->Pos.y,img->Extent.w,img->Extent.h
	);
}
static int NamedImg2dOpen(Img2dLib *this) {
	ThisToThat(NamedImg2dLib,Img2dLib);
	printf("\nImgPage::%s::%d::Open",that->Engine->Name,that->Num);
    return (0==0);
}
static void NamedImg2dClose(Img2dLib *this) {
	ThisToThat(NamedImg2dLib,Img2dLib);
	printf("\nImgPage::%s::%d::Close",that->Engine->Name,that->Num);
}
static void NamedImg2dFree(Img2dLib *this) {
	ThisToThat(NamedImg2dLib,Img2dLib);
	printf("\nImgPage::%s::%d::Free",that->Engine->Name,that->Num);
}

static Img2dLib *NamedImg2dLibNew(NamedRender2dEngine *Engine,int Xtrn,char *desc) {
	NamedImg2dLib *R;
	static struct Img2dLib Static = {
        NamedImg2dLibInfo,NamedImg2dLibDisplay,NamedImg2dOpen,NamedImg2dClose,NamedImg2dFree
	};
	rPush(R);
    R->Img2dLib.Static = &Static;
	R->Engine = Engine;
	R->Num = Engine->LibNb++;
	R->Xtrn = Xtrn;
	R->Desc.Dim.w = R->Desc.Dim.h = 0;
	R->Desc.Data = desc;
	if (Xtrn) {
		printf("\nNew::%d::Extern::Img2dLib::%s",R->Num,desc);
	} else {
		printf("\nNew::Internal::Img2dLib");
	}
	return &R->Img2dLib;
}

     /*--------*/


static Img2dLib *engNewImg2dLib(Render2dEngine *this,char *desc) { 
	ThisToThat(NamedRender2dEngine,Render2dEngine);
	printf("\nRender::%s::XtrnPage(\"%s\")",that->Name,desc);
	return NamedImg2dLibNew(that,(0==0),desc);
}
static Img2dLib *engImportImg2dLib(Render2dEngine *this,ImgDesc *Img) {
	ThisToThat(NamedRender2dEngine,Render2dEngine);
	printf("\nRender::%s::InternalPage",that->Name);
	return NamedImg2dLibNew(that,(0!=0),Img->Data);
}
static void engColorUpdateFront(Render2dEngine *this){
	ThisToThat(NamedRender2dEngine,Render2dEngine);
	printf("\nRender::%s::ColorUpdateFront",that->Name);
}
static void engColorUpdateBack(Render2dEngine *this){
	ThisToThat(NamedRender2dEngine,Render2dEngine);
	printf("\nRender::%s::ColorUpdateBack",that->Name);
}
static void engColorUpdateKey(Render2dEngine *this){
	ThisToThat(NamedRender2dEngine,Render2dEngine);
	printf("\nRender::%s::ColorUpdateKey",that->Name);
}
static void engiColorUpdateKey(Render2dEngine *this){
	ThisToThat(NamedRender2dEngine,Render2dEngine);
	printf("\nRender::%s::iColorUpdateKey",that->Name);
}
static void engAlphaKeyEnabledUpdate(Render2dEngine *this) {
	ThisToThat(NamedRender2dEngine,Render2dEngine);
	printf("\nRender::%s::KeyEnableUpdate",that->Name);
}
static void engUpdateClip(Render2dEngine *this) {
	ThisToThat(NamedRender2dEngine,Render2dEngine);
	printf("\nRender::%s::UpdateClip",that->Name);
}
static void engSetStyle(Render2dEngine *this,int linewidth,int jointpattern,int filltype,void *fillpattern){
	ThisToThat(NamedRender2dEngine,Render2dEngine);
	printf("\nRender::%s::SetStyle(%d,%d,%d,..)",that->Name,linewidth,jointpattern,filltype);
}
static void engPoint(Render2dEngine *this) {
	ThisToThat(NamedRender2dEngine,Render2dEngine);
	printf("\nRender::%s::Point",that->Name);
}
static void engLine(Render2dEngine *this,int dx,int dy) {
	ThisToThat(NamedRender2dEngine,Render2dEngine);
	printf("\nRender::%s::Line(%d,%d)",that->Name,dx,dy);
}
/*static void engCircle(Render2dEngine *this,int r) {
	ThisToThat(NamedRender2dEngine,Render2dEngine);
	printf("\nRender::%s::Circle(%d)",that->Name,r);
} */
static void engShape(Render2dEngine *this,Geo2dShape *Shape) {
	ThisToThat(NamedRender2dEngine,Render2dEngine);
	printf("\nRender::%s::Shape(..)",that->Name);
}
static void engBox(Render2dEngine *this,int w,int h) {
	ThisToThat(NamedRender2dEngine,Render2dEngine);
	printf("\nRender::%s::Box(%d,%d)",that->Name,w,h);
}

static Render2dEngine *EngNew(char *Id) {
	NamedRender2dEngine *R;
	static struct Render2dEngine Static = {
		engImportImg2dLib, 
		engColorUpdateFront, engColorUpdateBack, engColorUpdateKey, engiColorUpdateKey, engAlphaKeyEnabledUpdate,
		engUpdateClip, engSetStyle, 
		engPoint, engLine, engShape, engBox
	};
	rPush(R);
	R->Render2dEngine.Static = &Static;
	R->Name = Id;
	R->LibNb = 0;
	return &R->Render2dEngine;
}

/*---------------------*/

void PrintAllOutput(Render2dEngine *Engine,GuiOutput **Output,int Nb) {
	int i;
	NamedRender2dEngine *nm;
	nm = CastBack(NamedRender2dEngine,Render2dEngine,Engine);
	printf("\n\n In engine %s : displaying %d outputs \n",nm->Name,Nb);
	for (i=0;i<Nb;i++) {
		Call(Output[i],Display,1(Engine));
	}
}

main() {
	GuiLibrary *libr;
	GuiOutput *Output[20];
	struct {
		int Num;
	    Render2dEngine *Engine;
	} Engines[2];
	Geo2dRectangle Rect;
	int num,OutNb;
	ImgDesc Desc;
	EnvOpen(4096,4096);
	libr = GuiLibraryNew();
	Rect.Pos.x = 14; Rect.Pos.y = 60;
	Rect.Extent.w = 16; Rect.Extent.h = 16;
	Desc.Dim.h = Desc.Dim.w = 512;
	OutNb = 0;
	Output[OutNb++] = Call(libr,Picture,2(2,&Rect));
    Call(libr,PageLoad,2(0,"MyFirstPage"));
	num = Call(libr,GetPageId,0);
	printf("\n --> Got Id %d.\n",num);
	Call(libr,PageLoad,2(num,"MySecondPage"));
	Output[OutNb++] = Call(libr,Picture,2(0,&Rect));
	Call(libr,PageImport,2(2,&Desc));
	Engines[0].Engine = EngNew("FirstEngine");
	Engines[1].Engine = EngNew("SecondEngine");
    Engines[0].Num = Call(libr,RecordEngine,1(Engines[0].Engine));
	printf("\n --> Engine 0 recorded as %d.",Engines[0].Num);
	Output[OutNb++] = Call(libr,Picture,2(2,&Rect));
	Output[OutNb++] = GuiOutLine(25,50);
	Output[OutNb++] = Call(libr,Picture,2(1,&Rect));
	Output[OutNb++] = Call(libr,Picture,2(67,&Rect));
	PrintAllOutput(Engines[0].Engine,Output,OutNb);
	Call(libr,PageLoad,2(67,"ExternalPage"));
	PrintAllOutput(Engines[0].Engine,Output,OutNb);
	PrintAllOutput(Engines[1].Engine,Output,OutNb);
	Engines[1].Num = Call(libr,RecordEngine,1(Engines[1].Engine));
	printf("\n --> Engine 1 recorded as %d.",Engines[1].Num);
	PrintAllOutput(Engines[1].Engine,Output,OutNb);
	Call(libr,ReleasePageId,1(num));
	Call(libr,ReleaseEngine,1(Engines[1].Num));
	    printf("\n --> Engine 1: Released");
        printf("\n  --->  Print in engine 0 (recorded)");
	    PrintAllOutput(Engines[0].Engine,Output,OutNb);
		printf("\n  ---> Print in Engine 1 (not recorded)");
	    PrintAllOutput(Engines[1].Engine,Output,OutNb);
	Call(libr,ReleaseEngine,1(Engines[0].Num));
	    printf("\n --> Engine 0: Released");
    printf("\n");
	EnvClose();
}

