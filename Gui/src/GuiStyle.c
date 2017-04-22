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
#include <List.h>

/*-----------------------------------------------*/

typedef struct {
	unsigned char *p,*e;
	int pVal;
} BitPtr;
static int ReadBitPtrVal(BitPtr *p,int w) {
	int r,s,b,e;
	r = 0; s = 0;
	b = p->pVal;
    while ((w>0)&&(p->p<p->e)) {
		unsigned char c;
		int k;
		c = (*p->p)>>p->pVal;
		e = b+w;
		if (e>8) e = 8;
		k = e-b;
		c = (c>>b) & (0xff>>(8-k));
		r = r+(c<<s); s+=k; w-=k;
		if (e>=8) {p->p++; e-=8;} b = (e&7);
	}
	p->pVal = b;
	return r;
}
static void WriteBitPtrVal(BitPtr *p,int w,int Val) {
	int r,b,e;
	r = 0;
	b = p->pVal;
	while ((w>0)&&(p->p<p->e)) {
		unsigned char c;
		int k;
		e = 8; k = e-b;
        if (k>w) k = w;
		c = (Val & (0xff>>(8-k)))<<b;
		Val = Val>>k;
		*p->p = *p->p|c;
		w-=k;
		if (b+k>=8) {p->p++;} b=(b+k)&7;
	}
	p->pVal = b;
}

/*-----------------------------------------------*/

typedef struct {
	GuiFieldStyleInCtx GuiFieldStyleInCtx;
	GuiFieldStyleAttrDesc AttrDesc;
	struct {
	    GuiRgbPalette *Fg,*Bg;
	} *Palettes,cPal;
	int PalNb;
	int Attr,FontNum,PalNum,Fg,Bg;
	GuiRGBA rgbFg,rgbBg;
} GFSInCtx;

static void GFSInCtxSetFontNum(GuiFieldStyleInCtx *this,int *old,int new) { 
	ThisToThat(GFSInCtx,GuiFieldStyleInCtx);
	*old = that->FontNum;
	that->FontNum = new;
}
#define blkcpy(Q,P,w) { unsigned char *e,*q,*p; q=(unsigned char *)Q; p=(unsigned char *)P; e=p+w; while(p<e) *q++=*p++;}
#define blkset0(Q,w) {unsigned char *p,*e; p=(unsigned char *)Q; e=p+w; while(p<e) *p++=0;}
static void GFSInCtxSetTextAttr(GuiFieldStyleInCtx *this,int *old,int new) {
	ThisToThat(GFSInCtx,GuiFieldStyleInCtx);
	*old = that->Attr;
	that->Attr = new;
}
static void GFSInCtxSetPaletteNum(GuiFieldStyleInCtx *this,int *old,int new) {
	ThisToThat(GFSInCtx,GuiFieldStyleInCtx);
	if (new>=that->PalNb) new = that->PalNb-1;
	*old = that->PalNum;
	that->PalNum = new;
	that->cPal = that->Palettes[new];
}
static void GFSInCtxSetColorFg(GuiFieldStyleInCtx *this,GuiRGBA *old,GuiRGBA *new) {
	ThisToThat(GFSInCtx,GuiFieldStyleInCtx);
	*old = that->rgbFg;
	that->rgbFg = *new;
	that->Fg = Call(that->cPal.Fg,GetColor,1(new));
}
static void GFSInCtxSetColorBg(GuiFieldStyleInCtx *this,GuiRGBA *old,GuiRGBA *new) {
	ThisToThat(GFSInCtx,GuiFieldStyleInCtx);
	*old = that->rgbBg;
	that->rgbBg = *new;
	that->Bg = Call(that->cPal.Bg,GetColor,1(new));
}
static void GFSInCtxFillAttr(GuiFieldStyleInCtx *this,unsigned char *Attr) {
	BitPtr p;
	ThisToThat(GFSInCtx,GuiFieldStyleInCtx);
	p.p =  Attr; p.e = Attr+that->AttrDesc.AttrByte; p.pVal = 0;
    WriteBitPtrVal(&p,that->AttrDesc.AttrBit,that->Attr);
	WriteBitPtrVal(&p,that->AttrDesc.FntBit,that->FontNum);
	WriteBitPtrVal(&p,that->AttrDesc.PalBit,that->PalNum);
	WriteBitPtrVal(&p,that->AttrDesc.FgBit,that->Fg);
	WriteBitPtrVal(&p,that->AttrDesc.BgBit,that->Bg);
}
static void GFSInCtxClose(GuiFieldStyleInCtx *this) { }

static GuiFieldStyleInCtx *GFSInGetCtx(GuiFieldStyle *Style,GuiFieldStyleAttrDesc *AttrDesc) {
	GFSInCtx *R;
	static struct GuiFieldStyleInCtx Static = {
		GFSInCtxSetFontNum,GFSInCtxSetTextAttr,GFSInCtxSetPaletteNum,
		GFSInCtxSetColorFg,GFSInCtxSetColorBg,GFSInCtxFillAttr,GFSInCtxClose
	};
	rPush(R);
    R->GuiFieldStyleInCtx.Static = &Static;
	R->AttrDesc = *AttrDesc;
	R->Attr = 0; R->FontNum = 0; R->PalNum=0; R->Bg = 0; R->Fg = 1;
	R->PalNb = 1<<(R->AttrDesc.PalBit);
	rnPush(R->Palettes,R->PalNb);
	{   int i,ie;
		i = 0;
        if (AttrDesc->Palettes==0) { ie = 0; } else {
			ie = AttrDesc->ePal - AttrDesc->Palettes; 
		}
	    while (i<ie) {
            R->Palettes[i].Bg = AttrDesc->Palettes[i].bg;
		    R->Palettes[i].Fg = AttrDesc->Palettes[i].fg;
			i++;
		}
		while (i<R->PalNb) {
			R->Palettes[i].Bg = R->Palettes[i].Fg = GuiNoPalette();
			i++;
		}
        R->cPal = R->Palettes[0];
	}
	Call(R->cPal.Bg,GetRGBA,2(&R->rgbBg,R->Bg));
	Call(R->cPal.Fg,GetRGBA,2(&R->rgbFg,R->Fg));
    return &R->GuiFieldStyleInCtx;
}

/*-----------------------------------------------*/

typedef struct {
	GuiFieldStyleOutCtx GuiFieldStyleOutCtx;
	GuiFieldStyle *Style;
	GuiFieldStyleAttrDesc AttrDesc;
	struct {
		GuiRgbPalette *Fg,*Bg;
	} *Palettes,cPal;
	int PalNb;
	int Attr,FontNum,PalNum,Fg,Bg;
	GuiFontPageDesc *Page;
} GFSOutCtx;

static void GFSOutCtxGetAttr(GuiFieldStyleOutCtx *this,GuiRGBA *newfront,GuiRGBA *newback,unsigned char *attr) {
	BitPtr p;
	int Attr,FntNum,PalNum;
	ThisToThat(GFSOutCtx,GuiFieldStyleOutCtx);
	p.p = attr; p.e = attr+that->AttrDesc.AttrByte; p.pVal =0;
	Attr = ReadBitPtrVal(&p,that->AttrDesc.AttrBit);
	FntNum = ReadBitPtrVal(&p,that->AttrDesc.FntBit);
	if (that->AttrDesc.PalBit) {
	    PalNum = ReadBitPtrVal(&p,that->AttrDesc.PalBit);
	    if (PalNum!=that->PalNum) {
		    that->cPal = that->Palettes[PalNum];
	    }
	} else {
		PalNum = 0;
	}
	if (that->AttrDesc.FgBit) {
		int Fg;
	    Fg = ReadBitPtrVal(&p,that->AttrDesc.FgBit);
	    if ((Fg!=that->Fg)||(PalNum!=that->PalNum)) {
		    Call(that->cPal.Fg,GetRGBA,2(newfront,Fg));
		    that->Fg = Fg;
	    }
	}
	if (that->AttrDesc.BgBit) {
		int Bg;
	    Bg = ReadBitPtrVal(&p,that->AttrDesc.BgBit);
	    if ((Bg!=that->Bg)||(PalNum!=that->PalNum)) {
		    Call(that->cPal.Bg,GetRGBA,2(newback,Bg));
		    that->Bg = Bg;
	    }
	}
	that->PalNum = PalNum;
	if ((Attr!=that->Attr)||(FntNum!=that->FontNum)) {
		that->Page = Call(that->Style,GetFont,3(FntNum,that->AttrDesc.FontSize,Attr));
		that->Attr = Attr;
		that->FontNum = FntNum;
	}
}
static GuiOutput *GFSOutCtxGetTile(GuiFieldStyleOutCtx *this,int TileId) {
	ThisToThat(GFSOutCtx,GuiFieldStyleOutCtx);
	return Call(that->Page,GetGlyphe,1(TileId));
}
static void GFSOutCtxClose(GuiFieldStyleOutCtx *this) { }

static GuiFieldStyleOutCtx *GFSOutGetCtx(GuiFieldStyle *Style,GuiFieldStyleAttrDesc *Desc) {
	GFSOutCtx *R;
	static struct GuiFieldStyleOutCtx Static = {
		GFSOutCtxGetAttr,GFSOutCtxGetTile,GFSOutCtxClose
	};
	rPush(R);
	R->GuiFieldStyleOutCtx.Static = &Static;
	R->Style = Style;
	R->AttrDesc = *Desc;
	R->Attr = 0; R->FontNum = 0; R->PalNum=0; R->Fg = 1; R->Bg = 0;
	R->PalNb = 1<<(Desc->PalBit);
	rnPush(R->Palettes,R->PalNb);
	{   int i,ie;
		i =0;
		if (Desc->Palettes==0) {ie = 0;} else {
			ie = Desc->ePal-Desc->Palettes;
		}
		while (i<ie) {
	        R->Palettes[i].Bg = Desc->Palettes[i].bg;
	        R->Palettes[i].Fg = Desc->Palettes[i].fg;
			i++;
		}
		while (i<R->PalNb) {
	        R->Palettes[i].Bg = R->Palettes[i].Fg = GuiNoPalette();
			i++;
		}
		R->cPal = R->Palettes[0];
	}
	R->Page = Call(Style,GetFont,3(R->FontNum,R->AttrDesc.FontSize,0));
	return &R->GuiFieldStyleOutCtx;
}

/*-----------------------------------------------*/

typedef struct {
	GuiFontPageDesc *Desc;
	int Attr; 
} SizedFontDesc;
typedef struct {
	BinTree Size;
	SizedFontDesc **Attr;
} SizedFont;
typedef struct {
	BinTree NumHi;
	int AttrNb;
	BinTree Size[16];
} AllFonts;

static void AllFontsInit(AllFonts *n,int AttrNb) {
	int i;
	BinTreeInit(&n->NumHi);
	n->AttrNb = AttrNb;
	for (i=0;i<16;i++) BinTreeInit(n->Size+i);
}
static SizedFontDesc *SizedFontDescNew(int attr,GuiFontPageDesc *page) {
	SizedFontDesc *desc;
	rPush(desc);
	desc->Desc = page;
	desc->Attr = attr;
    return desc;
}
static void SizedFontInit(SizedFont *n,int AttrNb,int attr,GuiFontPageDesc *page) {
	SizedFontDesc *desc;
	int i;
	if (!AttrNb) AttrNb = 1;
	if (attr<0||attr>=AttrNb) attr = attr%AttrNb;
	rnPush(n->Attr,AttrNb);
	BinTreeInit(&n->Size);
	desc = SizedFontDescNew(attr,page);
	for(i=0;i<AttrNb;i++) { n->Attr[i] = desc; }
}
static void AddFont(AllFonts *af,int num,int size,int attr,GuiFontPageDesc *page) {
	int numhi;
	SizedFont *sized;
	SizedFontDesc *fd;
	numhi = num>>4;
	num = num & 0xf;
	if (numhi) {
		BinTree *f;
		f = BinTreeSeek(&af->NumHi,numhi);
		if (f==BinTreeEnd) {
			AllFonts *n;
			rPush(n);
			AllFontsInit(n,af->AttrNb);
			BinTreeSeekOrInsert(&af->NumHi,numhi,&n->NumHi);
			af = n;
		} else {
			af = CastBack(AllFonts,NumHi,f);
		}
	}
	/* Insert by size */ { 
	    BinTree *f;
		f = BinTreeSeek(&af->Size[num],size);
		if (f==BinTreeEnd) {
			rPush(sized);
			SizedFontInit(sized,af->AttrNb,attr,page);
			BinTreeSeekOrInsert(&af->Size[num],size,&sized->Size);
		} else {
			sized = CastBack(SizedFont,Size,f);
		}
	}
	/* Insert by attribute */{
		int attrnb;
		attrnb = (af->AttrNb>0)?af->AttrNb:1;
		if (attr<0 || attr>=attrnb) attr = attr%attrnb;
        if (attr!=sized->Attr[attr]->Attr) {
			sized->Attr[attr]= SizedFontDescNew(attr,page);
		}
	}
}

struct SeekNearest {
	int min,max,target;
	BinTree *fmin,*fmax;
};
static int SeekNearest(BinTree *tr,void *closure) {
	struct SeekNearest *clos;
	clos = closure;
	if (clos->fmin==BinTreeEnd) {
		clos->min = clos->max = tr->Id;
		clos->fmin = clos->fmax = tr;
	} else {
        if (clos->target>=tr->Id) {
			if ((tr->Id>clos->min)||(clos->min>clos->target)) {
				clos->min=tr->Id;
			    clos->fmin = tr;
			}
		} else {
			if ((tr->Id<clos->max)||(clos->max<clos->target)) {
				clos->max=tr->Id;
				clos->fmax = tr;
			}
		}
	}
	return (0!=0);
}
static SizedFontDesc *GetFontDesc(int *Success,AllFonts *af,int num,int Size,int Attr) {
	SizedFontDesc *R;
	int n16,success;
	BinTree *Found;
	R = 0;
	n16 = num & 0xf;
	num = num >> 4;
	Found = &af->NumHi;
	success = (num==0);
	if (!success) {
		Found = BinTreeSeek(&af->NumHi,num);
		success = (Found!=BinTreeEnd);
		if (success) { af = CastBack(AllFonts,NumHi,Found);}
	}
	if (success) {
		Found = BinTreeSeek(&af->Size[n16],Size);
		success = (Found!=BinTreeEnd);
		if (!success) { // Try nearest
			struct SeekNearest clos;
			clos.min = clos.max = -1;
			clos.fmin = clos.fmax = BinTreeEnd;
			clos.target = Size;
			BinTreeForEach(&af->Size[n16],&SeekNearest,&clos);
			if (clos.fmin!=BinTreeEnd) {
			    if (clos.max<clos.target) {clos.max = clos.min; clos.fmax = clos.fmin;}
				if (clos.min>clos.target) {clos.min = clos.max; clos.fmin = clos.fmax;}
				Found = ((clos.target-clos.min)<(clos.max-clos.target))?clos.fmin:clos.fmax;
			}
		}
		if (success) {
			SizedFont *sf;
			sf = CastBack(SizedFont,Size,Found);
			if (af->AttrNb>0) {
				if ((af->AttrNb<Attr)||(Attr<0)) { Attr = Attr%af->AttrNb;}
			} else {
				Attr = 0;
			}
			R = sf->Attr[Attr];
			success = (R!=0);
		}
	}
	*Success = success;
	return R;
}
static GuiFontPageDesc *GetFontPageDesc(int *Success,AllFonts *af,int num,int Size,int Attr) {
	GuiFontPageDesc *R;
	SizedFontDesc *p;
	R = &GuiFontPageDescNull;
    p = GetFontDesc(Success,af,num,Size,Attr);
	if (p) R = p->Desc;
	return R;
}
/*-----------------------------------------------*/

typedef struct {
	GuiFieldStyle GuiFieldStyle;
	MemStack *Mem;
	AllFonts Fonts;
} GFS;
static GuiFieldStyleInCtx *GFSOpenInCtx(GuiFieldStyle *this,GuiFieldStyleAttrDesc *Desc) {
	return GFSInGetCtx(this,Desc);
}
static GuiFieldStyleOutCtx *GFSOpenOutCtx(GuiFieldStyle *this,GuiFieldStyleAttrDesc *Desc) {
	return GFSOutGetCtx(this,Desc);
}
static void GFSAddFont(GuiFieldStyle *this,int num,int Size,int Attr,GuiFontPageDesc *Page) {
	ThisToThat(GFS,GuiFieldStyle);
    mIn(that->Mem,AddFont(&that->Fonts,num,Size,Attr,Page));
}
static GuiFontPageDesc *GFSGetFont(GuiFieldStyle *this,int num,int Size,int Attr) {
	int Success;
	GuiFontPageDesc *R;
	ThisToThat(GFS,GuiFieldStyle);
    mIn(that->Mem,R = GetFontPageDesc(&Success,&that->Fonts,num,Size,Attr));
	return R;
}

GuiFieldStyle *GuiFieldStyleNew(int AttrNb) {
	static struct GuiFieldStyle Static = {
		GFSOpenInCtx,GFSOpenOutCtx,GFSAddFont,GFSGetFont
	};
	MemStack *Mem;
	GFS *R;
	Mem = rFork(1024);
	mPush(Mem,R);
	R->Mem = Mem;
	R->GuiFieldStyle.Static = &Static;
	if (AttrNb<=0) AttrNb = 1;
    AllFontsInit(&R->Fonts,AttrNb);
	return &R->GuiFieldStyle;
}


