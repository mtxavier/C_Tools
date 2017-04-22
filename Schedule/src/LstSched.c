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

#include <Schedule.h>
#include <StackEnv.h>
#include <Classes.h>
#include <List.h>
#include <Tools.h>


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

static int ActorNullPerform(SchActor *this,int ticknb) {return 0;}
static struct SchActor ActorNullStatic = {ActorNullPerform};
SchActor SchActorNull = {&ActorNullStatic,"ThreadNull"};

/*_____________________________________________________________
 |
 | Psx threads
 |_____________________________________________________________
*/

typedef struct {
	dList dList;
	dList/*<IrqThreadDate>*/ date;
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
	SchStageName SchStageName;
	struct Stage *Stage;
	dList dList; // Inserted in priority list
	int Date;
	IrqThreadState Running;
	SchActor *Actor;
	priorityList *priority;
	// When the thread is asleep, chance is there is an alarm set to awaken it.
	// if the thread is awaken before the alarm has a chance to happen 
	// (or if the date of the alarm must be changed), we have to have a 
	// handle set to it.
	SchEvent *Alarm;
	struct {
		SchMsg Awaken;
	} Msg;
	dList/*<IrqEvent.dList>*/ PendingEvent;
};

struct Event {
	dList dList; // Inserted in a ThreadDate event queue
	SchEvent SchEvent;
	SchActor SchActor;
	struct Stage *Stage;
	struct StageName *Date; // used to interrupt or remove event.
	SchMsg *Msg;
	struct StageName *Target;
};

/*_________________________
 |
 |_________________________
*/

typedef union {
	struct {
		SchMsg SchMsg;
		TlsMemPool *Pool;
        void (*Interpret)(char val);
		char Val;
	} Char;
	/*  .... this union is just there for a size computation; let's just cut to the most expensive structure... */
	struct {
		SchMsg SchMsg;
		TlsMemPool *Pool;
		void (*Interpret)(int Idx,void *data);
		long data[5];
	} IdxPtr;
} IrqMsgSample;

/*__________________________________________________
 |
 |__________________________________________________
*/

struct Stage {
	SchStage SchStage;
	MemStack *Mem;
	TlsMemPool *NamePool;
	TlsMemPool *EventPool;
	TlsMemPool *MsgPool;
	struct {
	    dList Pool;
		priorityList *main;
	} Priority;
	dList Sleeping;
	SchStageName *current,*null;
	int Org,Deadline,IrqDate; // Org is only there to avoid overflow on date. 
	                   // The usual way to compare date would be (x0-org)<=(x1-org) (also (x0-x1)<=0)
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

static void IrqInsertThread(priorityList *s,struct StageName *N);
static void irqThreadReinsert(struct StageName *P);

static priorityList *NewLowestPriority(struct Stage *OS) {
    priorityList *n;
	mPush(OS->Mem,n);
	ddListInit(&n->date);
    dListChainBefore(&OS->Priority.Pool,&n->dList);
	return n;
}
static priorityList *NewHighestPriority(struct Stage *OS) {
	priorityList *n;
	mPush(OS->Mem,n);
	ddListInit(&n->date);
	dListChainAfter(&OS->Priority.Pool,&n->dList);
	return n;
}

/*______________________________________________
 |
 |______________________________________________
*/

static void IrqInsertThread(priorityList *s,struct StageName *N) {
	struct StageName *P;
	dList *p;
	int found;
	N->Running = Thread_Awake;
	N->priority = s;
	p = &s->date;
	found = (p->n==&s->date);
	while ((p->n!=&s->date)&&(!found)) {
		P = CastBack(struct StageName,dList,p->n);
		found = ((N->Date-P->Date)<=0);
		p = &P->dList;
	}
	dListChainBefore(p,&N->dList);
}

static void threadAwakenAbort(SchMsg *this) {}
static void threadAwakenInterpret(SchMsg *this,SchActor *Tgt) {
	ThisToThat(struct StageName,Msg.Awaken);
	that->Alarm = 0;
	// The awakening of the thread is performed automatically by the performing of the event.
	// IrqThreadAwaken((that->Stage->Deadline-that->Stage->current->Date),that);
}

/*_____________________________________
 |
 | We use the following function when 
 | a thread date/running state might 
 | have change....
 |_____________________________________
*/

static void irqThreadReinsert(struct StageName *P) {
    ddListUnchain(&P->dList); // the date/state of the thread has been altered, we reinsert it.
	if (P->Running == Thread_Awake) {
        IrqInsertThread(P->priority,P);
	} else {
		if (P->Running == Thread_Dead) {
			Call(P->Stage->NamePool,Free,1(P));
		} else { // Asleep or Hanged thread.
		    dListChainAfter(&P->Stage->Sleeping,&P->dList);
		}
    } 
}

static int irqThreadPerform(struct StageName *P,int TickNb) {
	int tlft,start;
	struct Stage *os;
	os = P->Stage;
	start = os->Deadline;
	os->Deadline = P->Date+TickNb;
	os->current = &P->SchStageName;
    tlft = Call(P->Actor,Perform,1(TickNb));
	P->Date = P->Date+TickNb-tlft;
	irqThreadReinsert(P);
	os->Deadline = start;
	return tlft;
}

/*__________________________________________________
 |
 | Performing
 |__________________________________________________
*/


// priorityLevelPerform():
// update all the threads in that priority level up to TickNb.
// Returns the ticknb left by the thread that lag the most behind.
// There are various way this functions might end:
// 1) all the thread in that level are up to date: return 0.
// 2) all threads in that level have died or gone asleep.
// 3) an interrupt has been issued. The lowest interrupt date is
//    kept in os->IrqDate

int priorityLevelPerform(struct Stage *os,priorityList *pl,int TickNb) {
	int r;
	int StartDate,EndDate;
	StartDate = os->Deadline;
	EndDate = StartDate+TickNb;
	{
		dList *p,*endthr;
		endthr = &pl->date;
		p = endthr->n;
		if (p==endthr) { // no threads in that level.
			r = 0;
		} else {
			struct StageName *P;
			int rDate;
			rDate = EndDate;
            os->IrqDate = EndDate;
			P = CastBack(struct StageName,dList,p);
			while ((p!=endthr)&&(TickNb>0)&&((P->Date-EndDate)<0)) {
				int tleft;
				tleft = irqThreadPerform(P,EndDate-P->Date);
				if (((EndDate-tleft)-rDate)<0) rDate = EndDate-tleft;
				if ((os->IrqDate-EndDate)<0) {
					// if one interruption has been provoked in this thread, 
					// the remaining threads won't be performed beyond this 
					// point... But we can't go back on the 
					// threads performed till then.
					EndDate = os->IrqDate;
				}
				// Since there might be a lot of moving around during the 
				// performance of the current thread, it can't be guaranted 
				// that p->n is the actual next thread to run.
				p = endthr->n;
				if (p!=endthr) P = CastBack(struct StageName,dList,p);
			}
			r = StartDate+TickNb-rDate;
		}
	} 
    os->Org = StartDate + TickNb - r;
	return r;
}

int priorityListPerform(struct Stage *os,priorityList *pl,int TickNb) {
	int r;
	int StartDate;
	dList *nl;
	StartDate = os->Deadline;
	nl = pl->dList.n;
	if (nl==&os->Priority.Pool) {// We're on the lowest priority level.
        r = priorityLevelPerform(os,pl,TickNb);
	} else {// nl points to some kind of lower priority level.
        struct StageName *P;
		dList *endthr,*p;
		priorityList *NL;
		NL = CastBack(priorityList,dList,nl);
		endthr = &pl->date;
		p = endthr->n;
		if (p==endthr) { // no thread in that priority level... perform the 
				         // next.
			r = priorityListPerform(os,NL,TickNb);
		} else {
			P = CastBack(struct StageName,dList,p);
			if ((StartDate+TickNb-P->Date)<=0) { // all threads in that 
					 // priority level have already passed that point...
				r = priorityListPerform(os,NL,TickNb);
			} else { 
				int deadline,tleft;
                deadline = os->IrqDate = P->Date;
				tleft = deadline-StartDate;
				// First bring the lower priority level up to date.
				while (((os->IrqDate-deadline)>=0)&&(tleft>0)) {
                     tleft = priorityListPerform(os,NL,deadline-StartDate);
				}
				if ((os->IrqDate-deadline)<0) {
					// some thread has been awaken; might be of higher 
					// priority, so we return.
					r += StartDate+TickNb-deadline;
				} else {
					// we only perform the threads that date matches the 
					// current date. They will usually send
					// interrupt signals to the lower priority levels. 
					// Interrupt signals must often be treated
					// sequentially so we have to give lower priority threads 
					// the opportunity to react.
					int base,tlft,edate;
					base = deadline;
					edate = deadline = StartDate + TickNb;
					while ((P->Date==base)&&(p!=endthr)) {
				        tlft = irqThreadPerform(P,deadline-base);
						if ((deadline-tlft)-edate<0) edate = deadline-tlft;
						if (((os->IrqDate-deadline)<0)&&(base-os->IrqDate<0)) {
							deadline = os->IrqDate;
						}
						p = endthr->n;
                        if (p!=endthr) P = CastBack(struct StageName,dList,p);
					}
					if (p!=endthr) { 
						if ((P->Date-edate)<0) {
							edate = P->Date;
						}
					}
					r = StartDate+TickNb-edate;
					// For readability purpose, we leave the performing from 
					// first date to second date to the
					// later iteration of this procedure...
				}
			}
		}
	}
    os->Org = StartDate + TickNb - r;
	return r;
}

/*__________________________________________________
 |
 |  These are from current thread perspective:
 |__________________________________________________
*/

static SchStage *SNGetStage(SchStageName *this) {
	ThisToThat(struct StageName,SchStageName);
	return &that->Stage->SchStage;
}

static void SNAlterPriority(SchStageName *this,SchStageName *base,int offset) {
	dList *endp,*n;
	priorityList *P;
	struct Stage *OS;
	struct StageName *Base;
	ThisToThat(struct StageName,SchStageName);
	Base = CastBack(struct StageName,SchStageName,base);
	OS = that->Stage;
	endp = &OS->Priority.Pool;
    n = &Base->priority->dList;
	while (offset>0 && n!=endp) {
		n = n->p;
		if (n!=endp) offset--;
	}
	while (offset>0) {
		offset--;
		n = &NewHighestPriority(OS)->dList;
	}
	while (offset<0 && n!=endp) {
		n = n->n;
		if (n!=endp) offset++;
	}
	while (offset<0) {
		offset++;
		n = &NewLowestPriority(OS)->dList;
	}
    P = CastBack(priorityList,dList,n);
	that->priority = P;
	irqThreadReinsert(that);
}

// Translate the date of the current thread to the target time.
// TimeLeft is measured in Tick to Deadline.
static int SNGetDate(SchStageName *this,int TimeLeft) {
	struct StageName *current;
	ThisToThat(struct StageName,SchStageName);
	current = CastBack(struct StageName,SchStageName,that->Stage->current);
	return TimeLeft + (current->Date-that->Date);
}

static int irqThreadSleep(int Timeleft,struct StageName *tgt,int delay) {
	int r;
	r = (tgt->Running==Thread_Awake);
	if (r) {
		tgt->Running = Thread_Asleep;
		if (!tgt->Alarm) tgt->Alarm = Call(&tgt->Stage->SchStage,InsertEvent,3(Timeleft-delay,&tgt->Msg.Awaken,&tgt->SchStageName));
	} else {
		r = (tgt->Running!=Thread_Dead);
		if (r) {
			Call(&tgt->SchStageName,Awaken,1(Timeleft));
			tgt->Running = Thread_Asleep;
            if (!tgt->Alarm) tgt->Alarm = Call(&tgt->Stage->SchStage,InsertEvent,3(Timeleft-delay,&tgt->Msg.Awaken,&tgt->SchStageName));
	    }
	}
	return r;
}
static void SNSleep(SchStageName *this,int Timeleft,int delay) {
	int alive;
	ThisToThat(struct StageName,SchStageName);
	alive = irqThreadSleep(Timeleft,that,delay);
	if (this!=that->Stage->current) {
		irqThreadReinsert(that);
	}
}

static void SNHang(SchStageName *this){
	ThisToThat(struct StageName,SchStageName);
	if ((that->Running == Thread_Awake)||(that->Running == Thread_Asleep)) {
		that->Running = Thread_Hanged;
	}
	if ((that->Running == Thread_Hanged)&&(this!=that->Stage->current)) {
        irqThreadReinsert(that);
	}
}

static void SNEnd(SchStageName *this) {
	ThisToThat(struct StageName,SchStageName);
	that->Running = Thread_Dead;
	// Remove Alarm Event
    if (that->Alarm != 0) {
		Call(that->Alarm,Remove,0);
		that->Alarm = 0;
	}
	// We should also remove the events pending on this thread...
	Call(this,FlushEvents,0);
	if (this!=that->Stage->current) {
		irqThreadReinsert(that);
	}
}

static void SNAwaken(SchStageName *this,int TimeLeft) {
	ThisToThat(struct StageName,SchStageName);
	if (that->Running == Thread_Hanged || that->Running == Thread_Asleep) {
        if (that->Alarm != 0) { // we have to remove the waking event...
			Call(that->Alarm,Remove,0);
			that->Alarm = 0;
		}
	    if (that->Running == Thread_Hanged) {
		    that->Running = Thread_Awake;
		    that->Date = that->Stage->Deadline-TimeLeft;
		    irqThreadReinsert(that);
	    } else {
		    that->Running = Thread_Awake;
		    irqThreadReinsert(that);
	    }
	}
}

static void SNFlushEvent(SchStageName *this) {
	dList *l;
	ThisToThat(struct StageName,SchStageName);
	l = that->PendingEvent.n;
	while (l!=&that->PendingEvent) {
		struct Event *L;
		L = CastBack(struct Event,dList,l);
		Call(&L->SchEvent,Remove,0);
	}
}

/*----------------------------------------------------------------------------*/

static struct StageName *sStageEnterActor(struct Stage *that,SchActor *t) {
	struct StageName *N;
	static struct SchMsg AwakenStatic = {
		threadAwakenInterpret,threadAwakenAbort
	};
	static struct SchStageName Static = {
		SNGetStage,SNAlterPriority,SNGetDate,SNSleep,
		SNHang,SNEnd,SNAwaken,SNFlushEvent
	};
    N = Call(that->NamePool,Alloc,0);
	N->SchStageName.Static = &Static;
	N->Date = that->Deadline;
	N->Actor = t;
	N->Running = Thread_Awake;
	N->Stage = that;
	N->Alarm = 0;
	N->Msg.Awaken.Static = &AwakenStatic;
	ddListInit(&N->PendingEvent);
	IrqInsertThread(that->Priority.main,N);
    return N;
}
static SchStageName *StageEnterActor(SchStage *this,SchActor *t) {
	ThisToThat(struct Stage,SchStage);
	return &sStageEnterActor(that,t)->SchStageName;
}

static int IrqEventPerform(SchActor *this,int TickNb) {
	int running;
	ThisToThat(struct Event,SchActor);
	running = (that->Target->Running == Thread_Awake);
	if (!running) {
		SNAwaken(&that->Target->SchStageName,TickNb);
		running = (that->Target->Date - (that->Target->Stage->Deadline-TickNb))>=0;
	}
	if (running) {
		// First bring the target up to date before issuing the event.
		SchActor *target;
	    ddListUnchain(&that->dList);
		target = that->Target->Actor;
        Call(that->Msg,Interpret,1(target));
		Call(&that->Date->SchStageName,End,0);
		Call(that->Stage->EventPool,Free,1(that));
	}
	return TickNb;
}

static void IrqRemoveEvent(SchEvent *this) {
	ThisToThat(struct Event,SchEvent);
	Call(that->Msg,Abort,0);
	ddListUnchain(&that->dList);
	Call(&that->Date->SchStageName,End,0);
	Call(that->Stage->EventPool,Free,1(that));
}

static SchEvent *StageInsertEvent(
	SchStage *this,int Timeleft,SchMsg *Msg,SchStageName *tgt
) {
	static struct SchActor ActorStatic = {IrqEventPerform};
	static struct SchEvent EventStatic = {IrqRemoveEvent};
	struct Event *event;
	struct StageName *Tgt;
	ThisToThat(struct Stage,SchStage);
	event = Call(that->EventPool,Alloc,0);
	event->SchActor.Static = &ActorStatic;
	event->SchActor.Id = "Evt";
	event->SchEvent.Static = &EventStatic;
	event->Stage = that;
	event->Msg = Msg;
	Tgt = CastBack(struct StageName,SchStageName,tgt);
	event->Target = Tgt;
	dListChainAfter(&Tgt->PendingEvent,&event->dList);
	event->Date = sStageEnterActor(that,&event->SchActor);
	event->Date->Date = that->Deadline - Timeleft; // (Hack: this thread will be reinserted in IrqSetPriority)
	if ((event->Date->Date-that->IrqDate)<0) {
		that->IrqDate = event->Date->Date;
	}
	Call(&event->Date->SchStageName,AlterPriority,2(tgt,+1));
	// Events ususally preempt their targeted thread.
	return &event->SchEvent;
}

static int StagePerform(SchActor *this,int TickNb) {
	dList *p;
	priorityList *P;
	int r;
	ThisToThat(struct Stage,SchStage.SchActor);
    while (TickNb>0) {
	     P =  CastBack(priorityList,dList,that->Priority.Pool.n); 
		 // new priority level might appear each iteration
	     r = priorityListPerform(that,P,TickNb);
		 that->Deadline = that->Deadline+TickNb-r;
         TickNb = r;
	}
	return r;
}

SchStage *SchRoundRobin(char *Label) {
	MemStack *mem;
	TlsMemPool *Pools;
	struct Stage *OS;
	static struct SchActor Actor = { StagePerform };
	static struct SchStage Static = { StageEnterActor,StageInsertEvent};
	mem = rFork(1024);
    mPush(mem,OS);
	OS->SchStage.Static = &Static;
	OS->SchStage.SchActor.Static = &Actor;
	OS->SchStage.SchActor.Id = Label;
	OS->Mem = mem;
	mIn(OS->Mem,Pools = TlsMemPoolNew(1024));
	OS->NamePool = Call(Pools,SizeSelect,1(sizeof(struct StageName)));
	OS->EventPool = Call(Pools,SizeSelect,1(sizeof(struct Event)));
	OS->MsgPool = Call(Pools,SizeSelect,1(sizeof(IrqMsgSample)));
	ddListInit(&OS->Sleeping);
	ddListInit(&OS->Priority.Pool);
	mPush(OS->Mem,OS->Priority.main);
	ddListInit(&OS->Priority.main->date);
	dListChainAfter(&OS->Priority.Pool,&OS->Priority.main->dList);
	OS->Deadline = 0;
	OS->IrqDate = 0;
	OS->Org = 0;
	OS->current = OS->null = &sStageEnterActor(OS,&SchActorNull)->SchStageName;
	return &OS->SchStage;
}

/*____________________________________________________________
 |
 | Specialized Thread : 'Real' thread.
 |
 |____________________________________________________________
*/

#include <Thd.h>

struct Thread {
    ThdItf ThdItf;
	SchActor SchActor;
	SchStageName SchStageName;
	ThdThread *Thread;
	SchStageName *StageName;
	SchThread *Main;
	int TimeLeft;
};

static SchStage *ThdGetStage(SchStageName *this) {
	ThisToThat(struct Thread,SchStageName);
	return Call(that->StageName,GetStage,0);
}
static void ThdAlterPriority(SchStageName *this,SchStageName *ref,int offset) {
	ThisToThat(struct Thread,SchStageName);
	Call(that->StageName,AlterPriority,2(ref,offset));
}
static int ThdGetDate(SchStageName *this,int SliceLeft) {
	ThisToThat(struct Thread,SchStageName);
	that->TimeLeft = SliceLeft;
	Call(that->Thread,Wait,0);
	return that->TimeLeft;
}
static void ThdSleep(SchStageName *this,int date,int delay){
	ThisToThat(struct Thread,SchStageName);
	Call(that->StageName,Sleep,2(date,delay));
}
static void ThdHang(SchStageName *this) {
	ThisToThat(struct Thread,SchStageName);
	Call(that->StageName,Hang,0);
}
static void ThdEnd(SchStageName *this) {
	ThisToThat(struct Thread,SchStageName);
	Call(that->StageName,End,0);
}
static void ThdAwaken(SchStageName *this,int date) {
	ThisToThat(struct Thread,SchStageName);
	Call(that->StageName,Awaken,1(date));
}
static void ThdFlushEvents(SchStageName *this) {
	ThisToThat(struct Thread,SchStageName);
	Call(that->StageName,FlushEvents,0);
}

static void ThreadMain(ThdItf *this,ThdThread *res) {
	ThisToThat(struct Thread,ThdItf);
	that->Thread = res;
	Call(res,Wait,0);
	Call(that->Main,Main,1(&that->SchStageName));
}
static int ThdThreadPerform(SchActor *this,int Slice) {
	ThisToThat(struct Thread,SchActor);
	that -> TimeLeft = Slice;
	Call(that->Thread,Wait,0);
	return that->TimeLeft;
}

void SchFiberLaunch(SchStage *Stage,char *Label,SchThread *Main,int StackSize,int pGrowth,int rGrowth) {
	struct Thread *r;
	static struct ThdItf ItfStatic = {ThreadMain};
	static struct SchActor Static = {ThdThreadPerform};
	static struct SchStageName NameStatic = {
		ThdGetStage,ThdAlterPriority,ThdGetDate,ThdSleep,ThdHang,ThdEnd,ThdAwaken,ThdFlushEvents
	};
	rPush(r);
	r->ThdItf.Static = &ItfStatic;
	r->SchActor.Static = &Static;
	r->SchActor.Id = Label;
	r->SchStageName.Static = &NameStatic;
	r->TimeLeft = 0;
	r->Main = Main;
	r->Thread = ThdEnvFiberLaunch(&r->ThdItf,StackSize,pGrowth,rGrowth);
	r->StageName = Call(Stage,EnterActor,1(&r->SchActor));
	Call(r->Thread,Wait,0);
} 

/*___________________________________________________________
 |
 | Some generic messages.
 |___________________________________________________________
*/

static void MsgNullInterpret(SchMsg *this,SchActor *tgt) { }
static void MsgNullAbort(SchMsg *this) { }
static struct SchMsg MsgNullStatic = {MsgNullInterpret,MsgNullAbort};
SchMsg SchMsgNull = {&MsgNullStatic};

static void MsgGoFirstInterpret(SchMsg *this,SchActor *tgt) {
	Call(tgt,Perform,1(0));
}
static struct SchMsg MsgGoFirstStatic = {MsgGoFirstInterpret,MsgNullAbort};
SchMsg SchMsgGoFirst = {&MsgGoFirstStatic};


/*--------------*/

#define MsgDefine1(tag,typ) \
typedef struct { \
    SchMsg SchMsg; \
	TlsMemPool *Pool;\
    void (*Fn)(typ P0); \
	typ P0; \
} msg##tag; \
static void msg##tag##Interpret(SchMsg *this,SchActor *tgt) { \
	ThisToThat(msg##tag,SchMsg); \
	that->Fn(that->P0); \
    Call(that->Pool,Free,1(that)); \
} \
static void msg##tag##Abort(SchMsg *this) { \
	ThisToThat(msg##tag,SchMsg); \
	Call(that->Pool,Free,1(that)); \
} \
\
SchMsg *SchMsg##tag(SchStageName *this,void (*Fn)(typ P0),typ P0) { \
	msg##tag *r; \
	static struct SchMsg Static = {msg##tag##Interpret,msg##tag##Abort}; \
	ThisToThat(struct StageName,SchStageName);\
    r = Call(that->Stage->MsgPool,Alloc,0); \
	r->SchMsg.Static = &Static; \
	r->Pool = that->Stage->MsgPool;\
	r->Fn = Fn; \
	r->P0 = P0; \
	return &r->SchMsg; \
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
    SchMsg SchMsg; \
	TlsMemPool *Pool;\
    void (*Fn)(typ0 P0,typ1 P1); \
	typ0 P0; \
	typ1 P1; \
} msg##tag; \
static void msg##tag##Interpret(SchMsg *this,SchActor *tgt) { \
	ThisToThat(msg##tag,SchMsg); \
	that->Fn(that->P0,that->P1); \
	Call(that->Pool,Free,1(that));\
} \
static void msg##tag##Abort(SchMsg *this) { \
	ThisToThat(msg##tag,SchMsg); \
	Call(that->Pool,Free,1(that));\
} \
\
SchMsg *SchMsg##tag(SchStageName *this,void (*Fn)(typ0 P0,typ1 P1),typ0 P0,typ1 P1) { \
	msg##tag *r; \
	static struct SchMsg Static = {msg##tag##Interpret,msg##tag##Abort}; \
	ThisToThat(struct StageName,SchStageName);\
    r = Call(that->Stage->MsgPool,Alloc,0); \
	r->SchMsg.Static = &Static; \
	r->Pool = that->Stage->MsgPool; \
	r->Fn = Fn; \
	r->P0 = P0; \
	r->P1 = P1; \
	return &r->SchMsg; \
} 

MsgDefine2(IdxChar,int,char)
MsgDefine2(IdxCharPtr,int,char *)
MsgDefine2(IdxInt,int,int)
MsgDefine2(IdxIntPtr,int,int *)
MsgDefine2(IdxPtr,int,void *)
MsgDefine2(IdxPtrPtr,int,void **)
MsgDefine2(SD,void *,void*)

