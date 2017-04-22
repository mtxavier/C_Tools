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
#include <GuiText.h>

/*-------------------------------------*/

#include <List.h>

typedef struct {
	Geo2dExtent Dim;
	int bppShift; // 0: 1bpp; 1 : 2bpp; 2: 4bpp; 3:8bpp:
	int pitch;
	char *data;
	Geo2dRectangle Sub;
} imgSub;

typedef struct {
	dList dList;
	ImgDesc Img;
	int PageId,Updated;
} pixmapPage;

typedef struct {
	Geo2dExtent PageDim;
	Geo2dExtent SlotDim;
    Geo2dPoint Curs;
	Geo2dExtent Next;
	dList *Pages;
	int bppShift,ColorNb; // 0: 1bpp; 1 : 2bpp; 2: 4bpp; 3:8bpp:
} pixmapPages;

#define DataAlign(s)  (((s)+((sizeof(void *))-1))&(-(sizeof(void *))))

static pixmapPage *pm256PageNextPage(pixmapPages *pp) {
	pixmapPage *r;
	rPush(r);
	r->Img.Dim = pp->PageDim;
	{
		int bpc,w;
	    int datasize;
		bpc = 8>>pp->bppShift;
		w = pp->PageDim.w;
	    r->Img.pitch = (w+(bpc-1))/bpc;
		datasize = DataAlign(pp->PageDim.h*r->Img.pitch);
	    rnPush(r->Img.Data,datasize);
	}
	r->Img.Format.Format = C_(Img,Format.Type,Grey);
	r->Img.Format.Desc.Palette.nb = pp->ColorNb;
	r->Img.Format.Desc.Palette.key = -1;
	r->Img.Format.Desc.Palette.Colors = 0;
	r->Updated = (0!=0);
	r->PageId = -1;
	pp->Curs.x = pp->Curs.y = 0;
	pp->Next = pp->SlotDim;
	if (pp->Pages) {
		dList *l,*o;
		o = pp->Pages;
		l = &r->dList;
		l->n = o; l->p = o->p;
		l->p->n = l->n->p = l;
	} else {
		r->dList.p = r->dList.n = &r->dList;
	}
	pp->Pages = &r->dList;
	return r;
}

static void pm256PageUpdate(GuiLibrary *Libr,pixmapPage *page) {
	if (page->Updated) {
		Call(Libr,PageImport,2(page->PageId,&page->Img));
		page->Updated = (0!=0);
	}
}

static void pm256PageGetSlot(GuiLibrary *Libr,pixmapPages *pp,int *RpageId,imgSub *R) {
	pixmapPage *page;
	int w,h;
	if (!pp->Pages) {
		page = pm256PageNextPage(pp);
		page->PageId = Call(Libr,GetPageId,0);
	} else {
		page = CastBack(pixmapPage,dList,pp->Pages);
	}
	w = pp->SlotDim.w; h = pp->SlotDim.h;
	if (pp->Curs.x+w > page->Img.Dim.w) {
		pp->Curs.x = 0;
		pp->Curs.y = pp->Curs.y+pp->Next.h;
		pp->Next.h = pp->SlotDim.h;
	} 
	if (pp->Curs.y+h > page->Img.Dim.h) {
		pm256PageUpdate(Libr,page);
		page = pm256PageNextPage(pp);
		page->PageId = Call(Libr,GetPageId,0);
	}
	if (h>pp->Next.h) pp->Next.h = h;
	R->bppShift = pp->bppShift;
	R->pitch = page->Img.pitch;
	R->Dim = page->Img.Dim;
	R->data = page->Img.Data;
	R->Sub.Pos = pp->Curs;
	R->Sub.Extent = pp->SlotDim;
	*RpageId = page->PageId;
	pp->Curs.x += w;
	page->Updated = (0==0);
}

/*-------------------------------------*/

typedef struct {
	BinTree Base;
	struct ttGlyphe {
		Geo2dPoint Grab;
		Geo2dExtent Extent;
		int dg;
        GuiOutput *Output;
	} Offset[256];
} ttGlypheCollect;

static ttGlypheCollect *ttGlypheCollectSeekOrInsert(BinTree *root,int Base) {
	ttGlypheCollect *r;
	BinTree *bt;
	bt = BinTreeSeek(root,Base);
	if (bt==BinTreeEnd) {
		struct ttGlyphe *p,*e;
		rPush(r);
		p = r->Offset+0; e = p+256;
		while (p<e) {
			p->Grab.x = p->Grab.y = 0;
			p->Extent.h = p->Extent.w = 0;
			p->dg = 0;
			p->Output = &GuiOutputNull;
			p++;
		}
		BinTreeSeekOrInsert(root,Base,&r->Base);
	} else {
		r = CastBack(ttGlypheCollect,Base,bt);
	}
	return r;
}


/*-------------------------------------*/

typedef struct {
	MemStack *Mem;
	GuiFontPageDesc GuiFontPageDesc;
	pixmapPages Pages;
	BinTree Glyphes;
	GuiLibrary *Libr;
	char *FileName;
	int FaceNum;
	Geo2dExtent Size;
	int Vertical,dg;
} TTFont;

static void TTFontPagesUpdate(TTFont *font) {
	dList *p;
	p = font->Pages.Pages;
	if (p) {
		dList *e;
		pixmapPage *page;
		e = p;
		do {
		    page = CastBack(pixmapPage,dList,p);
			pm256PageUpdate(font->Libr,page);
			p = p->n;
		} while (p!=e);
	}
}

static void TTFontGetRange(GuiFontPageDesc *this,int *Begin,int *end) {
	ThisToThat(TTFont,GuiFontPageDesc);
	*Begin = 0;
	*end = 256;
}
static int TTFontGetGlypheDesc(GuiFontPageDesc *this,GuiFontGlypheDesc *Desc,int Glyphe) {
	int gp,go;
	BinTree *bt;
	int r;
	ThisToThat(TTFont,GuiFontPageDesc);
    gp = Glyphe>>8;
	go = Glyphe&255;
	bt = BinTreeSeek(&that->Glyphes,gp);
	r = (bt!=BinTreeEnd);
	if (r) {
		ttGlypheCollect *gc;
		struct ttGlyphe *g;
		gc = CastBack(ttGlypheCollect,Base,bt);
        g = gc->Offset+go;
		r = (g->Output!=&GuiOutputNull);
        if (r) {
			Desc->Grab = g->Grab;
			Desc->Extent = g->Extent;
			if (that->Vertical) {
				Desc->dx.x = 0;  Desc->dx.y = g->dg;
				Desc->dy.x = that->dg; Desc->dy.y = 0;
			} else {
			    Desc->dx.x = g->dg; Desc->dx.y = 0;
			    Desc->dy.x = 0; Desc->dy.x = that->dg;
			}
		}
	} 
	if (!r) {
		Desc->Grab.x = Desc->Grab.y = 0;
		Desc->Extent.w = Desc->Extent.h = 0;
		Desc->dx.x = Desc->dx.y = 0;
		Desc->dy.x = Desc->dy.y = 0;
	}
	return r;
}
static GuiOutput *TTFontGetGlyphe(GuiFontPageDesc *this,int Glyphe) {
	GuiOutput *r;
	int gp,go;
	BinTree *bt;
	ThisToThat(TTFont,GuiFontPageDesc);
	r = &GuiOutputNull;
	gp = Glyphe>>8;
	go = Glyphe&0xff;
	bt = BinTreeSeek(&that->Glyphes,gp);
	if (bt!=BinTreeEnd) {
		ttGlypheCollect *gc;
		gc = CastBack(ttGlypheCollect,Base,bt);
        r = gc->Offset[go].Output;
	}
	return r;
}

static void loadFont(TTFont *Font);

GuiFontPageDesc *GuiTTFontLoad(GuiLibrary *Libr,Geo2dExtent *Size,int greynb,char *FileName,int FaceNum) {
	TTFont *R;
	MemStack *Mem;
	static struct GuiFontPageDesc Static = {
		TTFontGetRange,TTFontGetGlypheDesc,TTFontGetGlyphe
	};
	Mem = rFork(2048);
	mPush(Mem,R);
	R->Mem = Mem;
	R->GuiFontPageDesc.Static = &Static;
	R->Libr=Libr;
	R->FileName = FileName;
	R->Size = *Size;
	R->FaceNum = FaceNum;
	{
		int bppShift,bpp;
		if (greynb<=1) greynb = 2;
		if (greynb>256) greynb = 256;
		R->Pages.ColorNb = greynb;
		bppShift = 0; bpp = 1<<bppShift;
		while ((1<<bpp)<greynb) { bppShift++; bpp = 1<<bppShift;}
		R->Pages.bppShift = bppShift;
	    R->Pages.SlotDim = *Size;
	    R->Pages.PageDim.w = R->Pages.SlotDim.w<<4;
	    R->Pages.PageDim.h = R->Pages.SlotDim.h<<4;
	    R->Pages.Curs.x = R->Pages.Curs.y = 0;
	    R->Pages.Next = R->Pages.SlotDim;
	    R->Pages.Pages = 0;
	}
	BinTreeInit(&R->Glyphes);
	R->Vertical = (0!=0);
	R->dg = Size->w;
	mIn(R->Mem,loadFont(R));
    TTFontPagesUpdate(R); // Load pages into library
	return &R->GuiFontPageDesc;
}

/*-------------------------------------*/

typedef struct {
	int Xshft,Xmask,onemask,bpp;
	char *p;
	int mask,dx;
} pixptr;

#define pixptrInit(pp,xshft) { \
	(pp).Xshft = 3-xshft; \
	(pp).Xmask = (1<<(3-xshft))-1;\
	(pp).bpp = (1<<xshft); \
	(pp).onemask = (1<<(1<<xshft))-1;\
}
#define pixptrSet(pp,oX,X) {\
	(pp).p = oX+(X>>((pp).Xshft)); \
	(pp).dx = (((pp).Xmask-(X & (pp).Xmask))<<(3-(pp).Xshft));\
	(pp).mask = ((pp).onemask)<<((pp).dx); \
}
#define pixptrVal(r,pp) { r = ((*(pp).p)>>(pp).dx)&((pp).onemask);}
#define pixptrSetVal(pp,r) { \
	int _col; \
	_col = (r&((pp).onemask))<<(pp).dx; \
	*(pp).p = ((*(pp).p)&(~(pp).mask))|_col;\
}
#define pixptrInc(pp) {\
	if ((pp).mask==(pp).onemask) {\
		(pp).mask = (pp.onemask)<<(8-(pp).bpp);\
		(pp).dx = 8-(pp).bpp;\
		(pp).p++;\
	} else {\
		(pp).mask = (pp).mask>>(pp).bpp;\
		(pp.dx)-=(pp).bpp;\
	}\
}


static void cpyNToMClip(imgSub *dst,imgSub *src) {
	char *yp,*yq,*ey;
	pixptr q,p;
	int w,h,xp,xq;
	int colshift;
	w = (dst->Sub.Extent.w>src->Sub.Extent.w)?src->Sub.Extent.w:dst->Sub.Extent.w;
	h = (dst->Sub.Extent.h>src->Sub.Extent.h)?src->Sub.Extent.h:dst->Sub.Extent.h;
	pixptrInit(p,src->bppShift);
	yp = src->data + (src->pitch*src->Sub.Pos.y);
	xp = src->Sub.Pos.x;
    pixptrInit(q,dst->bppShift);
	yq = dst->data + (dst->pitch*dst->Sub.Pos.y);
	xq = dst->Sub.Pos.x;
	ey = yq + (dst->pitch*h);
	colshift = p.bpp-q.bpp;
	while (yq<ey) {
		int count;
		pixptrSet(p,yp,xp); yp += src->pitch;
		pixptrSet(q,yq,xq); yq += dst->pitch;
		count = w;
		while (count) {
			int col;
			pixptrVal(col,p);
			if (colshift>0) {
				col = col>>colshift;
			} else {
				int shift;
				shift = p.bpp;
				while (shift<q.bpp) {
					col = (col<<shift) | col;
					shift = (shift<<1);
				}
			}
			pixptrSetVal(q,col);
			pixptrInc(p);
			pixptrInc(q);
			count--;
		}
	}
}
/*
static void cpyMonoToMonoClip(imgSub *dst,imgSub *src) {
	char *yp,*yq,*ey;
	int ypmask,yqmask;
	int w,h;
	w = (dst->Sub.Extent.w>src->Sub.Extent.w)?src->Sub.Extent.w:dst->Sub.Extent.w;
	h = (dst->Sub.Extent.h>src->Sub.Extent.h)?src->Sub.Extent.h:dst->Sub.Extent.h;
    yp = src->data + (src->pitch*src->Sub.Pos.y) + (src->Sub.Pos.x>>3);
	ypmask = 0x80>>((src->Sub.Pos.x)&7);
	yq = dst->data + (dst->pitch*dst->Sub.Pos.y) + (dst->Sub.Pos.x>>3);
	yqmask = 0x80>>((dst->Sub.Pos.x)&7);
	ey = yq + h*dst->pitch;
	while (yq<ey) {
        char *xp,*xq;
		int xpmask,xqmask;
		int sval,dval;
		int count;
		count = w; shift = (dst->Sub.Pos.x)&7;
		xp = yp; xpmask = ypmask; yp += src->pitch;
		xq = yq; xqmask = yqmask; yq += dst->pitch;
		sval = *xp; dval = *xq;
		while (count) {
			dval = (sval & xpmask) ? (dval|xqmask):(dval&(~xqmask));
			count--;
			if (xqmask<2) {
                xqmask = 0x80;
				*xq++ = dval;
				if (count) dval = *xq;
			} else {
				xqmask = xqmask>>1;
			}
			if (xpmask<2) {
				xpmask = 0x80; xp++;
				if (count) sval = *xp;
			} else {
				xpmask = xpmask>>1;
			}
		}
		if (xqmask!=0x80) *xq = dval;
	}
}

static void cpyMonoTo256Clip(imgSub *dst,imgSub *src) {
	char *yp,*yq,*ey;
	int ypmask;
	int w,h;
	w = (dst->Sub.Extent.w>src->Sub.Extent.w)?src->Sub.Extent.w:dst->Sub.Extent.w;
	h = (dst->Sub.Extent.h>src->Sub.Extent.h)?src->Sub.Extent.h:dst->Sub.Extent.h;
    yq = dst->data + (dst->pitch*dst->Sub.Pos.y) + dst->Sub.Pos.x;
	ey = yq + (dst->pitch*h);
	yp = src->data + (src->pitch*src->Sub.Pos.y) + (src->Sub.Pos.x>>3);
	ypmask = 0x80>>((src->Sub.Pos.x)&7);
	while (yq<ey) {
		char *xp,*xq,*ex;
		int xpmask;
        xp = yp; xpmask = ypmask;
	   	xq = yq; ex = xq+w;
		yp += src->pitch; yq+=dst->pitch;
		while (xq<ex) {
			*xq++ = (*xp&xpmask)?0xff:0x0;
			if (xpmask<=1) {
				xpmask = 0x80; xp++;
			} else {
				xpmask = xpmask>>1;
			}
		}
	}
}

static void cpy256To256Clip(imgSub *dst,imgSub *src) {
	char *yp,*yq,*ey;
	int w,h;
	w = (dst->Sub.Extent.w>src->Sub.Extent.w)?src->Sub.Extent.w:dst->Sub.Extent.w;
	h = (dst->Sub.Extent.h>src->Sub.Extent.h)?src->Sub.Extent.h:dst->Sub.Extent.h;
    yq = dst->data + (dst->pitch*dst->Sub.Pos.y) + dst->Sub.Pos.x;
	ey = yq + (dst->pitch*h);
	yp = src->data + (src->pitch*src->Sub.Pos.y) + src->Sub.Pos.x;
	while (yq<ey) {
		char *xp,*xq,*ex;
        xp = yp;
	   	xq = yq; ex = xq+w;
		yp += src->pitch; yq += dst->pitch;
		while (xq<ex) { *xq++ = *xp++; }
	}
}
*/

static GuiOutput *BitmapCpy(TTFont *fnt,int monochrome,imgSub *src) {
	GuiOutput *r;
	imgSub dst;
	int PageId;
	pm256PageGetSlot(fnt->Libr,&fnt->Pages,&PageId,&dst);
    cpyNToMClip(&dst,src);
	/* if (monochrome) {
        cpyMonoTo256Clip(&dst,src);
	} else {
        cpy256To256Clip(&dst,src);
	} */
	r = Call(fnt->Libr,Picture,2(PageId,&dst.Sub));
	return r;
}

/*-------------------------------------*/


#include <ft2build.h>
//#include <freetype2/freetype/config/ftheader.h>
#include FT_FREETYPE_H

typedef struct {
	TTFont *Font;
	struct {
		FT_Bitmap_Size *Sizes;
	} faceDesc;
} freeFontDesc;

static void cpybitmap(TTFont *d,FT_Glyph_Metrics *metric,int Idx,FT_Bitmap *bm/*,int left,int top*/) {
	// Geo2dPoint pp,ep;
	// unsigned char *p,*e;
	int pitch,bpp,sbpp;
	struct ttGlyphe *gl;
	{
	    int pidx,oidx;
		ttGlypheCollect *coll;
		pidx = Idx>>8;
		oidx = Idx&0xff;
		coll = ttGlypheCollectSeekOrInsert(&d->Glyphes,pidx);
        gl = coll->Offset+oidx;
	}
	// ep.x = bm->width;
	// ep.y = bm->rows;
	// pitch = bm->pitch;
	// pp.x = 0; pp.y = 0;
	// p = bm->buffer+(pp.y*bm->pitch+pp.x);
	sbpp = 0;
	bpp = 1<<sbpp;
	if (bm->pixel_mode==FT_PIXEL_MODE_GRAY) {
		int nb,nn;
		nb = bm->num_grays;
		nn = 1<<bpp;
		while (nn<nb) {
			sbpp++;
			bpp = 1<<sbpp;
			nn = 1<<bpp;
		}
	}
	{
		GuiOutput *o;
		imgSub sub;
		gl->Extent.w = metric->width>>6;
		gl->Extent.h = metric->height>>6;
		if (!d->Vertical) {
            gl->Grab.x = -(metric->horiBearingX>>6);
			gl->Grab.y = metric->horiBearingY>>6;
			gl->dg = metric->horiAdvance>>6;
		} else {
			gl->Grab.x = -(metric->vertBearingX>>6);
			gl->Grab.y = -(metric->vertBearingY>>6);
			gl->dg = metric->vertAdvance>>6;
		}
		sub.bppShift = sbpp;
		sub.Dim = gl->Extent;
		sub.pitch = bm->pitch;
		sub.data = bm->buffer;
		sub.Sub.Pos.x = sub.Sub.Pos.y = 0;
		sub.Sub.Extent = sub.Dim;
		o = BitmapCpy(d,(bpp==1),&sub);
		gl->Output = GuiOut2dAttach(gl->Grab.x,gl->Grab.y,o);
	}
}

static void loadFont(TTFont *Font) {
	static int Inited=(0!=0);
	static int error=(0!=0);
	static FT_Library Library=NULL;
	FT_Face face;
    if (!Inited) {
	    error = FT_Init_FreeType(&Library);
		if(!error) { Inited = (0==0); }
	}
	if (!error) {
        int err,index;
		char *FileName;
		int FaceNum;
		FileName = Font->FileName;
		FaceNum = Font->FaceNum;
		err = FT_New_Face(Library,FileName,0/* facenum 0*/,&face);
		if (!err) {
			index = face->num_faces;
			err = index <= FaceNum;
		}
		if (!err && (FaceNum!=0)) {
			err = FT_New_Face(Library,FileName,FaceNum,&face);
		}
		if (!err) {
			err = FT_Select_Charmap(face,FT_ENCODING_UNICODE);
		}
		if (!err) {
			int idx,i,nb,d0;
			Font->Vertical = Font->Vertical && FT_HAS_VERTICAL(face);
			//nb = face->num_glyphs;
			nb = face->num_fixed_sizes;
			idx = 0; d0 = -1;
			for (i=0;i<nb;i++) {
                int tw,th,d1;
				tw = face->available_sizes[i].width;
				th = face->available_sizes[i].height;
				d1 = ((tw>Font->Size.w)?tw-Font->Size.w:Font->Size.w-tw)
					+((th>Font->Size.h)?th-Font->Size.h:Font->Size.h-th);
				if (d0<0) { idx=i; d0=d1; }
				if (d1<d0) { idx=i; d0=d1; }
			}
			if ((d0!=0) && (face->face_flags & FT_FACE_FLAG_SCALABLE)) {
				err = FT_Set_Pixel_Sizes(face,Font->Size.w,Font->Size.h);
			} else {
				Font->Size.w = face->available_sizes[idx].width;
				Font->Size.h = face->available_sizes[idx].height;
			    err = FT_Set_Pixel_Sizes(face,face->available_sizes[idx].width,face->available_sizes[idx].height);
				Font->Pages.Next = Font->Pages.SlotDim = Font->Size;
				Font->Pages.PageDim.w = Font->Pages.SlotDim.w<<4;
				Font->Pages.PageDim.h = Font->Pages.SlotDim.h<<4;
			}
			if (Font->Vertical) {
				Font->dg = Font->Size.w;
			} else {
				Font->dg = Font->Size.h;
			}
			if (!err) {
				int i,idx;
				for (i=0x20;i<0x80;i++) {
					idx = FT_Get_Char_Index(face,i);
					if (idx) {
						err = FT_Load_Glyph(face,idx,FT_LOAD_DEFAULT);
						if (!err && (face->glyph->format!=FT_GLYPH_FORMAT_BITMAP)) {
							err = FT_Render_Glyph(face->glyph,FT_RENDER_MODE_MONO);/*FT_RENDER_MODE_NORMAL*/
						}
						if (!err)  {
							cpybitmap(Font,&face->glyph->metrics,i,&face->glyph->bitmap);
								/*,face->glyph->bitmap_left,face->glyph->bitmap_top*/
						}
					}
					idx = FT_Get_Char_Index(face,i+0x80);
					if (idx) {
						err = FT_Load_Glyph(face,idx,FT_LOAD_DEFAULT);
						if (!err && (face->glyph->format!=FT_GLYPH_FORMAT_BITMAP)) {
							err = FT_Render_Glyph(face->glyph,FT_RENDER_MODE_MONO);/*FT_RENDER_MODE_NORMAL*/
						}
						if (!err) {
							cpybitmap(Font,&face->glyph->metrics,i+0x80,&face->glyph->bitmap);
								/*,face->glyph->bitmap_left,face->glyph->bitmap_top*/
						}
					}
				}
			}
		}
		if (err==FT_Err_Unknown_File_Format){
		} else if (err) {
		}
		if (!err) {
			FT_Done_Face(face);
		}
	}
	if (Inited) {
		FT_Done_FreeType(Library);
		Inited = (0!=0);
	}
}

