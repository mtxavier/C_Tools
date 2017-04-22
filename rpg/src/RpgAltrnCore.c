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
#include <RpgAltrn.h>
#include <List.h>

/*-----------------------------------------------*/

static void SceneNullPerform(RpgCoreScene *this) {}
static void SceneNullClose(RpgCoreScene *this) {}
static struct RpgCoreScene CoreSceneNullStatic = {
    SceneNullPerform,SceneNullClose
};
RpgCoreScene RpgCoreSceneNull = {&CoreSceneNullStatic};

/*-----------------------------------------------*/

#define l_(b,c) c_(Altrn,b,c)


/*-----------------------------------*/

typedef struct {
    dList dList;
    AltrnFightActor *Actor;
} ActorList;
typedef struct {
    dList dList;
    int Init;
    AltrnFightActor *Actor;
} SchedActorList;

typedef struct {
    AltrnFightScene AltrnFightScene;
    RpgCoreScene *Return;
    RpgSkillPool *Rslv;
    struct {
        dList Active,Downed,Fled;
    } Faction[2];
    struct {
        dList Current,Next;
        int RoundCount;
    } Schedule;
    int (*CheckFinished)(AltrnFightScene *t);
} MyFightScene;

static inline dList *ActorInList(dList *e,int Id) {
	dList *r,*f;
	ActorList *F;
	int found;
	found = (0!=0);
	r = e->n;
	while ((f!=e)&&(!found)) {
		F = CastBack(ActorList,dList,f);
		f = f->n;
		found = (F->Actor->Id == Id);
	}
	return r;
}
static void MyFightSceneDefeat(MyFightScene *scene,int faction,int Id) {
    dList *f,*e;
    ActorList *F;
    int found;
    e = &(scene->Faction[faction].Active);
    f = ActorInList(e,Id);
    if (found) {
        e = &(scene->Faction[faction].Downed);
        f->p->n=f->n; f->n->p=f->p;
        f->p = e; f->n = e->n;
        f->p->n = f->n->p = f;
    }
}

static void AFSMeleeAttackPerform(
    AltrnFightScene *this,AltrnFightActor *Atk,AltrnFightActor *Def
) {
    int hit,st0;
    ThisToThat(MyFightScene,AltrnFightScene);
    st0 = Call(Def,CheckStatus,1(l_(Status,Downed)));
    {
        int as,aa,df;
        as = Call(Atk->Atk.Skill,Val,0);
        aa = Call(Atk->Atk.Attr,Val,0);
        df = Call(Def->Def,Val,0);
        Call(that->Rslv,Init,2(aa,as));
        hit = RpgSkillPoolThrow(that->Rslv)-df;
    }
    if (hit>=0) {
        int dmg,dr;
        dr = Call(Def->DR,Val,0);
        Call(Atk->Dmg,nThrow,2(&dmg,(&dmg)+1));
        if (dmg>dr) {
            int st;
            Call(Def,TakeDmg,1(dmg-dr));
            st = Call(Def,CheckStatus,1(l_(Status,Downed)));
            if ((st)&&(!st0)) {
                MyFightSceneDefeat(that,Def->Faction,Def->Id);
            }
        }
    }
}

static void FightScenePerform(RpgCoreScene *this) {
    int Finished;
    ThisToThat(MyFightScene,AltrnFightScene.RpgCoreScene);
    Finished = that->CheckFinished(&that->AltrnFightScene);
    while (!Finished){
        dList *a,*e;
        e = &that->Schedule.Current;
        a = e->n;
        if (a==e) {
            a = &that->Schedule.Next;
            if (a->n!=a) {
                int rnd;
                *a = *e;
                e->p = e->n = e;
                e = a;
                a = e->n;
                rnd = that->Schedule.RoundCount+1;
                that->Schedule.RoundCount = rnd;
                while (a!=e) {
                    SchedActorList *A;
                    A = CastBack(SchedActorList,dList,a);
                    /* Do it now as the NewRound method might move the actor 
                     * in another list. */
                    a = a->n; 
                    Call(A->Actor,NewRound,2(&that->AltrnFightScene,rnd));
                }
                /* We test finished only at the end because no actor should be
                 * robbed of the NewRound event. */
                Finished = that->CheckFinished(&that->AltrnFightScene);
                a = e->n;
            }
        }
        Finished = Finished||(a==e);
        if (!Finished) {
            SchedActorList *A;
            A = CastBack(SchedActorList,dList,a);
            Call(A->Actor,Act,1(&that->AltrnFightScene));
            Finished = that->CheckFinished(&that->AltrnFightScene);
        }
    }
}


