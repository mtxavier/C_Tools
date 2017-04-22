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

#ifndef _Gui_h_
#define _Gui_h_

#include <Geo.h>

/*______________________________________________________________
 |
 |
 |______________________________________________________________
*/

struct C_Img {
    char _;
    struct {
        char _;
        struct {
            char _,Unknown,RGB,RGBA,Palette,Grey;
        } Type;
    } Format;
    struct {
        char _;
        struct {
            char _,Empty,Outlined,Filled,OutlinedFilled,Stippled,OutlinedStippled,Mapped,OutlinedMapped;
        } Polygon;
        struct {
            char _,None,Straight,Sharp,Round;
        } Join;
    } Style;
};

struct C_Gui {
    char _;
    struct {
        char _;
        char Processed,Ignored,Finished;
        struct {
            char _,Play,Pause,Stop;
        } Suspend;
    } Event;
    /* For terminal binding */
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

/*______________________________________________________________
 |
 |______________________________________________________________
*/

typedef struct {
    unsigned char r,g,b,a;
} GuiRGBA;

#define GuiRGBAlpha_Opaque 255
#define GuiRGBAlpha_Transparent 0

    /*-------*/

typedef struct { struct GuiRgbPalette *Static; } GuiRgbPalette;
struct GuiRgbPalette {
    void (*GetRGBA)(GuiRgbPalette *this,GuiRGBA *R,int Color);
    int (*GetColor)(GuiRgbPalette *this,GuiRGBA *Color);
    void (*SetColor)(GuiRgbPalette *this,int Color,GuiRGBA *Value);
};
extern GuiRgbPalette GuiRgbPaletteNull;
GuiRgbPalette *GuiNoPalette(void); // The NoPalette: 0 colors, keeps the RGB value
GuiRgbPalette *GuiMonochromePalette(GuiRGBA *Black,GuiRGBA *White);
GuiRgbPalette *GuiFixed8Palette(void);
GuiRgbPalette *GuiFixed16Palette(void);
GuiRgbPalette *GuiFixed256Palette(void);
GuiRgbPalette *Gui565Palette(void);
GuiRgbPalette *Gui24BitsPalette(void);
GuiRgbPalette *Gui32BitsPalette(void);
GuiRgbPalette *GuiInversedPalette(GuiRgbPalette *pal);
GuiRgbPalette *GuiHighlightedPalette(GuiRgbPalette *pal);
GuiRgbPalette *GuiVariablePalette(int bits);

    /*-------*/

typedef struct {
    int Format;
    union {
        struct {
            int nb;
            int key;
            GuiRgbPalette *Colors;
        } Palette;
        struct {
            int EndianSwap;
            struct {
                int depth,offset;
            } R,G,B,A,Key;
        } RGBA;
    } Desc;
} ImgFormat;

typedef struct {
    ImgFormat Format;
    Geo2dExtent Dim;
    int pitch;
    char *Data;
} ImgDesc;

ImgDesc *GuiLoadPngFile(char *filename,int OrgAtBottom,int PitchAlign);
ImgDesc *GuiLoadJpeg(char *filename,int OrgAtBottom,int PitchAlign);
    // Tiff and Gif Load returns a 0 terminated array of ImgDesc *
    // That's because those 2 formats might allow multiple pictures
    // stored in a single unit.
ImgDesc **GuiLoadTiff(char *filename,int OrgAtBottom,int PitchAlign);
ImgDesc **GuiLoadGif(char *filename,int OrgAtBottom,int PitchAlign);
ImgDesc **GuiLoadImage(char *fname,int OrgAtBottom,int PitchAlign);


/*______________________________________________________________
 |
 |______________________________________________________________
*/

#define InputDevice_Keyboard 0x100
#define InputDevice_Mouse 0x200
#define InputDevice_TouchScreen 0x300
#define InputDevice_Joypad 0x4000
#define InputDevice_JoypadAxis 0x8000

typedef struct {
    struct GuiInput *Static;
} GuiInput;
struct GuiInput {  // All those function return C_(Gui.Event,Finished) to signal end of processing.
    int (*KeyDown)(GuiInput *this,int device,int key);
    int (*KeyUp)(GuiInput *this,int device,int key);
    int (*Cursor)(GuiInput *this,int device,int x,int y);
    int (*msTime)(GuiInput *this,int *next,int current);
    int (*Suspend)(GuiInput *this,int evt);
};
extern GuiInput GuiInputNull;

/*______________________________________________________________
 |
 |
 |______________________________________________________________
*/

typedef struct Gui2dEngine Gui2dEngine;

typedef struct {
    struct GuiOutput *Static;
} GuiOutput;
struct GuiOutput {
    void (*Display)(GuiOutput *this,Gui2dEngine *Engine);
};

extern GuiOutput GuiOutputNull;

GuiOutput *GuiOut2dPos(int x,int y,GuiOutput *Child);
GuiOutput *GuiOut2dAttach(int x,int y,GuiOutput *Child);
GuiOutput *GuiOutRelClip(int x,int y,int w,int h,GuiOutput *Child);
GuiOutput *GuiOutLineWidth(int w,GuiOutput *Child);
GuiOutput *GuiOutFilledPoly(int ftype,void *FillPattern,GuiOutput *Child);
GuiOutput *GuiOutFgColor(unsigned char r,unsigned char g,unsigned char b,unsigned char a,GuiOutput *Child);
GuiOutput *GuiOutBgColor(unsigned char r,unsigned char g,unsigned char b,unsigned char a,GuiOutput *Child);

extern GuiOutput GuiOutClearedScreen;
extern GuiOutput GuiOutClearBack;

GuiOutput *GuiOutRectangle(int w,int h);
GuiOutput *GuiOutLine(int dx,int dy);
GuiOutput *GuiOutPoint(void);
GuiOutput *GuiOutCircle(int r,int w);
GuiOutput *GuiOutDisk(int r);
GuiOutput *GuiOutShape(Geo2dShape *Shape);
GuiOutput *GuiOutFrame(int w,int h,int wdth);

GuiOutput *GuiOutPair(GuiOutput *A,GuiOutput *B);
GuiOutput *GuiNOutput(int n,GuiOutput **Children);
GuiOutput *GuiXOutput(GuiOutput *c0,...);

/*--------------------------------*/

GuiOutput *GuiDOutContainer(GuiOutput **Child);
GuiOutput *GuiDOutPos2d(int *x,int *y,GuiOutput *Child);

    /*----------*/

typedef struct { struct GuiLibrary *Static;} GuiLibrary;
struct GuiLibrary {
    int (*GetPageId)(GuiLibrary *this); // -1 in case of failure; returns/reserve a free Id;
    void (*ReleasePageId)(GuiLibrary *this,int Id);
        // PageLoad/PageImport return (0!=0) if failed; if Id is in use, the associated lib will be released
    int (*PageLoad)(GuiLibrary *this,int Id,char *filename);
    int (*PageImport)(GuiLibrary *this,int Id,ImgDesc *Desc);
    GuiOutput *(*Picture)(GuiLibrary *this,int PageId,Geo2dRectangle *Dim);
    GuiOutput *(*SwapPalPicture)(GuiLibrary *this,GuiRgbPalette *Pal,int PageId,Geo2dRectangle *Dim);
    int (*RecordEngine)(GuiLibrary *this,Gui2dEngine *Engine);
    void (*ReleaseEngine)(GuiLibrary *this,int Engine);
};

extern GuiLibrary GuiLibraryNull;
GuiLibrary *GuiLibraryNew(void);

/*--------------------------------------*/

typedef struct {
    struct GuiEngine *Static;
} GuiEngine;
struct GuiEngine {
    int (*Open)(GuiEngine *this);
    void (*Close)(GuiEngine *this);
    void (*EventLoop)(GuiEngine *this,GuiOutput *Output,GuiInput *Input);
};

GuiEngine *SDLGuiEngineNew(char *WindowName,GuiLibrary *ImgLib,int ExpectedWidth,int ExpectedHeight,int *FrameRate);
GuiEngine *OGLGuiEngineNew(char *WindowName,GuiLibrary *Library,int w,int h,int *FrameRate);

#endif
