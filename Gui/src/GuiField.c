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
#include <Gui.local.h>
#include <GuiText.h>


/*-----------------------------------------------*/

/*
static void RepTileTranform(Geo2dPoint *a,Geo2dTileExtent *tl) {
	int x,y;
	x = a->x*tl->dx.x + a->y*tl->dy.x;
	y = a->x*tl->dx.y + a->y*tl->dy.y;
	a->x = x;
	a->y = y;
}
void FieldGuiWindowExtent(Geo2dRectangle *R,FieldGuiWindow *wnd) {
	Geo2dFieldSetting *fs;
	int xmin,xmax,ymin,ymax,i;
	Geo2dPoint coins[4];
	rOpen
	fs = Call(wnd->Field,GetSetting,0);
	coins[0].x = coins[0].y = 0;
	coins[2].x = fs->Dim.Columns-1;
	coins[2].y = fs->Dim.Lines-1;
	coins[1].x = coins[2].x; coins[1].y = coins[0].y;
	coins[3].x = coins[0].x; coins[3].y = coins[2].y;
	RepTileTransform(coins+0,&wnd->Show.Tile);
	RepTileTransform(coins+1,&wnd->Show.Tile);
	RepTileTransform(coins+2,&wnd->Show.Tile);
	RepTileTransform(coins+3,&wnd->Show.Tile);
	xmin = xmax = coins[0].x; ymin = ymax = coins[0].y;
	for (i=1;i<4;i++) {
		if (coins[i].x<xmin) xmin = coins[i].x;
		if (coins[i].x>xmax) xmax = coins[i].x;
		if (coins[i].y<ymin) ymin = coins[i].y;
		if (coins[i].y>ymax) ymax = coins[i].y;
	}
	R->Extent.w = (xmax-xmin)+1+wnd->Show.Tile.Extent.w;
	R->Extent.h = (ymax-ymin)+1+wnd->Show.Tile.Extent.h;
	R->Pos.x = xmin;
	R->Pos.y = ymin;
	rClose
}
*/

typedef struct {
	GuiOutput GuiOutput;
	Geo2dField *Field;
	GuiFieldStyle *Style;
	Geo2dTileExtent Tile;
	GuiFieldStyleAttrDesc AttrDesc;
} FieldGuiWindow;

typedef struct {
	Geo2dFieldPeek Geo2dFieldPeek;
	FieldGuiWindow *Window;
	int DataWidth;
	Geo2dPoint Org;
	GuiFieldStyleOutCtx *Style;
	Render2dEngine *Engine;
} FGWPeek;

static int FGWPeekAttrSet(Geo2dFieldPeek *this,unsigned char *Attr) {
	ThisToThat(FGWPeek,Geo2dFieldPeek);
	Call(that->Style,GetAttr,3(&(that->Engine->Front),&(that->Engine->Back),Attr));
    Call(that->Engine,ColorUpdateFront,0);
	Call(that->Engine,ColorUpdateBack,0);
	return (0!=0);
}
static int FGWPeekDisplay(Geo2dFieldPeek *this,int x,int y,unsigned char *Datab) {
	int data,shft,i;
	GuiOutput *til;
	ThisToThat(FGWPeek,Geo2dFieldPeek);
    data = *Datab; shft = 8;
	while (shft<that->DataWidth) { Datab++; data+=(*Datab)<<shft; shft+=8; }
	til = Call(that->Style,GetTile,1(data));
	that->Engine->Cursor.x = that->Org.x + x;
	that->Engine->Cursor.y = that->Org.y + y;
	Call(til,Display,1(that->Engine));
    return (0!=0);
}

static void FGWDisplay(GuiOutput *this,Render2dEngine *Engine) {
	static struct Geo2dFieldPeek Static = {&FGWPeekAttrSet,&FGWPeekDisplay};
	FGWPeek peek;
	Geo2dFieldSetting *fs;
	Geo2dPoint sCurs;
	Geo2dRectangle Window;
	GuiRGBA sFront,sBack;
	ThisToThat(FieldGuiWindow,GuiOutput);
	rOpen
	peek.Geo2dFieldPeek.Static = &Static;
	sCurs = Engine->Cursor;
	sFront = Engine->Front;
	sBack = Engine->Back;
	fs = Call(that->Field,GetSetting,0);
	peek.Window = that;
	peek.Engine = Engine;
	peek.DataWidth = fs->Data.DataWidth*8;
	peek.Style = Call(that->Style,OpenOutCtx,1(&that->AttrDesc));
	Window.Pos.x = Engine->Clip.Pos.x-Engine->Cursor.x;
	Window.Pos.y = Engine->Clip.Pos.y-Engine->Cursor.y;
	Window.Extent = Engine->Clip.Extent;
	peek.Org = sCurs;
	Call(that->Field,ForEachInBox,3(&Window,&that->Tile,&peek.Geo2dFieldPeek));
	Call(peek.Style,Close,0);
	Engine->Back = sBack;
	Engine->Front = sFront;
	Engine->Cursor = sCurs;
	rClose
}

GuiOutput *GuiFieldWindow(Geo2dField *Field,GuiFieldStyle *Style,GuiFieldStyleAttrDesc *ad,Geo2dTileExtent *Tile) {
	static struct GuiOutput Static = {FGWDisplay};
	FieldGuiWindow *R;
	Geo2dFieldSetting *fs;
	rPush(R);
	R->GuiOutput.Static = &Static;
	R->Field = Field;
	R->Style = Style;
	R->Tile = *Tile;
	R->AttrDesc = *ad;
	rOpen
	fs = Call(Field,GetSetting,0);
	if (fs->Data.AttrWidth<R->AttrDesc.AttrByte) { R->AttrDesc.AttrByte = fs->Data.AttrWidth; }
	rClose
	return &R->GuiOutput;
}

