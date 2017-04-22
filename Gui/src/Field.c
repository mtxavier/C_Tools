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
#include <GeoField.h>

#define Align4Length(l) ((((l)+3)>>2)<<2)
#define Align8Length(l) ((((l)+7)>>3)<<3)

#define Geo2dVectorSet(r,x,y) (r).w = x; (r).h=y
#define Geo2dPointSet(r,a,b) (r).x = a; (r).y=b

#define cpyBlk(P,Q,W) \
    { unsigned char *p,*q,*e; p=((unsigned char *)(P)); q=((unsigned char *)(Q)); e=q+(W); while (q<e) *p++ = *q++; }
#define setBlk(P,v,W) { unsigned char *p,*e; p=((unsigned char *)(P)); e=p+(W); while (p<e) *p++ = v; }

/*----------------------------------*/

static void nullFieldSettingEditMode(Geo2dFieldSetting *this) {}

static struct Geo2dFieldSetting nullFieldSettingStatic = { nullFieldSettingEditMode };

static int fIsSpaceNull(unsigned char *data) { return *data==' ';}

void *Geo2dFieldSettingNull(void) {
	static struct Geo2dFieldSetting Static = { nullFieldSettingEditMode };
	static Geo2dFieldSetting R;
	static unsigned char Attr[16];
	R.Static = &Static;
	R.Dim.Columns = R.Dim.Lines = 1;
	R.Data.DataWidth = 1; R.Data.AttrWidth = 0;
	R.Data.Blnk = R.Data.BlnkMsk = R.Data.Attr = R.Data.AttrMsk = Attr;
	R.EditMode.LineCut = C_(Txt,Paragraph.LineCut,Clip);
	R.EditMode.Align = C_(Txt,Paragraph.Align,Left);
	R.EditMode.fIsSpace = fIsSpaceNull;
	R.EditMode.Wrap = C_(Txt,Box.Wrap,Clip);
	return &R;
}

/*-------------------------------------*/

Geo2dTileExtent Geo2dTileExtent_Id = {{1,1},{1,0},{0,1}};

/*-------------------------------------*/

static Geo2dFieldSetting *g2dFnullGetSetting(Geo2dField *this) { return Geo2dFieldSettingNull(); }
static void  g2dFnullClear(Geo2dField *this,int From,int To) {}
static void  g2dFnullSetTileAttr(Geo2dField *this,unsigned char *Attrb,unsigned char *DataMask,unsigned char *AttrMask){}
static void  g2dFnullGotoXY(Geo2dField *this,int From,int dx,int dy) {}
static char *g2dFnullAddData(Geo2dField *this,int DataSize,char *sb,char *se) {return sb;}
static char *g2dFnullAddRawData(Geo2dField *this,char *sb,char *se) {return sb;}
static int   g2dFnullAddSeparator(Geo2dField *this, int Separator, int nb) {return nb;}
static void  g2dFnullGetCursorPos(Geo2dField *this, Geo2dPoint *R) {R->x = R->y = 0;}
static int   g2dFnullGetData(Geo2dField *this,unsigned char *Attr,unsigned char *Data,int x,int y) {return (0!=0);}
static void  g2dFnullForEachInBox(Geo2dField *this,Geo2dRectangle *Clip,Geo2dTileExtent *Tile,Geo2dFieldPeek *Perform){}
static void  g2dFnullStreamData(Geo2dField *this,Geo2dPoint *From,Geo2dFieldStream *Perform){}
static void  g2dFnullGetCharNum(Geo2dField *this,Geo2dPoint *c,int *y,int *x) { *y=-c->y; *x=0; }
static void  g2dFnullGetCharPos(Geo2dField *this,Geo2dPoint *Pos,int yLine,int Charnum) { Pos->x = Pos->y = 0; }

Geo2dField *Geo2dFieldNull(void) {
	static struct Geo2dField Static = {
		g2dFnullGetSetting,g2dFnullClear,g2dFnullSetTileAttr,g2dFnullGotoXY,
		g2dFnullAddData,g2dFnullAddRawData,g2dFnullAddSeparator,
		g2dFnullGetCursorPos,g2dFnullGetData,g2dFnullForEachInBox,g2dFnullStreamData,
		g2dFnullGetCharNum,g2dFnullGetCharPos
	};
	static Geo2dField R={&Static};
	return &R;
}

/*-----------------------------------*/

typedef struct fixed2dFieldSetting {
	Geo2dFieldSetting Geo2dFieldSetting;
	Geo2dFieldSetting *Target;
} fixed2dFieldSetting;

static void fixedFieldSettingEditMode(Geo2dFieldSetting *this) {
	ThisToThat(fixed2dFieldSetting,Geo2dFieldSetting);
	that->Target->EditMode = this->EditMode;
	Call(that->Target,SetEditMode,0);
}

Geo2dFieldSetting *Geo2dFieldSettingPtr(Geo2dFieldSetting *Target) {
	fixed2dFieldSetting *R;
	Geo2dFieldSetting *r;
	static struct Geo2dFieldSetting Static = { fixedFieldSettingEditMode };
	rPush(R);
	r = &R->Geo2dFieldSetting;
	*r = *Target;
	r->Static = &Static;
	{ 
		int w;
		unsigned char *f,*p,*q,*e;
	    w = Target->Data.DataWidth+Target->Data.AttrWidth;
		rnPush(f,Align4Length(3*w));
		r->Data.Blnk = f; f+=w;
		r->Data.Attr = f; f+=w;
		r->Data.AttrMsk = f;
		cpyBlk(r->Data.Blnk,Target->Data.Blnk,w);
		cpyBlk(r->Data.Attr,Target->Data.Attr,w);
		cpyBlk(r->Data.AttrMsk,Target->Data.AttrMsk,w);
	}
	R->Target = Target;
    return r;
}


/*-------------------------------------*/


static void Geo2dFieldSettingInit(Geo2dFieldSetting *Stng,int Columns,int Lines,unsigned char *bBlnk,unsigned char *eBlnk,int DataWidth) {
	unsigned char *fields;
	int fieldW;
	fieldW = eBlnk-bBlnk;
	rnPush(fields,Align4Length(3*fieldW));
	Stng->Dim.Columns = Columns;
	Stng->Dim.Lines = Lines;
	Stng->Data.DataWidth = DataWidth;
	Stng->Data.AttrWidth = fieldW-DataWidth;
	Stng->Data.Attr = fields; fields += fieldW;
	Stng->Data.AttrMsk = fields; fields += fieldW;
    Stng->Data.Blnk = fields; fields += fieldW;
	Stng->EditMode.LineCut = C_(Txt,Paragraph.LineCut,Symbol);
	Stng->EditMode.fIsSpace = fIsSpaceNull;
	Stng->EditMode.Align = C_(Txt,Paragraph.Align,Left);
	Stng->EditMode.Wrap = C_(Txt,Box.Wrap,Scroll);
    cpyBlk(Stng->Data.Blnk,bBlnk,fieldW);
	cpyBlk(Stng->Data.Attr,bBlnk,fieldW);
	setBlk(Stng->Data.AttrMsk,0xff,fieldW);
}

/*-------------------------------------*/

typedef struct {
	unsigned char *end;
	unsigned char *IsSpace;
} LineDesc;

#define LineDesc_Align(L) (*((L).IsSpace-1)&0xf)
#define LineDesc_SetAlign(L,V) (*((L).IsSpace-1)) = (*(((L).IsSpace-1))&0xf0)|((V)&0x0f)

#define LineDesc_Eol(L) (((*((L).IsSpace-1))&0x10)==0x10)
#define LineDesc_SetEol(L,V) if (V) {(*((L).IsSpace-1))|=0x10;} else {(*((L).IsSpace-1))&=(0xff-0x10);}

typedef struct {
	unsigned char *IsBg,Msk;
} lineIsBgPtr;
#define ibgp_Set(lfp,L,x) { lfp.IsBg = (L).IsSpace+((x)>>3); lfp.Msk=0x1<<((x)&7); }
#define ibgp_Inc(lfp) if (lfp.Msk==0x80) {lfp.Msk=0x1; lfp.IsBg++;} else {lfp.Msk = lfp.Msk<<1;}
#define ibgp_Dec(lfp) if (lfp.Msk==0x1) {lfp.Msk=0x80; lfp.IsBg--;} else {lfp.Msk = lfp.Msk>>1;}
#define ibgp_Val(a) (*a.IsBg & a.Msk)
#define ibgp_SetVal(a,v) if (v) {*a.IsBg|=a.Msk;} else {*a.IsBg&=(0xff-a.Msk);}
#define ibgp_Cpy(a,b) ibgp_SetVal(a,ibgp_Val(b))

typedef struct {
	unsigned char *Buffer;
	unsigned char *bBlank;
	int FieldWidth,Columns,LineWidth;
	int Align;
   	// By convention, neither b nor e is part of the buffer, so e==b+1 is actually a cleared buffer.
	LineDesc *bBuff,*eBuff;
	LineDesc *b,*cy;
	int cx;
	Geo2dPoint bChar; // the first lign might be the continuation of a cut previous lign.
} PageDesc;


static void pgPartLineClear(PageDesc *Pg,LineDesc *L,int bx,int ex) {
	unsigned char *p,*e,*q,*bq,*eq;
	lineIsBgPtr sp;
	LineDesc_SetAlign(*L,Pg->Align);
	if (bx<0) bx=0;
	if (ex>Pg->Columns) ex=Pg->Columns;
	ibgp_Set(sp,*L,bx);
	e = (L->end-Pg->LineWidth)+(ex*Pg->FieldWidth);
	p = (L->end-Pg->LineWidth)+(bx*Pg->FieldWidth); 
	bq = Pg->bBlank;
	eq = bq+Pg->FieldWidth;
    while (p<e) {
		ibgp_SetVal(sp,(0==0)); ibgp_Inc(sp);
		q = bq;
		while (q<eq) { *p++ = *q++;}
	}
}
static void pgEndLineClear(PageDesc *Pg,LineDesc *L,int x,int Eol) {
	LineDesc_SetEol(*L,Eol);
	pgPartLineClear(Pg,L,x,Pg->Columns);
}

static LineDesc *pgWrapAroundNextLine(PageDesc *Pg) {
	if (Pg->cy != Pg->eBuff) {
		LineDesc *pb;
		pb = Pg->eBuff-1;
	    Pg->cy = (Pg->cy==pb)?Pg->bBuff:Pg->cy+1;
		Pg->cx = 0;
    }
    return Pg->cy;
}

static LineDesc *pgDiscardNextLine(PageDesc *Pg) {
	if (Pg->cy<Pg->eBuff) {
		Pg->cy++;
		Pg->cx = 0;
	}
	return Pg->cy;
}

static int pgLineCharNum(PageDesc *Pg,LineDesc *L,int w) {
	int end,count;
	lineIsBgPtr isBg;
	ibgp_Set(isBg,*L,0);
	count = 0;
	while (w) {
		if (!ibgp_Val(isBg)) count++;
		ibgp_Inc(isBg);
        w--;
	}
	return count;
}
static int pgLineCharPos(PageDesc *Pg,LineDesc *L,int n) {
	int end,count;
	lineIsBgPtr isBg;
	ibgp_Set(isBg,*L,0);
	count = 0;
	while ((count<Pg->Columns)&&n) {
		if (!ibgp_Val(isBg)) { n--; }
		ibgp_Inc(isBg);
		count++;
	}
	if (count<Pg->Columns) { // Skip to next actual character
        int w;
		w = count;
		while ((w<Pg->Columns)&&(ibgp_Val(isBg))) { ibgp_Inc(isBg); w++; }
		if (w<Pg->Columns) count = w;
	}
	return count;
}
static int pgLineDataCount(PageDesc *Pg,LineDesc *L) {
	return pgLineCharNum(Pg,L,Pg->Columns);
}


static LineDesc *pgScrollNextLine(PageDesc *Pg) {
	if (Pg->cy != Pg->eBuff) {
		LineDesc *pb;
		pb = Pg->eBuff-1;
		Pg->cy = (Pg->cy==pb)?Pg->bBuff:Pg->cy+1;
		if (Pg->cy==Pg->b) {
			if (LineDesc_Eol(*Pg->cy)) {
				Pg->bChar.x = Pg->bChar.y = 0;
			} else {
				Pg->bChar.y ++;
				Pg->bChar.x += pgLineDataCount(Pg,Pg->cy);
			}
			pgEndLineClear(Pg,Pg->cy,0,(0!=0));
			Pg->b = (Pg->b==pb)? Pg->bBuff:Pg->b+1;
		}
		Pg->cx = 0;
	}
	return Pg->cy;
}

static LineDesc *pgGetLine(PageDesc *Pg,int y) {
	LineDesc  *R;
	R = Pg->b + y;
	if ((R>=Pg->eBuff)&&(Pg->b!=Pg->bBuff)) R+=(Pg->bBuff-Pg->eBuff);
    if ((R<Pg->bBuff)||(R>=Pg->eBuff)) R = Pg->eBuff;
	return R;
}

static int pgGotoXY(PageDesc *Pg,int x,int y) {
	int Success;
	int lnb;
	lnb = (Pg->eBuff-Pg->bBuff);
	Success = (x>=0) && (x<=Pg->Columns) && (y>=0) && (y<=lnb);
	if (y==lnb) x=0;
	if (Success) {
		Pg->cy = Pg->b+y;
		if (Pg->cy>Pg->eBuff) Pg->cy-=lnb;
		Pg->cx = x;
	}
	return Success;
}
static void pgGetCursXY(PageDesc *Pg,Geo2dExtent *R) {
	if (Pg->cy<Pg->b) {
        R->h = (Pg->cy-Pg->bBuff)+(Pg->eBuff-Pg->b);
	} else {
		R->h = Pg->cy-Pg->b;
	}
	if ((Pg->cy>=Pg->eBuff)||(Pg->cy<Pg->bBuff)) {
		R->w = 0;
	} else {
	    R->w = Pg->cx;
	}
}

static void pgGetCharNum(PageDesc *Pg,int yLine,int x,int *dy,int *CharNum) {
	LineDesc *c;
	c = pgGetLine(Pg,yLine);
	if (c==Pg->eBuff) {
		*dy = 0; *CharNum = 0;
	} else {
        int d,cnt,end,w;
		w = x; if (w<=0) w = 0; if (w>=Pg->Columns) w=Pg->Columns;
		d = 0; cnt = pgLineCharNum(Pg,c,w);
		do {
			end = (c==Pg->b);
			if (end) {
				cnt += Pg->bChar.x; d += Pg->bChar.y;
			} else {
				if (c==Pg->bBuff) { c = Pg->eBuff-1; } else { c = c-1; }
				end = LineDesc_Eol(*c);
				if (!end) {
					d++;
			        cnt += pgLineCharNum(Pg,c,Pg->Columns);
				}
			}
		} while(!end);
		*dy=d; *CharNum=cnt;
	}
}

static void pgGetCharPos(PageDesc *Pg,int *x,int *y,int yLine,int CharNum) {
	LineDesc *c;
	c = pgGetLine(Pg,yLine);
	if (c==Pg->eBuff) {
		Geo2dExtent R;
        pgGetCursXY(Pg,&R);
		*x = R.w; *y = R.h;
	} else {
		LineDesc *bc;
		int rx,ry,end;
		bc = c; rx = 0; ry = yLine;
		do { /* Fetch line start. */
            end = (bc==Pg->b);
			if (end) {
				CharNum -= Pg->bChar.x;
			} else {
				LineDesc *ic;
				if (bc==Pg->bBuff) { ic = Pg->eBuff-1; } else { ic = bc-1; }
				end = LineDesc_Eol(*ic);
				if (!end) { bc = ic; ry--; }
			}
		} while(!end);
		end = (CharNum<0);
		while (!end) {
            int lw;
			lw = pgLineCharNum(Pg,bc,Pg->Columns);
			end = ((CharNum<=0)||(lw>=CharNum));
			if (end) {
				rx = pgLineCharPos(Pg,bc,CharNum);
			} else {
				CharNum -= lw;
				end = LineDesc_Eol(*bc);
				if (!end) {
					c = (bc>=(Pg->eBuff-1)) ? Pg->bBuff:bc+1;
					end = (c==Pg->b);
					if (!end) { bc = c; ry++; }
				}
				if (end) { rx = pgLineCharPos(Pg,bc,lw); }
			}
		}
        *x = rx; *y = ry;
	}
}

static int pgLineRightMost(PageDesc *Pg,LineDesc *L) { // non inclue
	int end,skip,w;
	lineIsBgPtr isBg;
	w = Pg->Columns;
	ibgp_Set(isBg,*L,w);
	skip = 0;
	do {
		end = (skip>=w);
		if (!end) {
			ibgp_Dec(isBg);
            end = !ibgp_Val(isBg);
			if (!end) skip++;
		}
	} while (!end);
	return w-skip;
}
static int pgLineLeftMost(PageDesc *Pg,LineDesc *L) {
	int skip,w,end;
	lineIsBgPtr isBg;
	w = Pg->Columns;
	ibgp_Set(isBg,*L,0);
	skip = 0;
    do {
		end = (skip>=w);
		if (!end) {
			end = !ibgp_Val(isBg);
			ibgp_Inc(isBg);
			if (!end) skip++;
		}
	} while (!end);
	return skip;
}
static int pgLineAddnBg(PageDesc *Pg,LineDesc *L,int x,int n) {
	int w;
	w = Pg->Columns;
	if (x+n>w) n = w-x;
	if (x<0) { n+=x; x=0;}
	if (n>0) {
		int e;
		lineIsBgPtr isBg;
		e = x+n;
		ibgp_Set(isBg,*L,x);
		while (x<e) {
			ibgp_SetVal(isBg,(0==0));
			ibgp_Inc(isBg);
			x++;
		}
	} else {
		n = 0;
	}
	return n;
}
static int pgLineAddnData(PageDesc *Pg,LineDesc *L,int x,int n) {
	int w;
	w = Pg->Columns;
	if (x+n>w) n = w-x;
	if (x<0) { n+=x; x=0;}
	if (n>0) {
		int e;
		lineIsBgPtr p;
		e = x+n;
		ibgp_Set(p,*L,x);
		while (x<e) {
			ibgp_SetVal(p,(0!=0));
			ibgp_Inc(p);
			x++;
		}
	} else {
		n = 0;
	}
	return n;
}

static void pgLineShiftRight(PageDesc *Pg,LineDesc *L,int bb,int eb,int n) {
	int w,fw,sb;
	w = Pg->Columns;
	fw = Pg->FieldWidth;
	if (eb>w) eb = w;
	if (eb+n>w) sb = w-n; else sb=eb;
	if (bb<0) bb = 0;
	if (eb>bb) {
	    unsigned char *d,*p,*q,*ef,*e;
		lineIsBgPtr ip,iq;
	    d = L->end-Pg->LineWidth;
		e = d+(bb*fw);
        p = d+(sb*fw);
		ibgp_Set(ip,*L,sb);
		q = p+(n*fw);
		ibgp_Set(iq,*L,sb+n);
		while (p>e) {
			ef = p-fw;
			while (p>ef) { q--; p--; *q = *p; }
			ibgp_Dec(iq); ibgp_Dec(ip); ibgp_Cpy(iq,ip);
		}
		if (q>(d+(eb*fw))) {
			q = d+(eb*fw);
			ibgp_Set(iq,*L,eb);
		}
		while (q>e) {
			p = Pg->bBlank+fw;
			ef = p-fw;
			while (p>ef) { q--; p--; *q = *p;}
			ibgp_Dec(iq); ibgp_SetVal(iq,(0==0));
		}
	}
}
static void pgLineShiftLeft(PageDesc *Pg,LineDesc *L,int bb,int eb,int n) {
	int w,fw,sb;
	w = Pg->Columns;
	fw = Pg->FieldWidth;
	if (eb>w) eb = w;
	if (bb<0) bb = 0;
	if (bb-n<0) sb = n; else sb=bb;
	if (eb>bb) {
	    unsigned char *d,*p,*q,*ef,*e;
		lineIsBgPtr ip,iq;
		d = L->end-Pg->LineWidth;
		e = d + (fw*eb);
		p = d + (fw*sb);
		ibgp_Set(ip,*L,sb);
		q = p - (fw*n);
		ibgp_Set(iq,*L,sb-n);
		while (p<e) {
			ef = p+fw;
			while (p<ef) { *q++=*p++;}
			ibgp_Cpy(iq,ip); ibgp_Inc(iq); ibgp_Inc(ip);
		}
		if (q<d+(fw*bb)) {
			q = d+(fw*bb);
			ibgp_Set(iq,*L,bb);
		}
		while (q<e) {
			p = Pg->bBlank;
			ef = p+fw;
			while (p<ef) { *q++=*p++;}
			ibgp_SetVal(iq,(0==0)); ibgp_Inc(iq);
		}
	}
}
static void pgLineCompactLeft(PageDesc *Pg,LineDesc *L) {
	unsigned char *b,*p,*q,*e,*bgfill;
	int fw;
	lineIsBgPtr ip,iq;
	bgfill = Pg->bBlank;
    fw = Pg->FieldWidth;
    e = L->end;
	p = q = b = e-Pg->LineWidth;
	ibgp_Set(ip,*L,0);
	ibgp_Set(iq,*L,0);
	while (p<e) {
        if (!ibgp_Val(ip)) {
		    unsigned char *fe;
		    fe = p+fw;
            while (p<fe) { *q++ = *p++; }
			ibgp_SetVal(iq,(0!=0)); ibgp_Inc(iq);
		} else {
			p+=fw;
		}
		ibgp_Inc(ip);
	}
	while (q<e) {
		unsigned char *fe;
		p = bgfill; fe = p+fw; while (p<fe) { *q++=*p++;}
		ibgp_SetVal(iq,(0==0)); ibgp_Inc(iq);
	}
}
static void pgLineCompactRight(PageDesc *Pg,LineDesc *L) {
	unsigned char *d,*p,*q,*e,*bgfill;
	int fw;
	lineIsBgPtr ip,iq;
	bgfill = Pg->bBlank;
	fw = Pg->FieldWidth;
    e = L->end - Pg->LineWidth;
	p = q = L->end;
	ibgp_Set(ip,*L,Pg->Columns);
	ibgp_Set(iq,*L,Pg->Columns);
	while (p>e) {
		ibgp_Dec(ip);
		if (!ibgp_Val(ip)) {
			unsigned char *fe;
			fe = p-fw;
			while (p>fe) { p--; q--; *q = *p; } 
			ibgp_Dec(iq); ibgp_SetVal(iq,(0!=0));
		} else {
			p-=fw;
		}
	}
	while (q>e) {
		unsigned char *fe;
		p = bgfill+fw; e = bgfill; while (p>fe) { p--; q--; *q=*p; }
		ibgp_Dec(iq); ibgp_SetVal(iq,(0==0));
	}
}
static void pgLineExpand(PageDesc *Pg,LineDesc *L,int (*fIsSpace)(unsigned char *Field)) {
	int fw,lm,rm;
    fw = Pg->FieldWidth;
    lm = pgLineLeftMost(Pg,L);
    rm = pgLineRightMost(Pg,L);
	if ((lm>0)||(rm<Pg->Columns)) {
	    int bgCount,dataCount,spaceCount,gapCount;
		unsigned char *b,*e,*fe,*p,*q;
		unsigned char *IsSpace;
		lineIsBgPtr ip;
		rOpen
		rnPush(IsSpace,Pg->Columns);
		e = L->end;
		b = L->end-Pg->LineWidth;
		dataCount = 0;
		bgCount = 0;
		spaceCount =0;
		p = b;
		ibgp_Set(ip,*L,0);
		q = IsSpace;
		while (p<e) {
			if (!ibgp_Val(ip)) {
				if (fIsSpace(p)) { *q = 1; spaceCount++; } else { *q=0; dataCount++; }
			} else {
				*q = 3; bgCount++;
			}
			ibgp_Inc(ip); q++; p+=fw;
		}
		{
			int inSpace;
            gapCount = 0; inSpace=(0==0);
		    p = IsSpace; e = p+Pg->Columns;
			while (p<e) {
				if ((!inSpace)&&((*p!=0))) { gapCount++; }
				inSpace = *p++;
			}
			if (inSpace) gapCount--; // Last gap doesn't count.
		}
		if ((gapCount>=1)&&(bgCount>0)) { // bgs blocks have to be distributed on the line
			unsigned char *is,*blank,*ef;
			int gc,dstr,dstre,inSpace;
			lineIsBgPtr iq;
            pgLineCompactRight(Pg,L);
			blank = Pg->bBlank;
			q = b; p = b+(fw*bgCount); is=IsSpace; e=is+Pg->Columns; ibgp_Set(iq,*L,0);
		    gc = 0; dstr = 0;  inSpace=(0==0);
			while ((is<e)&&(dstr<bgCount)) {
                if ((!inSpace)&&((*is!=0))) {
					gc++;
					dstre = (bgCount*gc)/gapCount;
					while (dstr<dstre) {
						unsigned char *cb;
						cb = blank; ef = q+fw; while (q<ef) {*q++=*cb++;}
						ibgp_SetVal(iq,(0==0));
						dstr++; ibgp_Inc(iq);
					}
				}
				if (*is!=3) {
					ef = q+fw; while (q<ef) {*q++=*p++;}
					ibgp_SetVal(iq,(0!=0)); ibgp_Inc(iq);
				}
				inSpace = *is++;
			}
		}
		rClose
	}
}

static void pgDescInit(PageDesc *Pg,Geo2dFieldSetting *St) {
	unsigned char *IsSpace;
	int lnb,dIsSpace;
	dIsSpace = 1+((St->Dim.Columns+7)>>3);
	lnb = St->Dim.Lines;
	Pg->bBlank = St->Data.Blnk;
	Pg->FieldWidth = St->Data.DataWidth+St->Data.AttrWidth;
	Pg->Columns = St->Dim.Columns;
	Pg->LineWidth = Pg->FieldWidth*Pg->Columns;
	Pg->Align = C_(Txt,Paragraph.Align,Left);
	rnPush(Pg->Buffer,Align4Length(lnb*Pg->LineWidth));
	Pg->cx = 0;
	rnPush(Pg->bBuff,lnb+1);
	Pg->eBuff = Pg->bBuff + lnb;
	Pg->cy = Pg->b = Pg->bBuff;
	Pg->bChar.x = Pg->bChar.y = 0;
	rnPush(IsSpace,Align4Length(dIsSpace*lnb));
	IsSpace++;
    /* Fill */{ 
	    LineDesc *i,*e;
		unsigned char *buff,*isp;
		i = Pg->bBuff; e = i+lnb;
		buff = Pg->Buffer;
		isp = IsSpace;
		while (i<e) {
			buff += Pg->LineWidth; i->end = buff;
			i->IsSpace = isp; isp += dIsSpace;
			pgEndLineClear(Pg,i,0,(0!=0));
			i++;
	    }
		/* Last line is outside of the buffer. We make it point on the 1st line (helps scroll management). */
		i->IsSpace = IsSpace;
		i->end = Pg->Buffer+Pg->LineWidth;
	}
}

/*-------------------------------------*/

typedef struct fixed2dField {
	Geo2dField Geo2dField;
	Geo2dFieldSetting Setting;
	PageDesc Pg;
} fixed2dField;

static Geo2dFieldSetting *fixed2dFieldGetSetting(Geo2dField *this) {
	ThisToThat(fixed2dField,Geo2dField);
    return Geo2dFieldSettingPtr(&that->Setting);
}

#define trunk(x,min,max) { if (x<min) x=min; if (x>max) x=max;}

static void f2dfGetPos(fixed2dField *Fld,Geo2dExtent *R,int Pos) {
	// From,To : C_(Txt,ScopeMark,{Top,Bottom,BufferTop,BufferBottom,PageTop,PageBottom,Cursor,LineStart,LineEnd,PrgrphStart,PrgphEnd})
	if ((Pos==C_(Txt,ScopeMark,Top))||(Pos==C_(Txt,ScopeMark,BufferTop))||(Pos==C_(Txt,ScopeMark,PageTop))) {
		R->w = R->h = 0;
	} else {
	if ((Pos==C_(Txt,ScopeMark,Bottom))||(Pos==C_(Txt,ScopeMark,BufferBottom))||(Pos==C_(Txt,ScopeMark,PageBottom))) {
        R->w =0; R->h=Fld->Setting.Dim.Lines;
	} else {
	if ((Pos==C_(Txt,ScopeMark,PrgphStart))||(Pos==C_(Txt,ScopeMark,LineStart))) {
	    pgGetCursXY(&Fld->Pg,R);
		R->w = 0;
	} else {
	if ((Pos==C_(Txt,ScopeMark,LineEnd))|| (Pos==C_(Txt,ScopeMark,PrgphEnd))) {
	    pgGetCursXY(&Fld->Pg,R);
		if (R->h<Fld->Setting.Dim.Lines) {
			R->w = Fld->Setting.Dim.Columns;
		} else {
			R->h = Fld->Setting.Dim.Lines;
			R->w = 0;
		}
	} else {
	    pgGetCursXY(&Fld->Pg,R);
	}}}}
	trunk(R->w,0,Fld->Setting.Dim.Columns);
	trunk(R->h,0,Fld->Setting.Dim.Lines);
	if (R->h==Fld->Setting.Dim.Lines) R->w=0;
}
static void f2dfGetCursPos(fixed2dField *Fld,Geo2dExtent *R) {
	pgGetCursXY(&Fld->Pg,R);
	trunk(R->w,0,Fld->Setting.Dim.Columns);
	trunk(R->h,0,Fld->Setting.Dim.Lines);
	if (R->h==Fld->Setting.Dim.Lines) R->w=0;
}

static void fixed2dFieldClear(Geo2dField *this,int From,int To) {
	Geo2dExtent B,E;
	ThisToThat(fixed2dField,Geo2dField);
    f2dfGetPos(that,&B,From);
	f2dfGetPos(that,&E,To);
	if ((B.h<=E.h)&&(B.h<that->Setting.Dim.Lines)) {
		int x,y;
		LineDesc *yl;
		y = B.h;
		yl = pgGetLine(&that->Pg,y);
		if (B.h==E.h) { // One line
            if (B.w<E.w) {
				pgPartLineClear(&that->Pg,yl,B.w,E.w);
			}
		} else { // Multiple lines;
			if (B.w>0) {
				pgEndLineClear(&that->Pg,yl,B.w,(0!=0));
				y++;
			}
			while (y<(E.h-1)) {
				yl = pgGetLine(&that->Pg,y);
				pgEndLineClear(&that->Pg,yl,0,(0!=0));
				y++;
			}
            if (y!=that->Setting.Dim.Lines) {
				if (E.w>0) {
				    yl = pgGetLine(&that->Pg,y);
                    pgPartLineClear(&that->Pg,yl,0,E.w);
				}
			}
		}
	}
}
static void fixed2dFieldSetTileAttr(Geo2dField *this,unsigned char *Attrb,unsigned char *DataMask,unsigned char *AttrMask) {
	ThisToThat(fixed2dField,Geo2dField);
	cpyBlk(that->Setting.Data.Attr+that->Setting.Data.DataWidth,Attrb,that->Setting.Data.AttrWidth);
	cpyBlk(that->Setting.Data.AttrMsk,DataMask,that->Setting.Data.DataWidth);
	cpyBlk(that->Setting.Data.AttrMsk+that->Setting.Data.DataWidth,AttrMask,that->Setting.Data.AttrWidth);
}
static void fixed2dFieldGotoXY(Geo2dField *this,int From,int x,int y) {
	/*ToDo : include "From" */
	ThisToThat(fixed2dField,Geo2dField);
	trunk(x,0,that->Setting.Dim.Columns);
	trunk(y,0,that->Setting.Dim.Lines);
	if (y==that->Setting.Dim.Lines) { x=0; }
	pgGotoXY(&that->Pg,x,y);
}
#define cpyField(q,p,ep,msk) {\
	unsigned char *m,mq,mp; \
	m=msk; while(p<ep) {mp=*m++; mq=0xff^mp; *q=(*q&mq)|(*p&mp); p++; q++;}\
}
static int checkAttr(unsigned char *old,unsigned char *new,unsigned char *msk,unsigned char *emsk) {
	unsigned char *m;
	int diff;
	diff = (0!=0);
	m=msk;
	while ((!diff)&&(m<emsk)) {
		unsigned char cp,cq,cm;
		cm = *m++; cp = *old++; cq = *new++;
		diff = ((cp&cm)!=(cq&cm));
	}
	return diff;
}

static void f2dfLineExpand(fixed2dField *Fld) {
	if (Fld->Setting.EditMode.Align==C_(Txt,Paragraph.Align,Right)) {
		pgLineCompactRight(&Fld->Pg,Fld->Pg.cy);
	} else {
		if (Fld->Setting.EditMode.Align==C_(Txt,Paragraph.Align,Center)) {
			int charCount;
			pgLineCompactLeft(&Fld->Pg,Fld->Pg.cy);
			charCount = pgLineDataCount(&Fld->Pg,Fld->Pg.cy);
			pgLineShiftRight(&Fld->Pg,Fld->Pg.cy,0,charCount,(Fld->Pg.Columns-charCount)>>1);
		} else {
			if (Fld->Setting.EditMode.Align==C_(Txt,Paragraph.Align,Expand)) {
				pgLineExpand(&Fld->Pg,Fld->Pg.cy,Fld->Setting.EditMode.fIsSpace);
			}
		}
	}
}

static int f2dfLineContinue(fixed2dField *Fld,int Eol) {
	int r;
	r = (Fld->Setting.EditMode.LineCut!=C_(Txt,Paragraph.LineCut,Clip));
	if (!r) {
	    LineDesc_SetEol(*(Fld->Pg.cy),(0==0));
	} else {
	    LineDesc *d;
	    LineDesc_SetEol(*(Fld->Pg.cy),Eol);
	    f2dfLineExpand(Fld);
	    if (Fld->Setting.EditMode.Wrap==C_(Txt,Box.Wrap,Scroll)) {
		    d = pgScrollNextLine(&Fld->Pg);
	    } else {
		    if (Fld->Setting.EditMode.Wrap==C_(Txt,Box.Wrap,WrapAround)) {
			    d = pgWrapAroundNextLine(&Fld->Pg);
		    } else {
			    d = pgDiscardNextLine(&Fld->Pg);
		    }
	    }
	    r= (d!=Fld->Pg.eBuff);
	}
	if (r) {Fld->Pg.cx = 0;}
	return r;
}
static int f2dfWordFit(fixed2dField *Fld,int wSize) {
	int r;
	r = (0==0);
	if (Fld->Setting.EditMode.LineCut==C_(Txt,Paragraph.LineCut,Word) && (wSize<=Fld->Setting.Dim.Columns)) {
		int xleft; 
		xleft = Fld->Pg.Columns-Fld->Pg.cx;
		if (xleft<wSize) {
	        pgPartLineClear(&Fld->Pg,Fld->Pg.cy,Fld->Pg.cx,Fld->Pg.Columns);
	        Fld->Pg.cx = Fld->Pg.Columns;
			r = f2dfLineContinue(Fld,(0!=0));
		}
	}
	return r;
}
static char *fixed2dFieldAddData(Geo2dField *this,char *sb,char *se,int ds) {
	int end,n;
	ThisToThat(fixed2dField,Geo2dField);
	end = that->Setting.Data.DataWidth<1;
	n = ds;
	if (n>that->Setting.Data.DataWidth) n=that->Setting.Data.DataWidth;
	if (!end) end = !f2dfWordFit(that,(se-sb)/ds);
	if (!end) {
	    unsigned char *q,*p,*e,*bAttr,*Attr,*eAttr,*AttrMsk;
	    lineIsBgPtr iq;
	    LineDesc *y;
	    int cx;
		AttrMsk = that->Setting.Data.AttrMsk;
		bAttr = that->Setting.Data.Attr; 
		Attr = bAttr+that->Setting.Data.DataWidth;
		eAttr = Attr+that->Setting.Data.AttrWidth; 
	    setBlk(bAttr,0,that->Setting.Data.DataWidth);
	    while ((sb<se)&&(!end)) {
		    cx = that->Pg.cx;
	        y = that->Pg.cy; e = y->end; 
		    q = e-that->Pg.LineWidth+(cx*that->Pg.FieldWidth); ibgp_Set(iq,*y,cx);
			while ((q<e)&&(sb<se)) {
				unsigned char *p;
				p = bAttr;
				cpyBlk(bAttr,sb,n);
				cpyField(q,p,eAttr,AttrMsk);
				ibgp_SetVal(iq,(0!=0)) ibgp_Inc(iq);
				sb+=ds; cx++;
			}
			that->Pg.cx = cx;
			if (sb<se) {
				end = !f2dfLineContinue(that,(0!=0));
			}
	    }
	}
	return sb;
}
static char *fixed2dFieldAdd8BitData(Geo2dField *this,int DataSize,char *sb,char *se) {
	ThisToThat(fixed2dField,Geo2dField);
	if (that->Setting.Data.DataWidth>=1) {
		sb = fixed2dFieldAddData(this,sb,se,DataSize);
	}
	return sb;
}

static char *fixed2dFieldAddRawData(Geo2dField *this,char *sb,char *se) {
	int end;
	unsigned char *mask;
	ThisToThat(fixed2dField,Geo2dField);
	end = (sb>=se)||(that->Pg.cy>=that->Pg.eBuff);
	mask = that->Setting.Data.AttrMsk;
	while ((sb<se) && (!end)) {
	    LineDesc *y;
		unsigned char *p,*e;
		lineIsBgPtr ip;
		int x,fw;
		fw = that->Pg.FieldWidth;
		y = that->Pg.cy;
		x = that->Pg.cx;
		e = y->end;
		p = (y->end-that->Pg.LineWidth) + (fw*x);
		ibgp_Set(ip,*y,that->Pg.cx);
		while ((p<e)&&(sb<se)) {
			char *see;
			see = sb+fw;
            cpyField(p,sb,see,mask);
			ibgp_SetVal(ip,(0!=0)); ibgp_Inc(ip); x++;
		}
		that->Pg.cx=x;
		if (sb<se) {
			end = !f2dfLineContinue(that,(0==0));
		}
	}
	return sb;
}

static int f2dfAddnBlank(fixed2dField *Fld,int nb) {
	int end;
	end = (Fld->Pg.cy==Fld->Pg.eBuff);
	while (nb && !end) {
	    LineDesc *Y;
		int x,ex;
		Y = Fld->Pg.cy; x=Fld->Pg.cx;
	    ex = Fld->Pg.Columns;
		if (ex-x>nb) ex=x+nb;
		pgPartLineClear(&Fld->Pg,Y,x,ex);
		nb -= (ex-x);
		Fld->Pg.cx = ex;
		if (nb) { end = !f2dfLineContinue(Fld,(0!=0)); }
	}
	return nb;
}
static int f2dfAddnAttr(fixed2dField *Fld,int nb) {
	unsigned char *AttrMsk,*eAttr,*Attr;
	int end;
	AttrMsk = Fld->Setting.Data.AttrMsk+Fld->Setting.Data.DataWidth;
	Attr =    Fld->Setting.Data.Attr+Fld->Setting.Data.DataWidth;
	eAttr = Attr+Fld->Setting.Data.AttrWidth; 
	end = (Fld->Pg.cy==Fld->Pg.eBuff);
	while (nb && !end) {
		LineDesc *y;
		unsigned char *p,*e;
		int x,fw;
		lineIsBgPtr ip;
		fw = Fld->Pg.FieldWidth;
		y = Fld->Pg.cy;
		x = Fld->Pg.cx;
		e = y->end;
		p = (y->end-Fld->Pg.LineWidth) + (fw*x);
		ibgp_Set(ip,*y,Fld->Pg.cx);
		while ((p<e)&&(nb)) {
			unsigned char *q;
		    p += Fld->Setting.Data.DataWidth;
			q = Attr;
			cpyField(p,q,eAttr,AttrMsk);
            ibgp_SetVal(ip,(0!=0)) ibgp_Inc(ip);
			x++; nb--;
		}
		Fld->Pg.cx=x;
		if (nb) {
			end = !f2dfLineContinue(Fld,(0!=0));
		}
	}
	return nb;
}
static int f2dfAddnEol(fixed2dField *Fld,int nb) {
	int end;
	end = (Fld->Pg.cy==Fld->Pg.eBuff);
	while ((nb>0) && !end) {
		pgEndLineClear(&Fld->Pg,Fld->Pg.cy,Fld->Pg.cx,(0==0));
		end = !f2dfLineContinue(Fld,(0==0));
		nb--;
	}
	return nb;
}
static int f2dfJoinnLine(fixed2dField *Fld,int nb) {
	int end;
	end = (Fld->Pg.cy==Fld->Pg.eBuff);
	while ((nb>0) && !end) {
		Fld->Pg.cx = Fld->Pg.Columns;
		end = !f2dfLineContinue(Fld,(0!=0));
		nb--;
	}
	return nb;
}

static int fixed2dFieldAddSeparator(Geo2dField *this,int Separator,int nb) {
	ThisToThat(fixed2dField,Geo2dField);
	if (nb>0) {
		if ((Separator==C_(Txt,Paragraph.Mark,Char))||(Separator==C_(Txt,Paragraph.Mark,Blank))) {
			if (Separator == C_(Txt,Paragraph.Mark,Blank)) {
			    nb = f2dfAddnBlank(that,nb);
		    } else {
		        nb = f2dfAddnAttr(that,nb);
		    }
		} else {
			if (Separator ==  C_(Txt,Paragraph.Mark,JoinLine)) {
		        nb = f2dfJoinnLine(that,nb); // Join the following nb lines.
		    }
			if (Separator == C_(Txt,Paragraph.Mark,EndOfLine)) {
		        nb = f2dfAddnEol(that,nb); // Insert nb lines.
			}
		}
	}
	return nb;
}
static void fixed2dFieldGetCursorPos(Geo2dField *this,Geo2dPoint *R) {
	Geo2dExtent g;
	ThisToThat(fixed2dField,Geo2dField);
	f2dfGetCursPos(that,&g);
	R->x = g.w;
	R->y = g.h;
}
static int fixed2dFieldGetData(Geo2dField *this,unsigned char *Attr,unsigned char *Data,int x,int y) {
	int r;
	ThisToThat(fixed2dField,Geo2dField);
    r = (x>=0) && (x<that->Setting.Dim.Columns) && (y>=0) && (y<that->Setting.Dim.Lines);
	if (r) {
	    LineDesc *Y;
		unsigned char *p,*q,*e;
	    Y = pgGetLine(&that->Pg,y);
        p = (Y->end-that->Pg.LineWidth) + (that->Pg.FieldWidth*x);
		q = Data; e=q+that->Setting.Data.DataWidth; while (q<e) {*q++=*p++;}
		q = Attr; e=q+that->Setting.Data.AttrWidth; while (q<e) {*q++=*p++;}
	}
	return r;
}

static void repBackChange(Geo2dPoint *r,Geo2dTileExtent *Tile) {
	int det,x,y;
	// x1 = dx.x*x0 + dy.x*y0
	// y1 = dx.y*x0 + dy.y*y0
	// 
	// dy.y*x1 = dy.y*dx.x*x0 + dy.y*dy.x*y0
	// dy.x*y1 = dy.x*dx.y*x0 + dy.y*dy.x*y0
	// (dy.y*x1-dy.x*y1) = (dy.y*dx.x-dy.x*dx.y)*x0
	//
	// dx.y*x1 = dx.y*dx.x*x0 + dx.y*dy.x*y0
	// dx.x*y1 = dx.x*dx.y*x0 + dx.x*dy.y*y0
	// (dx.x*y1-dx.y*x1) = (dx.x*dy.y-dx.y*dy.x)*y0
	det = Tile->dx.x*Tile->dy.y - Tile->dx.y*Tile->dy.x;
	x = Tile->dy.y*r->x - Tile->dy.x*r->y;
	y = Tile->dx.x*r->y - Tile->dx.y*r->x;
	r->x = x/det;
	r->y = y/det;
}
static void repChange(Geo2dPoint *r,Geo2dTileExtent *stng) {
    int x,y;
	x = stng->dx.x*r->x + stng->dy.x*r->y;
	y = stng->dx.y*r->x + stng->dy.y*r->y;
	r->x = x;
	r->y = y;
}
static int mnmxVertCut(Geo2dTileExtent *stng,Geo2dPoint *Org,int x0) {
	int x,r;
	x = (x0-Org->x);
	r = x/stng->dx.x;
	return r;
}
static int mnmxHorzCut(Geo2dTileExtent *stng,Geo2dPoint *Org,int y0) {
	int x,r;
	x = (y0-Org->y);
	r = x/stng->dx.y;
	return r;
}
static void mnmxLineCut(int *xmin,int *xmax,Geo2dTileExtent *stng,Geo2dRectangle *clip,int y) {
	int cut[4];
	int order[4];
	int vert,horz;
	Geo2dPoint Org;
	*xmin = -1; 
	*xmax = -2;
	Org.x = 0; Org.y = y;
	repChange(&Org,stng);
	if (stng->dx.x!=0) {
        cut[0] = mnmxVertCut(stng,&Org,clip->Pos.x);
	    cut[1] = mnmxVertCut(stng,&Org,clip->Pos.x+clip->Extent.w-1);
	    if (cut[0]>cut[1]) { order[0]=1; order[1]=0; } else { order[0]=0; order[1]=1; }
	}
	if (stng->dx.y!=0) {
	    cut[2] = mnmxHorzCut(stng,&Org,clip->Pos.y);
	    cut[3] = mnmxHorzCut(stng,&Org,clip->Pos.y+clip->Extent.h-1);
	    if (cut[2]>cut[3]) { order[2]=3; order[3]=2; } else { order[2]=2; order[3]=3; }
	}
	if ((stng->dx.x!=0)&&(stng->dx.y!=0)) {
	    if (cut[order[1]]>cut[order[3]]) { int xchg; xchg = order[3]; order[3] = order[1]; order[1]= xchg; }
	    if (cut[order[0]]>cut[order[2]]) { int xchg; xchg = order[2]; order[2] = order[0]; order[0]= xchg; }
	    if (cut[order[1]]>cut[order[2]]) { int xchg; xchg = order[2]; order[2] = order[1]; order[1]= xchg; }
	    vert = (order[0]==0)||(order[0]==1);
	    horz = (order[1]==2)||(order[1]==3);
	    if (vert==horz) { *xmin = cut[order[1]]; *xmax = cut[order[2]]; }
	} else {
		if (stng->dx.x==0) {
			if ((Org.x>=clip->Pos.x)&&(Org.x<clip->Pos.x+clip->Extent.w)&&(stng->dx.y!=0)) {
				*xmin = cut[order[2]]; *xmax = cut[order[3]];
			}
		} else {
			if ((Org.y>=clip->Pos.y)&&(Org.y<clip->Pos.y+clip->Extent.h)) {
				*xmin = cut[order[0]]; *xmax = cut[order[1]];
			}
		}
	}
}

static void fixed2dFieldForEachInBox(Geo2dField *this,Geo2dRectangle *Clip,Geo2dTileExtent *Tile,Geo2dFieldPeek *Perform){
	Geo2dPoint coins[4];
	Geo2dRectangle clip;
    struct mnmxArray {
       int xmin,xmax,ymin,ymax,dy;
    } nx ;
	int i;
	ThisToThat(fixed2dField,Geo2dField);
	clip = *Clip;
	clip.Pos.x += 1-Tile->Extent.w;
	clip.Pos.y += 1-Tile->Extent.h;
	clip.Extent.w += 2*(Tile->Extent.w-1);
    clip.Extent.h += 2*(Tile->Extent.h-1);
	coins[0] = clip.Pos;
    coins[2].x = clip.Pos.x+clip.Extent.w-1; 
	coins[2].y = clip.Pos.y+clip.Extent.h-1;
	coins[1].x = coins[2].x; coins[1].y = coins[0].y;
	coins[3].x = coins[0].x; coins[3].y = coins[2].y;
	repBackChange(coins+0,Tile);
	repBackChange(coins+1,Tile);
	repBackChange(coins+2,Tile);
	repBackChange(coins+3,Tile);
	nx.ymin = nx.ymax = coins[0].y;
	nx.xmin = nx.xmax = coins[0].x;
	for (i=1;i<4;i++) {
	    if (coins[i].y<nx.ymin) nx.ymin = coins[i].y; 
		if (coins[i].y>nx.ymax) nx.ymax = coins[i].y;
	    if (coins[i].x<nx.xmin) nx.xmin = coins[i].x; 
		if (coins[i].x>nx.xmax) nx.xmax = coins[i].x;
	}
	if (nx.ymin<0) nx.ymin = 0;
	if (nx.ymax>that->Setting.Dim.Lines-1) nx.ymax = that->Setting.Dim.Lines-1;
	nx.dy = 1+nx.ymax-nx.ymin;
	if ((nx.dy>0)&&(nx.xmax>=0)&&(nx.xmin<that->Setting.Dim.Columns)) {
		int y,xmin,xmax,end;
		unsigned char *oldAttr,*amsk,*eamsk,*txt,*attr,*b;
		LineDesc *Y;
		lineIsBgPtr pY;
		Geo2dPoint Pos;
		y = nx.ymin;
		end = (0!=0);
		amsk = that->Setting.Data.AttrMsk+that->Setting.Data.DataWidth;
		eamsk = amsk+that->Setting.Data.AttrWidth;
		oldAttr = 0;
		while ((y<=nx.ymax)&&(!end)) {
            mnmxLineCut(&xmin,&xmax,Tile,&clip,y);
			if ((xmin<=xmax)&&(xmin<that->Setting.Dim.Columns)&&(xmax>=0)) {
				if (xmax>=that->Setting.Dim.Columns) xmax=that->Setting.Dim.Columns-1;
				if (xmin<0) xmin = 0;
		        Pos.x = xmin;
			    Pos.y = y;
                repChange(&Pos,Tile);
				Y = pgGetLine(&that->Pg,y);
				ibgp_Set(pY,*Y,xmin);
				b = Y->end-that->Pg.LineWidth;
				txt = b+(that->Pg.FieldWidth*xmin);
				if (!oldAttr) {
					oldAttr = txt+that->Setting.Data.DataWidth;
					end = Call(Perform,AttrSet,1(oldAttr));
				}
				while ((!end)&&(xmin<=xmax)) {
					attr = txt+that->Setting.Data.DataWidth;
					if (!ibgp_Val(pY)) {
				    if (checkAttr(oldAttr,attr,amsk,eamsk)) {
					    end = Call(Perform,AttrSet,1(attr));
					    oldAttr = attr;
				    }
					if (!end) {
						end = Call(Perform,Data,3(Pos.x,Pos.y,txt));
					}
					}
					txt+= that->Pg.FieldWidth; ibgp_Inc(pY);
					Pos.x += Tile->dx.x;
					Pos.y += Tile->dx.y;
					xmin++;
				}
			}
			y++;
		}
	}
}
static void fixed2dFieldStreamData(Geo2dField *this,Geo2dPoint *From,Geo2dFieldStream *Perform) {
	int x,y;
	ThisToThat(fixed2dField,Geo2dField);
	x = From->x; trunk(x,0,that->Setting.Dim.Columns);
	y = From->y; trunk(y,0,that->Setting.Dim.Lines);
	if (x==that->Setting.Dim.Columns) { x = 0; y++; }
	if (y<that->Setting.Dim.Lines) {
		int end;
		unsigned char *oldAttr,*amsk,*eamsk;
	    LineDesc *Y;
		amsk = that->Setting.Data.AttrMsk+that->Setting.Data.DataWidth;
		eamsk = amsk+that->Setting.Data.AttrWidth;
	    Y = pgGetLine(&that->Pg,y);
        oldAttr = (Y->end-that->Pg.LineWidth) + (that->Pg.FieldWidth*x) + that->Setting.Data.DataWidth;
		end = Call(Perform,AttrSet,1(oldAttr));
	    while ((y<that->Setting.Dim.Lines)&&(!end)) {
			unsigned char *txt,*attr,*e;
			lineIsBgPtr pY;
            Y = pgGetLine(&that->Pg,y);
			e = Y->end;
            txt =  (e-that->Pg.LineWidth) + (that->Pg.FieldWidth*x);
			ibgp_Set(pY,*Y,x);
			while ((txt<e)&&(!end)) {
			    attr = txt + that->Setting.Data.DataWidth;
				if (checkAttr(oldAttr,attr,amsk,eamsk)) {
					end = Call(Perform,AttrSet,1(attr));
					oldAttr = attr;
				}
				if (!end) {
				    if (ibgp_Val(pY)) {
					    end = Call(Perform,Data,2(txt,C_(Txt,Paragraph.Mark,Blank)));
					} else {
					    end = Call(Perform,Data,2(txt,C_(Txt,Paragraph.Mark,Char)));
					}
				    txt += that->Pg.FieldWidth; ibgp_Inc(pY);
				}
			}
			if (!end) {
				if (LineDesc_Eol(*Y)) {
					end = Call(Perform,Data,2(0,C_(Txt,Paragraph.Mark,EndOfLine)));
				}
			}
		    y++; x=0;
        }
		if (!end) {
			Call(Perform,Data,2(0,C_(Txt,Paragraph.Mark,EndOfBuffer)));
		}
	}
}

static void fixed2dFieldGetCharNum(Geo2dField *this,Geo2dPoint *CursorPos,int *yLineStart,int *xCharNum) {
	ThisToThat(fixed2dField,Geo2dField);
    pgGetCharNum(&that->Pg,CursorPos->y,CursorPos->x,yLineStart,xCharNum);
}
static void fixed2dFieldGetCharPos(Geo2dField *this,Geo2dPoint *Pos,int yLine,int CharNum) {
	ThisToThat(fixed2dField,Geo2dField);
    pgGetCharPos(&that->Pg,&Pos->x,&Pos->y,yLine,CharNum);
}

Geo2dField *Geo2dFieldNew(int DataWidth,int AttrWidth,int Columns,int Lines,unsigned char *Blank) {
	fixed2dField *R;
	Geo2dField *r;
	static struct Geo2dField Static = {
		fixed2dFieldGetSetting,
		fixed2dFieldClear,
		fixed2dFieldSetTileAttr,
		fixed2dFieldGotoXY,
		fixed2dFieldAdd8BitData,
		fixed2dFieldAddRawData,
		fixed2dFieldAddSeparator,
		fixed2dFieldGetCursorPos,
		fixed2dFieldGetData,
		fixed2dFieldForEachInBox,
		fixed2dFieldStreamData,
		fixed2dFieldGetCharNum,
		fixed2dFieldGetCharPos
	};
	if ((Columns<=0)||(Lines<=0)||(DataWidth+AttrWidth<=0)) {
		r = Geo2dFieldNull();
	} else {
		rPush(R);
	    r = &R->Geo2dField;
	    R->Geo2dField.Static = &Static;
	    R->Setting.Static = &nullFieldSettingStatic;
		Geo2dFieldSettingInit(&R->Setting,Columns,Lines,Blank,Blank+DataWidth+AttrWidth,DataWidth);
		pgDescInit(&R->Pg,&R->Setting);
	}
	return r;
}

