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

#ifndef _Field_h_
#define _Field_h_

#include <Geo.h>

/*--------------------------------------------*/

struct C_Txt {
    char _;
    struct {
        char _;
        struct {
            char _,Left,Right,Center,Expand;
        } Align;
        struct {
            char _,Clip,Symbol,Word;
        } LineCut;
        struct {
            char _,Begin,Blank,Char,Tab,JoinLine,EndOfLine,EndOfBuffer;
        } Mark;
    } Paragraph;
    struct {
        char _;
        struct {
            char _,Plain,Bold,Italic,_mskpad_0[1],Underscored;
        } Style;
    } Font;
    struct {
        char _;
        struct {
            char _,Clip,WrapAround,Scroll;
        } Wrap;
        struct {
                char _,No,XAxis,YAxis,XYAxis,SlideX,SlideXY;
        } Tiling;
    } Box;
    struct {
        char _,Text,Page,Paragraph,Line,Word,Char;
        struct {char _,Begin,Current,End;} Mark;
    } Scope;
    struct {
        char _,Top,Bottom,BufferTop,BufferBottom,PageTop,PageBottom,
        Cursor,LineStart,LineEnd,WordStart,WordEnd,PrgphStart,PrgphEnd;
    } ScopeMark;
};

/*------------------------------------------*/

typedef struct {
    Geo2dExtent Extent;
    Geo2dPoint dx,dy;
} Geo2dTileExtent;
extern Geo2dTileExtent Geo2dTileExtent_Id;

/*------------------------*/

typedef struct {
    struct Geo2dFieldSetting *Static;
    struct {
        int Columns,Lines;
    } Dim;
    struct {
        int DataWidth,AttrWidth;
        unsigned char *Blnk,*BlnkMsk;
        unsigned char *Attr,*AttrMsk;
    } Data;
    struct {
        int LineCut; // C_(Txt.Paragraph.Linecut,{Clip|Symbol|Word})
        int Align; // C_(Txt.Paragraph.Align,{Left|Right|Center|Expand})
        int (*fIsSpace)(unsigned char *Field); // use in conjonction with line expansion.
        int Wrap; // C_(Txt.Box.Wrap,{Clip,WrapAround,Page,Scroll})
    } EditMode;
} Geo2dFieldSetting;

struct Geo2dFieldSetting { void (*SetEditMode)(Geo2dFieldSetting *this); };
Geo2dFieldSetting *Geo2dFieldSettingPtr(Geo2dFieldSetting *Target);

/*_______________________________________________________________________
 |
 | Use Geo2dFieldPeek and Geo2dFieldStream to parse a field.
 |_______________________________________________________________________
*/

typedef struct {
    struct Geo2dFieldPeek *Static;
} Geo2dFieldPeek;
struct Geo2dFieldPeek {
    int (*AttrSet)(Geo2dFieldPeek *this,unsigned char *Attr);
    int (*Data)(Geo2dFieldPeek *this,int x,int y,unsigned char *Datab);
};
typedef struct {
    struct Geo2dFieldStream *Static;
} Geo2dFieldStream;
struct Geo2dFieldStream {
    int (*AttrSet)(Geo2dFieldStream *this,unsigned char *Attr);
    int (*Data)(Geo2dFieldStream *this,unsigned char *Data,int Mark); // Mark among: C_(Txt.Paragraph.Mark,{...})
};

typedef struct {
    struct Geo2dField *Static;
} Geo2dField;
struct Geo2dField {
    Geo2dFieldSetting *(*GetSetting)(Geo2dField *this);
    void (*Clear)(Geo2dField *this,int From,int To); // Clear area and jump to the beginning of the cleared area.
    void (*SetTileAttr)(Geo2dField *this,unsigned char *Attrb,unsigned char *DataMask,unsigned char *AttrMask);
    void (*GotoXY)(Geo2dField *this,int From,int dx,int dy);
    char *(*AddData)(Geo2dField *this,int DataSize,char *sb,char *se);
    char *(*AddRawData)(Geo2dField *this,char *sb,char *se); // Format is Packed nx[Data|Attr]
    int (*AddSeparator)(Geo2dField *this,int Separator,int nb); 
        // Separator:one of C_(Txt.Paragraph,{Blank,Char,Tab,JoinLine,EndOfLine})
        // JoinLine is used to actually join the next lines (that have been separated by EndOfLine)
        // Char will update the next characters attribute with current Attr

    void (*GetCursorPos)(Geo2dField *this,Geo2dPoint *R);
    int (*GetData)(Geo2dField *this,unsigned char *Attr,unsigned char *Data,int x,int y);
    void (*ForEachInBox)(Geo2dField *this,Geo2dRectangle *Clip,Geo2dTileExtent *Tile,Geo2dFieldPeek *Perform);
    void (*StreamData)(Geo2dField *this,Geo2dPoint *From,Geo2dFieldStream *Perform);

    void (*GetCharNum)(Geo2dField *this,Geo2dPoint *CursorPos,int *yLineStart,int *xCharnum);
        // yLineStart is relative to the CursorPos: 0 -> same line, -y -> y lines above.
    void (*GetCharPos)(Geo2dField *this,Geo2dPoint *Pos,int yLine,int Charnum);
        // Charnum taken from the beginning of the line. The beginning of the line might differ from yLine.
};


/*_____________________________________________________
 |
 | Default is:
 |    Wrap: Discard
 |    LineCut: Symbol
 |    Align(not relevant with LineCut==Symbol): Left
 |_____________________________________________________
*/
Geo2dField *Geo2dFieldNull(void);
Geo2dField *Geo2dFieldNew(int DataWidth,int AttrWidth,int Columns,int Lines,unsigned char *Blank);

#endif
