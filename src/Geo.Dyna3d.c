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
#include <List.h>
#include <Geo.h>
#include <TlsSched.h>

static long long Isqrt(long long a) {
    long long p,pp,inc,r;
    if (a<16) {
        static int appro[16] = {0,1,1,1,2,2,2,2,2,3,3,3,3,3,3,3};
        r = (a<=0)?0:appro[a];
    } else {
        /* 1st appro */{
            int lg2,l;
            static long long appro[16] = {
                     0, 65536, 92682,113512,
                131072,146543,160530,173392,
                185364,196608,207243,217358,
                227023,236293,245213,253820
            };
            TlsiLg2(long long,lg2,a);
            l = (lg2&-2)-2;
            pp = appro[a>>l];
            if (l<32) { pp=pp>>((32-l)>>1);} else { pp=pp<<((l-32)>>1); }
        }
        p = a/pp;
        // r stands for a value of a in the range [(r*r),(r*r)+(2*r)]
        // p*pp is a value in the range [1+(r*r)-pp,(r*r)+(2*r)]
        while ((pp>p+1)||(p>pp+1)) {
            pp = (p+pp)>>1; p = a/pp;
        }
        r = (pp<p)?pp:p;
        pp = r*r; inc = 1+(r<<1);
        while ((pp+inc)<=a) { pp+=inc; inc+=2; r++; }
    }
    return r;
}

typedef struct {
    struct TlsI3dDynScene *Static;
} TlsI3dDynScene;
typedef struct {
    struct TlsI3dDynObjHandle *Static;
} TlsI3dDynObjHandle;

typedef struct {
    /* Linear interpolation is expected here. Chose a sufficiently small delta to ensure this. */
    struct {
        TlsI3dReferential Ref;
        int t;
    } P0,P1;
} TlsI3dSolidMove;
typedef struct {
    struct TlsI3dDynObject *Static;
    TlsISphere Diameter;  // Approximate bounding location.
} TlsI3dDynObject;

typedef struct {
    int t;
    int event; // 0: entrance contact, 1: fully immersed, 2: Leave contact, 3: A fully out.
    TlsI3dPoint O; // contact point in the global referential.
    TlsI3dVec dV; // vB-vA at the intersection point. (usually not vB-vA !)
    struct {
        TlsI3dDynObject *Obj;
        TlsI3dPoint O;
        TlsI3dVec n;  // Intersection point and normal vector in the object referential.
        TlsI3dReferential Ref;
    } A,B;
} TlsI3dDynIntersectEvnt;

struct TlsI3dDynObject {
    /* in the Intersect event, the current object shall appears as A. */
    void (*CollisionTest)(TlsI3dDynObject *this,TlsI3dDynObject *B,void (*record)(TlsI3dDynIntersectEvnt *prm,void *Clos),void *clos);
    /* Next is called by the scene at the end of the move. The object is expected to update
     the movement for the next quantum of time. Notice that the object itself chose what its
     next quantum of time will be. That is, object with convoluted or sharp movements should
     chose short quantum of times while object with straight movements should chose longer 
     quantum of time. */
    int (*Next)(TlsI3dDynObject *this,TlsI3dSolidMove *nxt);
    void (*GetPos)(TlsI3dDynObject *this,TlsI3dReferential *R,int t);
    /* The object shouldn't peculiarly expect to be in the A or B position. 0, 1, 2... objects
    might see there movement altered during the reaction. Also objects might disappear, be 
    fused or appear during the reaction. Objects should use their handle to signal the 
    changes occuring. These changes will only take effect after the reaction function has returned. */
    void (*React)(TlsI3dDynObject *this,TlsI3dDynIntersectEvnt *evt);
};
extern TlsI3dDynObject TlsI3dDynObjectNull;

struct TlsI3dDynObjHandle {
    void (*Remove)(TlsI3dDynObjHandle *this);
    void (*Move)(TlsI3dDynObjHandle *this,TlsI3dSolidMove *dO);
    TlsI3dDynObject *(*GetObj)(TlsI3dDynObjHandle *this);
};
extern TlsI3dDynObjHandle TlsI3dDynObjHandleNull;

struct TlsI3dDynScene {
    TlsI3dDynObjHandle *(*AddObject)(TlsI3dDynScene *this,TlsI3dDynObject *Ob,TlsI3dSolidMove *Move);
    int (*Perform)(TlsI3dDynScene *this,int dt);
    /* Objects View: if Area is null, all the objects will be listed.  */
    void (*ObjectsView)(TlsI3dDynScene *this,TlsISphere *Area,int t,
          void (*Displ)(TlsI3dReferential *O,TlsI3dDynObject *obj,void *Clos),void *Clos
    );
};

/*_________________________________________________________________
 |
 |
 |_________________________________________________________________
*/

static void DynObjectNullCollisionTest(
    TlsI3dDynObject *this,TlsI3dDynObject *B,void (*record)(TlsI3dDynIntersectEvnt *prm,void *Clos),void *clos
) { }
static int DynObjectNullNext(TlsI3dDynObject *this,TlsI3dSolidMove *nxt){return (0!=0); }
static void DynObjectNullGetPos(TlsI3dDynObject *this,TlsI3dReferential *R,int t) {
    static TlsI3dReferential res = {{0,0,0},{0,0,0,0x1000000}};
}
static void DynObjectNullReact(TlsI3dDynObject *this,TlsI3dDynIntersectEvnt *evt) {}
static struct TlsI3dDynObject DynObjectNullStatic = {DynObjectNullCollisionTest,DynObjectNullNext,DynObjectNullGetPos,DynObjectNullReact};

TlsI3dDynObject TlsI3dDynObjectNull = {&DynObjectNullStatic,{{0,0,0},0}};

    /*-------------------------*/

static void HandleNullRemove(TlsI3dDynObjHandle *this) {}
static void HandleNullMove(TlsI3dDynObjHandle *this,TlsI3dSolidMove *dO){}
static TlsI3dDynObject *HandleNullGetObj(TlsI3dDynObjHandle *this) { return &TlsI3dDynObjectNull; }
static struct TlsI3dDynObjHandle HandleNullStatic = {
    HandleNullRemove,HandleNullMove,HandleNullGetObj
};
TlsI3dDynObjHandle TlsI3dDynObjHandleNull;

/*_________________________________________________________________
 |
 |
 |_________________________________________________________________
*/

typedef struct {
    TlsI3dDynScene TlsI3dDynScene;
    TlsNeighObject TlsNeighObject;
    int date;
    TlsISphere ViewArea;
    TlsStage *Sequence;
    TlsI3dNeighbourhood *Set;
    TlsMemPool *HandlePool,*CollidePool;
} DynScene;

typedef struct {
    TlsI3dDynObjHandle TlsI3dDynObjHandle;
    TlsNeighObject TlsNeighObject;
    TlsICylinder Bounding;
    TlsI3dDynObject *Obj;
    TlsI3dNeighHandle *Neighbours;
    TlsI3dSolidMove Pos;
    dList/*<CollideEvents.A/BColl>*/ Collides;
    int date;
    DynScene *Scene;
    TlsActor TlsActor;
    TlsStageName *StageName;
} HandleHook;

typedef struct {
    TlsSchMsg TlsSchMsg;
    TlsSchEvent *Abort;
    TlsI3dDynIntersectEvnt Prm;
    DynScene *Scene;
    dList/**/ AColl,BColl;
    HandleHook *A,*B;
} CollideEvt;

struct ObjectCheckIntersect {
    DynScene *Scene;
    int Date;
    HandleHook *ObjA,*ObjB;
};
static void CollideEvtInterpret(TlsSchMsg *this,TlsActor *Tgt);
static void CollideEvtAbort(TlsSchMsg *this);
static void ObjectRecordIntersect(TlsI3dDynIntersectEvnt *prm,void *Clos) {
    CollideEvt *r;
    static struct TlsSchMsg Static = { CollideEvtInterpret,CollideEvtAbort };
    struct ObjectCheckIntersect *clos;
    dList *l,*n;
    clos = Clos;
    r =  Call(clos->Scene->CollidePool,Alloc,0);
    r->TlsSchMsg.Static = &Static;
    r->Scene = clos->Scene;
    r->Prm=*prm;
    r->A=clos->ObjA;
    r->B=clos->ObjB;
    l=&(clos->ObjA->Collides); n=&r->AColl; 
    n->p=l; n->n=l->n; n->p->n = n->n->p = n;
    l=&(clos->ObjB->Collides); n=&r->BColl;
    n->p=l; n->n=l->n; n->p->n = n->n->p = n;
    r->Abort = Call(clos->Scene->Sequence,InsertEvent,3(clos->Date,&r->TlsSchMsg,clos->ObjA->StageName));
}
static int ObjectCheckIntersect(TlsI3dNeighHandle *ObjB,void *Clos) {
    struct ObjectCheckIntersect *clos;
    HandleHook *objB;
    clos = Clos;
    clos->ObjB = objB = CastBack(HandleHook,TlsNeighObject,ObjB->Obj);
    Call(clos->ObjA->Obj,CollisionTest,3(objB->Obj,ObjectRecordIntersect,clos));
    return (0==0);
}

static void DynObjectRemove(HandleHook *rmv) {
    Call(rmv->Neighbours,Remove,0);
    Call(rmv->StageName,FlushEvents,0);
    Call(rmv->StageName,End,0);
    Call(rmv->Scene->HandlePool,Free,1(rmv));
}
static void DynObjectMoveBound(TlsISphere *loc,HandleHook *mv) {
    long long r2,x;
    mv->Bounding.Base = mv->Obj->Diameter;
    TlsI3dVecDiff(mv->Bounding.d,mv->Pos.P0.Ref.O,mv->Pos.P1.Ref.O);
    loc->O.x[0]=(mv->Pos.P0.Ref.O.x[0]+mv->Pos.P1.Ref.O.x[0])>>1;
    loc->O.x[1]=(mv->Pos.P0.Ref.O.x[1]+mv->Pos.P1.Ref.O.x[1])>>1;
    loc->O.x[2]=(mv->Pos.P0.Ref.O.x[2]+mv->Pos.P1.Ref.O.x[2])>>1;
    x = mv->Bounding.d.x[0]; r2 = x*x;
    x = mv->Bounding.d.x[1]; r2 += x*x;
    x = mv->Bounding.d.x[2]; r2 += x*x;
    x = (Isqrt(r2)+1)>>1;
    loc->r = x+mv->Bounding.Base.r;
}
static void DynObjectMove(HandleHook *mv) {
    TlsISphere loc;
    struct ObjectCheckIntersect clos;
    DynObjectMoveBound(&loc,mv);
    Call(mv->Neighbours,Relocate,1(&loc));
    Call(mv->StageName,FlushEvents,0);
    clos.Scene = mv->Scene;
    clos.ObjA = mv;
    Call(mv->Neighbours,ForEachNeighbour,2(ObjectCheckIntersect,&clos));
}
static void CollideEvtInterpret(TlsSchMsg *this,TlsActor *Tgt) {
    dList *l;
    HandleHook *p;
    ThisToThat(CollideEvt,TlsSchMsg);
    l = &that->AColl;
    l->n->p=l->p; l->p->n=l->n; l->p=l->n=l;
    l = &that->BColl;
    l->n->p=l->p; l->p->n=l->n; l->p=l->n=l;
    Call(that->A->Obj,React,1(&that->Prm));
    Call(that->Scene->CollidePool,Free,1(that));
}
static void CollideEvtAbort(TlsSchMsg *this) {
    dList *l;
    ThisToThat(CollideEvt,TlsSchMsg);
    l = &that->AColl;
    l->n->p=l->p; l->p->n=l->n; l->p=l->n=l;
    l = &that->BColl;
    l->n->p=l->p; l->p->n=l->n; l->p=l->n=l;
    Call(that->Scene->CollidePool,Free,1(that));
}

static int HandleHookSCIntersect(TlsNeighObject *this,TlsI3dPoint *O,int s) {
    TlsISphere Cell;
    TlsICylinder *cyl;
    int rp,r;
    ThisToThat(HandleHook,TlsNeighObject);
    Cell.O=*O; Cell.r=s;
    cyl = &that->Bounding;
    int Intersect(TlsISphere *cell,void *clos)  { return TlsI3dCellRoundCylinderIntersect(cell,cyl); }
    return TlsI3dSubCellIntersect(&Cell,Intersect,0);
}

static int HandleHookPerform(TlsActor *this,int MaxEndDate) {
    int endate,ndate,delay;
    ThisToThat(HandleHook,TlsActor);
    endate = MaxEndDate;
    ndate = that->Pos.P1.t;
    if ((endate-ndate)>0) {
        int cont;
        that->date = ndate;
        cont = Call(that->Obj,Next,1(&that->Pos));
        delay = (that->Pos.P1.t-ndate);
        if (cont) {
            DynObjectMove(that);
            Call(that->StageName,Sleep,2(that->date,delay)); 
        } else {
            DynObjectRemove(that);
        }
    } else {
        Call(that->StageName,Sleep,2(that->date,ndate-that->date));
    }
    return that->date;
}
static void HandleHookRemove(TlsI3dDynObjHandle *this) {
    ThisToThat(HandleHook,TlsI3dDynObjHandle);
    DynObjectRemove(that);
}
static void HandleHookMove(TlsI3dDynObjHandle *this,TlsI3dSolidMove *d0) {
    ThisToThat(HandleHook,TlsI3dDynObjHandle);
    DynObjectMove(that);
}
static TlsI3dDynObject *HandleHookGetObj(TlsI3dDynObjHandle *this) {
    ThisToThat(HandleHook,TlsI3dDynObjHandle);
    return that->Obj;
}

/*-----------------*/


static int DynSceneCellIntersect(TlsNeighObject *this,TlsI3dPoint *O,int s) {
    int rp,r;
    TlsISphere Cell;
    ThisToThat(DynScene,TlsNeighObject);
    Cell.O=*O; Cell.r=s>>1;
    r = TlsI3dCellSphereIntersect(&Cell,&that->ViewArea);
    Cell.O.x[0]+=Cell.r;
    rp = TlsI3dCellSphereIntersect(&Cell,&that->ViewArea);
    r = r|(rp<<2);
    Cell.O.x[1]+=Cell.r;
    rp = TlsI3dCellSphereIntersect(&Cell,&that->ViewArea);
    r = r|(rp<<6);
    Cell.O.x[0]-=Cell.r;
    rp = TlsI3dCellSphereIntersect(&Cell,&that->ViewArea);
    r = r|(rp<<4);
    Cell.O.x[2]+=Cell.r;
    rp = TlsI3dCellSphereIntersect(&Cell,&that->ViewArea);
    r = r|(rp<<12);
    Cell.O.x[0]+=Cell.r;
    rp = TlsI3dCellSphereIntersect(&Cell,&that->ViewArea);
    r = r|(rp<<14);
    Cell.O.x[1]-=Cell.r;
    rp = TlsI3dCellSphereIntersect(&Cell,&that->ViewArea);
    r = r|(rp<<10);
    Cell.O.x[0]-=Cell.r;
    rp = TlsI3dCellSphereIntersect(&Cell,&that->ViewArea);
    r = r|(rp<<8);
    return r;
}
static TlsI3dDynObjHandle *DynSceneAddObject(TlsI3dDynScene *this,TlsI3dDynObject *Ob,TlsI3dSolidMove *Move) {
    HandleHook *R;
    static struct TlsI3dDynObjHandle HandleStatic = {
        HandleHookRemove,HandleHookMove,HandleHookGetObj
    };
    static struct TlsNeighObject NeighStatic = { HandleHookSCIntersect };
    static struct TlsActor TlsActorStatic = { HandleHookPerform };
    ThisToThat(DynScene,TlsI3dDynScene);
    R = Call(that->HandlePool,Alloc,0);
    R->TlsI3dDynObjHandle.Static = &HandleStatic;
    R->TlsNeighObject.Static = &NeighStatic;
    R->TlsActor.Static = &TlsActorStatic;
    R->TlsActor.Id = "";
    /**/ {
        TlsISphere Bound;
        R->Obj = Ob;
        R->Pos = *Move;
        DynObjectMoveBound(&Bound,R);
        R->Neighbours = Call(that->Set,AddObject,2(&Bound,&R->TlsNeighObject));
    }
    { dList *l; l=&R->Collides; l->p=l->n=l; }
    R->date = Move->P0.t;
    R->Scene = that;
    R->StageName = Call(that->Sequence,EnterActor,2(&R->TlsActor,R->date));
    /**/ {
        struct ObjectCheckIntersect clos;
        clos.Scene = that;
        clos.ObjA = R; 
        Call(R->Neighbours,ForEachNeighbour,2(ObjectCheckIntersect,&clos));
    }
    return &R->TlsI3dDynObjHandle;
}
static int DynScenePerform(TlsI3dDynScene *this,int MaxEndDate)
{
    TlsActor *act;
    ThisToThat(DynScene,TlsI3dDynScene);
    act = &that->Sequence->TlsActor;
    that->date = Call(act,Perform,1(MaxEndDate));
}
struct SceneObjectView {
    int t;
    void *Clos;
    void (*Displ)(TlsI3dReferential *O,TlsI3dDynObject *obj,void *Clos);
};
static int SceneObjectView(TlsI3dNeighHandle *H,void *Clos) {
   struct SceneObjectView *clos;
   TlsI3dReferential Pos;
   HandleHook *h;
   clos = Clos;
   h = CastBack(HandleHook,TlsNeighObject,H->Obj);
   Call(h->Obj,GetPos,2(&Pos,clos->t));
   clos->Displ(&Pos,h->Obj,clos->Clos);
   return (0==0);
}
static void DynSceneObjectView(TlsI3dDynScene *this,TlsISphere *Area,int t,
    void (*Displ)(TlsI3dReferential *O,TlsI3dDynObject *obj,void *Clos),void *Clos 
) {
    struct SceneObjectView clos;
    TlsI3dNeighHandle *H;
    ThisToThat(DynScene,TlsI3dDynScene);
    clos.t = t;
    clos.Clos = Clos;
    clos.Displ = Displ;
    that->ViewArea = *Area;
    H = Call(that->Set,AddObject,2(Area,&that->TlsNeighObject));
    Call(H,ForEachNeighbour,2(SceneObjectView,&clos));
    Call(H,Remove,0);
}

TlsI3dDynScene *TlsI3dDynSceneNew(void) {
    DynScene *r;
    TlsMemPool *Pool;
    static struct TlsI3dDynScene Static = {
        DynSceneAddObject,DynScenePerform,DynSceneObjectView
    };
    static struct TlsNeighObject NeighObjStatic = { DynSceneCellIntersect };
    rPush(r);
    r->TlsI3dDynScene.Static = &Static;
    r->TlsNeighObject.Static = &NeighObjStatic;
    r->date = 0;
    r->Sequence=TlsSchRoundRobin("3DSceneScheduler");
    Pool = TlsMemPoolNew(512);
    r->HandlePool = Call(Pool,SizeSelect,1(TlsAtomSize(HandleHook)));
    r->CollidePool = Call(Pool,SizeSelect,1(TlsAtomSize(CollideEvt)));
    r->Set = TlsI3dNeighbourhoodNew();
    return &r->TlsI3dDynScene;
}

