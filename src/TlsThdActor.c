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
#include <TlsSched.h>

/*____________________________________________________________
 |
 | Specialized Thread : 'Real' thread.
 |____________________________________________________________
*/

#include <Thd.h>

struct Thread {
    ThdItf ThdItf;
    TlsActor TlsActor;
    TlsStageName TlsStageName;
    ThdThread *Thread;
    TlsStageName *StageName;
    TlsSchThread *Main;
    int TimeLeft;
};

static TlsStage *ThdGetStage(TlsStageName *this) {
    ThisToThat(struct Thread,TlsStageName);
    return Call(that->StageName,GetStage,0);
}
static void ThdAlterPriority(TlsStageName *this,TlsStageName *ref,int offset) {
    ThisToThat(struct Thread,TlsStageName);
    Call(that->StageName,AlterPriority,2(ref,offset));
}
static int ThdGetDate(TlsStageName *this,int SliceLeft) {
    ThisToThat(struct Thread,TlsStageName);
    that->TimeLeft = SliceLeft;
    Call(that->Thread,Wait,0);
    return that->TimeLeft;
}
static void ThdSleep(TlsStageName *this,int date,int delay){
    ThisToThat(struct Thread,TlsStageName);
    Call(that->StageName,Sleep,2(date,delay));
}
static void ThdHang(TlsStageName *this) {
    ThisToThat(struct Thread,TlsStageName);
    Call(that->StageName,Hang,0);
}
static void ThdEnd(TlsStageName *this) {
    ThisToThat(struct Thread,TlsStageName);
    Call(that->StageName,End,0);
}
static void ThdAwaken(TlsStageName *this,int date) {
    ThisToThat(struct Thread,TlsStageName);
    Call(that->StageName,Awaken,1(date));
}
static void ThdFlushEvents(TlsStageName *this) {
    ThisToThat(struct Thread,TlsStageName);
    Call(that->StageName,FlushEvents,0);
}

static void ThreadMain(ThdItf *this,ThdThread *res) {
    ThisToThat(struct Thread,ThdItf);
    that->Thread = res;
    Call(res,Wait,0);
    Call(that->Main,Main,1(&that->TlsStageName));
}
static int ThdThreadPerform(TlsActor *this,int Slice) {
    ThisToThat(struct Thread,TlsActor);
    that -> TimeLeft = Slice;
    Call(that->Thread,Wait,0);
    return that->TimeLeft;
}

void TlsSchFiberLaunch(TlsStage *Stage,int date,char *Label,TlsSchThread *Main,int StackSize,int pGrowth,int rGrowth) {
    struct Thread *r;
    static struct ThdItf ItfStatic = {ThreadMain};
    static struct TlsActor Static = {ThdThreadPerform};
    static struct TlsStageName NameStatic = {
        ThdGetStage,ThdAlterPriority,ThdGetDate,ThdSleep,ThdHang,ThdEnd,ThdAwaken,ThdFlushEvents
    };
    rPush(r);
    r->ThdItf.Static = &ItfStatic;
    r->TlsActor.Static = &Static;
    r->TlsActor.Id = Label;
    r->TlsStageName.Static = &NameStatic;
    r->TimeLeft = 0;
    r->Main = Main;
    r->Thread = ThdEnvFiberLaunch(&r->ThdItf,StackSize,pGrowth,rGrowth);
    r->StageName = Call(Stage,EnterActor,2(&r->TlsActor,date));
    Call(r->Thread,Wait,0);
} 

