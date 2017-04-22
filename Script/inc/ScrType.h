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

#ifndef _ScrType_h_
#define _ScrType_h_

/*______________________________
 |
 | Base
 |______________________________
*/

typedef enum {
	ScbVoid,ScbPtr,ScbStruct,ScbArray,ScbFn,ScbList,ScbLabeled,ScbInt,ScbString,ScbScope,
	ScbPath,ScbChar,ScbShort,ScbLong,ScbFloat,ScbDouble
} ScrTypeBase;

/*______________________________
 |
 | Browser.
 |______________________________
*/

typedef struct { struct ScrTypeBrowser *Static; } ScrTypeBrowser;
extern ScrTypeBrowser ScrTypeBrowserNull;

/*______________________________
 |
 |
 |______________________________
*/

typedef struct { struct ScrType *Static;} ScrType;
extern ScrType ScrTypeNull;
struct ScrType {
	void (*Browse)(ScrType *this,ScrTypeBrowser *Tgt);
	ScrTypeBase (*Base)(ScrType *this);
	ScrType *(*GetPtr)(ScrType *this);
	struct {
		ScrType *(*Target)(ScrType *this);
	} Ptr;
	struct {
		int (*EltNb)(ScrType *this);
		ScrType *(*Elt)(ScrType *this,char **Label,int Idx);
		int (*EltNum)(ScrType *this,char *Label);
	} Struct;
	struct {
		ScrType *(*Result)(ScrType *this);
		ScrType *(*Param)(ScrType *this); // void, or normally a struct
	} Fn;
	struct {
		int (*EltNb)(ScrType *this); // negative if unspecified
		ScrType *(*EltType)(ScrType *this);
	} Array;
	struct {
		ScrType *(*EltType)(ScrType *this);
	} List;
	struct {
		char *(*Label)(ScrType *this);
		ScrType *(*Base)(ScrType *this);  // Also count for Struct static part.
	} Label;
};

typedef struct {ScrType ScrType,*Child;} ScrTypeThrou;

ScrTypeBase ScrTypeActualBase(ScrType *this);

  /*------*/

typedef struct { struct ScrTypeStructBrowser *Static; } ScrTypeStructBrowser;
extern ScrTypeStructBrowser ScrTypeStructBrowserNull;
struct ScrTypeStructBrowser {
	void (*Add)(ScrTypeStructBrowser *this,int num,char *Label,ScrType *Type);
	ScrType *(*End)(ScrTypeStructBrowser *this);

	void (*Map)(ScrTypeStructBrowser *this,int offset,int num,char *Label,ScrType *Type);
	ScrType *(*MapEnd)(ScrTypeStructBrowser *this,char *Label,int align,int Size);
};
struct ScrTypeBrowser {
	ScrType *(*Atom)(ScrTypeBrowser *this,ScrTypeBase Atom);
    ScrTypeStructBrowser *(*Struct)(ScrTypeBrowser *this,char *Label,ScrType *Base,int FieldNb);
	ScrType *(*Fn)(ScrTypeBrowser *this,ScrType *Result,ScrType *Params);
	ScrType *(*Ptr)(ScrTypeBrowser *this,ScrType *On);
	ScrType *(*Array)(ScrTypeBrowser *this,int EltNb,ScrType *Of);
	ScrType *(*List)(ScrTypeBrowser *this,ScrType *Of);
	ScrType *(*Labeled)(ScrTypeBrowser *this,char *Label,ScrType *Actual);

	ScrTypeThrou *(*Throu)(ScrTypeBrowser *this);
};

ScrTypeBrowser *ScrRawType(void);

#endif
