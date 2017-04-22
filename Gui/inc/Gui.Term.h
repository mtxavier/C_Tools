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

#ifndef _Gui_Term_h_
#define _Gui_Term_h_

#include <Tools.h>
#include <Geo.h>
#include <Gui.Core.h>

typedef struct {
    struct {
        struct {
            GeoRGBA *b,*e;
            TlsRange ModRange; // range of modifiable colors.
        } Palette;
        struct {
            GuiColorPair *b,*e;
            TlsRange ModRange; // range of modifiable pairs.
        } Pairs;
    } Color;
    struct {
        struct GuiTermFontDesc {
            int Id;
            int bold,ital; // boolean
        } *b,*e;
    } Font;
    Geo2dExtent Dim;
} GuiTermInfo;

typedef struct { struct GuiTermCtrl *Static; } GuiTermCtrl;
typedef struct { struct GuiTerm *Static; } GuiTerm;

struct GuiTermCtrl {
    GuiTermInfo *(*GetInfo)(GuiTermCtrl *this);
    GuiTerm *(*GetViewPort)(GuiTermCtrl *this);
    int (*SetColor)(GuiTermCtrl *this,int num,GeoRGBA *n);
    int (*SetColorPair)(GuiTermCtrl *this,int num,int bg,int fg);
    int (*GetCh)(GuiTermCtrl *this);
    void (*Close)(GuiTermCtrl *this);
};
struct GuiTerm {
    GuiTermInfo *(*GetInfo)(GuiTerm *this);
    void (*Clear)(GuiTerm *this);
    void (*FontSet)(GuiTerm *this,int fnt);
    void (*ColorSet)(GuiTerm *this,int Pair);
    void (*AddStr)(GuiTerm *this,int x,int y,const char *s);
    void (*Refresh)(GuiTerm *this);
};

#define GuiTextStyleDim    0
#define GuiTextStyleNormal 1
#define GuiTextStyleBold   2
#define GuiKeyNoKey      (-1)

/*_________________________________________________________________________
 |
 |
 |_________________________________________________________________________
*/


typedef struct {
    struct GuiAttrAsciiCtx *Static;
} GuiAttrAsciiCtx;

struct GuiAttrAsciiCtx {
    void (*Open)(GuiAttrAsciiCtx *this);
    void (*SetColor)(GuiAttrAsciiCtx *this,int fg,int bg);
    void (*SetFont)(GuiAttrAsciiCtx *this,int font);
    void (*Add)(GuiAttrAsciiCtx *this,TlsCharString *Strng);
    void (*Close)(GuiAttrAsciiCtx *this);
};

typedef struct {
    struct GuiAttrAsciiString *Static;
} GuiAttrAsciiString;

struct GuiAttrAsciiString  {
    int (*Length)(GuiAttrAsciiString *this);
    void (*Browse)(GuiAttrAsciiString *this,TlsRange s,GuiAttrAsciiCtx *Ctx);
};

/*__________________________________________________________________________
 |
 |
 |__________________________________________________________________________
*/

typedef struct {
    struct GuiTermEngine *Static;
} GuiTermEngine;
struct GuiTermEngine {
    GuiOut *(*ViewPort)(GuiTermEngine *this,GuiOut *c);
    GuiOut *(*Color)(GuiTermEngine *this,GuiColorPair *clr,GuiOut *c);
    GuiOut *(*Ascii)(GuiTermEngine *this,TlsCharString *String);
    GuiOut *(*Clear)(GuiTermEngine *this,GuiOut *c);
};

GuiTermEngine *GuiTermEngineNew(GuiTermCtrl *Display,int fgnb);
/*_________________________________________________________________________
 |
 |
 |_________________________________________________________________________
*/

GuiTermCtrl *GuiCurseTerm(void);

#endif
