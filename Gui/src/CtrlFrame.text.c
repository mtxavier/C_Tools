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

/*---------------------------------*/

typedef struct {
	GuiRGBA Bg,Fg;
	struct {
        GuiRGBA Color[4];
	    int Offset,Width;
	} Frame;
	struct {
		Geo2dPoint Org;
		Geo2dExtent Interspace;
	} WidgetMngr;
} GuiFormStyle;

typedef struct {
	GuiOutput GuiOutput;
    GuiFormStyle *Style;
	Geo2dExtent *Extent;
	int *State;
} GuiOutFrameHighlight;

static void frameHighlightDisplay(GuiOutput *this,Gui2dEngine *eng) {
	ThisToThat(GuiOutFrameHighlight,GuiOutput);
    rOpen {
	    GuiOuput *pic;
		GuiRGBA *col;
		int of;
		col = that->Style->Frame.Color[(that->State&3)];
		of = that->Style->Frame.Offset;
        pic = GuiOutFgColor(col->r,col->g,col->b,col->a,
			GuiOut2dPos(-of,-of,
					GuiOutFrame(that->Extent.w+(of<<1),that->Extent.h+(of<<1),that->Style->Frame.Width))
		);
		Call(pic,Display,1(eng));
	} rClose
}

static GuiOutput *outFrameHighlightNew(GuiFormStyle *Style,Geo2dExtent *Extent,int *State) {
	GuiOutFrameHighlight *r;
	static struct GuiOutput Static = {frameHighlightDisplay};
	rPush(r);
	r->GuiOutput.Static = &Static;
	r->Style = Style;
	r->State = State;
	r->Extent = Extent;
	return &r->GuiOutput;
}

/*-------------------------------------------*/

#include <List.h>

typedef struct {
	struct GuiCtrl *Static;
	GuiFormStyle *Style;
	Geo2dExtent Extent;
	GuiInput GuiInput;
	GuiOutput GuiOutput;
} GuiCtrl;

struct GuiCtrl {
	int (*Select)(GuiCtrl *this,int NewState);
};

static void CtrlNullDisplay(GuiOutput *this,GuiEngine *eng) {}
static int CtrlNullSelect(GuiCtrl *this,int newst) { return C_(Ctrl.Select,Disabled); }
static int CtrlNullKeyDown(GuiInput *this,int device,int key) { return C_(Gui.Event,Finished); }
static int CtrlNullKeyUp(GuiInput *this,int device,int key) {return C_(Gui.Event,Finished);}
static int CtrlNullCursor(GuiInput *this,int device,int x,int y) { return C_(Gui.Event,Finished); }
static int CtrlNullMsTime(GuiInput *this,int *next,int current) { *next=current+3000; return C_(Gui.Event,Finished);}
static int CtrlNullFrameSuspend(GuiInput *this,int Evt) { return C_(Gui.Event,Finished);}

extern GuiCtrl GuiCtrlNull;
static struct GuiCtrl CtrlNullStatic = {CtrlNullSelect};
static struct GuiOutput CtrlNullOutputStatic = {CtrlNullDisplay};
static struct GuiInput CtrlNullInputStatic = {
	CtrlNullKeyDown,CtrlNullKeyUp,CtrlNullCursor,CtrlNullMsTime,CtrlNullFrameSuspend
};
GuiCtrl GuiCtrlNull = {
	&CtrlNullStatic,0,{0,0},{&CtrlNullInputStatic},{&CtrlNullOutputStatic}
};

/*--------------------------------------------*/

typedef struct {
	GuiCtrl GuiCtrl;
	struct ctrlFrameChild {
		GuiCtrl *Ctrl;
		Geo2dPoint Pos;
		int State,nMs;
	} *b,*e,*ee,*c,*ch;
} ctrlVertFrame;

extern struct {
	char _;
	struct {
		char _,Disabled,Enabled,Highlighted,Selected;
	} Select;
	struct {
		char _,Up,Left,Down,Right,Valid,Escape,Extra;
	} MoveButtons;
} C_Ctrl;

static int ButtonTranslateToMove(int device,int key) {
	int r;
	r = C_(Ctrl.MoveButtons,Extra);
	if ((device&0xff00) == InputDevice_Keyboard) {
	switch (key&0xff) {
    case GuiKeyFn_Enter:
	case GuiKeyFn_xEnter:
	case GuiKey_Space:
		r = C_(Ctrl.MoveButtons,Valid);
	break;
	case GuiKeyFn_Esc:
	case GuiKeyFn_Suppr:
	case GuiKeyFn_xSuppr:
	    r = C_(Ctrl.MoveButtons,Escape);
	break;
    case GuiKeyFn_Left:
	case GuiKeyFn_xLeft:
        r = C_(Ctrl.MoveButtons,Left);
	break;
	case GuiKeyFn_Right:
	case GuiKeyFn_xRight:
	    r = C_(Ctrl.MoveButtons.Right);
	break;
	case GuiKeyFn_Up:
	case GuiKeyFn_xUp:
	    r = C_(Ctrl.MoveButtons,Up);
	break;
	case GuiKeyFn_Down:
	case GuiKeyFn_xDown:
	    r = C_(Ctrl.MoveButtons,Down);
	break;
	}
	}
	return r;
}

static int frameSubProcessed(ctrlVertFrame *Frame,int res) {
	int r;
	r = (res==C_(Gui.Event,Ignored))?res:C_(Gui.Event,Processed);
	if ((res==C_(Gui.Event,Finished))||(res==C_(Gui.Event,Suspend.Stop))) {
		Frame->c->State = Call(Frame->c->Ctrl,Select,1(C_(Ctrl.Select,Disabled)));
	} else {
		if (res==C_(Gui.Event,Suspend.Pause)) {
			Frame->c->State = Call(Frame->c->Ctrl,Select,1(C_(Ctrl.Select,Highlighted)));
		}
	}
	return r;
}
static int frameChildSelectSwitch(ctrlVertFrame *Frame) {
	if (Frame->ch->State!=C_(Ctrl.Select.Disabled)) {
        if (Frame->c->State!=C_(Ctrl.Select.Disabled)) {
			Frame->c->State = Call(Frame->c->Ctrl,Select,1(C_(Ctrl.Select.Enabled)));
        }
		Frame->ch->State = Call(Frame->ch->Ctrl,Select,1(C_(Ctrl.Select.Selected)));
		Frame->c = Frame->ch;
	}
	return C_(Gui.Event,Processed);
}
static int frameEscape(ctrlVertFrame *Frame) {
	if (Frame->ch->State!=C_(Ctrl.Select.Disabled)) {
		Frame->ch->State= Call(Frame->ch->Ctrl,Select,1(C_(Ctrl.Select.Enabled)));
	}
	if (Frame->c->State!=C_(Ctrl.Select.Disabled)) {
		Frame->c->State = Call(Frame->c->Ctrl,Select,1(C_(Ctrl.Select.Enabled)));
	}
	return C_(Gui.Event,Suspend.Pause);
}
static int frameHighlightSwitch(ctrlVertFrame *Frame,struct ctrlFrameChild *chld) {
	int r;
	r = C_(Gui.Event,Processed);
	if  (chld->State!=C_(Ctrl.Select,Disabled)) {
	    if ((Frame->ch->State!=C_(Ctrl.Select,Disabled))&&(Frame->ch->State!=C_(Ctrl.Select,Selected))) {
		    Frame->ch->State = Call(Frame->ch->Ctrl,Select,1(C_(Ctrl.Select,Enabled)));
	    }
	    if (chld->State==C_(Ctrl.Select,Highlighted)) {
		    chld->State = Call(chld->Ctrl,Select,1(C_(Ctrl.Select,Enabled)));
	    }
		Frame->ch = chld;
	}
	return r;
}
static struct ctrlFrameChild *frameChildGet(ctrlVertFrame *Frame,int x,int y) {
	struct ctrlFrameChild *c,*r,*e;
	c = Frame->b; r = e = Frame->e;
	do {
        if ((r->Pos.x<=x)&&(r->Pos.y<=y)&&(x<(r->Pos.x+r->Ctrl->Extent.w))&&(y<(r->Pos.y+r->Ctrl->Extent.h))) { r=c; }
		c++;
	} while((c<e)&&(r==e));
	return r;
}
static int framePrevious(ctrlVertFrame *Frame) {
	int r;
	r = C_(Gui.Event,Processed);
    if (Frame->ch>Frame->b) {
	    ctrlFrameChild *c;
		c = Frame->ch;
		do { c--; } while ((c->State==C_(Ctrl.Select,Disabled))&&(c>Frame->b));
		if (c->State!=C_(Ctrl.Select,Disabled)) { r = frameHighlightSwitch(Frame,c); }
	}
	return r;
}
static int frameNext(ctrlVertFrame *Frame) {
	int r;
	r = C_(Gui.Event,Processed);
    if (Frame->ch<Frame->e-1) {
		ctrlFrameChild *c;
		c = Frame->ch;
		do { c++; } while ((c->State==C_(Ctrl.Select,Disabled))&&(c<Frame->e-1));
		if (c->State!=C_(Ctrl.Select,Disabled)) { r = frameHighlightSwitch(Frame,c); }
	}
	return r;
}

static int vertFrameKeyDown(GuiInput *this,int device,int key) {
	int r,processed;
	ThisToThat(ctrlVertFrame,GuiCtrl.GuiInput);
	r = C_(Gui.Event,Ignored);
	processed = (that->c->Status==C_(Ctrl.Select,Selected));
    if (processed) {
		r = frameSubProcessed(that,Call(&(that->c->Ctrl->Input),KeyDown,2(device,key)));
		processed = (r!=C_(Gui.Event,Ignored));
	}
	if (!processed) {
		int k;
        k = ButtonTranslateToMove(device,key);
		if (k!=C_(Ctrl.MoveButtons,Extra)) {
            if ((k==C_(Ctrl.MoveButtons,Up))||(k==C_(Ctrl.MoveButtons,Down))) {
				if (k==C_(Ctrl.MoveButtons,Up)) {
				    r = framePrevious(that);
				} else {
					r = frameNext(that);
				}
			} else {
				if ((k==C_(Ctrl.MoveButtons,Escape))||(k==C_(Ctrl.MoveButtons,Valid))) {
					if (k==C_(Ctrl.MoveButton,Escape)) {
						r = frameEscape(that);
					} else {
						r = frameChildSelectSwitch(that);
					}
				}
			}
		} else {
			if ((device&0xff00)==InputDevice_Mouse) {
                if (key == 1) { r = frameChildSelectSwitch(that); }
			}
		}
	}
	return r;
}
static int vertFrameKeyUp(GuiInput *this,int device,int key) {
	int r;
	ThisToThat(ctrlVertFrame,GuiCtrl.GuiInput);
	if (that->c->State==C_(Ctrl.Select,Selected)) { 
		r = frameSubProcessed(that,Call(&(that->c->GuiCtrl.Input),KeyUp,2(device,key))); 
	} else { r = C_(Gui.Event,Ignored);}
	return r;
}
static int vertFrameCursor(GuiInput *this,int device,int x,int y) {
	int r,inside;
	ThisToThat(ctrlVertFrame,GuiCtrl.GuiInput);
	r = C_(Gui.Event,Ignored);
	if (((device&0xff00)==InputDevice_Mouse)||((device&0xff00)==InputDevice_TouchScreen)) {
		// Mouses and Touchscreen coordinates are relative to the position of the current widget.
	    inside = ((x>=0)&&(y>=0)&&(x<this->Extent.w)&&(y<this->Extent.h));
	    if (!inside) {
		    if (x<0) { x = that->c->Pos.x+x; }
		    if (x>=this->Extent.w) { x = that->c->Pos.x+that->c->Ctrl->Extent.w-this->Extent.w; }
		    if (y<0) { y = that->c->Pos.y+y; }
		    if (y>=this->Extent.h) { y = that->c->Pos.y+that->c->Ctrl->Extent.h-this->Extent.h; }
		    r = frameSubProcessed(that,Call(that->c,Cursor,3(device,x-that->c->Pos.x,y-that->c->Pos.y)));
			if (that->ch->State==C_(Ctrl.Select,Highlighted)) {
				that->ch->State = Call(that->ch->Ctrl,Select,1(C_(Ctrl.Select,Enabled)));
			}
	    } else {
	        if (that->c->State==C_(Ctrl.Select,Selected)) {
		        r = frameSubProcessed(that,Call(that->c,Cursor,3(device,x-that->c->Pos.x,y-that->c->Pos.y)));
            }
		    c = frameChildGet(that,x,y);
            if (c>=that->e) { c=that->c; }
			if (c!=that->ch) { frameHighlightSwitch(that,c); }
	    }
	} else {
		struct ctrlFrameChild *c;
		// Other devices probably aren't subject to relative positionning.
		if (that->c->State==C_(Ctrl.Select,Selected)) {
		    r = frameSubProcessed(that,Call(that->c,Cursor,3(device,x,y)));
		}
	}
	return r;
}
static int frameMsTime(GuiInput *this,int *next,int current) {
	struct ctrlFrameChild *c,*e;
	int r,n,min,dn;
	ThisToThat(ctrlVertFrame,GuiCtrl.GuiInput);
	c = that->b; e = that->e;
	min = TlsMaxSignedVal(int);
	while (c<e) {
		if ((c->State!=C_(Ctrl.Selected,Disabled))&&((current-c->nMs)>=0)) {
			r = Call(&(c->Ctrl.Input),MsTime,2(&n,current));
            c->nMs = n; dn = n-current;
			if ((dn>0)&&(dn<min)) min = dn;
			if ((r==C_(Gui.Event,Finished))||(r==C_(Gui.Event,Suspend.Stop))) {
				c->State = C_(Ctrl.Selected,Disabled);
			}
	    }
		c++;
	}
    *next = min+current;
	return C_(Gui.Event,Processed);
}
static int vertFrameSuspend(GuiInput *this,int evt) {
	ThisToThat(ctrlVertFrame,GuiCtrl.GuiInput);
	return C_(Gui.Event,Ignored);
}

static void vertFrameDisplay(GuiOutput *this,Gui2dEngine *Engine) {
	struct ctrlFrameChild *c,*e;
	int In;
	ThisToThat(ctrlVertFrame,GuiCtrl.GuiOutput);
    c = that->b; e = that->e;
	In = (0!=0);
	rOpen
	while ((c<e)&&(!In)) {
		In = (c->Pos.y+c->Ctrl->Extent.h)>0;
		if (!In) c++;
	}
	while ((c<e)&&(In)) {
		In = (c->Pos.y<that->GuiCtrl.Extent.h);
		if (In) {
		    GuiOutput *ou;
			ou = GuiOutPair(
				outFrameHighlightNew(that->GuiCtrl.Style,&c->Ctrl->Extent,&c->State),
				GuiOutRelClip(c->Pos.x,c->Pos.y,c->Ctrl->Extent.w,c->Ctrl->Extent.h,
				    GuiOut2dPos(c->Pos.x,c->Pos.y,&c->Ctrl.Output)
			));
			Call(ou,Display,1(Engine));
			c++;
		}
	}
	rClose
}
static void horzFrameDisplay(GuiOutput *this,Gui2dEngine *Engine) {
	struct ctrlFrameChild *c,*e;
	int In;
	ThisToThat(ctrlVertFrame,GuiCtrl.GuiOutput);
	c =  that->b; e = that->e;
	In = (0!=0);
	while ((c<e)&&(!In)) {
		In = (c->Pos.x+c->Ctrl->Extent.w)>0;
		if (!In) { c++; }
	}
	rOpen
	while ((c<e)&&(In)) {
		In = (c->Pos.x<that->GuiCtrl.Extent.w);
		if (In) {
			GuiOutput *ou;
			ou = GuiOutPair(
				outFrameHighlightNew(that->GuiCtrl.Style,&c->Ctrl->Extent,&c->State),
				GuiOutRelClip(c->Pos.x,c->Pos.y,c->Ctrl->Extent.w,c->Ctrl->Extent.h,
				    GuiOut2dPos(c->Pos.x,c->Pos.y,&c->Ctrl.Output)
			));
			Call(ou,Display,1(Engine));
			c++;
		}
	}
	rClose
}

static int frameSelect(GuiCtrl *this,int NewState) {
	ThisToThat(ctrlVertFrame,GuiCtrl);
	if (NewState!=that->State) {
		if ((NewState==C_(Ctrl.Select,Selected))||(NewState==C_(Ctrl.Select,Highlighted))) {
			if ((that->ch->State==C_(Ctrl.Select.Highlighted))) {
				Call(&(that->ch->Ctrl),Select,1(that->ch->State));
			}
			if ((that->c->State==C_(Ctrl.Select.Selected))||(that->c->State==C_(Ctrl.Select.Highlighted))) {
				Call(&(that->c->Ctrl),Select,1(that->c->State));
			}
		} else {
            if (that->ch->State!=C_(Ctrl.Select,Disabled)) {
				Call(&(that->ch->Ctrl),Select,1(C_(Ctrl.Select,Disabled)));
			}
			if (that->c->State!=C_(Ctrl.Select,Disabled)) {
				Call(&(that->c->Ctrl),Select,1(C_(Ctrl.Select,Disabled)));
			}
		}
		that->State = NewState;
	}
	return NewState;
}

static ctrlVertFrame *VertFrameNew(GuiFormStyle *Style,Geo2dExtent *Extent,int MaxChildrens) {
	ctrlVertFrame *r;
	static struct GuiCtrl Ctrl = {frameSelect};
	static struct GuiOutput Output = {vertFrameDisplay};
	static struct GuiInput Input = {
		vertFrameKeyDown,vertFrameKeyUp,vertFrameCursor,frameMsTime,vertFrameSuspend
	};
	rPush(r);
	r->GuiCtrl.Static = &Ctrl;
	r->GuiCtrl.GuiOutput.Static = &Output;
	r->GuiCtrl.GuiInput.Static = &Input;
	r->GuiCtrl.Extent = *Extent;
	r->GuiCtrl.Style = Style;
	rnPush(r->b,MaxChildrens+1);
	r->ee = r->b+MaxChildrens;
	r->c = r->ch = r->e = r->b;
	return r;
}
#include <stdarg.h>
static ctrlVertFrame *vertFrameAdd(ctrlVertFrame *f,GuiCtrl *c,...) {
	va_list l;
	GuiCtrl *n;
	struct ctrlFrameChild *p,*e;
	int y;
	GuiFormStyle *Style;
	Style = f->GuiCtrl.Style;
	p = f->e; e=f->ee; n = c;
	if (p==f->b) {
		y = Style->WidgetMngr.Org.y;
	} else {
		y = p->Pos.y+p->Ctrl->Extent.h+Style->WidgetMngr.Interspace.h;
	}
    va_start(l,c);
	while((n!=0)&&(p<e)) {
		p->Ctrl = n;
		p->State = C_(Ctrl.Select,Enabled);
		p->nMs = 0;
        p->Pos.x = Style->WidgetMngr.Org.x;
		p->Pos.y = y;
		y += Style->WidgetMngr.Interspace.h+p->Ctrl->Extent.h;
		n = va_arg(l,GuiCtrl *);
		p++;
	}
	va_end(l);
	f->e = p;
	return f;
}

/*---------------------------------*/

typedef struct {
	GuiInput *GuiInput;
	GuiOutput *GuiOutput;
	GuiLibrary *Libr;
} my_Gui;

static my_Gui *my_GuiNew(GuiLibrary *Libr) {
	my_Gui *r;
	rPush(r);
	r->Libr = Libr;
	return r;
}

extern GuiEngine *GuiEngineNew(char *WindowName,GuiLibrary *Library,int w,int h,int *FrameRate);
main() {
	GuiEngine *engine;
	GuiLibrary *Libr;
	int FrameRate;
	EnvOpen(4096,4096);
	FrameRate = 25;
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

