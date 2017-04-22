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


/*__________________________________________________
 |
 | Renders2d engines are supposed to 
 | be for internal use only.
 | They are here to provide common interface, to 
 | enable easy port from one API to another.
 |__________________________________________________
*/

#ifndef _Gui_local_h_
#define _Gui_local_h_

#include <Gui.h>

typedef struct {
	struct Img2dLib *Static;
	int Available;
} Img2dLib;

struct Img2dLib {
	void (*Info)(Img2dLib *this,int *w,int *h,ImgFormat *format);
	void (*Display)(Img2dLib *this,Geo2dRectangle *img);
	int (*Open)(Img2dLib *this);
	void (*Close)(Img2dLib *this);
	void (*Free)(Img2dLib *this);
};

extern Img2dLib Img2dLibNull;

/*--------------------------*/

struct Gui2dEngine {
	struct Render2dEngine *Static;
	Geo2dPoint Cursor;
	Geo2dRectangle Clip;
	GuiRGBA Front,Back,ColorKey;
	int iColorKey;
	int AlphaEnabled;
	int ColorKeyEnabled;
	int LineWidth;
	int JoinStyle;
	int FillType;
	void *FillPattern;
	int ScreenOrgAtBottom,MaxImagePitch,AlgnImagePitch;
};
typedef struct Gui2dEngine Render2dEngine;
void Gui2dEngineDefaultInit(Render2dEngine *n,int w,int h);

struct Render2dEngine {
	Img2dLib *(*ImportImg2dLib)(Render2dEngine *this,ImgDesc *Img);
	void (*ColorUpdateFront)(Render2dEngine *this);
	void (*ColorUpdateBack)(Render2dEngine *this);
	void (*ColorUpdateKey)(Render2dEngine *this);
	void (*iColorUpdateKey)(Render2dEngine *this);
	void (*AlphaKeyEnabledUpdate)(Render2dEngine *this);
	void (*UpdateClip)(Render2dEngine *this);
	void (*SetStyle)(Render2dEngine *this,int Linewidth,int JointPattern,int FillType,void *FillPattern);
	void (*Point)(Render2dEngine *this);
	void (*Line)(Render2dEngine *this,int dx,int dy);
	// void (*Circle)(Render2dEngine *this,int r);
	void (*Shape)(Render2dEngine *this,Geo2dShape *Shape);
	void (*Box)(Render2dEngine *this,int w,int h);
	void (*ScreenClear)(Render2dEngine *this);
};

/*----------------------------------------*/

#endif

