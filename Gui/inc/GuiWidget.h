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

#ifndef _Gui_Widget_h_
#define _Gui_Widget_h_

#include <Tools.h>
#include <Gui.Core.h>

struct C_Widget {
    struct {
        char StringEdit,IntEdit,vList,hList,Array,String,Int,nb;
    } Field;
    struct {
        char Val,MaxLength,Cursor,Focus,nb;
    } StringEdit;
    struct {
        char Val,Min,Max,Cursor,Focus,Display,nb;
    } IntEdit;
    struct {
        char Val,Cursor,Focus,cFocus,nb;
    } vList;
};

/*_____________________________________________________________________________
 |
 |_____________________________________________________________________________
*/

typedef struct {
    GuiOut GuiOut;
    struct GuiWidget *Static; 
} GuiWidget;

    /*------------------*/

typedef struct { struct GuiTrigger *Static; } GuiTrigger;
typedef struct { struct GuiIntTrigger *Static; } GuiIntTrigger;
typedef struct { struct GuiStreamTrigger *Static; } GuiStreamTrigger;

struct GuiTrigger {
    int (*Trigger)(GuiTrigger *this);
};
struct GuiIntTrigger {
    int (*Trigger)(GuiIntTrigger *this,int Val);
};
struct GuiStreamTrigger {
    int (*Trigger)(GuiStreamTrigger *this,void *b,void *e);
};

extern GuiTrigger GuiTriggerNull;
extern GuiIntTrigger GuiIntTriggerNull;
extern GuiStreamTrigger GuiStreamTriggerNull;

       /*-------------*/

typedef struct { 
    struct GuiWidgetIn *Static; 
    GuiInput GuiInput;
} GuiWidgetIn;
struct GuiWidgetIn {
    void (*Begin)(GuiWidgetIn *this,int TypeId,char *Type);
    GuiTrigger *(*Trigger)(GuiWidgetIn *this,
        int num,char *cId,GuiTrigger *Trg);
    GuiIntTrigger *(*IntTrigger)(GuiWidgetIn *this,
        int num,char *cId,GuiIntTrigger *Tgt);
    GuiStreamTrigger *(*StreamTrigger)(GuiWidgetIn *this,
        int num,char *cId,GuiStreamTrigger *Tgt);
    void (*End)(GuiWidgetIn *this);
};

/*------------------*/

typedef struct { struct GuiWidgetOut *Static; } GuiWidgetOut;

struct GuiWidget {
    int (*Focus)(GuiWidget *this,int On);
    void (*InputEnum)(GuiWidget *this,GuiWidgetIn *Tgt);
    void (*OutputEnum)(GuiWidget *this,GuiWidgetOut *Tgt);
};

struct GuiWidgetOut {
    void (*Begin)(GuiWidgetOut *this,int TypeId,char *cId);
    void (*Int)(GuiWidgetOut *this,int Id,char *cId,TlsIntVal *Val);
    void (*String)(GuiWidgetOut *this,int Id,char *cId,TlsStringVal *Val);
    void (*Widget)(GuiWidgetOut *this,int Id,char *cId,GuiWidget *Val);
    void (*Array)(GuiWidgetOut *this,int Id,char *cId,TlsIntVal *Nb,GuiWidget *Vals);
    void (*End)(GuiWidgetOut *this);
};

   /*--------------------*/

typedef struct {
    struct GuiWidgetCtx *Static;
} GuiWidgetCtx;
struct GuiWidgetCtx {
    GuiWidgetIn *(*GetWidgetIn)(GuiWidgetCtx *this);
    void (*Enter)(GuiWidgetCtx *this);
    void (*Leave)(GuiWidgetCtx *this);
};


GuiWidgetCtx *GuiBoxWidgetNew(TlsMemPool *Pool);

/*__________________________________________________________________________
 |
 |__________________________________________________________________________
*/

typedef struct {
    struct {
        struct GuiColorPair Text;
    } Default;
    struct {
        struct GuiColorPair Selected,Unselected,Cursor0,Cursor1;
        struct {int t1,t2; } CursorBlink;
    } AsciiTextField;
} GuiWidgetStyle;


#endif
