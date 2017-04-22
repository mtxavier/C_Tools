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

#ifndef _Builder_Loc_h_
#define _Builder_Loc_h_

/*______________________
 |
 | Type Builder
 |______________________
*/

typedef struct { struct ScrTypeStructBuilder *Static;} ScrTypeStructBuilder;
extern ScrTypeStructBuilder ScrTypeStructBuilderNull;
struct ScrTypeStructBuilder {
	// Partial Type should only be used for local calculation; Possibly as a child of ScrTypeThrou.
	ScrType *(*Partial)(ScrTypeStructBuilder *this); 
	int (*AddField)(ScrTypeStructBuilder *this,int Depth,char *Label,ScrType *EltType);
	ScrType *(*Close)(ScrTypeStructBuilder *this,ScrTypeBrowser *Dst,char *Label);
};

ScrTypeStructBuilder *ScrTypeStructNew(void);
ScrTypeStructBuilder *ScrTypeScopeNew(void);

/*______________________
 |
 | Expression Builder
 |______________________
*/

typedef struct {struct ScrExprScopeBuilder *Static;} ScrExprScopeBuilder;
struct ScrExprScopeBuilder {
	ScrType *(*GetType)(ScrExprScopeBuilder *this);
	int (*SetField)(ScrExprScopeBuilder *this,int Idx,ScrDataExpr *Data);
	int (*AddPrivate)(ScrExprScopeBuilder *this,ScrType *Type,char *Label);
	ScrType *(*GetPrivate)(ScrExprScopeBuilder *this,int *num,char *Label); // if (!Label), *num is taken instead.
    int (*SetPrivate)(ScrExprScopeBuilder *this,int Num,ScrDataExpr *Data);
	ScrDataExpr *(*Close)(ScrExprScopeBuilder *this,ScrExprBrowser *Tgt);
};
ScrExprScopeBuilder *ScrExprScopeBuild(ScrType *Type);
ScrExprScopeBuilder *ScrExprStructBuild(ScrType *Type,int Llvl);
ScrExprScopeBuilder *ScrExprFunctionBuild(ScrType *Type,int Llvl);

ScrExprScopeBuilder *ScrHiddenExprScopeBuilder(
		int ModuleNum,ScrTypeBrowser *TypeDst,ScrExprBrowser *ExprTgt,ScrTypeThrou *Type,ScrVocabulary *Voc);

#endif
