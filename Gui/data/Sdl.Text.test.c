#include <StackEnv.h>
#include <Classes.h>
#include <Gui.h>
#include <GuiText.h>
#include <Field.h>
#include <stdio.h>

static char fontname[] = "data/iso_8859-1.png";

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

static int my_GuiOutUpdated(GuiOutput *this) {
	ThisToThat(my_Gui,GuiOutput);
	return that->Updated;
}
static void my_GuiOutDisplay(GuiOutput *this,Render2dEngine *Engine) {
	ThisToThat(my_Gui,GuiOutput);
	Call(that->Text.Window,Display,1(Engine));
	that->Updated = (0!=0);
}

static void sFieldPrint(Geo2dField *dst,char *s,int ts) {
	char *p,*d;
	static char Tab[] = "                ";
	d = p = s;
	while (*p!=0) {
		if (*p=='\n') {
			Call(dst,AddData,3(1,d,p));
			Call(dst,AddSeparator,2(C_(Txt.Paragraph.Mark,EndOfLine),1));
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
	Call(dst,AddData,3(1,d,p));
}
static int myInKeyDown(GuiInput *this,int device,int key) {
	int Ended;
	ThisToThat(my_Gui,GuiInput);
	Ended = (0!=0);
	if ((device & 0xff00)==InputDevice_Keyboard) {
		Ended = (key=='q')||(key=='Q');
		if (!Ended) {
			if (!that->Appli.Inited) {
				that->Appli.f = fopen("src/Sdl.Text.test.c","r");
				that->Appli.End = (that->Appli.f==NULL);
			}
			if (!that->Appli.End) {
				char Buffer[256],*p,*e;
				int end;
				p = Buffer; e = p+255;
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
						fclose(that->Appli.f);
					}
				}
				*p = 0;
				sFieldPrint(that->Text.Text,Buffer,4);
			}
		}
	}
	return Ended;
}
static int myInKeyUp(GuiInput *this,int device,int key) { return (0!=0); }
static int myInCursor(GuiInput *this,int device,int x,int y) { return (0!=0); }
static int mymsTime(GuiInput *this,int *next,int current) { 
	*next = current+10;
	return (0!=0);
}
static int mySuspend(GuiInput *this,int evt) { return (0!=0); }


static my_Gui *my_GuiNew(void) {
	my_Gui *R;
	static struct GuiOutput OutStatic = {&my_GuiOutUpdated,&my_GuiOutDisplay};
	static struct GuiInput InStatic = {
		&myInKeyDown,&myInKeyUp,&myInCursor,&mymsTime,&mySuspend
	};
	GuiFontGlypheDesc fontMetric;
	rPush(R);
	R->GuiOutput.Static = &OutStatic;
    fontMetric.Grab.x = fontMetric.Grab.y = 0;
	fontMetric.Extent.w = 45;
	fontMetric.Extent.h = 49;
	fontMetric.dx.x = fontMetric.Extent.w;
	fontMetric.dx.y =0;
	fontMetric.dy.x = 0;
	fontMetric.dy.y = fontMetric.Extent.h;
	R->Font = GuiFontPage16x16Png(&fontMetric,fontname);
	R->Style = GuiFieldStyleNew(4);
	Call(R->Style,AddFont,4(0,12,0,R->Font));
	R->Text.Text = Geo2dFieldNew(8,0,80,25," ");
	R->Text.Ad.AttrByte = 0;
	R->Text.Ad.AttrBit = 0;
	R->Text.Ad.FntBit = 0;
	R->Text.Ad.PalBit = 0;
	R->Text.Ad.FgBit = 0;
	R->Text.Ad.BgBit = 0;
	R->Text.Ad.FontSize = 12;
	rnPush(R->Text.Ad.Palettes,1);
	R->Text.Ad.Palettes[0].fg = 0;
	R->Text.Ad.Palettes[1].bg = 0;
	R->Text.Tile.Extent = fontMetric.Extent;
	R->Text.Tile.dx = fontMetric.dx;
	R->Text.Tile.dy = fontMetric.dy;
	R->Text.Window = GuiFieldWindow(R->Text.Text,R->Style,&R->Text.Ad,&R->Text.Tile);
	R->Appli.Inited = (0!=0);
	return R;
}
main() {
	GuiEngine *engine;
	EnvOpen(4096,4096);
	engine = SDLGuiEngineNew();
	if (Call(engine,Open,0)) {
	    my_Gui *gui;
		gui = my_GuiNew();
		Call(engine,EventLoop,2(&gui->GuiOutput,&gui->GuiInput));
		Call(engine,Close,0);
	}
	EnvClose();
}
