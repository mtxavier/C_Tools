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
#include <GuiText.h>

/*--------------------------------*/


static void fontDescNullGetRange(GuiFontPageDesc *this,int *Begin,int *end) { *Begin=*end=0;}
static int fontDescNullGetGlypheDesc(GuiFontPageDesc *this,GuiFontGlypheDesc *Desc,int Glyphe) {return (0!=0);}
static GuiOutput *fontDescNullGetGlyphe(GuiFontPageDesc *this,int Glyphe) {return &GuiOutputNull;}
static struct GuiFontPageDesc fontDescNullStatic = {
	fontDescNullGetRange,fontDescNullGetGlypheDesc,fontDescNullGetGlyphe
};
GuiFontPageDesc GuiFontPageDescNull = {&fontDescNullStatic};

/*--------------------------------*/

typedef struct {
	GuiFontPageDesc GuiFontPageDesc;
	GuiFontGlypheDesc Metric;
	int Min,Max;
	GuiOutput **Known;
	GuiLibrary *Libr;
	int PageNum;
} regFontDesc;

static void regFontDescGetRange(GuiFontPageDesc *this,int *Begin,int *End) {
	ThisToThat(regFontDesc,GuiFontPageDesc);
    *Begin = that->Min;
	*End = that->Max;
}
static int regFontDescNullGetGlypheDesc(GuiFontPageDesc *this,GuiFontGlypheDesc *Desc,int Glyphe) {
	ThisToThat(regFontDesc,GuiFontPageDesc);
	*Desc = that->Metric;
	return (Glyphe>=that->Min && Glyphe<that->Max);
}
static GuiOutput *regFontDescGetGlyphe(GuiFontPageDesc *this,int Id) {
	ThisToThat(regFontDesc,GuiFontPageDesc);
	if ((Id<0)||(Id>255)) Id = 0;
	return that->Known[Id];
}


GuiFontPageDesc *GuiFontPage16x16Png(
	GuiLibrary *ImgLibr,
	char *PngFile, Geo2dPoint *LibOrg, Geo2dExtent *NextGlyph,
	GuiFontGlypheDesc *Metric
) {
	regFontDesc *R;
	GuiOutput **p,**e;
	Geo2dRectangle Rect;
	int ex;
	int pId;
	static struct GuiFontPageDesc Static = {
		regFontDescGetRange,regFontDescNullGetGlypheDesc,regFontDescGetGlyphe
	};
	rPush(R);
	R->GuiFontPageDesc.Static = &Static;
    R->Metric = *Metric;
	R->Min = 0; R->Max = 256;
	rnPush(R->Known,256);
	R->Libr = ImgLibr;
	R->PageNum = pId = Call(ImgLibr,GetPageId,0);
	Call(ImgLibr,PageLoad,2(pId,PngFile));
	p = R->Known; e = p+256;
	Rect.Pos = *LibOrg;
	Rect.Extent = Metric->Extent;
	ex = Rect.Pos.x + (NextGlyph->w<<4);
	while (p<e) {
		while (Rect.Pos.x<ex) {
			GuiOutput *Img;
			Img = Call(ImgLibr,Picture,2(pId,&Rect));
			if (Metric->Grab.x!=0 || Metric->Grab.y!=0) {
				*p++ = GuiOut2dAttach(Metric->Grab.x,Metric->Grab.y,Img);
			} else {
			    *p++ = Img;
			}
			Rect.Pos.x += NextGlyph->w;
		}
		Rect.Pos.x = LibOrg->x;
		Rect.Pos.y += NextGlyph->h;
	}
	return &R->GuiFontPageDesc;
}


/*--------------------------------*/


