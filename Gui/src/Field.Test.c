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
#include <GeoField.h>
#include <stdio.h>

typedef struct {
	Geo2dFieldPeek Geo2dFieldPeek;
	Geo2dFieldStream Geo2dFieldStream;
	unsigned char *Buffer,*p,*e;
	int y;
} fieldDisplay;

static int fieldPeekAttr(Geo2dFieldPeek *this,unsigned char *Attr) { return (0!=0); }
static int fieldPeekData(Geo2dFieldPeek *this,int x,int y,unsigned char *Datab) {
	int end;
	ThisToThat(fieldDisplay,Geo2dFieldPeek);
	end = (*Datab==128);
	if (!end) {
		if (that->p==that->e) { 
			unsigned char *p; 
			that->p = that->Buffer; that->y = y; p=that->p; while (p!=that->e) *p++=' '; 
	    }
		if (that->y!=y) {
			unsigned char *p;
			*that->p=0;
			printf(that->Buffer); printf("\n");
			p = that->p = that->Buffer;
            while (p!=that->e) *p++=' ';
			that->y = y;
		}
		*that->p++ = *Datab;
	} else {
		that->e[-1] = 0;
		printf(that->Buffer);
	}
	return end;
}

static int fieldStreamAttr(Geo2dFieldStream *this,unsigned char *Attr) { return (0!=0); }
static int fieldStreamData(Geo2dFieldStream *this,unsigned char *Data,int Mark) {
	ThisToThat(fieldDisplay,Geo2dFieldStream);
	if (that->p>=that->e-1) { 
		unsigned char *p; 
		that->p=that->Buffer; p=that->p; while (p!=that->e) *p++=' ';
	}
    if (Mark==C_(Txt,Paragraph.Mark,Char)) {
		*that->p++ = *Data;
	} else {
		if (Mark==C_(Txt,Paragraph.Mark,EndOfLine)) {
			*that->p++=0;
			printf(that->Buffer);
			printf("[]\n");
			that->p = that->Buffer;
		} else {
		    *that->p++ = 'µ';
		}
	}
	if (that->p>=that->e-1) {
		*that->p = 0;
		printf(that->Buffer);
		printf("\n");
		that->p = that->Buffer;
		{ unsigned char *p; p=that->p; while (p!=that->e) *p++=' ';}
	}
	return (0!=0);
}

static fieldDisplay *fieldDisplayNew(int BufferSize) {
	static struct Geo2dFieldPeek peekStatic = {fieldPeekAttr,fieldPeekData};
	static struct Geo2dFieldStream streamStatic = {fieldStreamAttr,fieldStreamData};
	fieldDisplay *R;
    rPush(R);
	R->Geo2dFieldPeek.Static = &peekStatic;
	R->Geo2dFieldStream.Static = &streamStatic;
	rnPush(R->Buffer,BufferSize+4);
	R->p = R->e = R->Buffer+BufferSize;
	*R->p = 0;
	R->y = 0;
	return R;
}

void FieldPrint(Geo2dField *f,char *s) {
	char *p,*e;
	unsigned char espace[8];
	p = e = s;
	espace[0] = ' ';
	espace[1] = ' ';
	espace[2] = ' ';
	espace[3] = ' ';
	while (*e) {
		if (*e=='\n') {
			int nb;
			if (p!=e) { Call(f,AddData,3(1,p,e)); }
			nb = 0;
            while (*e=='\n') {e++; nb++;}
			Call(f,AddSeparator,2(C_(Txt,Paragraph.Mark,EndOfLine),nb));
			p = e;
		} else {
			if (*e=='\t') {
				if (p!=e) {Call(f,AddData,3(1,p,e));}
				Call(f,AddData,3(1,espace,espace+4));
				p=e+1;
			}
			e++;
		}
	}
	if (p!=e) {Call(f,AddData,3(1,p,e));}
}


main () {
	Geo2dField *TheField;
	Geo2dRectangle TheFieldArea;
	unsigned char Blank[4],End[4];
	unsigned char *TheString,*se;
	fieldDisplay *fd;
	EnvOpen(4096,4096);
	rOpen
		TheFieldArea.Pos.x = 0;
	    TheFieldArea.Pos.y = 0;
		TheFieldArea.Extent.w = 40;
		TheFieldArea.Extent.h = 25;
        fd = fieldDisplayNew(80);
		Blank[0] = '_';
		End[0] = '-';
        //TheString = "Hello World!";
        TheString = "Hello World! from a scroll text perspective";
		se = TheString; while (*se) se++;
		TheField = Geo2dFieldNew(1,0,TheFieldArea.Extent.w,TheFieldArea.Extent.h,Blank);
		Call(TheField,Clear,2(C_(Txt,ScopeMark,Top),C_(Txt,ScopeMark,Bottom)));
		Call(TheField,AddData,3(1,TheString,se));
		Call(TheField,AddSeparator,2(C_(Txt,Paragraph.Mark,Blank),6));
		Call(TheField,AddData,3(1,End,End+1));
		Call(TheField,AddSeparator,2(C_(Txt,Paragraph.Mark,EndOfLine),5));
		Call(TheField,AddData,3(1,End,End+1));
		printf("\n");
		fd->p = fd->e;
		Call(TheField,StreamData,2(&TheFieldArea.Pos,&fd->Geo2dFieldStream));
		*fd->p = 0; printf(fd->Buffer);
		printf("\n    /*----------------*/\n");
		// TheFieldArea.Pos.x = 0;
		// TheFieldArea.Extent.w = 25;
		// TheFieldArea.Extent.h = 12;
		fd->p = fd->e;
		Call(TheField,ForEachInBox,3(&TheFieldArea,&Geo2dTileExtent_Id,&fd->Geo2dFieldPeek));
		*fd->p = 0; printf(fd->Buffer);
		printf("\n");
		{
	        FILE *thefile;
			thefile = fopen("src/Field.Test.c","r");
			if (thefile) {
				int s;
				fd->Buffer[80]=0;
				do {
					s = fread(fd->Buffer,1,80,thefile);
					fd->Buffer[s]=0;
                    FieldPrint(TheField,fd->Buffer);
				} while (s>=80);
				fclose(thefile);
			}
		}
		printf("\n\n    /*----------------*/\n\n");
		TheFieldArea.Pos.x = 0;
		TheFieldArea.Extent.w = 40;
		fd->p = fd->e;
		Call(TheField,ForEachInBox,3(&TheFieldArea,&Geo2dTileExtent_Id,&fd->Geo2dFieldPeek));
		*fd->p = 0; printf(fd->Buffer);
		printf("\n");
    rClose
	EnvClose();
}

