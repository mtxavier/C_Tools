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

#include <StackEnv.h>
#include <Classes.h>
#include <RpgCore.h>

int RpgSkillPoolThrow(RpgSkillPool *Pool) {
    int r;
    RpgSkillThrow *t;
    rOpen
    t = Call(Pool,Throw,0);
    r = Call(t,Val,0);
    rClose
    return r;
}

/*___________________________________________________________________________
 |
 |
 |___________________________________________________________________________
*/

typedef struct {
    RpgSkillPool RpgSkillPool;
    RpgDice *dice;
    int SkillLvl,Attr,Tool,Bonus,Support;
} DicePool;
static void DicePoolInit(RpgSkillPool *this,int attr,int Skill) {
    ThisToThat(DicePool,RpgSkillPool);
    that->SkillLvl = Skill;
    that->Attr = attr;
    that->Bonus = that->Tool = that->Support = 0;
}
static void DicePoolAddBonus(RpgSkillPool *this,int nb) {
    ThisToThat(DicePool,RpgSkillPool);
    that->Bonus += nb;
}
static void DicePoolAddSupport(RpgSkillPool *this,int nb) {
    ThisToThat(DicePool,RpgSkillPool);
    that->Support += nb;
}
static void DicePoolAddTool(RpgSkillPool *this,int nb) {
    ThisToThat(DicePool,RpgSkillPool);
    that->Tool += nb;
}
typedef struct {
    RpgSkillThrow RpgSkillThrow;
    RpgSkillThrowCmpnt d;
} DiceThrown;
static RpgSkillThrowCmpnt *DiceThrowCmpnt(RpgSkillThrow *this) {
    ThisToThat(DiceThrown,RpgSkillThrow);
    return &that->d;
}
static int DiceThrowVal(RpgSkillThrow *this) {
    int r,j,b,t,*p,*e,f;
    ThisToThat(DiceThrown,RpgSkillThrow);
    r = 0; f = 1;
    j = that->d.KeepNum; 
    p = that->d.Ordered; e = p+6;
    b = 0; t = b+*p; p++;
    while ((p<e)&&(t<j)) { 
        f++; b = t; t+=*p; p++;
    }
    if (p<e) {
        b=j; j+=3;
        while (b<j) {
            while ((p<e)&&(b>=t)) { f++; b = t; t+=*p; p++; }
            r+=f; b++;
	}
    } else {
        r = 18;
    }
    r+=that->d.Adj;
    if (r>18) { r-=16; }
    if (r<3) { r+=16; }
    return r+that->d.Base;
}
static RpgSkillThrow *DicePoolThrow(RpgSkillPool *this) {
    static struct RpgSkillThrow Static = {
        DiceThrowCmpnt,DiceThrowVal
    };
    DiceThrown  *r;
    int nbDice,lvl,kept,h;
    ThisToThat(DicePool,RpgSkillPool);
    rPush(r);
    r->RpgSkillThrow.Static = &Static;
    r->d.Base = that->Attr+that->Tool;
    lvl = that->SkillLvl+that->Bonus-that->Attr;
    r->d.Adj = 0;
    if (lvl>11) {
        nbDice = lvl-8;
        kept = nbDice-3;
    } else {
        kept = 0;
        if (lvl<9) {
            nbDice = 12-lvl;
        } else {
            nbDice = 3;
            if (lvl==9) { r->d.Adj = -1; }
            if (lvl==11) { r->d.Adj = 1; }
        }
    }
    h = that->Support;
    r->d.DiscardHigh = r->d.DiscardLow = 0;
    if (h>0) { 
        r->d.DiscardLow = h;
        nbDice += h;
        kept += h;
    }
    if (h<0) { 
        r->d.DiscardHigh = -h;
        nbDice += (-h);
    }
    rnPush(r->d.b,nbDice);
    r->d.e = r->d.b+nbDice;
    r->d.KeepNum = kept;
    Call(that->dice,nThrow,2(r->d.b,r->d.e));
    {
        int *p,*e,*q;
        p = r->d.Ordered; e = p+6; while (p<e) { *p++ = 0; }
        p = r->d.b; e = r->d.e;
        while (p<e) {
            r->d.Ordered[(*p)-1]++;
            p++;
        }
    }
    return &r->RpgSkillThrow;
}
RpgSkillPool *RpgSkillPoolNew(void) {
    DicePool *r;
    static struct RpgSkillPool Static = {
        DicePoolInit,DicePoolAddBonus,DicePoolAddSupport,
        DicePoolAddTool,DicePoolThrow
    };
    rPush(r);
    r->RpgSkillPool.Static = &Static;
    r->dice = RpgUniformDiceNew(1,7);
    Call(&r->RpgSkillPool,Init,2(0,0));
    return &r->RpgSkillPool;
}

/*___________________________________________________________
 |
 | Rpg system contains statics datas for skills, attributes,
 | dices, actions, templates ...
 |___________________________________________________________
*/


typedef struct RpgSkillNod {
    struct RpgSkillNod *Category;
    char *Id;
} RpgSkillNod;

typedef struct { 
    struct RpgSkillLvl *Static;
    RpgSkillNod *Nod;
} RpgSkillLvl;
struct RpgSkillLvl {
    int (*GetAttrBonus)(RpgSkillLvl *this);
    int (*GetLvl)(RpgSkillLvl *this);
};

typedef struct {
    struct RpgSkillCheck *Static;
    RpgSkillLvl *Lvl;
    int TechLvl;
} RpgSkillCheck;
struct RpgSkillCheck {
    int (*GetBonus)(RpgSkillCheck *this);
    int (*GetMalus)(RpgSkillCheck *this);
    int (*GetHandicap)(RpgSkillCheck *this);
    int (*GetSupport)(RpgSkillCheck *this);
    int (*GetTool)(RpgSkillCheck *this);
};

int RpgDoSkillCheck(RpgSkillPool *pool,RpgSkillCheck *sc) {
    int i,j,r;
    i = Call(sc->Lvl,GetLvl,0);
    j = Call(sc->Lvl,GetAttrBonus,0);
    Call(pool,Init,2(j,i));
    i = Call(sc,GetBonus,0);
    Call(pool,AddBonus,1(i));
    i = Call(sc,GetMalus,0);
    Call(pool,AddBonus,1(-i));
    i = Call(sc,GetHandicap,0);
    Call(pool,AddSupport,1(-i));
    i = Call(sc,GetSupport,0);
    Call(pool,AddSupport,1(i));
    i = Call(sc,GetTool,0);
    Call(pool,AddTool,1(i));
    r = RpgSkillPoolThrow(pool);
    return r-sc->TechLvl;
}

/*___________________________________________________________
 |
 | 
 |___________________________________________________________
*/


