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
#include <Gui/GuiKeyMap.h>
#include <Gui.Term.h>
#include <stdio.h>

GuiTermCtrl *GuiCurseTerm(void);

/*-------------------------------------------*/

typedef struct { struct GuiTermCtx *Static; } GuiTermCtx;
typedef struct { struct GuiTermClosure *Static; } GuiTermClosure;
typedef struct { struct GuiTermExpr *Static; } GuiTermExpr;

struct GuiTermExpr {
    int (*Invoke)(GuiTermExpr *this,int time);
    int (*Display)(GuiTermExpr *this,GuiTerm *Tgt,int time);
    int (*React)(GuiTermExpr *this,int Key);
};

/*-------------------------*/

typedef struct {
    GuiTermExpr GuiTermExpr;
    GuiTermExpr **bNod,**eNod,**cNod,*Transit;
    int Running,Skipped,TimeOrg;
} GuiTermSeq;

static int TermSeqInvoke(GuiTermExpr *this,int time) {
    ThisToThat(GuiTermSeq,GuiTermExpr);
    that->cNod = that->bNod;
    that->Running = (0!=0);
    that->TimeOrg = time;
    return (0==0);
}
static int TermSeqDisplay(GuiTermExpr *this,GuiTerm *Tgt,int time) {
    int r;
    ThisToThat(GuiTermSeq,GuiTermExpr);
    if (that->Skipped) {
        if (that->cNod!=that->eNod) {
            that->cNod = that->eNod;
        }
        that->Skipped = (0!=0);
    }
    r = (that->eNod!=that->cNod);
    if (r) {
        GuiTermExpr **cNod;
        cNod = that->cNod;
        if (that->Running) {
            that->Running = Call(*cNod,Display,2(Tgt,time-that->TimeOrg));
            if (!that->Running) { that->cNod++; }
        }
        if (!that->Running) { that->TimeOrg = time; }
        while ((!that->Running)&&(cNod!=that->eNod)){
            Call(*cNod,Invoke,1(0));
            that->Running = Call(*cNod,Display,2(Tgt,time-that->TimeOrg));
            if (!that->Running) {
                that->cNod++;
            }
        }
        that->cNod = cNod;
        r = that->Running;
    }
    if ((!r)&&(that->Transit)) {
        r = Call(that->Transit,Display,2(Tgt,time-that->TimeOrg));
    }
    return r;
}
static int TermSeqReact(GuiTermExpr *this,int Key) {
    ThisToThat(GuiTermSeq,GuiTermExpr);
    if (Key == GuiKey(Fn,Esc)) {
        that->Skipped = (0==0);
    } else {
        int r;
        if ((that->cNod!=that->eNod)&&(that->Running)) {
            that->Running = Call(*that->cNod,React,1(Key));
        }
    }
    return (0==0);
}


/*-------------------------------------------*/

static int MyStory0Invoke(GuiTermExpr *this,int time) { return (0==0); }
static int MyStory0Display(GuiTermExpr *this,GuiTerm *Tgt,int time) {
    int c,i,j;
    Geo2dExtent dims;
    char strng[80];
    GuiTermInfo *Info;
//    Call(Tgt,Clear,0);
    Call(Tgt,FontSet,1(GuiTextStyleBold));
    Call(Tgt,AddStr,3(0,0,"Hello World!"));
    for (i=0;i<8;i++) { 
        for (j=0;j<8;j++) {
            Call(Tgt,ColorSet,1((i<<3)+j));
            Call(Tgt,AddStr,3(8+i,1+j,"#"));
        }
    }
    Call(Tgt,ColorSet,1(0));
    Call(Tgt,FontSet,1(GuiTextStyleNormal));
    {
        int cols,pairs;
        Info = Call(Tgt,GetInfo,0);
        cols = Info->Color.Palette.e-Info->Color.Palette.b;
        pairs = Info->Color.Pairs.e-Info->Color.Pairs.b;
        sprintf(strng,"Rows:%d, Columns:%d, Colors/pairs:%d/%d",Info->Dim.h,Info->Dim.w,cols,pairs);
    }
    Call(Tgt,AddStr,3(4,9,strng));
    Call(Tgt,FontSet,1(GuiTextStyleDim));
    Call(Tgt,AddStr,3(4,10,"'F12' to quit."));
    return (0==0);
}
static int MyStory0React(GuiTermExpr *this,int Key) {
    return (Key!=GuiKey(Fn,F12));
}
static GuiTermExpr *MyStory0(void) {
    static struct GuiTermExpr Static = {
        MyStory0Invoke,MyStory0Display,MyStory0React
    };
    static GuiTermExpr R = {&Static};
    return &R;
}

/*---------------------------------------------*/

main() {
    GuiTermCtrl *cectrl;
    GuiTerm *ce;
    int cont,e,t;
    GuiTermExpr *Scene;
    EnvOpen(4096,4096);
    cectrl = GuiCurseTerm();
    {
        int bg,fg;
        for (bg=0;bg<8;bg++) {
            for (fg=0;fg<8;fg++) {
                if (bg||fg) { 
                    Call(cectrl,SetColorPair,3((fg<<3)+bg,bg,fg));
                }
            }
        }
    }
    ce = Call(cectrl,GetViewPort,0);
    Scene = MyStory0();
    t = 0;
    Call(Scene,Invoke,1(t));
    do {
        cont = Call(Scene,Display,2(ce,t));
        Call(ce,Refresh,0);
        if (cont) {
            e = Call(cectrl,GetCh,0);
            if (e!=GuiKeyNoKey) {
                cont = Call(Scene,React,1(e));
            }
        }
        t++;
    } while(cont);
    Call(cectrl,Close,0);
    EnvClose();
}

