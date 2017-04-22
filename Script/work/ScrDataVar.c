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
#include <ScrDataExpr.h>

/*_________________________
 |
 | Definitions
 |_________________________
*/

extern ScrPath ScrPathHere;
extern ScrPath ScrPathBack;
ScrPath *ScrPathStart(ScrVar *Tgt); // Might be used as ptr
ScrPath *ScrPathSideStep(int Steps);
ScrPath *ScrPathStepSelect(int Step);
ScrPath *ScrPathComb(ScrPath *a,ScrPath *b);
ScrPath *ScrPathChainStep(ScrPath **b,ScrPath **e);
ScrPath *ScrVarToVarPath(ScrVar *Dst,ScrVar *Src);

    /*_____________________
	 |
	 | Path
	 |_____________________
	*/



static ScrVar *HereFollow(ScrPath *this,ScrVar *Base) { return Base;}
static struct ScrPath HereStatic = {HereFollow};
ScrPath ScrPathHere = {&HereStatic};

     /*-------*/

static ScrVar *PreviousFollow(ScrPath *this,ScrVar *Base) {
	ScrVar *b;
	int idx;
	Call(Base,Address,2(&b,&idx))
	return b;
}
static struct ScrPath PreviousStatic = {PreviousFollow};
ScrPath ScrPathPrevious = {&PreviousStatic};


     /*------*/

typedef struct {
	ScrPath ScrPath;
	int Steps;
} PathStep;
static ScrVar *SideStepFollow(ScrPath *this,ScrVar *Base) {
	ScrVar *b;
	int idx;
	ThisToThat(PathStep,ScrPath);
    Call(Base,Address,2(&b,&idx));
	return Call(b,Elt,1(idx+that->Steps));
}
ScrPath *ScrPathSideStep(int Steps) {
	PathStep *r;
	static struct ScrPath Static = {SideStepFollow};
	rPush(r); r->ScrPath.Static = &Static; r->Steps = Steps;
	return &r->ScrPath;
}

    /*--------*/

static ScrVar *SelectFollow(ScrPath *this,ScrVar *Base) {
	ThisToThat(PathStep,ScrPath);
	return Call(Base,Elt,1(that->Steps));
}
ScrPath *ScrPathStepSelect(int Step) {
	PathStep *r;
	static struct ScrPath Static = {SelectFollow};
	rPush(r); r->ScrPath.Static = &Static; r->Steps = Step;
	return &r->ScrPath;
}
   
    /*---------*/

typedef struct {
	ScrPath ScrPath;
	ScrPath *a,*b;
} PathComb;
static ScrVar *PathCombFollow(ScrPath *this,ScrVar *Base) {
	ScrVar *r;
	ThisToThat(PathComb,ScrPath);
	r = Call(that->a,Follow,1(Base));
	r = Call(that->b,Follow,1(r));
	return r;
}
ScrPath *ScrPathComb(ScrPath *a,ScrPath *b) {
	PathComb *r;
	static struct ScrPath Static = {PathCombFollow};
	rPush(r); r->ScrPath.Static = &Static;
	r->a = a; r->b = b;
	return &r->ScrPath;
}

    /*---------*/

typedef struct {
	ScrPath ScrPath;
	ScrPath **b,**e;
} PathChain;
static ScrVar *PathChainFollow(ScrPath *this,ScrVar *Base) {
	ScrPath **p;
	ThisToThat(PathChain,ScrPath);
	p = that->b;
	while (p<that->e){
		Base = Call(*p,Follow,1(Base));
		p++;
	}
	return Base;
}
ScrPath *ScrPathChainStep(ScrPath **b,ScrPath **e) {
	PathChain *r;
	static struct ScrPath Static = {PathChainFollow};
	rPush(r); r->ScrPath.Static = &Static; 
	r->b = b; r->e = e;
	return &r->ScrPath;
}

    /*--------*/

ScrPath *ScrVarToVarPath(ScrVar *Dst,ScrVar *Src) {
	int found;
	SrcPath *r;
	r = &ScrPathHere;
	found = (Src==Dst);
	if (!found) {
	    BinTree/*<PathRecord.Elt>*/ nods;
	    BinTreeInit(&nods);
        #define idx(ptr) (((void *)ptr)-((void *)0))
		#define vidx(i)  (ScrVar *)(((void *)0)+i)
	    pOpen {
			int end,Idx,i;
			BinTree *f,*ne;
		    ScrVar *common,*p,*n;
            n = src;
			do {
				p = n;
				pPush(ne);
				BinTreeSeekOrInsert(&nods,idx(p),ne);
                Call(p,Address,2(&n,&i));
			} while (p!=n);
			n = dst; common = 0;
			do {
                p = n;
				f = BinTreeSeek(&nods,idx(p));
				if (f==BinTreeEnd) {
					pPush(ne);
					BinTreeSeekOrInsert(&nods,idx(p),ne);
					n = Call(p,Address,2(&n,&i));
				} else {
					common = p;
				}
			} while ((p!=n) && (!common))
		} pClose
		if (common) {
			int count;
			ScrPath **R,**q;
			ScrVar *p,*n;
			count = 0;
			p = src; while (p!=common) { int i; count++; p=Call(p,Address,2(&p,&i));}
			p = dst; while (p!=common) { int i; count++; p=Call(p,Address,2(&p,&i));}
			rnPush(R,count);
			q = R+count; I=Dst;
			while (I!=common) {
                ScrVar *b; int i;
				q--; Call(I,Address,2(&b,&i));
				*q = ScrPathStepSelect(i);
				I = b;
			}
			while (q>R) { q--; *q=&ScrPathPrevious; }
            r = ScrPathChainStep(R,R+count);
		}
        #undef idx
        #undef vidx
	}
	return r;
}

    /*_____________________
	 |
	 | Var
	 |_____________________
	*/

typedef struct {
	ScrVar ScrVar;
	ScrDataVal **Val;
	ScrDataVar *Base;
	int Idx;
} VarCell;

static ScrDataVal *VarCellValue(ScrVar *this) {
	ThisToThat(VarCell,ScrVar);
	return *that->Val;
}
static ScrVar *VarCellElt(ScrVar *this,int idx) { return &ScrVarNull; }
static void VarCellAddress(ScrVar *this,ScrVar **Base,int *Idx) {
	*Base = that->Base; *Idx = that->Idx;
}
static void VarCellSet(ScrVar *this,ScrDataVal *Value) {
	ThisToThat(VarCell,ScrVar);
	*that->Val = Value;
}
static ScrVar *VarCellNew(ScrDataVar *Base,int Idx,ScrDataVal **Val) {
	VarCell *r;
	static struct ScrVar Static = {
		VarCellValue,VarCellElt,VarCellAddress,VarCellSet
	};
	rPush(r); r->ScrVar.Static = &Static;
	r->Base = Base; r->Idx = Idx; r->Val = Val;
	return &r->ScrVar;
}

/*------------------*/

typedef struct {
	ScrVar ScrVar;
	ScrDataVal ScrDataVal;
	ScrDataVal **Values,**ValueEnd;
	ScrType *Type;
	ScrVar *Base;
	int Idx;
} VarSpace;

static void VarSpaceBrowse(ScrDataVal *this,ScrDataBrowser *Tgt) { }
static ScrType *VarSpaceGetType(ScrDataVal *this) {
	ThisToThat(VarSpace,ScrDataVal);
	return that->Type;
}
static ScrDataVal *VarSpaceEval(ScrDataVal *this,ScrDataCtx *Ctx,ScrDataVal *Param) { return &ScrDataValNull; }
static int VarSpaceInt(ScrDataVal *this) { return 0;}
static ScrString *VarSpaceString(ScrDataVal *this) { return &ScrStringNull; }
static ScrArray *VarSpaceArray(ScrDataVal *this){ return &ScrArrayNull(&ScrDataValNull);}
static ScrDataVal *VarSpaceGetField(ScrDataVal *this,int Idx) {
	ThisToThat(VarSpace,ScrDataVal);
	return that->Values[Idx];
}

static ScrDataVal *VarSpaceValue(ScrVar *this) {
	ThisToThat(VarSpace,ScrVar);
    return &that->ScrDataVal;
}
static ScrVar *VarSpaceElt(ScrVar *this,int idx) {
	ThisToThat(VarSpace,ScrVar);
	return VarCellNew(this,idx,that->Values[idx]);
}
static void VarSpaceAddress(ScrVar *this,ScrVar **Base,int *Idx) {
	ThisToThat(VarSpace,ScrVar);
	*Base = that->Base; *Idx = that->Idx;
}
static void VarSpaceSet(ScrVar *this,ScrDataVal *Value) {
	ScrDataVal **p,**e;
	int n;
	ThisToThat(VarSpace,ScrVar);
	n = 0; p = that->Values; e=that->ValueEnd;
	while (p<e) {
		*p = Call(Value,GetField,1(n));
		p++; n++;
	}
}
static ScrVar *VarSpaceNew(ScrVar *Base,int Idx,ScrType *Type,int size) {
	VarSpace *r;
    static struct ScrDataVal ValStatic = {
		VarSpaceBrowse,VarSpaceGetType,VarSpaceEval,VarSpaceInt,VarSpaceString,VarSpaceArray,VarSpaceGetField
	};
	static struct ScrVar Static = {
		VarSpaceValue,VarSpaceElt,VarSpaceAddress,VarSpaceSet
	};
	rPush(r); r->ScrVar.Static = &Static; r->ScrDataVal.Static = &ValStatic;
	r->Base = Base; r->Idx = Idx; r->Type = Type;
	rnPush(r->Values,size);
    r->ValueEnd = r->Values+size;
	return &r->ScrVar;
}

