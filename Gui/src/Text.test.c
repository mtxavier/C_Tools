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
#include <Gui.h>
#include <GuiText.h>
#include <GeoField.h>
#include <stdio.h>

typedef struct {
	GuiInput GuiInput;
	GuiOutput GuiOutput;
	GuiFontPageDesc *Font;
	GuiFieldStyle *Style;
	struct {
	    Geo2dField *Text;
	    GuiFieldStyleAttrDesc Ad;
		Geo2dTileExtent Tile;
	    GuiOutput *Window;
	} Text;
	struct {
	    int Inited,End;
		FILE *f;
	} Appli;
	int Updated;
} my_Gui;

static void my_GuiOutDisplay(GuiOutput *this,Gui2dEngine *Engine) {
	ThisToThat(my_Gui,GuiOutput);
	if (that->Updated) {
		that->Updated = !(that->Updated);
	    Call(that->Text.Window,Display,1(Engine));
	}
}

static void sFieldPrint(Geo2dField *dst,char *s,int ts) {
	char *p,*d,c;
	static char Tab[] = "                ";
	d = p = s;
	Tab[ts]=0;
	while (*p!=0) {
		if (*p=='\n') {
			Call(dst,AddData,3(1,d,p));
			Call(dst,AddSeparator,2(C_(Txt,Paragraph.Mark,EndOfLine),1));
			p++; d=p;
		} else {
			if (*p=='\t') {
	            Geo2dPoint Pos;
				Call(dst,AddData,3(1,d,p));
	            Call(dst,GetCursorPos,1(&Pos));
				Call(dst,AddData,3(1,Tab+(Pos.x%ts),Tab+ts));
				p++; d=p;
			} else {
				p++;
			}
		}
	}
	Tab[ts] = ' ';
	Call(dst,AddData,3(1,d,p));
}
static int myInKeyDown(GuiInput *this,int device,int key) {
	int Ended;
	ThisToThat(my_Gui,GuiInput);
	Ended = (0!=0);
	key = key&0xff;
	if ((device & 0xff00)==InputDevice_Keyboard) {
		Ended = (key=='q')||(key=='Q');
		if (!Ended) {
			that->Updated = (0==0);
			if (!that->Appli.Inited) {
				that->Appli.Inited = (0==0);
				that->Appli.f = fopen("data/Sdl.Text.test.c","r");
				that->Appli.End = (that->Appli.f==NULL);
			}
			if (!that->Appli.End) {
				char Buffer[256],*p,*e;
				int end;
				p = Buffer; e = p+255; *e = 0;
				end = feof(that->Appli.f);
				if (end) { that->Appli.End = end;}
				while (!end) {
					int c;
					c = fgetc(that->Appli.f);
					end = (c==EOF);
					if (!end) {
						*p++ = c;
						end = (p==e)||(c=='\n');
					} else {
						that->Appli.End = end;
						fclose(that->Appli.f);
					}
				}
				*p = 0;
				sFieldPrint(that->Text.Text,Buffer,4);
			}
		}
	}
	return (Ended?C_(Gui,Event,Finished):C_(Gui,Event,Processed));
}
static int myInKeyUp(GuiInput *this,int device,int key) { return C_(Gui,Event,Processed); }
static int myInCursor(GuiInput *this,int device,int x,int y) { return C_(Gui,Event,Ignored); }
static int mymsTime(GuiInput *this,int *next,int current) { 
	*next = current+10;
	return C_(Gui,Event,Processed);
}
static int mySuspend(GuiInput *this,int evt) { return C_(Gui,Event,Processed); }


static my_Gui *my_GuiNew(GuiLibrary *ImgLibr) {
	my_Gui *R;
	GuiFontGlypheDesc fontMetric;
	static struct GuiOutput OutStatic = {&my_GuiOutDisplay};
	static struct GuiInput InStatic = {
		&myInKeyDown,&myInKeyUp,&myInCursor,&mymsTime,&mySuspend
	};
	rPush(R);
	R->GuiOutput.Static = &OutStatic;
	R->GuiInput.Static = &InStatic;
	/* {
        static char fontname[] = "data/iso_8859-1.png";
	    Geo2dPoint liborg;
	    Geo2dExtent NextGlyph;
	    liborg.x = 6; liborg.y = 13;
 	    NextGlyph.w = 45; NextGlyph.h = 49;
        fontMetric.Grab.x = fontMetric.Grab.y = 0;
	    fontMetric.dx.x = 30-liborg.x; fontMetric.dx.y =0;
	    fontMetric.dy.x = 0; fontMetric.dy.y = 45-liborg.y;
	    fontMetric.Extent.w = fontMetric.dx.x; fontMetric.Extent.h = fontMetric.dy.y;
	    R->Font = GuiFontPage16x16Png(ImgLibr,fontname,&liborg,&NextGlyph,&fontMetric);
	} */
	{
		fontMetric.Grab.x = 0;
		fontMetric.Grab.y = 12;
		fontMetric.dx.x = 7; fontMetric.dx.y = 0;
		fontMetric.dy.x = 0; fontMetric.dy.y = 14;
		fontMetric.Extent.w = fontMetric.dx.x; fontMetric.Extent.h = fontMetric.dy.y;
	    R->Font = GuiTTFontLoad(ImgLibr,&fontMetric.Extent,2,"/usr/share/fonts/X11/misc/7x14.pcf.gz",0);
	}
	R->Style = GuiFieldStyleNew(4);
	Call(R->Style,AddFont,4(0,12,0,R->Font));
	R->Text.Text = Geo2dFieldNew(8,0,80,20," ");
	R->Text.Ad.AttrByte = 0;
	R->Text.Ad.AttrBit = 0;
	R->Text.Ad.FntBit = 0;
	R->Text.Ad.PalBit = 0;
	R->Text.Ad.FgBit = 0;
	R->Text.Ad.BgBit = 0;
	R->Text.Ad.FontSize = 12;
	R->Text.Ad.Palettes = R->Text.Ad.ePal = 0; 
	// rnPush(R->Text.Ad.Palettes,1);
	// R->Text.Ad.Palettes[0].fg = GuiNoPalette();
	// R->Text.Ad.Palettes[0].bg = GuiNoPalette();
	R->Text.Tile.Extent = fontMetric.Extent;
	R->Text.Tile.dx = fontMetric.dx;
	R->Text.Tile.dy = fontMetric.dy;
	R->Text.Window = GuiOutPair(
		&GuiOutClearBack,
	    GuiOutFgColor(255,255,0,GuiRGBAlpha_Opaque,
		  GuiOutBgColor(0,0,128,GuiRGBAlpha_Opaque,
		    GuiOut2dPos(fontMetric.Grab.x,fontMetric.Grab.y,
			  GuiFieldWindow(R->Text.Text,R->Style,&R->Text.Ad,&R->Text.Tile)
		)))
	);
	R->Updated = (0==0);
	R->Appli.Inited = (0!=0);
	return R;
}

extern GuiEngine *GuiEngineNew(char *WindowName,GuiLibrary *Library,int w,int h,int *FrameRate);

main() {
	GuiEngine *engine;
	int FrameRate;
	GuiLibrary *Libr;
	EnvOpen(4096,4096);
	FrameRate = 30;
	Libr = GuiLibraryNew();
	engine = GuiEngineNew("TextTest",Libr,800,600,&FrameRate);
	if (Call(engine,Open,0)) {
	    my_Gui *gui;
		gui = my_GuiNew(Libr);
		Call(engine,EventLoop,2(&gui->GuiOutput,&gui->GuiInput));
		Call(engine,Close,0);
	}
	EnvClose();
}
