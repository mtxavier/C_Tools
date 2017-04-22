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

#ifndef _Rpg_Core_h_
#define _Rpg_Core_h_

#include <Tools.h>

/*_______________________________________________________________________
 |
 | RpgDice
 |_______________________________________________________________________
*/

typedef struct { struct RpgDice *Static; } RpgDice;
struct RpgDice {
    TlsRange (*Range)(RpgDice *this); /* As usual: b included, e exclude.*/
    void (*nThrow)(RpgDice *this,int *b,int *e);
};

RpgDice *RpgDiceConst(int val);
RpgDice *RpgUniformDiceNew(int b,int e);
RpgDice *RpgNDices(int n,RpgDice *base);
RpgDice *RpgDiceSum(RpgDice *A,...); /* 0 terminated */
RpgDice *RpgLDiceSum(RpgDice **b,RpgDice **e);
RpgDice *RpgDiceMod(RpgDice *d,int mod); /* allows for d2,d3,d5 dices... range in the [0..mod[ */

    /*-------*/

typedef struct { struct RpgDiceCollect *Static; } RpgDiceCollect;
typedef struct {
    struct RpgDiceVector *Static;
    RpgDice RpgDice;
} RpgDiceVector;
struct RpgDiceVector {
    RpgDiceCollect *(*Collection)(RpgDiceVector *this);
    void (*AddCmpnt)(RpgDiceVector *this,int Id,int nb);
    int (*CmpntPop)(RpgDiceVector *this,int Id);
    void (*Add)(RpgDiceVector *this,RpgDiceVector *A);
    void (*Scale)(RpgDiceVector *this,int n);
};
struct RpgDiceCollect {
    int (*CmpntNb)(RpgDiceCollect *this);
    RpgDice *(*Cmpnt)(RpgDiceCollect *this,int Id);
    RpgDiceVector *(*GetNDices)(RpgDiceCollect *this,int Id,int nb);
};

RpgDiceCollect *RpgDiceCollectNew(RpgDice *d0,...);

/*_______________________________________________
 |
 | RpgCoreVal
 |_______________________________________________
*/

typedef struct {
    TlsIntVal TlsIntVal;
    int Val;
} RpgCoreDirectVal;

TlsIntVal *RpgCoreDirectValInit(RpgCoreDirectVal *r,int val);
TlsIntVal *RpgCoreDirectValNew(int val);

/*_______________________________________________
 |
 | Objects, Attributes
 |_______________________________________________
*/

typedef struct { struct RpgStruct *Static;} RpgStruct;
typedef struct { struct RpgModListAccess *Static; } RpgModListAccess;
struct RpgModListAccess {
    TlsDyndList/*(TlsIntVal *)*/ *(*Access)(RpgModListAccess *this,RpgStruct *Base);
};

/*-----------------*/

typedef struct { struct RpgObject *Static; } RpgObject;
struct RpgObject {
    int (*GetAttr)(RpgObject *this,int Id);
};
extern RpgObject RpgObjectNull;

typedef struct {
    struct RpgMappedObject *Static;
} RpgMappedObject;
struct RpgMappedObject {
    RpgObject *(*GetObject)(RpgMappedObject *this);
    void (*SetVal)(RpgMappedObject *this,int Id,int Val);
};
RpgMappedObject *RpgMappedObjectMap(int bId,int eId,int *Vals);
RpgMappedObject *RpgMappedObjectNew(int bId,int eId);

/*----------------------------*/

typedef struct { struct RpgObjAttribute *Static; } RpgObjAttribute;
struct RpgObjAttribute {
    int (*Val)(RpgObjAttribute *this,RpgObject *base);
};
extern RpgObjAttribute RpgObjAttributeNull;

RpgObjAttribute *RpgObjDirectAttr(int Id);
RpgObjAttribute *RpgObjConstAttr(int Val);
RpgObjAttribute *RpgObjScaleAttr(int scale,int Id);
RpgObjAttribute *RpgObjDownscaleAttr(int a,int b,int Id);
RpgObjAttribute *RpgObjAttrCombine(int (*Combine)(int a,int b),int Ida,int Idb);
RpgObjAttribute *RpgObj3AttrCombine(int (*Combine)(int a,int b,int c),int Ida,int Idb,int Idc);
RpgObjAttribute *RpgObj4AttrCombine(int (*Combine)(int a,int b,int c,int d),int Ida,int Idb,int Idc,int Idd);

/*________________________________________
 |
 | Modifiers
 |________________________________________
*/

typedef struct { struct RpgModCtx *Static; } RpgModCtx;
typedef struct { struct RpgModifierCalc *Static; } RpgModifierCalc;
typedef struct { 
    struct RpgModifier *Static;
    int Type;
} RpgModifier;
struct RpgModifier {
    int (*Val)(RpgModifier *this,RpgModCtx *Ctx);
};
struct RpgModifierCalc {
    void (*Open)(RpgModifierCalc *this,RpgModCtx *Ctx);
    void (*AddModifier)(RpgModifierCalc *this,RpgModifier *Mod);
    void (*Calc)(RpgModifierCalc *this);
};
extern RpgModifier RpgModifierNull;
extern RpgModCtx RpgModCtxNull;

RpgModifier *RpgStraightModifier(int Type,int Val);

/*_____________________________________________________________________________
 |
 | Skills
 |_____________________________________________________________________________
*/

typedef struct {
    int *b,*e;
    int Base,DiscardLow,DiscardHigh,KeepNum,Adj;
    int Ordered[6];
} RpgSkillThrowCmpnt;
typedef struct { struct RpgSkillThrow *Static; }  RpgSkillThrow;
struct RpgSkillThrow {
    RpgSkillThrowCmpnt *(*Component)(RpgSkillThrow *this);
    int (*Val)(RpgSkillThrow *this);
};

typedef struct { struct RpgSkillPool *Static; } RpgSkillPool;
struct RpgSkillPool {
    void (*Init)(RpgSkillPool *this,int Attribute,int SkillLvl);
    void (*AddBonus)(RpgSkillPool *this,int nb);
    void (*AddSupport)(RpgSkillPool *this,int nb);
    void (*AddTool)(RpgSkillPool *this,int nb);
    RpgSkillThrow *(*Throw)(RpgSkillPool *this);
};

int RpgSkillPoolThrow(RpgSkillPool *Pool);

RpgSkillPool *RpgSkillPoolNew(void);

/*______________________________________________
 |
 | Active Objects
 |______________________________________________
*/

typedef struct {
    struct RpgActiveObj *Static;
} RpgActiveObj;
struct RpgActiveObj {
    int (*GetAttr)(RpgActiveObj *this,int AttrId,RpgModCtx *Ctxt);
    RpgSkillPool *(*GetSkill)(RpgActiveObj *this,
        int SkillId,int AffId,RpgModCtx *Ctxt
    );
};

/*______________________________________________
 |
 | Templates
 |______________________________________________
*/

typedef struct { struct RpgRemoveHandle *Static; } RpgRemoveHandle;
typedef struct { struct RpgObjTemplate *Static; } RpgObjTemplate;

struct RpgRemoveHandle {
    void (*Remove)(RpgRemoveHandle *this);
};
struct RpgObjTemplate {
    void (*AllBonus)(RpgObjTemplate *this,
        int (*Bonus)(int StatId,RpgModifier *Value,void *clos),void *clos
    );
    void (*StatBonus)(RpgObjTemplate *this,int StatId,
	int (*Bonus)(RpgModifier *Value,void *clos),void *clos
    );
    RpgRemoveHandle *(*AddModifier)(RpgObjTemplate *this,
        int StatId,RpgModifier *Mod
    );
    void (*Clear)(RpgObjTemplate *this);
};

RpgObjTemplate *RpgObjTemplateNew(TlsMemPool *Pool);

/*______________________________________________
 |
 | Category
 |______________________________________________
*/

typedef struct { struct RpgCategory *Static; } RpgCategory;
struct RpgCategory {
    int (*IsA)(RpgCategory *this,RpgCategory *Super);
    RpgCategory *(*Core)(RpgCategory *this);
    int (*GetDepth)(RpgCategory *this);
    // returns the nature of the relation
    RpgCategory *(*IsTiedTo)(RpgCategory *this,RpgCategory *Other); 
};

/*_______________________________________________
 |
 |
 |_______________________________________________
*/

typedef struct { struct RpgSerial *Static; } RpgSerial;
typedef struct { struct RpgModule *Static; } RpgModule;
struct RpgModule {
    void (*Save)(RpgModule *this,RpgSerial *tgt);
};

/*_______________________________________________
 |
 |
 |_______________________________________________
*/

typedef struct { struct RpgCoreAbility *Static; } RpgCoreAbility;
struct RpgCoreAbility {
    char *(*Id)(RpgCoreAbility *this);
    TlsRange (*Range)(RpgCoreAbility *this);
    int (*Cost)(RpgCoreAbility *this,int lvl);
};

typedef struct { struct RpgCoreSkill *Static; } RpgCoreSkill;
struct RpgCoreSkill {
    char *(*Id)(RpgCoreSkill *this);
    TlsRange (*Range)(RpgCoreSkill *this);
    int (*Cost)(RpgCoreSkill *this,int Rank);
};

/*------*/

typedef struct { 
    struct RpgCoreSystem *Static;
    RpgModule RpgModule;
} RpgCoreSystem;

typedef struct {
    struct RpgCoreUniverse *Static;
    RpgModule RpgModule;
    RpgCoreSystem *System;
} RpgCoreUniverse;

typedef struct {
    struct RpgCoreWorld *Static;
    RpgModule RpgModule;
    RpgCoreSystem *System;
    RpgCoreUniverse *Universe;
} RpgCoreWorld;

typedef struct {
    struct RpgCorePlay *Static;
    RpgModule RpgModule;
    RpgCoreSystem *System;
    RpgCoreUniverse *Universe;
    RpgCoreWorld *World;
} RpgCorePlay;

struct RpgCoreSystem {
    RpgCoreUniverse *(*NewUniverse)(RpgCoreSystem *this,char *Seed);
    RpgCoreAbility *(*GetAbility)(RpgCoreSystem *this,char *Id);
    RpgCoreSkill *(*GetSkill)(RpgCoreSystem *this,char *Id);
};
struct RpgCoreUniverse {
    RpgCoreWorld *(*NewWorld)(RpgCoreUniverse *this,char *Seed);
};


#endif
