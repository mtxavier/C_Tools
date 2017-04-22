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
#include <List.h>
#include <Gui.Term.h>
#include <Gui/GuiKeyMap.h>
#include <time.h>
#include <GuiWidget.h>


#define l_(a,b) C_(Gui,a,b)

GuiWidget *GuiWidgetFixedAsciiInput(
    char *bTgt,char *eTgt,int *WndWidth,GuiTermEngine *Out,
    int *tms,GuiWidgetStyle *St
);

GuiWidget *GuiVertWidgetsNew(
    GuiWidgetCtx *Ctx,GuiWidget **b,GuiWidget **e,
    GuiTermEngine *Outp,int *Height,int *tms
);

GuiWidget *GuiNumFieldNew(
    int *Val,int Min,int Max,GuiTermEngine *Outp,int *tms,GuiWidgetStyle *St
);

/*_____________________________________________________________________
 |
 |
 |_____________________________________________________________________
*/

static int GtTime(void) {
    int t;
    struct timespec tns;
    t = clock_gettime(CLOCK_REALTIME/*CLOCK_MONOTONIC*/,&tns);
    t = tns.tv_nsec/1000000;
    t+= tns.tv_sec*1000;
    return t;
}

static void initTxt(char *dst,char *src) {
    char *p,*q;
    q = dst; p=src;
    while (*p) {*q++=*p++;}
    *q=0;
}
static void perform(GuiTermCtrl *Inp) {
    int K,clck,nclck,Inpt[4];
    char Res[320],*p,*q;
    Geo2dRectangle Clip;
    Geo2dPoint org,org1;
    GuiInput *Reac;
    GuiOut *Displ;
    GuiTermEngine *Outp;
    GuiTerm *Term;
    GuiWidgetCtx *WidgCtx;
    GuiWidgetIn *wIO;
    GuiWidget *Widg;
    GuiWidget *bWidg[8],**pWidg;
    GuiWidgetStyle Style;
    Inpt[0] = 0; Inpt[1]=4; Inpt[2]=13; Inpt[3]=124;
    Style.Default.Text.fg=6;
    Style.Default.Text.bg=0;
    Style.AsciiTextField.Selected.fg=3;
    Style.AsciiTextField.Selected.bg=4;
    Style.AsciiTextField.Unselected=Style.Default.Text;
    Style.AsciiTextField.Cursor0.fg=4;
    Style.AsciiTextField.Cursor0.bg=3;
    Style.AsciiTextField.Cursor1.fg=3;
    Style.AsciiTextField.Cursor1.bg=6;
    Style.AsciiTextField.CursorBlink.t1=500;
    Style.AsciiTextField.CursorBlink.t2=1000;
    Term = Call(Inp,GetViewPort,0);
    Outp = GuiTermEngineNew(Inp,8);
    clck = GtTime();
    WidgCtx = GuiBoxWidgetNew(0);
    wIO = Call(WidgCtx,GetWidgetIn,0);
    Reac = &(wIO->GuiInput);
    Clip.Pos.x = Clip.Pos.y = 0;
    Clip.Extent.w = 32; Clip.Extent.h = 1;
    org.x = 2; org.y = 5;
    p = Res;
    pWidg = bWidg;
    initTxt(p,"Essai0");
    *pWidg++=GuiWidgetFixedAsciiInput(p,p+32,&Clip.Extent.w,Outp,&clck,&Style);
    p+=32;
    initTxt(p,"Essai1");
    *pWidg++=GuiWidgetFixedAsciiInput(p,p+32,&Clip.Extent.w,Outp,&clck,&Style);
    p+=32;
    initTxt(p,"Test0");
    *pWidg++=GuiWidgetFixedAsciiInput(p,p+32,&Clip.Extent.w,Outp,&clck,&Style);
    p+=32;
    initTxt(p,"Test1");
    *pWidg++=GuiWidgetFixedAsciiInput(p,p+32,&Clip.Extent.w,Outp,&clck,&Style);
    p+=32;
    *pWidg++=GuiNumFieldNew(Inpt+0,-5,10,Outp,&clck,&Style);
    *pWidg++=GuiNumFieldNew(Inpt+1,0,99,Outp,&clck,&Style);
    *pWidg++=GuiNumFieldNew(Inpt+2,0,250,Outp,&clck,&Style);
    Widg = GuiVertWidgetsNew(WidgCtx,bWidg,pWidg,Outp,&Clip.Extent.h,&clck);
    Displ = &Widg->GuiOut;
    Displ = GuiOutPos(&org,Displ);
    Displ = Call(Outp,ViewPort,1(Displ));
    Call(Widg,InputEnum,1(wIO));
    Call(Widg,Focus,1(l_(Reflex.Focus,Unselected)));
    {
        GuiOut *n;
        TlsCharString Title;
        p = "Input test: press F12 to end.";
        rnPush(Title.b,80);
        Title.e = Title.b; 
        org1.x = 10; org1.y = 1;
        while (*Title.e=*p) {Title.e++; p++;}
        n = Call(Outp,Ascii,1(&Title));
        n = Call(Outp,Color,2(&Style.Default.Text,n));
        n = GuiOutPos(&org1,n);
        n = Call(Outp,ViewPort,1(n));
        Call(n,Display,1(0));
        Call(Displ,Display,1(0));
        Call(Term,Refresh,0);
        do {
            K = Call(Inp,GetCh,0);
        } while(K!=GuiKey(Fn,F12));
    }
    Call(Widg,Focus,1(l_(Reflex.Focus,Selected)));
    do {
        K = Call(Inp,GetCh,0);
        clck = GtTime();
        if (K==GuiKeyNoKey) {
            Call(Reac,msTime,2(&nclck,clck));
        } else {
            if (K!=GuiKey(Fn,F12)) {
                Call(Reac,KeyDown,2(l_(Device,Keyboard[0]),K));
            }
        }
        Call(Displ,Display,1(0));
        Call(Term,Refresh,0);
    } while(K!=GuiKey(Fn,F12));
}

/*-------------------*/

int main(int argc,char *argv[]) {
    GuiTermCtrl *IO;
    EnvOpen(4096,4096);
    IO = GuiCurseTerm();
    perform(IO);
    Call(IO,Close,0);
    EnvClose();
    return 0;
}
