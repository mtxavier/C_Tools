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
#include <Schedule.h>
#include <RpgCore.h>

typedef struct { struct RpgCombatFaction *Static; } RpgCombatFaction;
typedef struct { struct RpgCombatArena *Static; } RpgCombatArena;
typedef struct { struct RpgCombatResult *Static; } RpgCombatResult;

struct RpgCombatArena {
    void (*AddFaction)(RpgCombatArena *this,RpgCombatFaction *Faction);
    int (*PerformRound)(RpgCombatArena *this);
    int (*RoundNum)(RpgCombatArena *this);
    void (*Close)(RpgCombatArena *this);
};

struct RpgCombatResult {
    int (*Check)(RpgCombatResult *this);
};

/*------------------*/

typedef struct {
    int Success,Quality;
} RpgTestResult;
typedef struct { struct RpgSkillTest *Static; } RpgSkillTest;
struct RpgSkillTest {
    RpgTestResult *(*Throw)(RpgSkillTest *this,RpgTestResult *r,int ThreshHold);
    void (*AddMod)(RpgSkillTest *this,RpgCategory *Source,int Gravity);
    void (*RemoveMod)(RpgSkillTest *this,RpgCategory *Source);
};

typedef struct { struct RpgSkillScore *Static; } RpgSkillScore;
struct RpgSkillScore {
    RpgSkillTest *(*Value)(RpgSkillScore *this);
};

typedef struct { struct RpgCmbtActor *Static; } RpgCmbtActor;
struct RpgCmbtActor {
    void (*Atk)(RpgCmbtActor *this);
};

/*------------------*/

typedef struct { struct RpgActionCtx *Static; } RpgActionCtx;
struct RpgActionCtx {
    RpgSkillTest *(*AddMods)(RpgActionCtx *this,RpgSkillTest *Test,RpgCategory *ActionType);
};

typedef struct { struct RpgAction *Static; } RpgAction;
struct RpgAction {
    void (*Perform)(RpgAction *this);
};

typedef struct {
    RpgAction RpgAction;
    RpgActionCtx *Ctx;
    RpgPhysActor *Actor;
    RpgPhysObject *Target;
} AtkAction;


static void ActnAttackPerform(RpgAction *this) {
    ThisToThat(AtkAction,RpgAction);
    Atker = Call(Ctx,GetActor,1("Actr"));
    Tgt = Call(Ctx,GetActor,1("Tgt"));
    Atk = Call(Atker,GetAttribute,1("Atk"),Ctx);
    Dfns = Call(Tgt,GetAttribute,1("Dfn"),Ctx);
    RpgThrowDice(&Throw,Atk,Dfns);
    rOpen {
    if (Throw.Success) {
        Call(Ctx,Log,1(RpgMsgAtkHitSucceed(Atker->Desc,Dfns->Desc)));
        Dmg = Call(Atker,DoDmg,1(Ctx,Throw.Marge));
        Call(Dfns,TakeDmg,1(Ctx,Dmg));
    } else {
        Call(Ctx,Log,1(RpgMsgAtkHitFailed(Atker->Desc,Dfns->Desc)));
    }
    } rClose
}


