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

#include <Classes.h>
#include <StackEnv.h>
#include <Gui.h>

static int sqrDist(GuiRGBA *A,GuiRGBA *B) {
	int r,g,b;
	r = B->r-A->r;
	g = B->g-A->g;
	b = B->b-A->b;
	return (r*r)+(g*g)+(b*b);
}

static int sumDist(GuiRGBA *A,GuiRGBA *B) {
	int r,g,b;
	r = B->r>A->r?B->r-A->r:A->r-B->r;
	g = B->g>A->g?B->g-A->g:A->g-B->g;
	b = B->b>A->b?B->b-A->b:A->b-B->b;
	return r+g+b;
}

/*---------------------------------------*/

static void noPalGetRGBA(GuiRgbPalette *this,GuiRGBA *R,int Color) {}
static int noPalGetColor(GuiRgbPalette *this,GuiRGBA *Col) { return 0;}
static void noPalSetColor(GuiRgbPalette *this,int Color,GuiRGBA *Value) {}
GuiRgbPalette *GuiNoPalette(void) {
	static struct GuiRgbPalette Static = {
		noPalGetRGBA,noPalGetColor,noPalSetColor
	};
	static GuiRgbPalette Pal = {&Static};
	return &Pal;
}

/*---------------------------------------*/

static void palNullGetRGBA(GuiRgbPalette *this,GuiRGBA *R,int Color) { 
	R->a=Color>>24;
	R->r=Color>>16;
    R->g=Color>>8;
	R->b=Color; 
}
static int palNullGetColor(GuiRgbPalette *this,GuiRGBA *Color) {
	return (Color->a<<24)+(Color->r<<16)+(Color->g<<8)+(Color->b);
}
static void fixedSetColor(GuiRgbPalette *this,int Color,GuiRGBA *Value) {}

static struct GuiRgbPalette palNullStatic = {
	palNullGetRGBA,palNullGetColor,fixedSetColor
};
GuiRgbPalette GuiRgbPaletteNull = {&palNullStatic};

/*-------------------------------------*/

typedef struct {
	GuiRgbPalette GuiRgbPalette;
	GuiRGBA colors[2];
} monocPal;

static void monocGetRGBA(GuiRgbPalette *this,GuiRGBA *R,int Color) {
	ThisToThat(monocPal,GuiRgbPalette);
	*R = that->colors[Color&1];
}
static int monocGetColor(GuiRgbPalette *this,GuiRGBA *col) {
	int R,d0,d1;
	ThisToThat(monocPal,GuiRgbPalette);
	d0 = sqrDist(that->colors+0,col);
	d1 = sqrDist(that->colors+1,col);
	return (d0<d1) ? 0:1;
}
GuiRgbPalette *GuiMonochromePalette(GuiRGBA *Black,GuiRGBA *White) {
	monocPal *R;
	static struct GuiRgbPalette Static = {
		monocGetRGBA,monocGetColor,fixedSetColor
	};
	rPush(R);
	R->GuiRgbPalette.Static = &Static;
	R->colors[0] = *Black;
	R->colors[1] = *White;
	return &R->GuiRgbPalette;
}

/*-------------------------------------*/

static void pal565GetRGBA(GuiRgbPalette *this,GuiRGBA *R,int Color) {
	unsigned char r,g,b;
	R->a = GuiRGBAlpha_Opaque;
	r = (Color>>8) & 0xf8;
	g = (Color>>3) & 0xfc;
	b = (Color<<3);
	R->r = r+(r>>5);
	R->g = g+(g>>6);
	R->b = b+(b>>5);
}
static int pal565GetColor(GuiRgbPalette *this,GuiRGBA *Color) {
	return ((Color->r&0xf8)<<8)|((Color->g&0xfc)<<3)|((Color->b)>>3);
}

GuiRgbPalette *Gui565Palette(void) {
	static struct GuiRgbPalette Static = {
		pal565GetRGBA,pal565GetColor,fixedSetColor
	};
	static GuiRgbPalette Palette = {&Static};
	return &Palette;
}

/*-------------------------------------*/

static void pal24BitsRGBA(GuiRgbPalette *this,GuiRGBA *R,int Color) {
	R->a = GuiRGBAlpha_Opaque;
	R->r = Color>>16;
	R->g = Color>>8;
	R->b = Color;
}
static int pal24BitsGetColor(GuiRgbPalette *this,GuiRGBA *Color) {
	return (Color->r<<16)+(Color->g<<8)+(Color->b);
}
GuiRgbPalette *Gui24BitsPalette(void) {
	static struct GuiRgbPalette Static = {
		pal24BitsRGBA,pal24BitsGetColor,fixedSetColor
	};
	static GuiRgbPalette Palette = {&Static};
	return &Palette;
}

/*-------------------------------------*/

typedef struct {
	GuiRgbPalette GuiRgbPalette;
	GuiRgbPalette *Child;
} GuiRgbPaletteChild;

static void inversedPalGetRGBA(GuiRgbPalette *this,GuiRGBA *R,int Color) {
	GuiRGBA rc;
	ThisToThat(GuiRgbPaletteChild,GuiRgbPalette);
	Call(that->Child,GetRGBA,2(&rc,Color));
	R->a = rc.a;
	R->r = 255-rc.r;
	R->g = 255-rc.g;
	R->b = 255-rc.b;
}
static int inversedPalGetColor(GuiRgbPalette *this,GuiRGBA *Color) {
	ThisToThat(GuiRgbPaletteChild,GuiRgbPalette);
	Call(that->Child,GetColor,1(Color));
}
static void inversedPalSetColor(GuiRgbPalette *this,int Color,GuiRGBA *Value) {
	ThisToThat(GuiRgbPaletteChild,GuiRgbPalette);
	Call(that->Child,SetColor,2(Color,Value));
}
GuiRgbPalette *GuiInversedPalette(GuiRgbPalette *pal){
	GuiRgbPaletteChild *R;
	static struct GuiRgbPalette Static = {
		inversedPalGetRGBA,inversedPalGetColor,inversedPalSetColor
	};
	rPush(R);
	R->GuiRgbPalette.Static = &Static;
	R->Child = pal;
	return &R->GuiRgbPalette;
}

/*-------------------------------------*/

static void highPalGetRGBA(GuiRgbPalette *this,GuiRGBA *R,int Color) {
	GuiRGBA rc;
	ThisToThat(GuiRgbPaletteChild,GuiRgbPalette);
    Call(that->Child,GetRGBA,2(&rc,Color));
	R->a = rc.a;
	R->r = (256+rc.r)>>1;
	R->g = (256+rc.g)>>1;
	R->b = (256+rc.b)>>1;
}
static int highPalGetColor(GuiRgbPalette *this,GuiRGBA *R) {
	ThisToThat(GuiRgbPaletteChild,GuiRgbPalette);
	return Call(that->Child,GetColor,1(R));
}
static void highPalSetColor(GuiRgbPalette *this,int Color,GuiRGBA *Value) {
	ThisToThat(GuiRgbPaletteChild,GuiRgbPalette);
	Call(that->Child,SetColor,2(Color,Value));
}
GuiRgbPalette *GuiHighlightedPalette(GuiRgbPalette *pal) {
	GuiRgbPaletteChild *R;
	static struct GuiRgbPalette Static = {
		highPalGetRGBA,highPalGetColor,highPalSetColor
	};
	rPush(R);
    R->GuiRgbPalette.Static = &Static;
	R->Child=pal;
	return &R->GuiRgbPalette;
}

/*-------------------------------------*/

static void fxd8PalGetRGBA(GuiRgbPalette *this,GuiRGBA *R,int Color) {
	R->a = GuiRGBAlpha_Opaque;
	R->r = (Color&4)?255:0;
	R->g = (Color&2)?255:0;
	R->b = (Color&1)?255:0;
}
static int fxd8PalGetColor(GuiRgbPalette *this,GuiRGBA *col) {
	int R;
	R = 0;
	if (col->r>=128) R+=4;
	if (col->g>=128) R+=2;
	if (col->b>=128) R+=1;
	return R;
}
GuiRgbPalette *GuiFixed8Palette(void) {
	static struct GuiRgbPalette Static = {
		fxd8PalGetRGBA,fxd8PalGetColor,fixedSetColor
	};
	static GuiRgbPalette Pal = {&Static};
	return &Pal;
}

/*-------------------------------------*/

static void fxd16PalGetRGBA(GuiRgbPalette *this,GuiRGBA *R,int Color) {
	int intns0,intns1;
	intns0 = (Color&8)?64:0;
	intns1 = (Color&8)?192:255;
	R->a = GuiRGBAlpha_Opaque;
	R->r = (Color&4)?intns1:intns0;
	R->g = (Color&2)?intns1:intns0;
	R->b = (Color&1)?intns1:intns0;
}
static int fxd16PalGetColor(GuiRgbPalette *this,GuiRGBA *col) {
	int R,r,g,b;
	R = 0;
	if (col->r>=128) R+=4;
	if (col->g>=128) R+=2;
	if (col->b>=128) R+=1;
	r = col->r-128;
	g = col->g-128;
	b = col->b-128;
	if (((r*r)+(g*g)+(b*b))<=(3*64*64)) R+=8;
	return R;
}
GuiRgbPalette *GuiFixed16Palette(void) {
	static struct GuiRgbPalette Static = {
		fxd16PalGetRGBA,fxd16PalGetColor,fixedSetColor
	};
	static GuiRgbPalette Pal = {&Static};
	return &Pal;
}

/*-------------------------------------*/

static void fxd256PalGetRGBA(GuiRgbPalette *this,GuiRGBA *R,int Color) {
	R->a = GuiRGBAlpha_Opaque;
	if (Color<216) {
		R->b = 51*(Color%6); Color = Color/6;
		R->g = 51*(Color%6); Color = Color/6;
		R->r = 51*Color;
	} else {
		int subg,min,max,inc;
		Color = Color-216;
		if (Color<36) { /* Plan r+g+b = 384 */
			/*
			       O---O---O---O
				  /	      /   / \
			     O   O   O   O   O
                /       /   /     \
			   O   O   O---O   O   O
			  /       /     \       \
			 O---O---O   X   O   O   O
			  \       \     / \     /
			   O---O---O---O   O   O
			    \           \   \ /
			     O   O   O   O   O
				  \	          \ /
			       O---O---O---O
			 
			 */
			int x,y,n,c[3];
			x = Color&3; Color = Color>>2;
			y = Color%3; n = Color/3;
			c[0] = 255-(x*42); // >= 129
			c[1] = y*43; // <= 86
			c[2] = 384-(c[0]+c[1]); // 255>=x>=43
			R->r = c[n]; n=(n==2)?0:n+1;
			R->g = c[n]; n=(n==2)?0:n+1;
			R->b = c[n];
		} else { /* Transparent */
			static GuiRGBA cl[4] = {
			    {128,128,128,GuiRGBAlpha_Opaque},
			    {255,255,255,128},
			    {255,255,255,GuiRGBAlpha_Transparent},
			    {0,0,0,GuiRGBAlpha_Transparent}	
			};
			*R = cl[Color&3];
		}
	}
}
static int fxd256PalGetColor(GuiRgbPalette *this,GuiRGBA *col) {
	int R;
	GuiRGBA r;
	if (col->a!=GuiRGBAlpha_Opaque) {
		r.a = (col->a>128)? col->a-128:128-col->a;
		if ((r.a)<32) {
            R = 253;
		} else {
			if ((col->r+col->g+col->b)>=384) {
				R = 254;
			} else {
				R = 255;
			}

		}
	} else {
		int d0,d1;
		r.a = GuiRGBAlpha_Opaque;
	    r.r = col->r/43; r.g = col->g/43; r.b = col->b/43;
	    R = (r.r*36)+(r.g*6)+r.b;
	    r.r = 51*r.r; r.g=51*r.g; r.b=51*r.b;
		d0 = sqrDist(col,&r); // max: 3*26*26
        r = *col;
		d1 = 384-(r.r+r.g+r.b);
		if ((d1>=-78)&&(d1<=78)) {
			d1 = d1/3;
			r.r = (r.r+d1)/37;
			r.b = (r.b+d1)/37;
			r.g = 9-(r.r+r.b);
			if (r.r>=0 && r.r<=6 && r.b>=0 && r.b<=6 && r.g>=0 && r.g<=6) {
				int R1,n,select,x,y;
				int c[3];
				static int indexSelect[8] = {-1,+2,+1,+2,+0,+0,+1,-1};
				c[0] = r.r; c[1] = r.g; c[2] = r.b;
				select = (c[0]<3)?0:4;
				select += (c[1]<3)?0:2;
				select += (c[2]<3)?0:1;
				select = indexSelect[select];
				if (select==-1) {
					R1 = 252;
				} else {
					n = select;
                    x = 6-c[n]; n = (n==2)?0:n+1;
					y = c[n];
					n = select ? (3-select):0;
					R1 = 216 + (n * 12)+(y*4)+x;
				}
				r.r = r.r*42;
				r.g = r.g*42;
				r.b = r.b*42;
				d1 = sqrDist(col,&r);
				if (d1<=d0) R = R1;
			}
		}
    }
	return R;
}
GuiRgbPalette *GuiFixed256Palette(void) {
	static struct GuiRgbPalette Static = {
		fxd256PalGetRGBA,fxd256PalGetColor,fixedSetColor
	};
	static GuiRgbPalette Pal = {&Static};
	return &Pal;
}

/*--------------------------------------*/

typedef struct {
	GuiRgbPalette GuiRgbPalette;
	struct {
	    int Max,Current,bpi;
		unsigned int *Map,*eMap;
	} Map;
	GuiRGBA *Colors;
} palncolors;

static void palncolorsGetRGBA(GuiRgbPalette *this,GuiRGBA *R,int col) {
	ThisToThat(palncolors,GuiRgbPalette);
	if (col>=that->Map.Max) col = that->Map.Max-1;
	*R = that->Colors[col];
}
static int palncolorsGetColor(GuiRgbPalette *this,GuiRGBA *col) {
	int idx,R,d,dmin,dmax;
	GuiRGBA *p,*e;
	unsigned int *ip,cp,mcp;
	/* ToDo: this is obviously grossly innefficient! */
	ThisToThat(palncolors,GuiRgbPalette);
	R = 0;
	p = that->Colors; e = p+that->Map.Max;
	d = dmin = dmax = sqrDist(col,p);
	p++; ip = that->Map.Map; cp = *ip; mcp = 2;
    while (p<e) {
		if (cp&mcp) {
            d = sqrDist(col,p);
		    if (d<dmin) { R = p-that->Colors; d=dmin;}
		}
		p++; mcp = mcp<<1; if (!mcp) {mcp=1; ip++; cp=*ip;}
	}
	return R;
}
static void palncolorsSetColor(GuiRgbPalette *this,int idx,GuiRGBA *Color) {
	ThisToThat(palncolors,GuiRgbPalette);
	if (idx>=that->Map.Max) idx = that->Map.Max-1;
	that->Colors[idx] = *Color;
	that->Map.Map[idx>>that->Map.bpi]|=1<<(idx%that->Map.bpi);
}

static int bppicalc(void) {
    unsigned int a0,a1;
	int r;
	r = -8; a0=a1=-1;
	do { a1 = a0; a0 = a0<<8; r++;} while (a0!=a1);
	return r;
}
GuiRgbPalette *GuiVariablePalette(int bits) {
	palncolors *R;
	static struct GuiRgbPalette Static = {
		palncolorsGetRGBA,palncolorsGetColor,palncolorsSetColor
	};
	rPush(R);
	R->GuiRgbPalette.Static = &Static;
	if (bits<0) bits = 0;
    R->Map.Max = 1<<bits;
    if (R->Map.Max <=0) R->Map.Max = 1;
	R->Map.bpi = bppicalc();
	R->Map.Current = 0;
	rnPush(R->Map.Map,((R->Map.Max+R->Map.bpi)-1)/R->Map.bpi);
	R->Map.eMap = R->Map.Map + (((R->Map.Max+R->Map.bpi)-1)/R->Map.bpi);
	{int *p,*e; p=R->Map.Map; e = R->Map.eMap; while (p<e) *p++=0; }
	rnPush(R->Colors,R->Map.Max);
	{ int i;
		GuiRGBA black;
		black.a = GuiRGBAlpha_Opaque;
		black.r = black.g = black.b = 0;
		for (i=0;i<R->Map.Max;i++) R->Colors[i] = black;
	}
	return &R->GuiRgbPalette;
}

/*--------------------------------------*/

GuiRgbPalette *Gui32BitsPalette(void) { return &GuiRgbPaletteNull; }





