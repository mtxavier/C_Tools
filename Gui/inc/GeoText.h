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

#ifndef _GeoText_h_
#define _GeoText_h_

typedef struct { struct GeoTextLine *Static; } GeoTextLine;
typedef struct { struct GeoText *Static; } GeoText;

struct GeoTextLine {
	int (*Length)(GeoTextLine *this);
	char *(*GetPlainText)(GeoTextLine *this,char *r,int bPos,int ePos);
	char *(*GetAttrText)(GeoTextLine *this,char *r,int bPos,int ePos);
	// GetText return the end of filled buffer. r if nothing as been filled.
	void (*Insert)(GeoTextLine *this,char *Attribut,int Pos,char *b,char *e);
	void (*Remove)(GeoTextLine *this,int Pos,int Length);
	void (*AttrSet)(GeoTextLine *this,int bPos,int ePos,char *Mask,char *Attr);
};
extern GeoTextLine GeoTextLineNull;

struct GeoText {
    int (*LineNb)(GeoText *this);
	GeoTextLine *(*GetLine)(GeoText *this,int *Layout,int num); // Layout: Left, right, centered expanded.
	void (*Insert)(GeoText *this,int num,int nb,int Layout);
	void (*Remove)(GeoText *this,int num,int nb);
	void (*LayoutSet)(GeoText *this,int num,int nb,int Layout); 
};
extern GeoText GeoTextNull;

GeoText *GeoTextEdit(int AttrWidth,int CharWidth);

#endif
