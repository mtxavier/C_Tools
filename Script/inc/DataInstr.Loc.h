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

#ifndef _DataInstr_Loc_h_
#define _DataInstr_Loc_h_

ScrInstruction *ScrInstrLocalArray(ScrDataTgt *ArrayPtr,ScrDataExpr *Size);
ScrInstruction *ScrInstrSet(ScrDataTgt *Tgt,ScrDataExpr *Val);
ScrInstruction *ScrInstrSequence(ScrInstruction **Begin,ScrInstruction **End);
ScrInstruction *ScrInstrWhile(ScrDataExpr *Cond,ScrInstruction *Inst);
ScrInstruction *ScrInstrUntil(ScrInstruction *Inst,ScrDataExpr *Cond);
ScrInstruction *ScrInstrCond(ScrDataExpr *Cond,ScrInstruction *IfTrue);
ScrInstruction *ScrInstrAlternate(ScrDataExpr *Cond,ScrInstruction *ifTrue,ScrInstruction *Else);
ScrInstruction *ScrInstrInvoke(ScrDataTgt *Result,ScrDataExpr *Param,int Depth,ScrType *Local,ScrInstruction *Block);
ScrInstruction *ScrInstrSelect(ScrDataExpr *Select,ScrArray *Choices);

ScrDataExpr *ScrValProcedure(int Depth,ScrType *Prototype,ScrType *Local,ScrInstruction *Body);

#endif
