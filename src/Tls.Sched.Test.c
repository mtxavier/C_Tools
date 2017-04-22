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
#include <Tools.h>
#include <TlsSched.h>

#include <stdio.h>

typedef struct {
    TlsActor TlsActor;
    int Iter,Rate;
    int Org,Date,nDate;
    TlsStageName *SN;
} myActor;

static int myActorPerform(TlsActor *this,int MaxEndDate) {
    ThisToThat(myActor,TlsActor);
    if ((MaxEndDate-that->nDate)>=0) {
        printf("%d : %s %d.\n",that->nDate-that->Org,this->Id,that->Iter);
        that->Date = that->nDate;
        that->Iter++;
        that->nDate += that->Rate;
    }
    Call(that->SN,Sleep,2(that->Date,that->nDate-that->Date));
    return that->Date;
}
static myActor *myActorNew(TlsStage *Stage,char *name,int Org,int Rate) {
    static struct TlsActor Static = {myActorPerform};
    myActor *r;
    rPush(r);
    r->TlsActor.Static = &Static;
    r->TlsActor.Id = name;
    r->Iter = 0;
    r->Rate = Rate;
    r->Org = r->Date = r->nDate = Org;
    r->SN = Call(Stage,EnterActor,2(&r->TlsActor,Org));
    return r;
}

void DateRangeTest(TlsStage *Stg,int b,int e) {
    myActor *Actors[3];
    int Date;
    printf("\n\nTrying from %d to %d:\n\n",b,e);
    Date = b;
    rOpen
        Actors[0] = myActorNew(Stg,"->",b,50);
        Actors[1] = myActorNew(Stg,"----->",b,120);
        Actors[2] = myActorNew(Stg,"---------->",b,180);
        do {
             Call(&Stg->TlsActor,Perform,1(Date));
             Date = Date+103;
        } while((e-Date)>0);
        Call(Actors[0]->SN,End,0);
        Call(Actors[1]->SN,End,0);
        Call(Actors[2]->SN,End,0);
    rClose
}


main() {
    TlsStage *Stg;
    EnvOpen(4096,4096);
    rOpen
        unsigned int mid;
        mid = -1; mid = mid>>1;
        Stg = TlsSchRoundRobin("TheStage");
        DateRangeTest(Stg,-500,500);
        DateRangeTest(Stg,0,1000);
        DateRangeTest(Stg,mid-500,mid+500);
    rClose
    EnvClose();
}


