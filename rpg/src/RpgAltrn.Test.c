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
#include <RpgAltrn.h>
#include <List.h>

/*________________________________________
 |
 |
 |________________________________________
*/

#define l_(b,c) c_(Altrn,b,c)

/*--------------------------------*/

typedef struct {
    struct AltrnFormulas *Static;
    RpgDice *d1,*d2,*d4,*d6,*d8,*d10,*d12,*d20,*d100;
    RpgDiceCollect *Dices;
} AltrnFormulas;
struct AltrnFormulas {
    RpgDice *(*BonusDice)(AltrnFormulas *this,int AttrLvl);
    int (*CplxAffinity)(AltrnFormulas *this,RpgObject *base,int Id);
    int (*AffinModStacking)(AltrnFormulas *this,int Id,int mod1,int mod2);
    int (*SkillModStacking)(AltrnFormulas *this,int Id,int mod1,int mod2);
    RpgModifierCalc *(*AffinCalc)(
        AltrnFormulas *this,int *Tgt,int Base
    );
    RpgModifierCalc *(*SkillCalc)(
	AltrnFormulas *this,RpgSkillPool *Tgt,int SkillBase,int AffVal
    );
    RpgModifierCalc *(*EffectCalc)(
        AltrnFormulas *this,RpgDiceVector *Tgt,int AffVal
    );
    RpgActiveObj *(*ActiveObjNew)(
        AltrnFormulas *this,RpgObject *Base,RpgObjTemplate *Mod
    );
};

    /*----------------*/

static RpgDice *AltrnBonusDice(AltrnFormulas *that,int AttrLvl) {
    RpgDice *r;
    if (AttrLvl<1) {
	r = RpgDiceConst(AttrLvl);
    } else {
	int n,rn;
	switch (AttrLvl) {
	case 1: r = that->d2; break;
	case 2: r = that->d4; break;
	case 3: r = that->d6; break;
	case 4: r = that->d8; break;
	case 5: r = that->d10; break;
	case 6: r = that->d12; break;
	case 7: r = RpgNDices(3,that->d4); break;
	case 8: r = RpgDiceSum(that->d6,RpgNDices(2,that->d4),0); break;
	case 9: r = RpgDiceSum(that->d4,RpgNDices(2,that->d6),0); break;
	case 10: r = RpgNDices(3,that->d6); break;
	case 11: r = RpgDiceSum(that->d8,RpgNDices(2,that->d6),0); break;
	case 12: r = RpgDiceSum(that->d6,RpgNDices(2,that->d8),0); break;
	case 13: r = RpgNDices(3,that->d8); break;
	case 14: r = RpgDiceSum(that->d10,RpgNDices(2,that->d8),0); break;
	case 15: r = RpgDiceSum(that->d8,RpgNDices(2,that->d10),0); break;
	case 16: r = RpgNDices(3,that->d10); break;
	case 17: r = RpgDiceSum(that->d12,RpgNDices(2,that->d10),0); break;
	case 18: r = RpgDiceSum(that->d10,RpgNDices(2,that->d12),0); break;
	case 19: r = RpgNDices(3,that->d12); break;
	default : {
	    rn = AttrLvl%13;
	    n = AttrLvl/13;
	    if (rn>=7) {
		r = RpgDiceSum(
		RpgNDices((2*(n-1)),that->d12),AltrnBonusDice(that,rn+13),0);
		}
	    }}
	}
	return r;
}

/*________________________________________
 |
 |
 |________________________________________
*/

typedef struct AltrnAffinityCalc {
    RpgModifierCalc RpgModifierCalc;
    RpgModCtx *Ctx;
    AltrnFormulas *Formulas;
    int Base,*Tgt;
    int *Terms;
} AltrnAffinityCalc;
#define T_(Id) that->Terms[l_(Modifier,Id)]
static void AACalcCalc(RpgModifierCalc *this) {
    int r;
    ThisToThat(AltrnAffinityCalc,RpgModifierCalc);
    r = T_(Base)+T_(Rank);
    if (r<T_(AltBase)) r = T_(AltBase);
    *(that->Tgt) = r+T_(Wound)+T_(Buff)+T_(Debuff);
}
static void AACalcOpen(
    RpgModifierCalc *this,RpgModCtx *Ctx
){
    int *p,*e;
    ThisToThat(AltrnAffinityCalc,RpgModifierCalc);
    that->Ctx = Ctx;
    p = that->Terms; e = p+l_(Modifier,nb);
    while (p<e) { *p++=0; }
    T_(AltBase) = T_(Base) = that->Base;
}
#undef T_
static void AAAddModifier(RpgModifierCalc *this,RpgModifier *Mod) {
    int v,ModId;
    ThisToThat(AltrnAffinityCalc,RpgModifierCalc);
    ModId = Mod->Type;
    if ((ModId>=0)&&(ModId<l_(Modifier,nb))) {
        v = Call(Mod,Val,1(that->Ctx));
	that->Terms[ModId] = Call(
            that->Formulas,AffinModStacking,3(ModId,that->Terms[ModId],v)
        );
    }
}

static RpgModifierCalc *RpgAttrCalcNew(
    AltrnFormulas *this,int *Val,int Base
) {
    AltrnAffinityCalc *r;
    int *p,*e;
    static struct RpgModifierCalc Static = {
        AACalcOpen,AAAddModifier,AACalcCalc
    };
    rPush(r);
    r->RpgModifierCalc.Static = &Static;
    r->Formulas = this;
    r->Tgt = Val;
    r->Base = Base;
    r->Ctx = &RpgModCtxNull;
    rnPush(r->Terms,l_(Modifier,nb));
    p = r->Terms; e = p+l_(Modifier,nb);
    while (p<e) { *p++ = 0; }
    return &r->RpgModifierCalc;
}

/*-------------------*/

typedef struct {
    RpgModifierCalc RpgModifierCalc;
    AltrnFormulas *Formulas;
    RpgSkillPool *Tgt;
    RpgModCtx *Ctx;
    int SkllBase,AffVal;
    int *Terms;
} ASkillCalc;
static void ASCalcOpen(RpgModifierCalc *this,RpgModCtx *Ctx) {
    int *p,*e;
    ThisToThat(ASkillCalc,RpgModifierCalc);
    that->Ctx = Ctx;
    p = that->Terms;
    e = p+l_(Modifier,nb);
    while (p<e) { *p++=0; }
}
static void ASAddModifier(RpgModifierCalc *this,RpgModifier *Mod) {
    int v,ModId;
    ThisToThat(ASkillCalc,RpgModifierCalc);
    ModId = Mod->Type;
    if ((ModId>=0)&&(ModId<l_(Modifier,nb))) {
        v = Call(Mod,Val,1(that->Ctx));
        that->Terms[ModId] = Call(
            that->Formulas,SkillModStacking,3(ModId,that->Terms[ModId],v)
        );
    }
}
static void ASCalc(RpgModifierCalc *this) {
    int Bonus,Support,Tool,*t;
    ThisToThat(ASkillCalc,RpgModifierCalc);
    t = that->Terms;
    #define T_(id) t[l_(Modifier,id)]
    Bonus = Support = Tool = 0;
    Bonus = T_(Buff)+T_(Environment)+T_(Expertise)+T_(Debuff);
    Tool = T_(Workshop)+T_(Tool)+T_(Wound)+T_(Requisit);
    Support = T_(Support)+T_(Opposition);
    Call(that->Tgt,Init,2(that->AffVal,that->SkllBase));
    Call(that->Tgt,AddBonus,1(Bonus));
    Call(that->Tgt,AddSupport,1(Support));
    Call(that->Tgt,AddTool,1(Tool));
    #undef T_
}
static RpgModifierCalc *RpgSkillCalcNew(AltrnFormulas *this,
    RpgSkillPool *Tgt,int SkllBase,int AffVal
) {
    static struct RpgModifierCalc Static = {
        ASCalcOpen,ASAddModifier,ASCalc
    };
    ASkillCalc *r;
    rPush(r);
    r->RpgModifierCalc.Static = &Static;
    r->Formulas = this;
    r->Tgt = Tgt;
    r->SkllBase = SkllBase;
    r->AffVal = AffVal;
    rnPush((r->Terms),l_(Modifier,nb));
    ASCalcOpen(&r->RpgModifierCalc,&RpgModCtxNull);
    return &r->RpgModifierCalc;
}

/*------------------------*/

typedef struct {
   RpgModifierCalc RpgModifierCalc;
   AltrnFormulas *Form;
   RpgDiceVector *Tgt;
   RpgModCtx *Ctx;
   int AffVal,nb;
} EffectCalc;
static void EffectCalcOpen(RpgModifierCalc *this,RpgModCtx *Ctx) {
   ThisToThat(EffectCalc,RpgModifierCalc);
   that->Ctx=Ctx;
   Call(that->Tgt,Scale,1(0));
}
static void EffectCalcAddModifier(RpgModifierCalc *this,RpgModifier *Mod) {
    int d,v;
    ThisToThat(EffectCalc,RpgModifierCalc);
    d = Mod->Type;
    if ((d>0)&&(d<that->nb)) {
        v=Call(Mod,Val,1(that->Ctx));
        Call(that->Tgt,AddCmpnt,2(d,v));
    }
}
static void EffectCalcCalc(RpgModifierCalc *this) {
    ThisToThat(EffectCalc,RpgModifierCalc);
}
static RpgModifierCalc *RpgEffectCalc(AltrnFormulas *this,
    RpgDiceVector *Tgt,int AffVal
) {
   EffectCalc *r;
   struct RpgModifierCalc Static = {
       EffectCalcOpen,EffectCalcAddModifier,EffectCalcCalc
   };
   rPush(r);
   r->RpgModifierCalc.Static = &Static;
   r->Form = this;
   r->Tgt = Tgt;
   r->Ctx = &RpgModCtxNull;
   r->AffVal = AffVal;
   rOpen {
       RpgDiceCollect *Coll;
       Coll = Call(Tgt,Collection,0);
       r->nb = Call(Coll,CmpntNb,0);
   } rClose
   return &r->RpgModifierCalc;
}

/*________________________________________
 |
 |
 |________________________________________
*/

typedef struct {
    RpgActiveObj RpgActiveObj;
    RpgObject *Base;
    RpgObjTemplate *Mod;
    AltrnFormulas *Form;
} CompstObj;

static int CmpstObjGetAttr(RpgActiveObj *this,int AttrId,RpgModCtx *Ctx) {
    int Base,r;
    int Add(RpgModifier *mod,void *Clos) {
        RpgModifierCalc *clos;
        clos = Clos;
        Call(clos,AddModifier,1(mod));
        return (0!=0);
    }
    ThisToThat(CompstObj,RpgActiveObj);
    Base = Call(that->Base,GetAttr,1(AttrId));
    r = 0;
    rOpen
        RpgModifierCalc *clc;
        clc = Call(that->Form,AffinCalc,2(&r,Base));
        Call(clc,Open,1(Ctx));
        Call(that->Mod,StatBonus,3(AttrId,Add,clc));
        Call(clc,Calc,0);
    rClose
    return r;
}
static RpgSkillPool *CmpstObjGetSkill(
    RpgActiveObj *this,int SkillId,int AttrId,RpgModCtx *Ctx
) {
    RpgSkillPool *r;
    int BaseSkll,Aff;
    int Add(RpgModifier *mod,void *Clos) {
        RpgModifierCalc *clos;
        clos = Clos;
        Call(clos,AddModifier,1(mod));
        return (0!=0);
    }
    ThisToThat(CompstObj,RpgActiveObj);
    r = RpgSkillPoolNew();
    Aff = CmpstObjGetAttr(this,AttrId,Ctx);
    BaseSkll = Call(that->Base,GetAttr,1(SkillId));
    rOpen
        RpgModifierCalc *clc;
        clc = Call(that->Form,SkillCalc,3(r,BaseSkll,Aff));
        Call(clc,Open,1(Ctx));
        Call(that->Mod,StatBonus,3(SkillId,Add,clc));
        Call(clc,Calc,0);
    rClose
    return r;
}
static RpgActiveObj *AltrnActiveObjNew(
    AltrnFormulas *this,RpgObject *Base,RpgObjTemplate *Mod
) {
   CompstObj *r;
   static struct RpgActiveObj Static = {
       CmpstObjGetAttr,CmpstObjGetSkill
   };
   rPush(r);
   r->RpgActiveObj.Static = &Static;
   r->Base = Base;
   r->Mod = Mod;
   r->Form = this;
   return &r->RpgActiveObj;
}

/*________________________________________________________________
 |
 |
 |________________________________________________________________
*/

typedef int (*ModStacking)(int mod1,int mod2);
typedef struct {
    AltrnFormulas AltrnFormulas;
    RpgObjAttribute **bAffin,**eAffin;
    ModStacking *bModAffin;
    ModStacking *bModSkill;
} theFormulas;

static int theFormulasBaseCplxAff(AltrnFormulas *this,RpgObject *base,int Id) {
    int b,e,r;
    ThisToThat(theFormulas,AltrnFormulas);
    b = c_(Altrn,PC,Affinity);
    e = b+(that->eAffin-that->bAffin);
    if ((Id>=b)&&(Id<e)) {
        RpgObjAttribute *attr;
        attr = that->bAffin[Id-b];
        r = Call(attr,Val,1(base));
    } else {
        r = Call(base,GetAttr,1(Id));
    }
    return r;
}
static int theFormulasAffinStacking(AltrnFormulas *this,
    int Id,int mod1,int mod2
){
    int i,r;
    ThisToThat(theFormulas,AltrnFormulas);
    r = 0;
    if ((Id<0)||(Id>=l_(Modifier,nb))) {
        r = 0;
    } else {
        r = that->bModAffin[i](mod1,mod2);
    }
    return r;
}
static int theFormulasSkillStacking(AltrnFormulas *this,
    int Id,int mod1,int mod2
){
    int i,r;
    ThisToThat(theFormulas,AltrnFormulas);
    if ((Id<0)||(Id>=l_(Modifier,nb))) {
        r = 0;
    } else {
        r = that->bModSkill[i](mod1,mod2);
    }
    return r;
}

static int BaseCplxNull(RpgObject *base,int Id){ return 0; }
static int unAverage(int a,int b) { return ((a+b)+((a>b)?1:0))>>1; }
static int Average3(int a,int b,int c) { return (2+a+b+c)/3; }
static int StackingCumulate(int a,int b) { return a+b; }
static int StackingMax(int a,int b) { return (a>b)?a:b;}
static int StackingMin(int a,int b) { return (a>b)?b:a;}
static AltrnFormulas *theFormulasNew(void){
    theFormulas *r;
    AltrnFormulas *R;
    int i,nb;
    static struct AltrnFormulas Static = {
        AltrnBonusDice,
        theFormulasBaseCplxAff,theFormulasAffinStacking,
	theFormulasSkillStacking,RpgAttrCalcNew,RpgSkillCalcNew,
        RpgEffectCalc,AltrnActiveObjNew
    };
    rPush(r);
    r->AltrnFormulas.Static = &Static;
    /*------*/
    R = &r->AltrnFormulas;
    R->d1 = RpgDiceConst(1);
    R->d2 = RpgUniformDiceNew(1,3);
    R->d4 = RpgUniformDiceNew(1,5);
    R->d6 = RpgUniformDiceNew(1,7);
    R->d8 = RpgUniformDiceNew(1,9);
    R->d10 = RpgUniformDiceNew(1,11);
    R->d12 = RpgUniformDiceNew(1,13);
    R->d20 = RpgUniformDiceNew(1,21);
    R->d100 = RpgUniformDiceNew(1,101);
    R->Dices = RpgDiceCollectNew(
        R->d1,
	R->d2,R->d4,R->d6,R->d8,R->d10,R->d12,R->d20,R->d100,
	0
    );
	   /*-----*/
    nb = c_(Altrn,PC.Affinity,nb);
    rnPush(r->bAffin,nb);
    r->eAffin = r->bAffin+nb;
    for (i=0;i<nb;i++) {
        r->bAffin[i] = &RpgObjAttributeNull;
    }
    #define DefCplx(attr) r->bAffin[c_(Altrn,PC.Affinity,attr)]
	
    #define MapDirect(attr,cplx) \
        DefCplx(attr) = RpgObjDirectAttr(c_(Altrn,PC,Base.attr)); \
        DefCplx(attr) = RpgObjScaleAttr(-1,c_(Altrn,PC,Base.attr))

    #define Mix(A,B) \
        RpgObjAttrCombine(unAverage,c_(Altrn,PC,Affinity.A),\
            c_(Altrn,PC,Affinity.B))
    #define Mix3(A,B,C) \
        RpgObj3AttrCombine(Average3,c_(Altrn,PC,Affinity.A),\
            c_(Altrn,PC,Affinity.B),c_(Altrn,PC,Affinity.C))
    MapDirect(Balance,Inbalance);
    MapDirect(Blue,Body);
    MapDirect(Green,Mind);
    MapDirect(Red,Soul);
    MapDirect(Earth,Perception);
    MapDirect(Water,Sharp);
    MapDirect(Air,Resilience);
    MapDirect(Fire,Power);
    DefCplx(Metal) = Mix3(Earth,Sharpness,Resilience);
    DefCplx(Wood) = Mix3(Earth,Water,Power);
    DefCplx(Lightning) = Mix(Air,Fire);
    DefCplx(Ice) = Mix(Power,Water);
    DefCplx(Heat) = Mix(Fire,Sharpness);
    DefCplx(Magma) = Mix(Fire,Earth);
    DefCplx(Crystal) = Mix(Sharpness,Earth);
    DefCplx(Acid) = Mix(Fire,Resilience);
    DefCplx(Darkness) = Mix3(Air,Earth,Power);
    DefCplx(Light) = Mix3(Air,Fire,Perception);
    DefCplx(Force) = Mix(Blue,Power);
    DefCplx(Sound) = Mix(Air,Perception);
    DefCplx(Poison) = Mix3(Green,Fire,Resilience);
    DefCplx(Holy) = Mix3(Soul,Fire,Water);
    DefCplx(Curse) = Mix3(Red,Power,Water);
    DefCplx(Constitution) = Mix(Body,Resilience);
    DefCplx(Strength) = Mix(Body,Power);
    DefCplx(Reaction) = Mix(Body,Perception);
    DefCplx(Agility) = Mix(Body,Sharpness);
    DefCplx(Understanding) = Mix(Mind,Perception);
    DefCplx(Technicity) = Mix(Mind,Sharpness);
    DefCplx(Sanity) = Mix(Mind,Resilience);
    DefCplx(Memory) = Mix(Mind,Power);
    DefCplx(Intuition) = Mix(Soul,Perception);
    DefCplx(Charisma) = Mix(Soul,Sharpness);
    DefCplx(Determination) = Mix(Soul,Resilience);
    DefCplx(Creativity) = Mix(Soul,Power);
    DefCplx(Magic) = Mix3(Mind,Blue,Red);
    DefCplx(Ki) = Mix3(Body,Green,Red);
    DefCplx(Channel) = Mix3(Soul,Blue,Green);
    /* Modifier Calc */{
        int i,nb;
        ModStacking *Affin,*Skill;
        nb = l_(Modifier,nb);
        rnPush(r->bModAffin,nb);
        rnPush(r->bModSkill,nb);
        Affin = r->bModAffin;
        Skill = r->bModSkill;
        for (i=0;i<nb;i++) { Affin[i] = Skill[i] = &StackingMax; }
        #define Set2Mod(Id,Mod) \
             Affin[l_(Modifier,Id)]=Skill[l_(Modifier,Id)]=&Stacking##Mod
        Set2Mod(Rank,Cumulate);
        Set2Mod(Requisit,Cumulate);
        Set2Mod(Wound,Cumulate);
        Set2Mod(Debuff,Min);
        Set2Mod(Opposition,Min);
        Set2Mod(Vulnerability,Min);
        #undef Set2Mod
    }
    return &r->AltrnFormulas;
}

typedef struct {
    RpgObject RpgObject;
    RpgObject *Base;
    AltrnFormulas *Formulas;
} AltrnObjectBase2Cmplx;
static int Base2CmplxGetAffinity(RpgObject *this,int Id) {
    ThisToThat(AltrnObjectBase2Cmplx,RpgObject);
    return Call(that->Formulas,CplxAffinity,2(that->Base,Id));
}

/*________________________________________
 |
 |
 |________________________________________
*/

typedef struct {
    struct AltrnEquipment *Static;
} AltrnEquipment;

typedef struct {
    int Type;
    AltrnEquipment *Content;
} AltrnEquipSlot;

typedef struct {
    RpgActiveObj *RpgActiveObj;
    RpgObjTemplate *Race;
    RpgObjTemplate *Lvl;
    RpgObjTemplate *Equip;
} AltrnMonster;

typedef struct {
    int JobId,Pex;
    RpgObjTemplate **bLvl,**eLvl;
} AltrnPCJob;
typedef struct {
    int Pex,Lvl;
    AltrnMonster AltrnMonster;
    RpgObjTemplate *Prog;
    AltrnPCJob Job;
} AltrnPC;

/*________________________________________
 |
 |
 |________________________________________
*/

typedef struct {
    int v,max;
} RpgCoreJauge;

typedef struct {
    RpgCoreDirectVal v,max;
} RpgCoreValJauge;

typedef struct {
    AltrnFightActor AltrnFightActor;
    AltrnFightScene *Scene;
    char *Name;
    int Race;
    RpgCoreDirectVal AC,DR;
    RpgCoreValJauge Hp;
    struct {
        int Downed;
    } Status;
    struct {
        RpgCoreDirectVal skill,attr;
    } Atk,Def;
    struct {
        RpgCoreJauge Action,Move,Reaction;
    } Resource;
} FightActor;

static void FightActorNewRound(
    AltrnFightActor *this,AltrnFightScene *scene,int round
) {
    ThisToThat(FightActor,AltrnFightActor);
    that->Resource.Action.v = that->Resource.Action.max;
    that->Resource.Move.v = that->Resource.Move.max;
    that->Resource.Reaction.v = that->Resource.Reaction.max;
}
static void FightActorTakeDmg(AltrnFightActor *this,int Dmg) {
    RpgCoreJauge Hp;
    int KO;
    ThisToThat(FightActor,AltrnFightActor);
    Hp.v = that->Hp.v.Val;
    Hp.max = that->Hp.max.Val;
    KO = that->Status.Downed;
    if (!KO) {
        Hp.v -= Dmg;
    }
    if (Hp.v>Hp.max) {Hp.v=Hp.max;}
    KO = (KO||(Hp.v<=0));
    if (KO) {
        Hp.v = 0;
    }
    that->Hp.v.Val = Hp.v;
    that->Status.Downed = KO;
}
static int FightActorCheckStatus(AltrnFightActor *this,int Status) {
    int r;
    ThisToThat(FightActor,AltrnFightActor);
    r = (0!=0);
    if (Status == l_(Status,Downed)) {
        r = that->Status.Downed;
    }
    return r;
}
static void FightActorAct(AltrnFightActor *this,AltrnFightScene *scene) {
    ThisToThat(FightActor,AltrnFightActor);
}

static AltrnFightActor *FightActorNew(AltrnFightScene *Scene,int Id,int Faction,RpgObject *Object) {
    FightActor *r;
    static struct AltrnFightActor Static = {
    FightActorNewRound,FightActorTakeDmg,FightActorCheckStatus,FightActorAct
    };
    rPush(r);
    r->Scene = Scene;
    r->Name = "";
    r->Race = 0;
    r->AltrnFightActor.Id = Id;
    r->AltrnFightActor.Faction = Faction;
    r->AltrnFightActor.Atk.Attr = &r->Atk.attr.TlsIntVal;
    r->AltrnFightActor.Atk.Skill = &r->Atk.skill.TlsIntVal;
    r->AltrnFightActor.Def = &r->AC.TlsIntVal;
    r->AltrnFightActor.Hp = &r->Hp.v.TlsIntVal;
    RpgCoreDirectValInit(&r->AC,8+Call(Object,GetAttr,1(l_(PC,Affinity.Reaction))));
    RpgCoreDirectValInit(&r->DR,Call(Object,GetAttr,1(l_(PC,Body.DR))));
    RpgCoreDirectValInit(&r->Hp.v,Call(Object,GetAttr,1(l_(PC,Body.Hp))));
    RpgCoreDirectValInit(&r->Hp.max,Call(Object,GetAttr,1(l_(PC,Body.MaxHp))));
    RpgCoreDirectValInit(&r->Atk.skill,Call(Object,GetAttr,1(l_(PC,Skills.General.Melee))));
    RpgCoreDirectValInit(&r->Atk.attr,Call(Object,GetAttr,1(l_(PC,Affinity.Strength))));
    RpgCoreDirectValInit(&r->Def.skill,Call(Object,GetAttr,1(l_(PC,Skills.General.Defense))));
    RpgCoreDirectValInit(&r->Def.attr,Call(Object,GetAttr,1(l_(PC,Affinity.Agility))));
    r->Resource.Action.max = 5;
    r->Resource.Action.v = 0;
    r->Resource.Move.max = 5;
    r->Resource.Move.v = 0;
    r->Resource.Reaction.max = 5;
    r->Resource.Reaction.v = 0;
    r->Status.Downed = (0!=0);
    return &r->AltrnFightActor;
}

/*-------------------------------*/

main() {
    EnvOpen(4096,4096);
    EnvClose();
}

