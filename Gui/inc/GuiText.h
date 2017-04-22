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

#ifndef _GuiText_h_
#define _GuiText_h_

#include <Gui.h>

/*--------------------------------------------*/

typedef struct {
    Geo2dPoint Grab;
    Geo2dExtent Extent;
    Geo2dPoint dx,dy;
} GuiFontGlypheDesc;

typedef struct { struct GuiFontPageDesc *Static; } GuiFontPageDesc;

struct GuiFontPageDesc {
    void (*GetRange)(GuiFontPageDesc *this,int *Begin,int *end);
    int (*GetGlypheDesc)(GuiFontPageDesc *this,GuiFontGlypheDesc *Desc,int Glyphe);
    GuiOutput *(*GetGlyphe)(GuiFontPageDesc *this,int Gluphe);
};
extern GuiFontPageDesc GuiFontPageDescNull;

GuiFontPageDesc *GuiFontPage16x16Png(
    GuiLibrary *ImgLibr,
    char *PngFile,Geo2dPoint *LibOrg,Geo2dExtent *NextGlyph,
    GuiFontGlypheDesc *ExpectedMetric
);

// Font loading using FreeType Library. GreyNb in range [2..256]; normally only 2,4,16,256 are allowed.
// In SDL engine, they will all take the same memory space, however, if color is required, the monochrome
// format (2 greys) will be much faster.
GuiFontPageDesc *GuiTTFontLoad(GuiLibrary *Libr,Geo2dExtent *Size,int greynb,char *FileName,int FaceNum);

/*--------------------------------------------*/

typedef struct { struct GuiFieldStyle *Static; } GuiFieldStyle;
typedef struct { struct GuiFieldStyleOutCtx *Static; } GuiFieldStyleOutCtx;
typedef struct { struct GuiFieldStyleInCtx *Static; } GuiFieldStyleInCtx;

typedef struct {
    int AttrByte;
    int AttrBit,FntBit,PalBit,FgBit,BgBit;
    int FontSize;
    struct {GuiRgbPalette *fg,*bg;} *Palettes,*ePal;
} GuiFieldStyleAttrDesc;
struct GuiFieldStyle {
    GuiFieldStyleInCtx *(*OpenInCtx)(GuiFieldStyle *this,GuiFieldStyleAttrDesc *Dsc);
    GuiFieldStyleOutCtx *(*OpenOutCtx)(GuiFieldStyle *this,GuiFieldStyleAttrDesc *AttrBytes);
    void (*AddFont)(GuiFieldStyle *this,int num,int Size,int Attr,GuiFontPageDesc *Page);
    GuiFontPageDesc *(*GetFont)(GuiFieldStyle *this,int num,int Size,int Attr);
};
struct GuiFieldStyleOutCtx {
    void (*GetAttr)(GuiFieldStyleOutCtx *this,GuiRGBA *newfront,GuiRGBA *newback,unsigned char *Attr);
    GuiOutput *(*GetTile)(GuiFieldStyleOutCtx *this,int TileId);
    void (*Close)(GuiFieldStyleOutCtx *this);
};
struct GuiFieldStyleInCtx {
    void (*SetFontNum)(GuiFieldStyleInCtx *this,int *old,int New);
    void (*SetTextAttr)(GuiFieldStyleInCtx *this,int *old,int New);
    void (*SetPaletteNum)(GuiFieldStyleInCtx *this,int *old,int New);
    void (*SetColorFg)(GuiFieldStyleInCtx *this,GuiRGBA *old,GuiRGBA *New);
    void (*SetColorBg)(GuiFieldStyleInCtx *this,GuiRGBA *old,GuiRGBA *New);
    void (*FillAttr)(GuiFieldStyleInCtx *this,unsigned char *Attr);
    void (*Close)(GuiFieldStyleInCtx *this);
};

GuiFieldStyle *GuiFieldStyleNew(int AttrNb);

/*-----------------------*/

#include <GeoField.h>
GuiOutput *GuiFieldWindow(Geo2dField *Field,GuiFieldStyle *Style,GuiFieldStyleAttrDesc *ad,Geo2dTileExtent *Tile);

#endif
