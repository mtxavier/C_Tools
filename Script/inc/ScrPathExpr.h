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

#ifndef _ScrPathExpr_h_
#define _ScrPathExpr_h_

#include <ScrDataExpr.h>
#include <ScrModule.h>

typedef struct { struct ScrPathExpr *Static; } ScrPathExpr;
extern ScrPathExpr ScrPathExprNull;
struct ScrPathExpr {
	ScrType *(*GetType)(ScrPathExpr *this,int *IsLvalue);
	ScrDataTgt *(*Left)(ScrPathExpr *this,int *Success);
	ScrDataExpr *(*Right)(ScrPathExpr *this);
	/* The following are not strictly required, they might provide optimisation in some cases. */
	ScrPathExpr *(*Address)(ScrPathExpr *this);
	ScrPathExpr *(*Idrct)(ScrPathExpr *this);
	ScrInstruction *(*Set)(ScrPathExpr *this,ScrDataTgt *Tgt);
};
 
ScrPathExpr *ScrPathExprVar(ScrExprBrowser *Hld,ScrDataTgt *Tgt);
ScrPathExpr *ScrPathExprData(ScrExprBrowser *Hld,ScrDataExpr *Val);
ScrPathExpr *ScrPathExprAccess(ScrExprBrowser *Hld,ScrDataAccess *Acc);

ScrPathExpr *ScrPathExprField(ScrExprBrowser *Hld,ScrPathExpr *Base,int Id);
ScrPathExpr *ScrPathExprElt(ScrExprBrowser *Hld,ScrPathExpr *Base,ScrDataExpr *Id);
ScrPathExpr *ScrPathExprApply(ScrExprBrowser *Hld,ScrPathExpr *Base,ScrDataExpr *Param);
ScrPathExpr *ScrPathAddress(ScrExprBrowser *Hld,ScrPathExpr *Base);
ScrPathExpr *ScrPathIdrct(ScrExprBrowser *Hld,ScrPathExpr *Ptr);
ScrPathExpr *ScrPathPath(ScrExprBrowser *Hld,ScrType *Type,ScrPathExpr *End,ScrPathExpr *Org);

#endif
