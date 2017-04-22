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
#include <List.h>
#include <Tools.h>
#include <TlsSched.h>


#define ddListInit(r) {(r)->p = (r)->n = (r); }
#define ddListUnchain(r) { \
	(r)->p->n = (r)->n; (r)->n->p = (r)->p; (r)->n = (r)->p = (r); \
}
#define dListChainBefore(r,l) { \
	(l)->n=(r); (l)->p=(r)->p; (l)->n->p=(l); (l)->p->n=(l); \
}
#define dListChainAfter(r,l) { \
	(l)->n=(r)->n; (l)->p=(r); (l)->n->p=(l); (l)->p->n=(l); \
}

static int ActorNullPerform(TlsActor *this,int ticknb) {return 0;}
static struct TlsActor ActorNullStatic = {ActorNullPerform};
TlsActor TlsActorNull = {&ActorNullStatic,"ThreadNull"};


/*_____________________________________________________________
 |
 | Psx threads
 |_____________________________________________________________
*/

typedef struct {
    TlsIntTrie Date;
    dList/*<priorityList.dList>*/ priorities;
} ActorDate;
typedef struct {
    dList dList;
    ActorDate *Date;
    int lvl;
    dList/*<struct StageName.dList>*/ Actors;
} priorityList;

// The difference between hanged and asleep threads is that date of 
// hanged thread will be set when they are awakened. 
// Recorded date of asleep threads is the date when they became asleep.

typedef enum {
    Thread_Awake,
    Thread_Asleep,
    Thread_Hanged,
    Thread_Dead
} IrqThreadState;

struct StageName {
    TlsStageName TlsStageName;
    struct Stage *Stage;
    dList dList; // Inserted in priority list
    // Busy actors are currently performing and shouldn't be moved
    // along the timeline or the sleep/hang queue. They will be
    // inserted at the right place at the end of their performance.
    int Date,Busy;
    IrqThreadState Running;
    TlsActor *Actor;
    priorityList *priority;
    int Lvl; // priority lvl.
    TlsSchEvent *Alarm;
    struct { TlsSchMsg Awaken; } Msg;
    dList/*<Event.dList>*/ PendingEvent;
};

struct Event {
    dList dList; // Inserted in a StageName event queue
    TlsSchEvent TlsSchEvent;
    TlsActor TlsActor;
    struct Stage *Stage;
    struct StageName *Date; // used to interrupt or remove event.
    TlsSchMsg *Msg;
    struct StageName *Target;
};

/*__________________________________________________
 |
 |__________________________________________________
*/

struct Stage {
    TlsStage TlsStage;
    MemStack *Mem;
    TlsMemPool *NamePool,*EventPool,*MsgPool,*DatePool,*PriorityPool;
    struct { int lvl; } Priority;
    TlsIntTrie/*<ActorDate.Date>*/ Timeline;
    // in our case, we require (e-b)>=0 but we can and will have e<b.
    TlsRange DateRange;
    dList Sleeping;
};



/*_____________________________________________________________________________
 |
 |
 |_____________________________________________________________________________
*/

/*______________________________________________
 |
 | Priority management;
 |______________________________________________
*/

static void IrqInsertThread(struct StageName *N);
static void irqThreadReinsert(struct StageName *P);

/*_____________________________________________________________________
 |
 | These procedures are used to insert/remove event/thread on the
 | timeline.
 |_____________________________________________________________________
*/

static priorityList *StageGetOrInsertPriority(struct Stage *stg,int lvl,int date) {
    TlsIntTrie *f;
    ActorDate *F;
    priorityList *r;
    dList *p,*n,*e;
    int found;
    f = TlsIntTrieSeek(&stg->Timeline,date);
    if (f==&stg->Timeline) {
        if (TlsIntTrieIsEmpty(&stg->Timeline)) { stg->DateRange.b=stg->DateRange.e=date; }
        F = Call(stg->DatePool,Alloc,0);
        ddListInit(&F->priorities);
        f = TlsIntTrieSeekOrInsert(&stg->Timeline,date,&F->Date);
        if ((date-stg->DateRange.b)<0) { stg->DateRange.b = date; }
        if ((stg->DateRange.e-date)<0) { stg->DateRange.e = date; }
    } else {
        F = CastBack(ActorDate,Date,f);
    }
    p = e = &F->priorities;
    n = e->n; found = (0!=0);
    while ((n!=e)&&(!found))  {
        r = CastBack(priorityList,dList,n);
        found = (r->lvl==lvl);
        if (!found) {
            if (r->lvl<lvl) { n=e; } else { p=n; n=n->n; }
        }
    }
    if (!found) {
        r = Call(stg->PriorityPool,Alloc,0);
        n = &r->dList;
        r->Date = F;
        r->lvl = lvl;
        ddListInit(&r->Actors);
        dListChainAfter(p,n);
    }
    return r;
}
static void IrqInsertThread(struct StageName *N) {
    priorityList *s;
    N->Running = Thread_Awake;
    N->priority = s = StageGetOrInsertPriority(N->Stage,N->Lvl,N->Date);
    dListChainBefore(&s->Actors,&N->dList);
}

static void StageNameExtract(struct StageName *P) {
    if (P->priority) {
        dList *p,*n;
        TlsIntTrie d;
        priorityList *pl;
        struct Stage *stg;
        stg = P->Stage;
        ddListUnchain(&P->dList);
        pl = P->priority;
        p = &pl->Actors;
        if (p->n==p) {
            ActorDate *ad;
            int date;
            ad = pl->Date;
            date = ad->Date.Key;
            p = &ad->priorities;
            ddListUnchain(&pl->dList);
            Call(stg->PriorityPool,Free,1(pl));
            if (p->n==p) { // no more actor at this date
                TlsIntTrieSeg nxt;
                if (stg->DateRange.e==date) {
                    TlsIntTrieBoundaries(&nxt,&stg->Timeline,date-1);
                    stg->DateRange.e=nxt.Begin->Key;
                }
                if (stg->DateRange.b==date) {
                    TlsIntTrieBoundaries(&nxt,&stg->Timeline,date);
                    stg->DateRange.b=nxt.End->Key;
                }
                TlsIntTrieRemove(&stg->Timeline,date);
                Call(stg->DatePool,Free,1(ad));
            }
        }
        P->priority=0;
    }
}

/*___________________________________________________
 |
 |___________________________________________________
*/

static void threadAwakenAbort(TlsSchMsg *this) {}
static void threadAwakenInterpret(TlsSchMsg *this,TlsActor *Tgt) {
    ThisToThat(struct StageName,Msg.Awaken);
    that->Alarm = 0;
    // The awakening of the thread is performed automatically by the performing of the event.
}

/*_____________________________________
 |
 | We use the following function when 
 | a thread date/running state might 
 | have change....
 |_____________________________________
*/

static void irqThreadReinsert(struct StageName *P) {
    if (!P->Busy) {
        StageNameExtract(P);
        if (P->Running == Thread_Awake) {
            IrqInsertThread(P);
        } else {
            if (P->Running == Thread_Dead) {
                Call(P->Stage->NamePool,Free,1(P));
            } else { // Asleep or Hanged thread.
                dListChainAfter(&P->Stage->Sleeping,&P->dList);
            }
        }
    } 
}

/*__________________________________________________
 |
 | Performing
 |__________________________________________________
*/

/*__________________________________________________
 |
 |  These are from current thread perspective:
 |__________________________________________________
*/

static TlsStage *SNGetStage(TlsStageName *this) {
    ThisToThat(struct StageName,TlsStageName);
    return &that->Stage->TlsStage;
}

static void SNAlterPriority(TlsStageName *this,TlsStageName *base,int offset) {
    struct StageName *Base;
    ThisToThat(struct StageName,TlsStageName);
    Base = CastBack(struct StageName,TlsStageName,base);
    that->Lvl = Base->Lvl+offset;
    irqThreadReinsert(that);
}

// Translate the date of the current thread to the target time.
// TimeLeft is measured in Tick to Deadline.
static int SNGetDate(TlsStageName *this,int MyDate) {
    ThisToThat(struct StageName,TlsStageName);
    return that->Date;
}

static int irqThreadSleep(int date,struct StageName *tgt,int delay) {
    int r;
    r = (tgt->Running==Thread_Awake);
    if (r) {
        tgt->Running = Thread_Asleep;
        if (!tgt->Alarm) tgt->Alarm = Call(&tgt->Stage->TlsStage,InsertEvent,3(date+delay,&tgt->Msg.Awaken,&tgt->TlsStageName));
    } else {
        r = (tgt->Running!=Thread_Dead);
        if (r) {
            Call(&tgt->TlsStageName,Awaken,1(date));
            tgt->Running = Thread_Asleep;
            if (!tgt->Alarm) tgt->Alarm = Call(&tgt->Stage->TlsStage,InsertEvent,3(date+delay,&tgt->Msg.Awaken,&tgt->TlsStageName));
        }
    }
    return r;
}
static void SNSleep(TlsStageName *this,int date,int delay) {
    ThisToThat(struct StageName,TlsStageName);
    irqThreadSleep(date,that,delay);
    irqThreadReinsert(that);
}

static void SNHang(TlsStageName *this){
    ThisToThat(struct StageName,TlsStageName);
    if ((that->Running == Thread_Awake)||(that->Running == Thread_Asleep)) {
        that->Running = Thread_Hanged;
    }
    if (that->Running == Thread_Hanged) {
        irqThreadReinsert(that);
    }
}

static void SNEnd(TlsStageName *this) {
    ThisToThat(struct StageName,TlsStageName);
    that->Running = Thread_Dead;
    // Remove Alarm Event
    if (that->Alarm != 0) {
        Call(that->Alarm,Remove,0);
        that->Alarm = 0;
    }
    // We should also remove the events pending on this thread...
    Call(this,FlushEvents,0);
    irqThreadReinsert(that);
}

static void SNAwaken(TlsStageName *this,int date) {
    ThisToThat(struct StageName,TlsStageName);
    if (that->Running == Thread_Hanged || that->Running == Thread_Asleep) {
        if (that->Alarm != 0) { // we have to remove the waking event...
            Call(that->Alarm,Remove,0);
            that->Alarm = 0;
        }
        if (that->Running == Thread_Hanged) {
            that->Running = Thread_Awake;
            that->Date = date;
            irqThreadReinsert(that);
        } else {
            that->Running = Thread_Awake;
            irqThreadReinsert(that);
        }
    }
}

static void SNFlushEvent(TlsStageName *this) {
    dList *l;
    ThisToThat(struct StageName,TlsStageName);
    l = that->PendingEvent.n;
    while (l!=&that->PendingEvent) {
        struct Event *L;
        L = CastBack(struct Event,dList,l);
        Call(&L->TlsSchEvent,Remove,0);
    }
}

/*----------------------------------------------------------------------------*/

static struct StageName *NewStageName(struct Stage *that,TlsActor *t,int date) {
    struct StageName *N;
    static struct TlsSchMsg AwakenStatic = {
        threadAwakenInterpret,threadAwakenAbort
    };
    static struct TlsStageName Static = {
        SNGetStage,SNAlterPriority,SNGetDate,SNSleep,
        SNHang,SNEnd,SNAwaken,SNFlushEvent
    };
    N = Call(that->NamePool,Alloc,0);
    N->TlsStageName.Static = &Static;
    N->Date = date;
    N->Actor = t;
    N->Running = Thread_Awake;
    N->Stage = that;
    N->Alarm = 0;
    N->priority = 0;
    N->Lvl=0;
    N->Busy = (0!=0);
    N->Msg.Awaken.Static = &AwakenStatic;
    ddListInit(&N->PendingEvent);
    return N;
}
static TlsStageName *StageEnterActor(TlsStage *this,TlsActor *t,int date) {
    struct StageName *n;
    ThisToThat(struct Stage,TlsStage);
    n = NewStageName(that,t,date);
    IrqInsertThread(n);
    return &n->TlsStageName; // &sStageEnterActor(that,t,date)->TlsStageName;
}

static int IrqEventPerform(TlsActor *this,int date) {
    int running,myDate;
    ThisToThat(struct Event,TlsActor);
    running = (that->Target->Running == Thread_Awake);
    myDate = that->Date->Date;
    if (!running) {
        SNAwaken(&that->Target->TlsStageName,myDate);
        // First bring the target up to date before issuing the event.
        running = (that->Target->Date - myDate)>=0;
    }
    if (running) {
        TlsActor *target;
        ddListUnchain(&that->dList);
        target = that->Target->Actor;
        Call(that->Msg,Interpret,1(target));
        Call(&that->Date->TlsStageName,End,0);
        Call(that->Stage->EventPool,Free,1(that));
    }
    return myDate;
}

static void IrqRemoveEvent(TlsSchEvent *this) {
    ThisToThat(struct Event,TlsSchEvent);
    Call(that->Msg,Abort,0);
    ddListUnchain(&that->dList);
    Call(&that->Date->TlsStageName,End,0);
    Call(that->Stage->EventPool,Free,1(that));
}

static TlsSchEvent *StageInsertEvent(
    TlsStage *this,int Date,TlsSchMsg *Msg,TlsStageName *tgt
) {
    static struct TlsActor ActorStatic = {IrqEventPerform};
    static struct TlsSchEvent EventStatic = {IrqRemoveEvent};
    struct Event *event;
    struct StageName *Tgt;
    ThisToThat(struct Stage,TlsStage);
    event = Call(that->EventPool,Alloc,0);
    event->TlsActor.Static = &ActorStatic;
    event->TlsActor.Id = "Evt";
    event->TlsSchEvent.Static = &EventStatic;
    event->Stage = that;
    event->Msg = Msg;
    Tgt = CastBack(struct StageName,TlsStageName,tgt);
    event->Target = Tgt;
    dListChainAfter(&Tgt->PendingEvent,&event->dList);
    event->Date = NewStageName(that,&event->TlsActor,Date);
    // Events ususally preempt their targeted thread.
    event->Date->Lvl = Tgt->Lvl+1;
    IrqInsertThread(event->Date);
    return &event->TlsSchEvent;
}

struct nxtDate {
    int Date,Lvl;
    priorityList *Priority;
};

static void nxtDate(struct nxtDate *r,TlsIntTrie *tme,int date) {
    TlsIntTrieSeg nxt;
    int dt;
    TlsIntTrieBoundaries(&nxt,tme,date);
    dt = nxt.End->Key-nxt.Begin->Key;
    if (dt>0) {
        ActorDate *ad;
        r->Date = nxt.End->Key;
        ad = CastBack(ActorDate,Date,nxt.End);
        r->Priority = CastBack(priorityList,dList,ad->priorities.n);
        r->Lvl = r->Priority->lvl;
    } else {
        r->Priority=0;
    }
}
static int StagePerform(TlsActor *this,int MaxEndDate) {
    int date,lvl;
    TlsIntTrie *tme;
    ThisToThat(struct Stage,TlsStage.TlsActor);
    date = that->DateRange.b;
    tme = &that->Timeline;
    while (((date-MaxEndDate)<0)&&(!TlsIntTrieIsEmpty(tme))) {
        TlsIntTrie *f;
        dList *n;
        struct nxtDate nxt;
        struct StageName *sn;
        ActorDate *ad;
        f = TlsIntTrieSeek(tme,date);
        ad = CastBack(ActorDate,Date,f);
        n = ad->priorities.n;
        nxt.Priority = CastBack(priorityList,dList,n);
        n = nxt.Priority->Actors.n;
        sn = CastBack(struct StageName,dList,n);
        lvl = sn->Lvl;
        do {
            nxt.Date = MaxEndDate;
            nxt.Lvl = lvl+1;
            nxtDate(&nxt,tme,date);
            date = nxt.Date;
        } while((lvl>=nxt.Lvl)&&(MaxEndDate-date>0));
        if (MaxEndDate-date<0) date=MaxEndDate;
        sn->Busy=(0==0);
        StageNameExtract(sn);
        sn->Date=Call(sn->Actor,Perform,1(date));
        sn->Busy=(0!=0);
        irqThreadReinsert(sn);
        date = that->DateRange.b;
    }
    return MaxEndDate;
}

/*  .... this union is just there for a size computation; */
typedef union {
    struct {
        TlsSchMsg TlsSchMsg;
        TlsMemPool *Pool;
        void (*Interpret)(char val);
        char Val;
    } Char;
    struct {
        TlsSchMsg TlsSchMsg;
        TlsMemPool *Pool;
        void (*Interpret)(int Idx,void *data);
        long data[5];
    } IdxPtr;
} IrqMsgSample;

TlsStage *TlsSchRoundRobin(char *Label) {
    MemStack *mem;
    TlsMemPool *Pools;
    struct Stage *OS;
    static struct TlsActor Actor = { StagePerform };
    static struct TlsStage Static = { StageEnterActor,StageInsertEvent};
    mem = rFork(1024);
    mPush(mem,OS);
    OS->TlsStage.Static = &Static;
    OS->TlsStage.TlsActor.Static = &Actor;
    OS->TlsStage.TlsActor.Id = Label;
    OS->Mem = mem;
    mIn(OS->Mem,Pools = TlsMemPoolNew(1024));
    OS->NamePool = Call(Pools,SizeSelect,1(TlsAtomSize(struct StageName)));
    OS->EventPool = Call(Pools,SizeSelect,1(TlsAtomSize(struct Event)));
    OS->MsgPool = Call(Pools,SizeSelect,1(TlsAtomSize(IrqMsgSample)));
    OS->DatePool = Call(Pools,SizeSelect,1(TlsAtomSize(ActorDate)));
    OS->PriorityPool = Call(Pools,SizeSelect,1(TlsAtomSize(priorityList)));
    ddListInit(&OS->Sleeping);
    TlsIntTrieInit(&OS->Timeline);
    OS->Priority.lvl=0;
    OS->DateRange.b = OS->DateRange.e = 0;
    return &OS->TlsStage;
}

/*___________________________________________________________
 |
 | Some generic messages.
 |___________________________________________________________
*/

static void MsgNullInterpret(TlsSchMsg *this,TlsActor *tgt) { }
static void MsgNullAbort(TlsSchMsg *this) { }
static struct TlsSchMsg MsgNullStatic = {MsgNullInterpret,MsgNullAbort};
TlsSchMsg TlsSchMsgNull = {&MsgNullStatic};

static void MsgGoFirstInterpret(TlsSchMsg *this,TlsActor *tgt) {
    Call(tgt,Perform,1(0));
}
static struct TlsSchMsg MsgGoFirstStatic = {MsgGoFirstInterpret,MsgNullAbort};
TlsSchMsg TlsSchMsgGoFirst = {&MsgGoFirstStatic};


/*--------------*/

#define MsgDefine1(tag,typ) \
typedef struct { \
    TlsSchMsg TlsSchMsg; \
    TlsMemPool *Pool;\
    void (*Fn)(typ P0); \
    typ P0; \
} msg##tag; \
static void msg##tag##Interpret(TlsSchMsg *this,TlsActor *tgt) { \
    ThisToThat(msg##tag,TlsSchMsg); \
    that->Fn(that->P0); \
    Call(that->Pool,Free,1(that)); \
} \
static void msg##tag##Abort(TlsSchMsg *this) { \
    ThisToThat(msg##tag,TlsSchMsg); \
    Call(that->Pool,Free,1(that)); \
} \
\
TlsSchMsg *TlsSchMsg##tag(TlsStageName *this,void (*Fn)(typ P0),typ P0) { \
    msg##tag *r; \
    static struct TlsSchMsg Static = {msg##tag##Interpret,msg##tag##Abort}; \
    ThisToThat(struct StageName,TlsStageName);\
    r = Call(that->Stage->MsgPool,Alloc,0); \
    r->TlsSchMsg.Static = &Static; \
    r->Pool = that->Stage->MsgPool;\
    r->Fn = Fn; \
    r->P0 = P0; \
    return &r->TlsSchMsg; \
} 

MsgDefine1(Char,char)
MsgDefine1(CharPtr,char *)
MsgDefine1(Int,int)
MsgDefine1(IntPtr,int *)
MsgDefine1(Ptr,void *)
MsgDefine1(PtrPtr,void **)

/*--------------*/

#define MsgDefine2(tag,typ0,typ1) \
typedef struct { \
    TlsSchMsg TlsSchMsg; \
    TlsMemPool *Pool;\
    void (*Fn)(typ0 P0,typ1 P1); \
    typ0 P0; \
    typ1 P1; \
} msg##tag; \
static void msg##tag##Interpret(TlsSchMsg *this,TlsActor *tgt) { \
    ThisToThat(msg##tag,TlsSchMsg); \
    that->Fn(that->P0,that->P1); \
    Call(that->Pool,Free,1(that));\
} \
static void msg##tag##Abort(TlsSchMsg *this) { \
    ThisToThat(msg##tag,TlsSchMsg); \
    Call(that->Pool,Free,1(that));\
} \
\
TlsSchMsg *TlsSchMsg##tag(TlsStageName *this,void (*Fn)(typ0 P0,typ1 P1),typ0 P0,typ1 P1) { \
    msg##tag *r; \
    static struct TlsSchMsg Static = {msg##tag##Interpret,msg##tag##Abort}; \
    ThisToThat(struct StageName,TlsStageName);\
    r = Call(that->Stage->MsgPool,Alloc,0); \
    r->TlsSchMsg.Static = &Static; \
    r->Pool = that->Stage->MsgPool; \
    r->Fn = Fn; \
    r->P0 = P0; \
    r->P1 = P1; \
    return &r->TlsSchMsg; \
} 

MsgDefine2(IdxChar,int,char)
MsgDefine2(IdxCharPtr,int,char *)
MsgDefine2(IdxInt,int,int)
MsgDefine2(IdxIntPtr,int,int *)
MsgDefine2(IdxPtr,int,void *)
MsgDefine2(IdxPtrPtr,int,void **)
MsgDefine2(SD,void *,void*)

