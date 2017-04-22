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

#ifndef _ScrModule_h_
#define _ScrModule_h_

#include <ScrBuilder.h>

/*______________________________________________________
 |
 | Lexical scope is the base data structure for modules 
 | More precisely: modules makes the root level of 
 | Lexical scope.
 |______________________________________________________
*/

#include <ScrDataExpr.h>

    /*-------- Worst idea ever ----------*/

typedef struct { struct ScrDataAccess *Static; } ScrDataAccess;
extern ScrDataAccess ScrDataAccessNull;
struct ScrDataAccess {
	ScrType *(*GetType)(ScrDataAccess *this);
	ScrInstruction *(*SetValue)(ScrDataAccess *this,ScrDataExpr *Val);
	ScrDataExpr *(*GetValue)(ScrDataAccess *this);
	ScrDataTgt *(*GetVar)(ScrDataAccess *this);
};

    /*------------------*/

typedef struct { struct ScrScope *Static; } ScrScope;
extern ScrScope ScrScopeNull;
struct ScrScope {
	ScrVocabulary *(*GetVocabulary)(ScrScope *this);
	ScrTypeBrowser *(*TypeTgt)(ScrScope *this);
	ScrExprBrowser *(*ExprTgt)(ScrScope *this);
	   // useful if you want a scope to share its vocabulary with its children for instance.

	int (*AddType)(ScrScope *this,char *Label); // returns (0!=0) if Label is already there
	int (*SetType)(ScrScope *this,char *Label,ScrType *Type); // returns (0!=0) if type is not there or already set
	ScrType *(*GetType)(ScrScope *this,char *Label);

	int (*AddData)(ScrScope *this,char *Label,ScrType *Type);
    ScrDataAccess *(*AddVar)(ScrScope *this,char *Label,ScrType *Type);
	ScrDataAccess *(*GetData)(ScrScope *this,char *Label);
	ScrDataAccess *(*AddPrivate)(ScrScope *this,ScrType *Type);

	ScrScope *(*StructTypedefOpen)(ScrScope *this/*,ScrTypeStructBuilder *Bld*/);
	ScrScope *(*ExtendOpen)(ScrScope *this);
	ScrScope *(*ExprStructOpen)(ScrScope *this,ScrType *Type);
	ScrScope *(*ExprFnOpen)(ScrScope *this,ScrType *Type);

    ScrScope *(*ProcedureOpen)(ScrScope *this,ScrType *Type);

	ScrDataExpr *(*Close)(ScrScope *this,ScrInstruction *Block);
	ScrType *(*TypeClose)(ScrScope *this,char *Label);
};

/*-------------------------*/

typedef struct { struct ScrModuleDesc *Static; } ScrModuleDesc;
struct ScrModuleDesc {
	void (*Open)(ScrModuleDesc *this,ScrScope *Base);
	void (*Close)(ScrModuleDesc *this,ScrScope *Base);
};

typedef struct { struct ScrModuleParser *Static;} ScrModuleParser;
struct ScrModuleParser {
	ScrModuleDesc *(*Source)(ScrModuleParser *this,char *Name);
};
ScrModuleParser *ScrSetParserPath(char **Path,int dumpLog);

/*------------------------*/

typedef struct { struct ScrScopeBrowser *Static; } ScrScopeBrowser;
extern ScrScopeBrowser ScrScopeBrowserNull;

typedef struct {struct ScrLinkScope *Static;} ScrLinkScope;
struct ScrLinkScope {
	int (*InsertModule)(ScrLinkScope *this,char *Label,ScrModuleDesc *Desc,char **Dependencies);
	int (*GetNum)(ScrLinkScope *this,char *Label);
	ScrDataVal *(*GetModule)(ScrLinkScope *this,int Idx);
	void (*ReleaseModule)(ScrLinkScope *this,int Idx);
	void (*Browse)(ScrLinkScope *this,int num,ScrScopeBrowser *Tgt);
};
ScrLinkScope *ScrLinkScopeNew(ScrVocabulary *Voc);

/*--------------------------*/

struct ScrScopeBrowser {
	void (*Open)(ScrScopeBrowser *this,int TypeNb);
	void (*AddType)(ScrScopeBrowser *this,int num,char *Label,ScrType *Type);
	void (*AddExpr)(ScrScopeBrowser *this,ScrDataExpr *Expr);
	void (*Close)(ScrScopeBrowser *this);
};


#endif
