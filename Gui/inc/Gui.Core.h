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

#ifndef _Gui_Core_h_
#define _Gui_Core_h_

#include <Gui.h>

/*
struct C_Gui {
    char _;
    struct {
        char _,Processed,Ignored,Finished;
        struct {
            char _,Play,Pause,Stop;
        } Suspend;
    } Event;
    struct {
        char _,Up,Down,Left,Right,Home,End,PgUp,PgDown,Enter,Del,Insert
        ,Backspace,Next,Previous,Escape,Validate,Cancel
        ,nb;
        struct {
            char _,AddChar,nb;
        } Int;
        struct {
            char _,Disabled,Unselected,Highlighted,Selected;
        } Focus;
    } Reflex;
    struct {
        char _,Keyboard[4];
        struct {
            char _,Key,Cursor;
        } Mouse[4];
        struct {
            char _,Key,Trigger[4],Cursor[6];
        } Joypad[8];
    } Device;
    struct {
        char _,Dim,Normal,Bold,Italic;
    } TextStyle;
};
*/

/*__________________________________________________________________________
 |
 |
 |__________________________________________________________________________
*/

typedef struct {
    struct GuiOutCtx *Static;
    Geo2dPoint Pos;
    Geo2dRectangle ViewPort;
} GuiOutCtx;
typedef struct {
    struct GuiOut *Static;
} GuiOut;
struct GuiOut {
    void (*Display)(GuiOut *this,GuiOutCtx *Ctx);
    Geo2dRectangle (*Domain)(GuiOut *this);
};

extern GuiOut GuiOutNull;
extern GuiOutCtx GuiOutCtxNull;

GuiOut *GuiOutPos(Geo2dPoint *Pos,GuiOut *c);
GuiOut *GuiOutOrg(Geo2dPoint *Org,GuiOut *c);

GuiOut *GuiOutClip(Geo2dRectangle *Clip,GuiOut *c);
GuiOut *GuiOutPage(Geo2dRectangle *Frame,GuiOut *c);

GuiOut *GuiOutTrmPair(GuiOut *c0,GuiOut *c1);
GuiOut *GuiOutSequence(GuiOut **b,GuiOut **e);
GuiOut *GuiOutSeq(GuiOut *fst,...);

GuiOut *GuiOutSelect(int *Sel,GuiOut **b,GuiOut **e);
GuiOut *GuiOutSel(int *Sel,...);

/*_____________________________________________________________________________
 |
 |_____________________________________________________________________________
*/

/*
#define InputDevice_Keyboard 0x100
#define InputDevice_Mouse 0x200
#define InputDevice_TouchScreen 0x300
#define InputDevice_Joypad 0x4000
#define InputDevice_JoypadAxis 0x8000

typedef struct {
    struct GuiInput *Static;
} GuiInput;
struct GuiInput {  
    int (*KeyDown)(GuiInput *this,int device,int key);
    int (*KeyUp)(GuiInput *this,int device,int key);
    int (*Cursor)(GuiInput *this,int device,int x,int y);
    int (*msTime)(GuiInput *this,int *next,int current);
    int (*Suspend)(GuiInput *this,int evt);
};
extern GuiInput GuiInputNull;
*/

/*__________________________________________________________________________
 |
 |__________________________________________________________________________
*/

typedef struct GuiColorPair {
    int fg,bg;
} GuiColorPair;


/*-------------------------------*/


#endif
