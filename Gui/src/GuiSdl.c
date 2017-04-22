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
#include <List.h>

#include <SDL/SDL.h>

#include <Gui.local.h>

// #define TIMER_USE_SDL
#ifndef TIMER_USE_SDL
#include <GuiTime.h>
#endif

#define rgbaCode(code,color) { \
	union {int Code; GuiRGBA Color;} thacode; \
	thacode.Code = 0;\
	thacode.Color = (color); \
	code = thacode.Code; \
}
/*--------------------------------------------------
 |
 | SDL Specialisation for Render2dEngine
 |___________________________________________________
*/

static void getSdlPixelFormat(ImgFormat *dst,SDL_PixelFormat *src) {
	if (src->palette) {
		SDL_Color *color;
		dst->Format = C_(Img,Format.Type,Palette);
		dst->Desc.Palette.nb = src->palette->ncolors;
		dst->Desc.Palette.key = src->colorkey;
		/* ToDo : Palette parsing */
		dst->Desc.Palette.Colors = 0; 
		color = src->palette->colors;
	} else {
		unsigned int mask;
		int nb;
		dst->Format = C_(Img,Format.Type,RGBA);
		dst->Desc.RGBA.EndianSwap = (0!=0);

		dst->Desc.RGBA.R.offset = src->Rshift;
		nb = 0; mask = src->Rmask >> src->Rshift;
		while (mask&1) {nb++; mask>>1;}
		dst->Desc.RGBA.R.depth = nb;

		dst->Desc.RGBA.G.offset = src->Gshift;
		nb = 0; mask = src->Gmask >> src->Gshift;
		while (mask&1) {nb++; mask>>1;}
		dst->Desc.RGBA.G.depth = nb;

		dst->Desc.RGBA.B.offset = src->Bshift;
		nb = 0; mask = src->Bmask >> src->Bshift;
		while (mask&1) {nb++; mask>>1;}
		dst->Desc.RGBA.B.depth = nb;

		dst->Desc.RGBA.A.offset = src->Ashift;
		nb = 0; mask = src->Amask >> src->Ashift;
		while (mask&1) {nb++; mask>>1;}
		dst->Desc.RGBA.A.depth = nb;
        
		if (!dst->Desc.RGBA.A.depth) {
			dst->Format = C_(Img,Format.Type,RGB);
		}

	}
}

typedef struct {
	Render2dEngine Render2dEngine;
	GuiEngine GuiEngine;
	MemStack *Mem;
	struct {
        Geo2dExtent Dim;
		int bpp;
		int FrameRate;
	} Expected;
	struct {
	    List/*<sdlImg2dLib.List>*/ Lib,Free;
		GuiLibrary *Library;
		int EngineId;
	} Img2dLib;
	char *DriverName;
	Uint32 VideoInitFlags;
	SDL_Surface *Screen;
	SDL_VideoInfo const *ScreenInfo;
	struct {
		int Fg,Bg,Key;
		SDL_PixelFormat *sdlFormat;
		Uint32 sdlFg,sdlBg;
	} Color;
	struct geoShape {
	     Geo2dShapeMap Geo2dShapeMap;
		 int LastFg,LastY;
		 unsigned char Fg8Colors[32],*Fg8ColorsEnd[8];
		 unsigned char *dataBase,*pLastY;
		 int pitch,BytesPP;
	} Shape;
	struct {
        #define SDLBaseTime 30
		SDL_TimerID TimerId;
		int TimerRunning;
		int Last,Next;
	} Time;
} SDLEngine;

/*------------------------------*/

typedef struct {
	MemStack *Mem;
	Img2dLib Img2dLib;
	List/*<sdlImg2dLib.List>*/ List;
	SDLEngine *Engine;
    SDL_Surface *Surface;
	int Fg,Bg;
	SDL_Color *Colors; // for paletted surface only
	char *org,*desc; // The external Image Id (filename) for external Image; the data for internal Image.
	// For internal image: org is the original buffer and desc the internal buffer.
    int w,h,pitch;
	ImgFormat Format;
} sdlImg2dLib;

/*------------------------------*/

static void sdlImg2dLibInfo(Img2dLib *this,int *w,int *h,ImgFormat *format) {
	ThisToThat(sdlImg2dLib,Img2dLib);
	if (w) *w = that->w;
	if (h) *h = that->h;
    if (format) *format = that->Format;
}

static void sdlImg2dLibDisplay(Img2dLib *this,Geo2dRectangle *img) {
	SDL_Rect Dst,Area;
	SDLEngine *Engine;
	ThisToThat(sdlImg2dLib,Img2dLib);
	Engine = that->Engine;
	Area.w = Dst.w = img->Extent.w;
	Area.h = Dst.h = img->Extent.h;
	Dst.x = Engine->Render2dEngine.Cursor.x;
	Dst.y = Engine->Render2dEngine.Cursor.y;
	Area.x = img->Pos.x; 
	Area.y = img->Pos.y;
	if (this->Available) {
        if (SDL_BlitSurface(that->Surface,&Area,Engine->Screen,&Dst)==-2) { // the Img2dLib was lost somewhere.
			Call(this,Close,0);
			if (Call(this,Open,0)) {
				SDL_BlitSurface(that->Surface,&Area,Engine->Screen,&Dst);
			}
		}
	} else {
		if (Call(this,Open,0)) {
			SDL_BlitSurface(that->Surface,&Area,Engine->Screen,&Dst);
		}
	}
}

static void sdlImg2dLibGreyDisplay(Img2dLib *this,Geo2dRectangle *img) {
	ThisToThat(sdlImg2dLib,Img2dLib);
	if ((that->Engine->Color.Bg!=that->Bg)||(that->Engine->Color.Fg!=that->Fg)) {
		int i,cnb;
	    int key;
	    GuiRGBA bg,fg;
		SDL_Color *c,*ce;
	    bg = that->Engine->Render2dEngine.Back;
	    fg = that->Engine->Render2dEngine.Front;
	    key = that->Format.Desc.Palette.key;
		that->Bg = that->Engine->Color.Bg;
		that->Fg = that->Engine->Color.Fg;
		cnb = that->Format.Desc.Palette.nb;
		c = that->Colors+0;
		ce = c+(cnb-1);
		c->r = bg.r;
		c->g = bg.g;
		c->b = bg.b;
	    if (bg.a == GuiRGBAlpha_Transparent) key = 0;
		if (cnb>2) { // We keep cnb==2 as a special optimized case.
			struct {
			    int delta,acc;
			} r,g,b;
			cnb--;
			c++;
			r.delta = r.acc = fg.r-bg.r; r.acc += bg.r*cnb;
			g.delta = g.acc = fg.g-bg.g; g.acc += bg.g*cnb;
			b.delta = b.acc = fg.b-bg.b; b.acc += bg.b*cnb;
			while (c<ce) {
				c->r = (r.acc/cnb);  r.acc += r.delta;
				c->g = (g.acc/cnb);  g.acc += g.delta;
				c->b = (b.acc/cnb);  b.acc += b.delta;
				c++;
			}
		} else {
			c = ce;
		}
		c->r = fg.r;
		c->g = fg.g;
		c->b = fg.b;
	    if (fg.a == GuiRGBAlpha_Transparent) key = i;
	    SDL_SetPalette(that->Surface,SDL_LOGPAL,that->Colors,0,that->Format.Desc.Palette.nb);
	    if (key>=0) {
		    SDL_SetColorKey(that->Surface,SDL_SRCCOLORKEY/*|SDL_RLEACCEL*/,key);
	    } else {
		    SDL_SetColorKey(that->Surface,0,0);
	    }
	}
	sdlImg2dLibDisplay(this,img);
}
/*
static int sdlImg2dLibOpen(Img2dLib *this) {
	ThisToThat(sdlImg2dLib,Img2dLib);
	if (!this->Available) {
	    that->Surface = IMG_Load(that->desc);
	    this->Available = (that->Surface!=NULL);
		if (this->Available) {
			that->w = that->Surface->w;
			that->h = that->Surface->h;
			getSdlPixelFormat(&that->Format,that->Surface->format);
		} else {
			that->w = that->h = 0;
			that->Format.Format = C_(Img,Format.Type,Unknown);
		}
	}
	return this->Available;
}
*/
static void sdlImg2dLibClose(Img2dLib *this) {
	ThisToThat(sdlImg2dLib,Img2dLib);
	if ((this->Available)&&(that->Surface!=NULL)) {
		SDL_FreeSurface(that->Surface);
	}
	that->Surface = NULL;
	this->Available = (0!=0);
}
static void sdlImg2dLibFree(Img2dLib *this) {
	ThisToThat(sdlImg2dLib,Img2dLib);
	if (this->Available) { Call(this,Close,0); }
	{ 
	     List *n,*t;
		 n = &that->Engine->Img2dLib.Lib;
		 t = &that->List;
		 while ((n->n!=ListEnd) && (n->n!=t)) n = n->n;
		 if (n->n==t) {
			 n->n = t->n;
			 t->n = that->Engine->Img2dLib.Free.n;
			 that->Engine->Img2dLib.Free.n = t;
		 }
	}
}

/*----------------------------------*/

static char *translate256Pixmap(sdlImg2dLib *r) {
	int nb,bpp,opitch,mask;
	unsigned char *p,*q,*eq,*e,*ei;
	nb = r->Format.Desc.Palette.nb;
	opitch = r->pitch;
	p = (unsigned char *)(r->desc); ei = p+r->w*r->h;
	eq = (unsigned char *)(r->org);
	bpp = 1;
	while (nb>2) {bpp++; nb=nb>>1;}
	mask = (bpp>=8)?0xff:(0xff>>(8-bpp));
	while (p<ei) {
		unsigned int color,shift;
		e = p+r->w;
		color = 0; shift = 0;
		q = eq; eq += opitch;
		while (p<e) {
		    while (shift<bpp) {
                unsigned int ncol;
				ncol = *q++;
				color = (color<<8)|ncol;
				shift+=8;
			}
			*p++ = (color>>(shift-bpp))&mask;
			shift = shift-bpp;
	    }
	}
	return r->desc;
}

static void sdlImg2dMapFree(Img2dLib *this) {
	ThisToThat(sdlImg2dLib,Img2dLib);
	mLeave(that->Mem);
	sdlImg2dLibFree(this);
}
static int sdlImg2dMapOpen(Img2dLib *this) {
	ThisToThat(sdlImg2dLib,Img2dLib);
	if (!this->Available) {
		if ((that->Format.Format == C_(Img,Format.Type,Palette))||(that->Format.Format==C_(Img,Format.Type,Grey))) {
			// 8bits: normally, other depths should be taken care of (read translated) at ImgLib creation.
            if (that->desc!=that->org)  translate256Pixmap(that);
			that->Surface = SDL_CreateRGBSurfaceFrom(
				that->desc,that->w,that->h,8,that->w,0,0,0xff,0
			);
			if (that->Surface) {
			    SDL_SetPalette(that->Surface,SDL_LOGPAL,that->Colors,0,that->Format.Desc.Palette.nb);
				if (that->Format.Desc.Palette.key!=-1) {
		            SDL_SetColorKey(that->Surface,SDL_SRCCOLORKEY/*|SDL_RLEACCEL*/,that->Format.Desc.Palette.key);
				} else {
		            SDL_SetColorKey(that->Surface,0,0);
				}
			}
		} else {
		    int depth,Rmask,Bmask,Gmask,Amask;
            #define addDepth(cmpnt) \
			    depth += that->Format.Desc.RGBA.cmpnt.depth; \
			    cmpnt##mask = (-1)<<that->Format.Desc.RGBA.cmpnt.offset;\
			    cmpnt##mask = cmpnt##mask ^ (cmpnt##mask<<that->Format.Desc.RGBA.cmpnt.depth)
		    depth = 0;
			addDepth(R);
            addDepth(G);
            addDepth(B);
			addDepth(A);
            #undef addDepth
		    that->Surface = SDL_CreateRGBSurfaceFrom(
			    that->desc,that->w,that->h,depth,that->pitch,Rmask,Bmask,Gmask,Amask
		    );
        }
		this->Available = (that->Surface!=NULL);
	}
	return this->Available;
}

/*------------------------------*/

static int sdleShapeMapOpen(SDLEngine *eng) {
	SDL_Surface *scr;
	int r;
	scr = eng->Screen;
	r = (SDL_LockSurface(scr)>=0);
	if (r) {
		int Bpp,nFg;
        struct geoShape *shp;
        shp = &eng->Shape;
		shp->pitch = scr->pitch;
		shp->pLastY = shp->dataBase = scr->pixels;
		shp->LastY = 0;
		Bpp = scr->format->BytesPerPixel;
		nFg = eng->Color.Fg;
		if ((Bpp!=shp->BytesPP)||(nFg!=shp->LastFg)) {
			unsigned char *col,*ecol,*p,**pe,**epe;
		    shp->BytesPP = Bpp;
			shp->LastFg = nFg;
			p = shp->Fg8Colors;
			pe = shp->Fg8ColorsEnd; epe = pe+8;
			col = (unsigned char *)(&eng->Color.sdlFg); ecol = col+Bpp;
			while (pe<epe) {
				while (col<ecol) {*p++=*col++;}  /* !!! ToDo be careful of endianness here !!!! */
				*pe++ = p;
			    col = (unsigned char *)(&eng->Color.sdlFg);
			}
		}
	}
	return r;
}
static void sdleShapeMapClose(SDLEngine *eng) {
	SDL_UnlockSurface(eng->Screen);
}

static void sdleShapeMapBoundary(Geo2dShapeMap *this,Geo2dRectangle *R) {
	ThisToThat(SDLEngine,Shape.Geo2dShapeMap);
	*R = that->Render2dEngine.Clip;
	R->Pos.x -= that->Render2dEngine.Cursor.x;
	R->Pos.y -= that->Render2dEngine.Cursor.y;
}
static int sdleShapeMapLine(Geo2dShapeMap *this,int x,int y,int dx) {
	unsigned char *p,*col,*ecol;
	SDLEngine *eng;
	ThisToThat(struct geoShape,Geo2dShapeMap);
	eng = CastBack(SDLEngine,Shape,that);
	x+=eng->Render2dEngine.Cursor.x;
	y+=eng->Render2dEngine.Cursor.y;
	if (y!=that->LastY) {
		if (y==that->LastY+1) { that->pLastY += that->pitch; } else {
			if (y==that->LastY-1) { that->pLastY -= that->pitch;} else {
                that->pLastY = that->dataBase + (y*that->pitch); 
		}}
		that->LastY = y;
	}
	p = that->pLastY+(that->BytesPP*x);
	ecol = that->Fg8ColorsEnd[7];
	while (dx>8) {
		col = that->Fg8Colors;
		while (col<ecol) *p++=*col++;
		dx-=8;
	}
	if (dx) {
		col = that->Fg8Colors;
		ecol = that->Fg8ColorsEnd[dx-1];
		while (col<ecol) *p++=*col++;
	}
	return (0!=0);
}

static sdleShapeInit(SDLEngine *eng) {
	static struct Geo2dShapeMap ShapeStatic = {sdleShapeMapBoundary,sdleShapeMapLine};
    struct geoShape *shp;
	shp = &eng->Shape;
	shp->Geo2dShapeMap.Static = &ShapeStatic;
	shp->LastFg = 0;
	shp->LastY = 0;
	shp->pitch = 8;
	shp->dataBase = shp->pLastY = 0;
	shp->BytesPP = 3;
	{
	    unsigned char *p,*ep,**pe,**epe;
	    p = shp->Fg8Colors; 
	    pe = shp->Fg8ColorsEnd; 
	    epe = pe+8; 
	    while (pe<epe) { 
			ep=p+shp->BytesPP; 
			while (p<ep) *p++=0; 
			*pe++=p;
		}
	}
}

/*------------------------------*/

static sdlImg2dLib *GetImg2dLibSlot(SDLEngine *eng) {
	sdlImg2dLib *r;
	if (eng->Img2dLib.Free.n!=ListEnd) {
		List *R;
		R = eng->Img2dLib.Free.n;
		eng->Img2dLib.Free.n = R->n;
		R->n = ListEnd;
		r = CastBack(sdlImg2dLib,List,R);
	} else {
		MemStack *Mem;
		Mem = MemStackFork(eng->Mem,sizeof(*r)+16);
	    mPush(eng->Mem,r);
		r->Mem = Mem;
		r->List.n = ListEnd;
	}
	r->List.n = eng->Img2dLib.Lib.n;
	eng->Img2dLib.Lib.n = &r->List;
	return r;
}

/*
static Img2dLib *sdleNewImg2dLib(Render2dEngine *this,char *desc) {
	sdlImg2dLib *r;
	static struct Img2dLib Static = {
		sdlImg2dLibInfo,sdlImg2dLibDisplay,
		sdlImg2dLibOpen,sdlImg2dLibClose,sdlImg2dLibFree
	};
	ThisToThat(SDLEngine,Render2dEngine);
	r = GetImg2dLibSlot(that);
	r->Img2dLib.Static = &Static;
	r->Engine = that;
	r->desc = desc;
	r->Surface = 0;
	r->w = r->h = 0;
	r->Format.Format = C_(Img.Format.Type,Unknown);
	r->Img2dLib.Available = (0!=0);
	r->Img2dLib.Available = Call(&r->Img2dLib,Open,0);
	return &r->Img2dLib;
}
*/

static Img2dLib *sdleImportImg2dLib(Render2dEngine *this,ImgDesc *Desc) {
	sdlImg2dLib *r;
	static struct Img2dLib Static = {
		sdlImg2dLibInfo,sdlImg2dLibDisplay,
		sdlImg2dMapOpen,sdlImg2dLibClose,sdlImg2dMapFree
	};
	ThisToThat(SDLEngine,Render2dEngine);
	r = GetImg2dLibSlot(that);
	mEnter(r->Mem);
	r->Img2dLib.Static = &Static;
	r->Engine = that;
	r->Surface = NULL;
	r->org = r->desc = Desc->Data;
	r->w = Desc->Dim.w;
	r->h = Desc->Dim.h;
	r->pitch = Desc->pitch;
	r->Format = Desc->Format;
	if (r->Format.Format==C_(Img,Format.Type,Palette)||r->Format.Format==C_(Img,Format.Type,Grey)) {
		int nb,grey;
		GuiRgbPalette *col;
		grey = (r->Format.Format==C_(Img,Format.Type,Grey));
		col = r->Format.Desc.Palette.Colors;
		nb = r->Format.Desc.Palette.nb;
		if ((nb<=128)||(nb>256)) {
			int size;
			size = ((r->w*r->h)+0x7)&(-8);
	        mnPush(r->Mem,r->desc,size);
		    mnPush(r->Mem,r->Colors,nb);
		} else {
		    if (nb>=256) nb = 256;
		    mnPush(r->Mem,r->Colors,nb);
		}
		if (grey) {
			int i;
			GuiRGBA c;
	        static struct Img2dLib Static = {
		        sdlImg2dLibInfo,sdlImg2dLibGreyDisplay,
		        sdlImg2dMapOpen,sdlImg2dLibClose,sdlImg2dMapFree
	        };
	        r->Img2dLib.Static = &Static;
			c.a = GuiRGBAlpha_Opaque;
			c.r = c.g = c.b = 0; rgbaCode(r->Bg,c);
			c.r = c.g = c.b = 255; rgbaCode(r->Fg,c);
		    for (i=0;i<nb;i++) {
			    r->Colors[i].r = r->Colors[i].g = r->Colors[i].b = (nb>1)?(i*255)/(nb-1):0;
		    }
		} else {
			int i;
		    for (i=0;i<nb;i++) {
			    GuiRGBA c;
			    Call(col,GetRGBA,2(&c,i));
			    r->Colors[i].r = c.r;
			    r->Colors[i].g = c.g;
			    r->Colors[i].b = c.b;
		    }
		}
	}
	r->Img2dLib.Available = (0!=0);
	r->Img2dLib.Available = Call(&r->Img2dLib,Open,0);
	return &r->Img2dLib;
}
static void sdleColorUpdateFront(Render2dEngine *this) {
	ThisToThat(SDLEngine,Render2dEngine);
	that->Color.sdlFg = SDL_MapRGBA(that->Color.sdlFormat,this->Front.r,this->Front.g,this->Front.b,this->Front.a);
	rgbaCode(that->Color.Fg,this->Front);
}
static void sdleColorUpdateBack(Render2dEngine *this) {
	ThisToThat(SDLEngine,Render2dEngine);
	that->Color.sdlBg = SDL_MapRGBA(that->Color.sdlFormat,this->Back.r,this->Back.g,this->Back.b,this->Back.a);
	rgbaCode(that->Color.Bg,this->Back);
}
static void sdleColorUpdateKey(Render2dEngine *this) {
	ThisToThat(SDLEngine,Render2dEngine);
	rgbaCode(that->Color.Key,this->ColorKey);
}
static void sdleiColorUpdateKey(Render2dEngine *this) {
	ThisToThat(SDLEngine,Render2dEngine);
}
static void sdleAlphaKeyEnabledUpdate(Render2dEngine *this) {
	ThisToThat(SDLEngine,Render2dEngine);
}
static void sdleUpdateClip(Render2dEngine *this) {
	SDL_Rect rect;
	ThisToThat(SDLEngine,Render2dEngine);
	rect.x=this->Clip.Pos.x; rect.y=this->Clip.Pos.y; rect.w=this->Clip.Extent.w; rect.h=this->Clip.Extent.h;
	SDL_SetClipRect(that->Screen,&rect);
}
static void sdleSetStyle(Render2dEngine *this,int LineWidth,int JoinStyle,int FillType,void *FillPattern) {
	ThisToThat(SDLEngine,Render2dEngine);
	this->LineWidth = LineWidth;
	this->JoinStyle = JoinStyle;
	this->FillType = FillType;
	this->FillPattern = FillPattern;
}
static void sdlePoint(Render2dEngine *this) {
	SDL_Surface *scr;
	ThisToThat(SDLEngine,Render2dEngine);
	scr = that->Screen;
	if (SDL_LockSurface(scr)>=0) {
		if (
			this->Cursor.y>=this->Clip.Pos.y && this->Cursor.y<this->Clip.Pos.y+this->Clip.Extent.h 
			&& this->Cursor.x>=this->Clip.Pos.x && this->Cursor.x<=this->Clip.Pos.x+this->Clip.Extent.w
		) {
		    unsigned char *bdata,*data,*col,*ecol;
			int bpp;
			bpp = scr->format->BytesPerPixel;
		    bdata = scr->pixels;
		    data = bdata + (this->Cursor.y * scr->pitch) + (this->Cursor.x*bpp);
			col = (unsigned char *)(&that->Color.sdlFg); ecol = col+bpp;
			while (col<ecol) *data++=*col++;
		}
		SDL_UnlockSurface(that->Screen);
	}	
}
static void sdleLine(Render2dEngine *this,int dx,int dy) {
	ThisToThat(SDLEngine,Render2dEngine);
	if (sdleShapeMapOpen(that)) {
		Geo2dShape *Line;
		Geo2dPoint B,O;
		rOpen
		O.x = O.y = 0;
		B.x = dx;
		B.y = dy;
		Line = Geo2dShapeLine(&O,&B);
		Call(Line,ForEach,1(&that->Shape.Geo2dShapeMap));
		rClose
		sdleShapeMapClose(that);
	}
}
static void sdleShape(Render2dEngine *this,Geo2dShape *Shape) {
	ThisToThat(SDLEngine,Render2dEngine);
	if (sdleShapeMapOpen(that)) {
		Call(Shape,ForEach,1(&that->Shape.Geo2dShapeMap));
		sdleShapeMapClose(that);
	}
}
static void sdleBox(Render2dEngine *this,int w,int h) {
	SDL_Rect rect;
	ThisToThat(SDLEngine,Render2dEngine);
	rect.x = this->Cursor.x;
	rect.y = this->Cursor.y;
	rect.w = w;
	rect.h = h;
    SDL_FillRect(that->Screen,&rect,that->Color.sdlFg);
}
static void sdleScreenClear(Render2dEngine *this) {
	SDL_Rect rect;
    ThisToThat(SDLEngine,Render2dEngine);
    rect.x = rect.y = 0;
	rect.w = that->Screen->w;
	rect.h = that->Screen->h;
	SDL_FillRect(that->Screen,&rect,that->Color.sdlBg);
}

static Uint32 SDLTimerCallBack(Uint32 NextTick,void *sdlengine) {
	SDL_Event event;
	SDL_UserEvent userevent;
	userevent.type = SDL_USEREVENT;
	userevent.code = 0;
	userevent.data1 = NULL;
	userevent.data2 = NULL;
	event.type = SDL_USEREVENT;
	event.user = userevent;
	SDL_PushEvent(&event);
	return NextTick;
}

static int sdleOpen(GuiEngine *this) {
	int Continue;
	Uint32 InitFlag;
	ThisToThat(SDLEngine,GuiEngine);
    #ifdef TIMER_USE_SDL
	    InitFlag = SDL_INIT_TIMER|SDL_INIT_AUDIO|SDL_INIT_VIDEO|SDL_INIT_JOYSTICK;
    #else
	    InitFlag = SDL_INIT_AUDIO|SDL_INIT_VIDEO|SDL_INIT_JOYSTICK;
    #endif
	Continue = (SDL_Init(InitFlag)==0);
	if (Continue) {
		int Continue;
		that->ScreenInfo = SDL_GetVideoInfo();
		that->Screen = SDL_SetVideoMode(
			that->Expected.Dim.w,that->Expected.Dim.h,0,
			SDL_HWSURFACE|SDL_RESIZABLE|SDL_NOFRAME
		);
		Continue = (that->Screen != NULL);
		if (Continue) {
			that->Color.sdlFormat = that->Screen->format;
            #ifdef TIMER_USE_SDL
			    that->Time.Last = that->Time.Next = 0; // SDL_GetTicks();
			    if (!that->Time.TimerRunning) {
			        that->Time.TimerId = SDL_AddTimer(SDLBaseTime,SDLTimerCallBack,that);
			        that->Time.TimerRunning = (0==0);
			    }
            #else
				that->Time.Last = that->Time.Next = GuiTimeGetTime();
            #endif
		    Gui2dEngineDefaultInit(&that->Render2dEngine,that->Expected.Dim.w,that->Expected.Dim.h);
	        if (that->Img2dLib.EngineId == -1) {
			    that->Img2dLib.EngineId = Call(that->Img2dLib.Library,RecordEngine,1(&that->Render2dEngine));
			}
		} else {
			SDL_Quit();
		}
	}
	return Continue;
}

static void sdleClose(GuiEngine *this) {
	ThisToThat(SDLEngine,GuiEngine);
	Call(that->Img2dLib.Library,ReleaseEngine,1(that->Img2dLib.EngineId));
    /* Lib unloading */ {
		List *l;
        sdlImg2dLib *L;
		l = that->Img2dLib.Lib.n;
		while (l!=ListEnd) {
			L = CastBack(sdlImg2dLib,List,l);
            Call(&L->Img2dLib,Close,0);
			Call(&L->Img2dLib,Free,0);
			l->n = that->Img2dLib.Free.n;
			l = that->Img2dLib.Lib.n;
		}
	    if (that->Img2dLib.EngineId != -1) {
		   	Call(that->Img2dLib.Library,ReleaseEngine,1(that->Img2dLib.EngineId));
			that->Img2dLib.EngineId = -1;
		}
	}
    #ifdef TIMER_USE_SDL
	if (that->Time.TimerRunning) {
	    SDL_RemoveTimer(that->Time.TimerId);
	    that->Time.TimerRunning = (0!=0);
	}
    #endif
	SDL_VideoQuit();
	that->Screen = NULL;
	SDL_Quit();
}

#include <GuiKeyMap.h>
static void KeyboardTranslateInit(int *K) {
    int i,d;
    for (i=0;i<SDLK_LAST-SDLK_FIRST;i++) {
        K[i] = (i+SDLK_FIRST)&0xff;
    }
    d = SDLK_FIRST;
#define All(t,S,f) K[SDLK_##S-d]= GuiKeyTyp##t|(C_(Key,Scan,t.f)<<8)|(C_(Key,Scan,t.f))
    // GuiKeyTyp##t|((GuiKey##t##_##f<<8)|GuiKey##t##_##f)
#define Prt(t,S,f) K[SDLK_##S-d] |= GuiKeyTyp##t|(C_(Key,Scan,t.f)<<8)
    // GuiKeyTyp##t|(GuiKey##t##_##f<<8)
#define xAll(t,S,f,v) K[SDLK_##S-d]= GuiKeyTyp##t|(C_(Key,Scan,t.f)<<8)|v
    // GuiKeyTyp##t|(GuiKey##t##_##f<<8)|v
	xAll(Fn,ESCAPE,Esc,27);
	All(Fn,BACKSPACE,BackSpace);
	xAll(Fn,TAB,Tab,'\t');
	xAll(Fn,RETURN,Enter,'\n');
	All(Fn,LEFT,Left);
	All(Fn,RIGHT,Right);
	All(Fn,UP,Up);
	All(Fn,DOWN,Down);
	All(Fn,INSERT,Insert);
	All(Fn,DELETE,Suppr);
	All(Fn,PAGEUP,PgUp);
	All(Fn,PAGEDOWN,PgDown);
	All(Fn,HOME,Begin);
	All(Fn,END,End);
	All(Fn,F1,F1);
	All(Fn,F2,F2);
	All(Fn,F3,F3);
	All(Fn,F4,F4);
	All(Fn,F5,F5);
	All(Fn,F6,F6);
	All(Fn,F7,F7);
	All(Fn,F8,F8);
	All(Fn,F9,F9);
	All(Fn,F10,F10);
	All(Fn,F11,F11);
	All(Fn,F12,F12);
	xAll(Fn,KP0,xInsert,'0');
	xAll(Fn,KP1,xEnd,'1');
	xAll(Fn,KP2,xDown,'2');
	xAll(Fn,KP3,xPgDown,'3');
	xAll(Fn,KP4,xLeft,'4');
	xAll(Fn,KP5,x5,'4');
	xAll(Fn,KP6,xRight,'6');
	xAll(Fn,KP7,xBegin,'7');
	xAll(Fn,KP8,xUp,'8');
	xAll(Fn,KP9,xPgUp,'9');
	xAll(Fn,KP_PERIOD,xSuppr,'.');
	xAll(Fn,KP_DIVIDE,xDiv,'/');
	xAll(Fn,KP_MULTIPLY,xMul,'*');
	xAll(Fn,KP_MINUS,xMinus,'-');
	xAll(Fn,KP_PLUS,xPlus,'+');
	xAll(Fn,KP_ENTER,xEnter,'\n');
	xAll(Fn,KP_EQUALS,xEqual,'=');

	All(Fn,SCROLLOCK,ScrollLock);
	All(Fn,PRINT,Print);
	All(Fn,BREAK,Pause);

	All(Alt,LALT,AltLeft);
	All(Alt,RALT,AltRight);
	All(Alt,LCTRL,CtrlLeft);
	All(Alt,RCTRL,CtrlRight);
	All(Alt,LSHIFT,ShiftLeft);
	All(Alt,RSHIFT,ShiftRight);
	All(Alt,CAPSLOCK,CapsLock);
	All(Alt,NUMLOCK,NumLock);
#undef xAll
#undef Prt
#undef All
}

#include <stdio.h>

static void sdleEventLoop(GuiEngine *this,GuiOutput *Output,GuiInput *Input) {
	SDL_Surface *Picture;
	// #define SDLImgPerSecond 30
	int SDLImgPerSecond;
	int End;
	int tNextImg,tImg;
    #ifndef TIMER_USE_SDL
	int tNow,tLast,tPoll;
    #endif
	int KeyboardTranslate[SDLK_LAST-SDLK_FIRST];
	ThisToThat(SDLEngine,GuiEngine);
	SDLImgPerSecond = that->Expected.FrameRate;
	End = C_(Gui,Event,Processed);
	tImg = tNextImg = 0;
    #ifndef TIMER_USE_SDL
	tPoll = 0;
	tNow = tLast = GuiTimeGetTime()/1000;
    #endif
	KeyboardTranslateInit(KeyboardTranslate);
	while (End!=C_(Gui,Event,Finished)) {
		SDL_Event e;
		if (tImg>=tNextImg) {
		    SDL_UpdateRect(that->Screen,0,0,that->Screen->w,that->Screen->h);
			Call(Output,Display,1(&that->Render2dEngine));
			if (tImg>=(1000*SDLImgPerSecond)) tImg = tImg%(1000*SDLImgPerSecond);
			tNextImg = (((tImg/1000)+1)*1000);
		}
        #ifdef TIMER_USE_SDL
		    SDL_WaitEvent(&e);
        #else
		{
			#define WrapAround (GuiTimSWrapAround*1000)
			int event;
			do {
				tLast = (tNow>=WrapAround)?tNow-WrapAround:tNow;
			    tNow = GuiTimeGetTime()/1000;
				if (tNow<tLast) { tNow += WrapAround; }
				that->Time.Last += (tNow-tLast);
				if ((that->Time.Last-that->Time.Next)>=0) {
					int next,last,dt;
				    End = Call(Input,msTime,2(&next,that->Time.Last));
					if ((next-that->Time.Last)>0) {
						that->Time.Next = next;
					} else {
						that->Time.Next = that->Time.Last+SDLBaseTime;
					}
					last = tNow;
			        tNow = GuiTimeGetTime()/1000;
					dt = (tNow>=last) ? tNow-last:tNow+WrapAround-last;
				    if (tNow<tLast) { tNow += WrapAround; }
					that->Time.Last += dt;
				}
			    tImg += (tNow-tLast)*SDLImgPerSecond;
				tPoll += (tNow-tLast); if (tPoll>=SDLBaseTime) tPoll=tPoll%SDLBaseTime;
				event = SDL_PollEvent(&e);
			    if (!event) { e.type = SDL_USEREVENT; }
				// if (tImg>=tNextImg) { printf("\nframe %d",tNextImg/1000);}
				if ((tImg<tNextImg)&&(!event)){
					int overf,dt,dEv,dImg,dAppli;
					dAppli = (that->Time.Next-that->Time.Last);
					dImg = (tNextImg-tImg)/SDLImgPerSecond;
					dEv = tPoll;
					dt = (dAppli>dImg)?dImg:dAppli;
					dt = (dt>tPoll)?tPoll:dt;
					if (dt>0) { GuiTimeSleep(&overf,1000 * dt); }
				}
			} while ((!event)&&(tImg<tNextImg)&&(End!=C_(Gui,Event,Finished)));
		}
        #endif
		do {
			switch (e.type) {
			#ifdef TIMER_USE_SDL
			case SDL_USEREVENT: {
				that->Time.Last += SDLBaseTime;
				tImg += SDLBaseTime*SDLImgPerSecond;
				if ((that->Time.Last-that->Time.Next)>=0) {
					int next;
				    End = Call(Input,msTime,2(&next,that->Time.Last));
					if ((next-that->Time.Last)>0) {
						that->Time.Next = next;
					} else {
						that->Time.Next = that->Time.Last+SDLBaseTime;
					}
				}
			} break;
			#endif
			case SDL_KEYDOWN:
			    End = Call(Input,KeyDown,2(InputDevice_Keyboard|0,KeyboardTranslate[e.key.keysym.sym-SDLK_FIRST]));
			break;
			case SDL_KEYUP:
			    End = Call(Input,KeyUp,2(InputDevice_Keyboard|0,KeyboardTranslate[e.key.keysym.sym-SDLK_FIRST]));
			break;
			case SDL_JOYHATMOTION:
			    End = Call(Input,KeyDown,2(InputDevice_Joypad|(e.jhat.hat<<8)|e.jhat.which,e.jhat.value));
            break;
			case SDL_JOYAXISMOTION:
				End = Call(Input,Cursor,3(InputDevice_JoypadAxis|(e.jaxis.axis<<8)|e.jaxis.which,e.jaxis.value,0));
			break;
			case SDL_JOYBUTTONDOWN:
			    End = Call(Input,KeyDown,2(InputDevice_Joypad|e.jbutton.which,e.jbutton.button));
			break;
			case SDL_JOYBUTTONUP:
			    End = Call(Input,KeyUp,2(InputDevice_Joypad|e.jbutton.which,e.jbutton.button));
			break;
			case SDL_MOUSEBUTTONDOWN:
			    End = Call(Input,Cursor,3(InputDevice_Mouse|e.button.which,e.button.x,e.button.y));
			    if (End!=C_(Gui,Event,Finished)) 
					End = Call(Input,KeyDown,2(InputDevice_Mouse|e.button.which,e.button.button));
			break;
			case SDL_MOUSEBUTTONUP:
			    End = Call(Input,Cursor,3(InputDevice_Mouse|e.button.which,e.button.x,e.button.y));
			    if (End!=C_(Gui,Event,Finished)) 
					End = Call(Input,KeyUp,2(InputDevice_Mouse|e.button.which,e.button.button));
			break;
			case SDL_MOUSEMOTION:
			    End = Call(Input,Cursor,3(InputDevice_Mouse|0,e.motion.x,e.motion.y));
			break;
			case SDL_ACTIVEEVENT: {
				if (e.active.gain) {
					End = Call(Input,Suspend,1(C_(Gui,Event.Suspend,Play)));
				} else {
					End = Call(Input,Suspend,1(C_(Gui,Event.Suspend,Pause)));
				}
			} break;
			case SDL_VIDEORESIZE: {
			    SDL_SetVideoMode(e.resize.w,e.resize.h,0,SDL_HWSURFACE|SDL_RESIZABLE|SDL_NOFRAME);
			} break;
			case SDL_VIDEOEXPOSE: {
			} break;
			case SDL_QUIT: {
				Call(Input,Suspend,1(C_(Gui,Event.Suspend,Stop)));
                End = C_(Gui,Event,Finished);
			} break;
			default:
			break;
			}
		} while ((SDL_PollEvent(&e))&&(End!=C_(Gui,Event,Finished)));
	}
}

GuiEngine *SDLGuiEngineNew(char *WindowName,GuiLibrary *ImgLib,int Width,int Height,int *FrameRate) {
	static struct GuiEngine GuiStatic = {sdleOpen,sdleClose,sdleEventLoop};
    static struct Render2dEngine sdleStatic = {
		sdleImportImg2dLib,
	    sdleColorUpdateFront,sdleColorUpdateBack,sdleColorUpdateKey,sdleiColorUpdateKey,
	    sdleAlphaKeyEnabledUpdate,sdleUpdateClip,sdleSetStyle,
	    sdlePoint,sdleLine,sdleShape,sdleBox,sdleScreenClear
    };
	MemStack *Mem;
	SDLEngine *r;
	Mem = rFork(1024);
	mPush(Mem,r);
	r->Mem = Mem;
	r->Render2dEngine.Static = &sdleStatic;
	r->Render2dEngine.ScreenOrgAtBottom = (0!=0);
	r->Render2dEngine.MaxImagePitch = 0;
	r->Render2dEngine.AlgnImagePitch = 1;
	r->GuiEngine.Static = &GuiStatic;
    sdleShapeInit(r);
	{
		r->DriverName = 0;
		r->VideoInitFlags = 0;
		r->Screen = NULL;
		r->ScreenInfo = 0;
		r->Time.TimerRunning = (0!=0);
		r->Time.Last = r->Time.Next = 0;
		r->Img2dLib.Lib.n = ListEnd;
		r->Img2dLib.Free.n = ListEnd;
		if (*FrameRate<5||*FrameRate>60) { *FrameRate = 30; }
		r->Expected.FrameRate = *FrameRate;
		r->Expected.Dim.w = Width;
		r->Expected.Dim.h = Height;
        r->Expected.bpp = 32;
	}
	r->Img2dLib.Library = ImgLib;
	r->Img2dLib.EngineId = -1;//Call(ImgLib,RecordEngine,1(&r->Render2dEngine));
	return &r->GuiEngine;
}

