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
#include <List.h>

#include <stdio.h>

#include <RpgCmbt.h>

/*_________________________________
 |
 | RpgCoreValMod
 |_________________________________
*/

typedef struct {
    int Val,Min,Max;
} RpgGauge;

/*--------------------------*/


typedef struct { struct AppliedStatMod *Static;} AppliedStatMod;
typedef struct { struct StatMod *Static; } StatMod;
struct AppliedStatMod {
    void (*Remove)(AppliedStatMod *this);
};
struct StatMod {
    AppliedStatMod *(*Apply)(StatMod *this,TlsMemPool *Mem,RpgStruct *s);
};

     /*-------*/

typedef struct MemberStatus {
    dList/*<CmbtActrList.dList>*/ dList;
    CmbtActr *Actr;
} CmbtActrList;

typedef struct {
    dList/*<CmbtFactionTie.dList>*/ dList;
    CmbtFaction *Other;
    int FriendLvl;
} CmbtFactionTie;

struct CmbtFaction {
    char *Name;
    struct {
        TlsMemPool *Actr,*Others;
    } Mem;
    int Status;
    RpgGauge Effectif;
    dList/*<CmbtActrList.dList>*/ Members;
    dList/*<CmbtFactionTie.dList>*/ Others;
};
#define CmbtFactionDefeated 0
#define CmbtFactionFighting 1
#define CmbtFactionEmpty 2

static CmbtActr *CmbtFactionForEachMember(
    CmbtFaction *t,int (*Cont)(CmbtActr *Actr,void *cls),void *cls
) {
    dList *rl,*dl;
    CmbtActr *r;
    rl = &t->Members; dl = rl->n;
    r = 0;
    while ((r==0)&&(dl!=rl)) {
        CmbtActrList *al;
        al = CastBack(CmbtActrList,dList,dl);
        if (!Cont(al->Actr,cls)) {
            r = al->Actr;
        }
        dl = dl->n;
    }
    return r;
}
static CmbtFaction *CmbtFactionForEachOther(
    CmbtFaction *t,int (*Cont)(CmbtFaction *f,int FriendLvl,void *cls),void *cls
) {
    dList *rl,*dl;
    CmbtFaction *r;
    rl = &t->Others; dl=rl->n;
    r=0;
    while ((dl!=rl)&&(r==0)) {
        CmbtFactionTie *ft;
        ft = CastBack(CmbtFactionTie,dList,dl);
        if (!Cont(ft->Other,ft->FriendLvl,cls)) {
            r=ft->Other;
        }
        dl = dl->n;
    }
    return r;
}

static CmbtFaction *CmbtFactionInit(CmbtFaction *r,char *Name,TlsMemPool *mem) {
    dList *dl;
    r->Name=Name;
    r->Mem.Actr = Call(mem,SizeSelect,1(sizeof(CmbtActrList)));
    r->Mem.Others = Call(mem,SizeSelect,1(sizeof(CmbtFactionTie)));
    r->Status=CmbtFactionEmpty;
    r->Effectif.Min=r->Effectif.Max=r->Effectif.Val=0;
    dl = &r->Members; dl->p=dl->n=dl;
    dl = &r->Others; dl->p=dl->n=dl;
    return r;
}
static void FactionAddOther(CmbtFaction *t,CmbtFaction *other,int Lvl) {
    CmbtFactionTie *add;
    dList *dl,*rl;
    add = Call(t->Mem.Others,Alloc,0);
    add->Other = other;
    add->FriendLvl = Lvl;
    dl = &add->dList; rl = &t->Others;
    dl->n=rl; dl->p=rl->p; dl->p->n=rl->p=dl;
}
static MemberStatus *FactionAddMember(CmbtFaction *t,CmbtActr *act) {
    CmbtActrList *add;
    dList *dl,*rl;
    int able;
    add = Call(t->Mem.Actr,Alloc,0);
    add->Actr = act;
    dl = &add->dList; rl = &t->Members;
    dl->n=rl; dl->p=rl->p; dl->p->n=rl->p=dl;
    able = ActrIsAble(act);
    ActrSetStatus(act,able);
    t->Effectif.Max++;
    if (able) {
        t->Status = CmbtFactionFighting;
        t->Effectif.Val++;
    }
    return add;
}
static MemberStatus *MemberGetStatus(CmbtFaction *t,CmbtActr *a) {
    MemberStatus *s;
    int found;
    dList *dl,*rl;
    s = 0;
    rl = &t->Members; dl=rl->n;
    found = (0!=0);
    while(!found&&(dl!=rl)) {
        s = CastBack(MemberStatus,dList,dl);
        found = (s->Actr==a);
        if (!found) { dl=dl->n; }
    }
    return s;
}
static int MemberStatusChanged(CmbtFaction *t,MemberStatus *a) {
    int able;
    able = ActrIsAble(a->Actr);
    if (able!=ActrGetStatus(a->Actr)) {
        int eff;
        ActrSetStatus(a->Actr,able);
        eff = t->Effectif.Val;
        if (able) {
            eff++;
            if (eff>0) { t->Status = CmbtFactionFighting; }
        } else {
            eff--;
            if (eff<=0) { t->Status = CmbtFactionDefeated; }
        }
        t->Effectif.Val=eff;
    }
}
static int FactionRemoveMember(CmbtFaction *t,MemberStatus *s) {
    if (s) {
        dList *dl;
        dl = &s->dList;
        t->Effectif.Max--;
        if (ActrGetStatus(s->Actr)) { t->Effectif.Val--; }
        if (t->Effectif.Val<=0) {
            t->Status = CmbtFactionDefeated;
        }
        dl->n->p=dl->p; dl->p->n=dl->n;
        Call(t->Mem.Actr,Free,1(s));
    }
    return (s!=0);
}

static void FactionFree(CmbtFaction *t) {
    dList *dl,*rl;
    TlsMemPool *mem;
    mem=t->Mem.Actr;
    rl=&t->Members; 
    dl=rl->n; 
    rl->n=rl->p=rl;
    while (dl!=rl) {
        CmbtActrList *f;
        f=CastBack(CmbtActrList,dList,dl);
        Call(mem,Free,1(f));
        dl=dl->n;
    }
    mem=t->Mem.Others;
    rl=&t->Others; dl=rl->n;
    rl->n=rl->p=rl;
    while (dl!=rl) {
        CmbtFactionTie *f;
        f=CastBack(CmbtFactionTie,dList,dl);
        Call(mem,Free,1(f));
        dl=dl->n;
    }
}

/*_____________________________________________
 |
 |
 |_____________________________________________
*/

typedef struct RpgSchedule RpgSchedule;

typedef struct { struct EventTurn *Static;} EventTurn;
struct EventTurn {
    int (*Perform)(EventTurn *this,RpgSchedule *Schedule);
    EventTurn *(*Abort)(EventTurn *this);
};
typedef struct {
    dList dList;
    int Date;
    EventTurn *Event;
} EventDate;

typedef struct {
    EventTurn EventTurn;
    struct {
        TlsMemPool *Date;
    } Mem;
    dList Schedule;
    int Date;
} TimeLine;

static EventDate *TimeLineAddEvent(TimeLine *t,int date,EventTurn *ev);
static void TimeLineRemoveEvent(TimeLine *t,EventDate *Date);
static TimeLine *TimeLineNew(TlsMemPool *Mem);

static EventDate *TimeLineAddEvent(TimeLine *t,int date,EventTurn *ev) {
    EventDate *r;
    dList *dl,*rl,*nl;
    int found;
    r = Call(t->Mem.Date,Alloc,0);
    r->Date = date;
    r->Event = ev;
    rl = &t->Schedule; dl=rl->p;
    found = (dl==rl);
    while (!found) {
        EventDate *d;
        d = CastBack(EventDate,dList,dl);
        found = (d->Date<=date);
        if (!found) {
            dl=dl->p;
            found = (dl==rl);
        }
    }
    nl = &r->dList;
    nl->n=dl->n; nl->p=dl; dl->n=nl->n->p=nl;
    return r;
}
static void TimeLineRemoveEvent(TimeLine *t,EventDate *Date) {
    dList *dl;
    dl = &Date->dList;
    dl->n->p=dl->p; dl->p->n=dl->n; dl->n=dl->p=dl;
    Call(t->Mem.Date,Free,1(Date));
}
static int TimeLinePerform(EventTurn *this,RpgSchedule *Schedule) {
    dList *dl,*rl,*nl;
    int date,found;
    ThisToThat(TimeLine,EventTurn);
    rl = &that->Schedule;
    date = that->Date;
    dl = rl->n;
    found = (dl!=rl);
    if (found) {
        EventDate *D;
        EventTurn *E;
        int t0,dt;
        D = CastBack(EventDate,dList,dl);
        E = D->Event; t0 = D->Date;
        TimeLineRemoveEvent(that,D);
        dt = Call(E,Perform,1(Schedule));
        if (dt>=0) {
            TimeLineAddEvent(that,t0+dt,E);
        } else {
            Call(E,Abort,0);
        }
        dl = rl->n;
        found = (dl!=rl);
        if (dl!=rl) {
            D = CastBack(EventDate,dList,dl);
            that->Date = D->Date;
        }
    }
    return found?that->Date-date:-1;
}
static EventTurn *TimeLineAbort(EventTurn *this) {
    dList *dl,*rl;
    ThisToThat(TimeLine,EventTurn);
    rl = &that->Schedule;
    dl = rl->n;
    while (dl!=rl) {
        EventDate *D;
        D = CastBack(EventDate,dList,dl);
        rl->n=dl->n; dl->n->p=rl; dl->n=dl->p=dl;
        Call(D->Event,Abort,0);
        dl=dl->n;
        Call(that->Mem.Date,Free,1(D));
    }
    rl->n=rl->p=0;
    {
        TlsMemPool *Mem;
        Mem = Call(that->Mem.Date,SizeSelect,1(sizeof(TimeLine)));
        Call(Mem,Free,1(that));
    }
    return 0;
}
static TimeLine *TimeLineNew(TlsMemPool *Mem) {
    static struct EventTurn ETStatic = {TimeLinePerform,TimeLineAbort};
    TimeLine *r;
    dList *rl;
    Mem = Call(Mem,SizeSelect,1(sizeof(TimeLine)));
    r = Call(Mem,Alloc,0);
    r->EventTurn.Static = &ETStatic;
    r->Mem.Date = Call(Mem,SizeSelect,1(sizeof(EventDate)));
    r->Date = 0;
    rl=&r->Schedule;
    rl->p=rl->n=rl;
    return r;
}

/*_____________________________________________
 |
 |
 |_____________________________________________
*/

typedef struct {
    struct {
        TlsMemPool *Factions,*Actr;
    } Mem;
    struct {
        RpgDiceCollect *Collect;
        RpgDice *d2,*d3,*d4,*d6,*d8,*d10,*d12,*d20,*d100;
    } Dices;
    struct {
        struct {
            struct {
                StatMod *Stance[11],*Bonus[11];
            } DmgRes,Def,Init,Atk;
        } Actr;
    } Bonus;
    RpgSkillPool *SkillPool;
    CmbtFaction Factions;
    int FactionNum;
    int Status;
    TimeLine *TimeLine;
    List/*<CmbtActrList.List>*/ Act,Disabled,Sleeping;
} RpgCmbtCtx;
#define CmbtCtxRunning 1
#define CmbtCtxFinnished 0

static int CmbtEnded(RpgCmbtCtx *cmbt) {
    int AliveNb;
    dList *rl,*dl;
    rl = &cmbt->Factions.Others;
    dl = rl->n;
    AliveNb = 0;
    while (dl!=rl) {
        CmbtFactionTie *fct;
        fct = CastBack(CmbtFactionTie,dList,dl);
        if (fct->Other->Status==CmbtFactionFighting) {
            AliveNb++;
        }
        dl=dl->n;
    }
    return AliveNb<=1; 
}

static RpgCmbtCtx *CmbtCtxNew(void) {
    RpgCmbtCtx *r;
    TlsMemPool *Mem;
    rPush(r);
    Mem = TlsMemPoolNew(1024);
    r->Mem.Actr=Mem;
    r->Mem.Factions=Call(Mem,SizeSelect,1(sizeof(CmbtFaction)));
    r->Dices.d2 = RpgUniformDiceNew(1,3);
    r->Dices.d3 = RpgUniformDiceNew(1,4);
    r->Dices.d4 = RpgUniformDiceNew(1,5);
    r->Dices.d6 = RpgUniformDiceNew(1,7);
    r->Dices.d8 = RpgUniformDiceNew(1,9);
    r->Dices.d10 = RpgUniformDiceNew(1,11);
    r->Dices.d12 = RpgUniformDiceNew(1,13);
    r->Dices.d20 = RpgUniformDiceNew(1,21);
    r->Dices.d100 = RpgUniformDiceNew(1,101);
    r->Dices.Collect = RpgDiceCollectNew(
        RpgDiceConst(1),r->Dices.d2,r->Dices.d3,r->Dices.d4,r->Dices.d6,
        r->Dices.d8,r->Dices.d10,r->Dices.d12,r->Dices.d20,r->Dices.d20,
    0);
    CmbtFactionInit(&r->Factions,"Alls",r->Mem.Factions);
    r->SkillPool = RpgSkillPoolNew();
    r->Status=CmbtCtxRunning;
    r->TimeLine=TimeLineNew(Mem);
    return r;
}

static CmbtFaction *CmbtCtxAddFaction(RpgCmbtCtx *this,char *Name) {
    CmbtFaction *r;
    r = Call(this->Mem.Factions,Alloc,0);
    CmbtFactionInit(r,Name,this->Mem.Factions);
    FactionAddOther(&this->Factions,r,0);
    return r;
}

/*--------------------------*/

typedef struct {
    TlsIntVal *Base,*Skill,*Bonus,*Support,*Tools;
    TlsDyndListNod/*<TlsIntFn *>*/ *Stance,*Enhance,*Help;
} RpgObjStat;

/*--------------------------*/

typedef struct {
    TlsIntVal *Val;
    TlsDyndList/*<IntVal *>*/  *Bonus,*Malus;
} BonMalList;

static BonMalList *BonMalListInit(BonMalList *r,TlsMemPool *Mem) {
    TlsIntVal *Zero,*Min,*Max;
    Zero = TlsConstIntVal(0);
    r->Bonus=TlsDyndListAlloc(Mem);
    TlsDyndListPush(r->Bonus,Zero);
    Max=TlsIntValdListLCmbnd(TlsIntOpMax,r->Bonus);
    r->Malus=TlsDyndListAlloc(Mem);
    TlsDyndListPush(r->Malus,Zero);
    Min=TlsIntValdListLCmbnd(TlsIntOpMin,r->Malus);
    r->Val=TlsIntValSum(Min,Max);
}
static void BonMalListFree(BonMalList *o) {
    TlsDyndListFree(o->Bonus);
    TlsDyndListFree(o->Malus);
}

static TlsDyndListNod *BonMalListAddBonus(BonMalList *bm,TlsIntVal *V) {
    int v;
    TlsDyndListNod *r;
    v = Call(V,Val,0);
    if (v>=0) {
        r=TlsDyndListPush(bm->Bonus,V);
    } else {
        r=TlsDyndListPush(bm->Malus,V);
    }
    return r;
}
static TlsIntVal *BonMalListFreeBonus(BonMalList *bm,TlsDyndListNod *n) {
    TlsIntVal *r;
    int v;
    r = TlsDyndListGetItem(n);
    v = Call(r,Val,0);
    if (v>=0) {
        TlsDyndListRemove(bm->Bonus,n);
    } else {
        TlsDyndListRemove(bm->Malus,n);
    }
    return r;
}

/*---------------*/

typedef struct {
    TlsMemPool *Mem;
    TlsIntVal *Val;
    TlsDyndList/*<TlsIntVal *>*/ *Mods;
    BonMalList Stance,Bonus;
} RpgPassiveStat;

static RpgPassiveStat *PassiveStatInit(RpgPassiveStat *n,TlsMemPool *Mem,int Val) {
    TlsIntFn *Formula;
    TlsDyndList *L;
    TlsIntVal **Bonus;
    n->Mem=Mem;
    BonMalListInit(&n->Stance,Mem);
    BonMalListInit(&n->Bonus,Mem);
    n->Mods=TlsDyndListAlloc(Mem);
    TlsDyndListPush(n->Mods,TlsConstIntVal(Val));
    TlsDyndListPush(n->Mods,n->Stance.Val);
    TlsDyndListPush(n->Mods,n->Bonus.Val);
    n->Val=TlsIntValdListLCmbnd(TlsIntOpSum,n->Mods);
    return n;
}
static void PassiveStatFree(RpgPassiveStat *n) {
    TlsDyndListFree(n->Mods);
    BonMalListFree(&n->Stance);
    BonMalListFree(&n->Bonus);
}

/*----------------*/

typedef struct {
    RpgModListAccess RpgModListAccess;
    int dA;
} ModListField;
static TlsDyndList *ModListFieldAccess(RpgModListAccess *this,RpgStruct *Base) {
    void *st,*fld;
    ThisToThat(ModListField,RpgModListAccess);
    st=Base;
    fld=st+that->dA;
    return fld;
}
RpgModListAccess *RpgModListField(RpgStruct *St,TlsDyndList *Fld) {
    ModListField *r;
    void *fld,*st;
    static struct RpgModListAccess Static = {ModListFieldAccess};
    rPush(r);
    r->RpgModListAccess.Static = &Static;
    fld=Fld; st=St;
    r->dA=fld-st;
    return &r->RpgModListAccess;
}

/*---------------------------*/


typedef struct {
    StatMod StatMod;
    RpgModListAccess *Access;
    TlsIntVal *Val;
} PassiveStatBonus;
typedef struct {
    AppliedStatMod AppliedStatMod;
    TlsMemPool *Mem;
    TlsDyndListNod *Nod;
    TlsDyndList *L;
} AppliedPassiveStatBonus;
static void AppliedPassiveStatBonusRemove(AppliedStatMod *this) {
    ThisToThat(AppliedPassiveStatBonus,AppliedStatMod);
    TlsDyndListRemove(that->L,that->Nod);
    Call(that->Mem,Free,1(that));
}
static AppliedStatMod *PassiveStatBonusApply(StatMod *this,TlsMemPool *Mem,RpgStruct *s){
    AppliedPassiveStatBonus *r;
    static struct AppliedStatMod Static = {AppliedPassiveStatBonusRemove};
    ThisToThat(PassiveStatBonus,StatMod);
    Mem = Call(Mem,SizeSelect,1(sizeof(AppliedPassiveStatBonus)));
    r=Call(Mem,Alloc,0);
    r->AppliedStatMod.Static = &Static;
    r->Mem=Mem;
    r->L=Call(that->Access,Access,1(s));
    r->Nod = TlsDyndListQueue(r->L,that->Val);
    return &r->AppliedStatMod;
}
StatMod *RpgStatModNew(RpgModListAccess *Access,TlsIntVal *Val) {
    PassiveStatBonus *r;
    static struct StatMod Static = {PassiveStatBonusApply};
    rPush(r);
    r->StatMod.Static = &Static;
    r->Access = Access;
    r->Val=Val;
    return &r->StatMod;
}

    /*----------------*/

typedef struct {
    StatMod StatMod;
    StatMod **b,**e;
} StatModVec;
typedef struct {
    AppliedStatMod AppliedStatMod;
    TlsMemPool *Mem;
    AppliedStatMod **b,**e;
} AppliedStatModVec;

static void StatModVecRemove(AppliedStatMod *this) {
    AppliedStatMod **p,**e;
    int sz;
    TlsMemPool *M;
    ThisToThat(AppliedStatModVec,AppliedStatMod);
    p=that->b; e=that->e;
    /**/ {
        unsigned char *cb,*ce;
        cb=(unsigned char *)p; ce=(unsigned char *)e;
        sz=ce-cb;
    }
    while (p<e) {
        Call(*p,Remove,0);
        p++;
    }
    M = Call(that->Mem,SizeSelect,1(sz));
    Call(M,Free,1(that->b));
    M = Call(that->Mem,SizeSelect,1(sizeof(AppliedStatModVec)));
    Call(M,Free,1(that));
}
static AppliedStatMod *StatModVecApply(StatMod *this,TlsMemPool *Mem,RpgStruct *tgt) {
    AppliedStatModVec *r;
    TlsMemPool *M;
    int nb;
    static struct AppliedStatMod Static = {StatModVecRemove};
    ThisToThat(StatModVec,StatMod);
    M = Call(Mem,SizeSelect,1(sizeof(AppliedStatModVec)));
    r = Call(M,Alloc,0);
    r->AppliedStatMod.Static = &Static;
    nb = that->e-that->b;
    M = Call(Mem,SizeSelect,1(nb*sizeof(AppliedStatMod *)));
    r->b = Call(M,Alloc,0);
    r->e = r->b+nb;
    {
        AppliedStatMod **ap;
        StatMod **p,**e;
        p=that->b; e=that->e;
        ap = r->b;
        while (p<e) {
            *ap = Call(*p,Apply,2(Mem,tgt));
            ap++; p++;
        }
    }
    return &r->AppliedStatMod;
}
StatMod *RpgStatModVecNew(TlsVoidVec/*<StatMod *>*/ *Vec) {
    StatModVec *r;
    static struct StatMod Static = {StatModVecApply};
    rPush(r);
    r->StatMod.Static = &Static;
    r->b = (StatMod **)(Vec->b);
    r->e = (StatMod **)(Vec->e);
    return &r->StatMod;
}

/*--------------------------*/

#define RoundLength 0x2000
#define RoundOrg 0x1000

struct CmbtActr {
    char *Name;
    int Status;
    EventTurn EventTurn;
    RpgStruct RpgStruct;
    EventDate *Date;
    CmbtFaction *Faction;
    MemberStatus *FactionStatus;
    RpgCmbtCtx *Ctx;
    RpgPassiveStat Atk,DmgRes,Def,Init;
    RpgDice *StrDmg,*WeaponDmg;
    RpgGauge Hp;
};

static int ActrGetStatus(CmbtActr *a) { return a->Status; }
static void ActrSetStatus(CmbtActr *a,int val) { a->Status=val; }

static void ActAtk(CmbtActr *atk,CmbtActr *tgt);

struct AtkTgtSelect {
    CmbtActr *Tgt;
    int Score;
};

static int AtkTgtScore(CmbtActr *a) {
    int variance;
    Call(a->Ctx->Dices.d100,nThrow,2(&variance,(&variance)+1));
    return 1+variance;
}
static int AtkTgtMemberSelect(CmbtActr *a,void *cls) {
    struct AtkTgtSelect *c;
    c = cls;
    if (ActrIsAble(a)) {
        int Score;
        Score = AtkTgtScore(a);
        if ((c->Tgt==0)||(c->Score<Score)) {
            c->Tgt=a;
            c->Score= Score;
        }
    }
    return (0==0);
}
static int AtkTgtSelect(CmbtFaction *t,int FriendLvl,void *cls) {
    if (FriendLvl<0) {
        CmbtFactionForEachMember(t,AtkTgtMemberSelect,cls);
    }
    return (0==0);
}

static int CmbtActrPerform(EventTurn *this,RpgSchedule *Schedule) {
    ThisToThat(CmbtActr,EventTurn);
    if (ActrIsAble(that)) {
    /*Atk*/ 
        struct AtkTgtSelect sel;
        sel.Tgt=0;
        sel.Score=0;
        CmbtFactionForEachOther(that->Faction,AtkTgtSelect,&sel);
        if (sel.Tgt!=0) {
            ActAtk(that,sel.Tgt);
        }
    }
    return RoundLength;
}
static EventTurn *CmbtActrAbort(EventTurn *this) {
    return 0;
}

static int ActrIsAble(CmbtActr *a) {
    return (a->Hp.Val>0);
}

static CmbtActr *CmbtActrNew(RpgCmbtCtx *Ctx,char *Name) {
    static struct EventTurn ActStatic = {CmbtActrPerform,CmbtActrAbort};
    TlsMemPool *Mem;
    CmbtActr *r;
    Mem = Ctx->Mem.Actr;
    Call(Mem,SizeSelect,1(sizeof(CmbtActr)));
    r=Call(Mem,Alloc,0);
    r->EventTurn.Static = &ActStatic;
    r->Status = (0==0);
    r->Date = 0;
    r->Name = Name;
    r->Faction = 0;
    r->FactionStatus = 0;
    r->Ctx = Ctx;
    PassiveStatInit(&r->Atk,Mem,7);
    PassiveStatInit(&r->Def,Mem,6);
    PassiveStatInit(&r->Init,Mem,0);
    PassiveStatInit(&r->DmgRes,Mem,6);
    r->Hp.Min = 0; r->Hp.Max=10; r->Hp.Val=r->Hp.Max;
    r->StrDmg = Ctx->Dices.d6;
    r->WeaponDmg = Ctx->Dices.d6;
    return r;
}
static void CmbtActrFree(CmbtActr *a) {
    TlsMemPool *Mem;
    Mem = a->Ctx->Mem.Actr;
    PassiveStatFree(&a->Atk);
    PassiveStatFree(&a->Def);
    PassiveStatFree(&a->Init);
    PassiveStatFree(&a->DmgRes);
    Mem=Call(Mem,SizeSelect,1(sizeof(CmbtActr)));
    Call(Mem,Free,1(a));
}

static void CmbtCtxAddFighter(RpgCmbtCtx *this,CmbtFaction *Faction,CmbtActr *Actr) {
    int init,rndnum;
    FactionAddMember(&this->Factions,Actr);
    Actr->FactionStatus = FactionAddMember(Faction,Actr);
    Actr->Faction = Faction;
    Call(this->Dices.d20,nThrow,2(&init,(&init)+1));
    rndnum = this->TimeLine->Date&(-RoundLength);
    init += rndnum+RoundOrg+Call(Actr->Init.Val,Val,0);
    Actr->Date=TimeLineAddEvent(this->TimeLine,init,&Actr->EventTurn);
}

/*-----------------*/

static void ActAtk(CmbtActr *atk,CmbtActr *tgt) {
    RpgCmbtCtx *Ctx;
    RpgSkillPool *dp;
    int thw,dmg,wdmg,mHp,Hp,Def,DmgRes,Atk;
    Ctx = atk->Ctx;
    dp = Ctx->SkillPool;
    Atk=Call(atk->Atk.Val,Val,0);
    Call(dp,Init,2(0,Atk));
    thw = RpgSkillPoolThrow(dp);
    Def=Call(tgt->Def.Val,Val,0);
    printf("\n%s attaque %s... (%d/%d) ",atk->Name,tgt->Name,thw,Def);
    if (thw>=Def) {
        printf("et touche! ");
        Hp = tgt->Hp.Val;
        mHp = tgt->Hp.Min;
        Call(atk->StrDmg,nThrow,2(&dmg,(&dmg)+1));
        Call(atk->WeaponDmg,nThrow,2(&wdmg,(&wdmg)+1));
        dmg = dmg+wdmg;
        DmgRes=Call(tgt->DmgRes.Val,Val,0);
        if (dmg<=DmgRes) {
            printf("%s encaisse sans broncher(%d/%d).",tgt->Name,dmg,DmgRes);
            dmg=0;
        } else {
            dmg-=DmgRes;
            if (Hp>0) {
                if (dmg>Hp) {dmg=Hp;}
                Hp = Hp-dmg;
                tgt->Hp.Val=Hp;
                printf("\n    %s subit %d de degat (il lui reste %d/%dHp).",tgt->Name,dmg,Hp,tgt->Hp.Max);
                if ((Hp<=0)&&(tgt->FactionStatus)) {
                    printf(".. et tombe.");
                    MemberStatusChanged(tgt->Faction,tgt->FactionStatus);
                    TimeLineRemoveEvent(Ctx->TimeLine,tgt->Date);
                    tgt->Date=0;
                }
            }
        }
    } else {
        printf("mais rate.");
    }
}

/*_____________________________________________________________
 |
 |
 |_____________________________________________________________
*/



/*_____________________________________________________________
 |
 |
 |_____________________________________________________________
*/

main() {
    CmbtActr *c1,*c2;
    RpgCmbtCtx *Cmbt;
    CmbtFaction *Blue,*Red,*Win;
    int end;
    EnvOpen(4096,4096);
    Cmbt = CmbtCtxNew();
    Blue = CmbtCtxAddFaction(Cmbt,"bleue");
    Red = CmbtCtxAddFaction(Cmbt,"rouge");
    FactionAddOther(Blue,Red,-1);
    FactionAddOther(Red,Blue,-1);
    CmbtCtxAddFighter(Cmbt,Blue,CmbtActrNew(Cmbt,"Michel"));
    CmbtCtxAddFighter(Cmbt,Blue,CmbtActrNew(Cmbt,"Marc"));
    CmbtCtxAddFighter(Cmbt,Red,CmbtActrNew(Cmbt,"Borgia"));
    CmbtCtxAddFighter(Cmbt,Red,CmbtActrNew(Cmbt,"Bernard"));
    printf("\n");
    while(!CmbtEnded(Cmbt)) {
        Call(&(Cmbt->TimeLine->EventTurn),Perform,1(0));
    }
    if (Blue->Status==CmbtFactionFighting) {
        Win = Blue;
    } else {
        Win = Red;
    }
    printf("\nL'équipe %s l'emporte.\n",Win->Name);
    EnvClose();
}

