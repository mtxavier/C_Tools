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
#include <Geo.h>

#include <stdio.h>

typedef struct {
    TlsNeighObject TlsNeighObject;
    TlsI3dNeighHandle *Handle;
    TlsISphere Loc;
    char *ID;
} ObjSphere;
static int ObjSphereSubIntersect(TlsNeighObject *this,TlsI3dPoint *O,int s) {
    TlsISphere cell,*obj;
    int Intersect(TlsISphere *Cell,void *clos) { return TlsI3dCellSphereIntersect(Cell,obj); }
    ThisToThat(ObjSphere,TlsNeighObject);
    cell.O=*O; cell.r=s;
    obj = &that->Loc;
    return TlsI3dSubCellIntersect(&cell,Intersect,0);
}
static ObjSphere *ObjSphereNew(TlsISphere *Loc,char *Id) {
    ObjSphere *r;
    static struct TlsNeighObject Static = {ObjSphereSubIntersect};
    rPush(r);
    r->TlsNeighObject.Static = &Static;
    r->Handle = 0;
    r->Loc = *Loc;
    r->ID = Id;
    return r;
}

/*----------------------------*/

typedef struct {
    struct SphereCollec *Static;
} SphereCollec;
struct SphereCollec {
    void (*ForEach)(SphereCollec *this,void (*Fn)(TlsISphere *Sph,char *ID,void *Clos),void *Clos);
};

typedef struct {
    TlsISphere Loc;
    char *ID;
} SphereObjDesc;

typedef struct {
    SphereCollec SphereCollec;
    SphereObjDesc *b,*e;
} SphereArrayCollec;

static void SphereArrayCollecForEach(SphereCollec *this,void (*Fn)(TlsISphere *Sph,char *ID,void *Clos),void *Clos) {
    SphereObjDesc *p,*e;
    ThisToThat(SphereArrayCollec,SphereCollec);
    p = that->b; e = that->e;
    while (p!=e) {
        Fn(&p->Loc,p->ID,Clos);
        p++;
    } 
}
SphereCollec *SphereArrayCollecNew(SphereObjDesc *b,SphereObjDesc *e) {
    SphereArrayCollec *r;
    static struct SphereCollec Static = {SphereArrayCollecForEach};
    rPush(r);
    r->SphereCollec.Static = &Static;
    r->b = b; r->e = e;
    return &r->SphereCollec;
}

/*--------------------*/

static SphereCollec *TetraSphereNew(char *id,int r,int d) {
    SphereObjDesc *b,*p;
    TlsISphere Loc;
    char *ID;
    rnPush(b,4);
    p = b;
    Loc.r = r;
    Loc.O.x[0] = Loc.O.x[1] = Loc.O.x[2] = 0;
    rnPush(ID,64); sprintf(ID,"%s.Sp[0]",id);
    p->Loc = Loc; p->ID=ID; p++;
    Loc.O.x[0] = d;
    rnPush(ID,64); sprintf(ID,"%s.Sp[1]",id);
    p->Loc = Loc; p->ID=ID; p++;
    Loc.O.x[0] = d>>1;
    Loc.O.x[1] = (d*887)>>10;
    rnPush(ID,64); sprintf(ID,"%s.Sp[2]",id);
    p->Loc = Loc; p->ID=ID; p++;
    Loc.O.x[1] = (d*296)>>10;
    Loc.O.x[2] = (d*836)>>10;
    rnPush(ID,64); sprintf(ID,"%s.Sp[3]",id);
    p->Loc = Loc; p->ID=ID; p++;
    return SphereArrayCollecNew(b,b+4);
}

static SphereCollec *HexaSphereNew(char *id,int r,int d) {
    /* Located on an octaedra vertices */
    SphereObjDesc *b,*p;
    TlsISphere Loc;
    char *ID;
    rnPush(b,6);
    p = b;
    Loc.r = r;
    Loc.O.x[0] = d; Loc.O.x[1] = Loc.O.x[2] = 0;
    rnPush(ID,64); sprintf(ID,"%s.Sp[0]",id);
    p->Loc = Loc; p->ID=ID; p++;
    Loc.O.x[0] = 0; Loc.O.x[1] = d;
    rnPush(ID,64); sprintf(ID,"%s.Sp[1]",id);
    p->Loc = Loc; p->ID=ID; p++;
    Loc.O.x[1] = 0; Loc.O.x[2] = d;
    rnPush(ID,64); sprintf(ID,"%s.Sp[2]",id);
    p->Loc = Loc; p->ID=ID; p++;
    Loc.O.x[2] =0; Loc.O.x[0] = -d;
    rnPush(ID,64); sprintf(ID,"%s.Sp[3]",id);
    p->Loc = Loc; p->ID=ID; p++;
    Loc.O.x[0] =0; Loc.O.x[1] = -d;
    rnPush(ID,64); sprintf(ID,"%s.Sp[4]",id);
    p->Loc = Loc; p->ID=ID; p++;
    Loc.O.x[1] =0; Loc.O.x[2] = -d;
    rnPush(ID,64); sprintf(ID,"%s.Sp[5]",id);
    p->Loc = Loc; p->ID=ID; p++;
    return SphereArrayCollecNew(b,b+6);
}

static SphereCollec *OctoSphereNew(char *id,int r,int d) {
    /* Located on a cubic vertices */
    SphereObjDesc *b,*p;
    TlsISphere Loc;
    char *ID;
    rnPush(b,8);
    p = b;
    Loc.r = r;
    Loc.O.x[0] = Loc.O.x[1] = Loc.O.x[2] = d;
    rnPush(ID,64); sprintf(ID,"%s.Sp[0]",id);
    p->Loc = Loc; p->ID=ID; p++;
    Loc.O.x[0] = -d;
    rnPush(ID,64); sprintf(ID,"%s.Sp[1]",id);
    p->Loc = Loc; p->ID=ID; p++;
    Loc.O.x[1] = -d;
    rnPush(ID,64); sprintf(ID,"%s.Sp[2]",id);
    p->Loc = Loc; p->ID=ID; p++;
    Loc.O.x[0] = d;
    rnPush(ID,64); sprintf(ID,"%s.Sp[3]",id);
    p->Loc = Loc; p->ID=ID; p++;
    Loc.O.x[2] = -d;
    rnPush(ID,64); sprintf(ID,"%s.Sp[4]",id);
    p->Loc = Loc; p->ID=ID; p++;
    Loc.O.x[0] = -d;
    rnPush(ID,64); sprintf(ID,"%s.Sp[5]",id);
    p->Loc = Loc; p->ID=ID; p++;
    Loc.O.x[1] = d;
    rnPush(ID,64); sprintf(ID,"%s.Sp[6]",id);
    p->Loc = Loc; p->ID=ID; p++;
    Loc.O.x[0] = d;
    rnPush(ID,64); sprintf(ID,"%s.Sp[7]",id);
    p->Loc = Loc; p->ID=ID; p++;
    return SphereArrayCollecNew(b,b+8);
}

/*----------------------------------------*/

typedef struct {
    SphereCollec SphereCollec;
    SphereCollec **b,**e;
} SphCollComp;
static void SphCollCompForEach(SphereCollec *this,void (*Fn)(TlsISphere *Sph,char *Id,void *Clos),void *Clos) {
    SphereCollec **p,**e;
    ThisToThat(SphCollComp,SphereCollec);
    p = that->b; e = that->e;
    while (p!=e) {
        Call(*p,ForEach,2(Fn,Clos));
        p++;
    }
}
static SphereCollec *SphCollCompNew(SphereCollec **b,SphereCollec **e) {
    SphCollComp *r;
    static struct SphereCollec Static = {SphCollCompForEach};
    rPush(r);
    r->SphereCollec.Static = &Static;
    r->b = b; r->e = e;
    return &r->SphereCollec;
}


/*----------------------------------------*/

typedef struct {
    float xx[3],xy[3],aa,ax[3];
} FullQuat;
typedef struct {
    float v[3][3];
} Mat3x3;
#define FullQuatGet(f,q) {\
   float _x_,_y_,_z_,_a_;\
   _a_ = (q).x[0]; (f).aa = _a_*_a_;\
   _x_ = (q).x[1]; (f).xx[0]= _x_*_x_; (f).ax[0]=_a_*_x_;\
   _y_ = (q).x[2]; (f).xx[1]= _y_*_y_; (f).ax[1]=_a_*_y_;\
   _z_ = (q).x[3]; (f).xx[2]= _z_*_z_; (f).ax[2]=_a_*_z_;\
   (f).xy[0] = _x_*_y_; (f).xy[1]=_x_*_z_; (f).xy[2]=_y_*_z_;\
}
static void Ref2Mat3x3(Mat3x3 *r,TlsfQuaternion *q) {
    FullQuat sqrq;
    FullQuatGet(sqrq,*q);
    r->v[0][0] = 1.-2.*(sqrq.xx[0]+sqrq.aa);
    r->v[1][1] = 1.-2.*(sqrq.xx[1]+sqrq.aa);
    r->v[2][2] = 1.-2.*(sqrq.xx[2]+sqrq.aa);
    r->v[0][1] = -2.*(sqrq.xy[0]+sqrq.ax[2]);
    r->v[1][0] = -2.*(sqrq.xy[0]-sqrq.ax[2]);
    r->v[0][2] = -2.*(sqrq.xy[1]-sqrq.ax[1]);
    r->v[2][0] = -2.*(sqrq.xy[1]+sqrq.ax[1]);
    r->v[1][2] = -2.*(sqrq.xy[2]+sqrq.ax[0]);
    r->v[2][1] = -2.*(sqrq.xy[2]-sqrq.ax[0]);
}

static void ChangeRef(TlsI3dPoint *rA,Mat3x3 *rot,TlsI3dPoint *O,TlsI3dPoint *A) {
    TlsI3dVec T;
    T.x[0] = rot->v[0][0]*A->x[0]+rot->v[0][1]*A->x[1]+rot->v[0][2]*A->x[2];
    T.x[1] = rot->v[1][0]*A->x[0]+rot->v[1][1]*A->x[1]+rot->v[1][2]*A->x[2];
    T.x[2] = rot->v[2][0]*A->x[0]+rot->v[2][1]*A->x[1]+rot->v[2][2]*A->x[2];
    TlsI3dVecSum(*rA,*O,T);
}

/*----------------------------------------*/

typedef struct {
    SphereCollec SphereCollec;
    TlsI3dReferential *Ref;
    SphereCollec *Child;
} SphereCollecAway;
static void CollecAwayForEach(SphereCollec *this,void (*Fn)(TlsISphere *Sph,char *Id,void *Clos),void *Clos) {
    Mat3x3 rot;
    TlsI3dPoint O;
    void MyFn(TlsISphere *Sph,char *Id,void *clos) {
        TlsISphere sph;
        sph.r = Sph->r;
        ChangeRef(&sph.O,&rot,&O,&Sph->O);
        Fn(&sph,Id,clos);
    }
    ThisToThat(SphereCollecAway,SphereCollec);
    O = that->Ref->O;
    Ref2Mat3x3(&rot,&that->Ref->v);
    Call(that->Child,ForEach,2(&MyFn,Clos));
}

SphereCollec *SphereCollecLocNew(TlsI3dReferential *Ref,SphereCollec *C) {
    static struct SphereCollec Static = {CollecAwayForEach};
    SphereCollecAway *R;
    rPush(R);
    R->SphereCollec.Static = &Static;
    R->Ref = Ref;
    R->Child = C;
    return &R->SphereCollec;
}

/*-----------------------------------*/

static SphereCollecIntersect(SphereCollec *Collec) {
    int itrcnt;
    TlsI3dNeighbourhood *neigh;
    void NeighEnter(TlsISphere *Sph,char *Id,void *Clos) {
        ObjSphere *Ob;
        Ob = ObjSphereNew(Sph,Id);
        Ob->Handle = Call(neigh,AddObject,2(Sph,&Ob->TlsNeighObject));
        printf("%s (%d) recorded at [%d,%d,%d].\n",Id,Sph->r,Sph->O.x[0],Sph->O.x[1],Sph->O.x[2]);
    }
    int SignalInteract(TlsI3dNeighHandle *h1,TlsI3dNeighHandle *h2,void *Clos) {
        TlsNeighObject *o;
        ObjSphere *o1,*o2;
        o = h1->Obj; o1 = CastBack(ObjSphere,TlsNeighObject,o);
        o = h2->Obj; o2 = CastBack(ObjSphere,TlsNeighObject,o);
        printf("%s <-?-> %s.\n",o1->ID,o2->ID);
        itrcnt++;
        return (0==0);
    }
    itrcnt = 0;
    rOpen
    neigh = TlsI3dNeighbourhoodNew();
    Call(Collec,ForEach,2(NeighEnter,0));
    Call(neigh,ForEachInteract,2(SignalInteract,0));
    if (itrcnt==0) {
        printf("No possible interaction recorded.\n");
    }
    rClose
}

/*-----------------------------------*/

main() {
    SphereCollec *Sc0,*Sc1;
    EnvOpen(4096,4096);
    rOpen
    printf("\nCubic arrangement: overlap.\n");
    Sc0 = OctoSphereNew("OctoSphere",40,50);
    SphereCollecIntersect(Sc0);
    rClose
    printf("\n");
    rOpen {
         TlsI3dReferential Ref;
         SphereCollec *Comp[2];
         printf("Mixed arrangement 1: disjoint / Overlap. \n");
         Comp[0] = OctoSphereNew("OctoSphere",40,128);
         Ref.O.x[0] = Ref.O.x[1] = Ref.O.x[2] = 128;
         Ref.v.x[0] = .0; Ref.v.x[1] = 1.;  Ref.v.x[2] = .0; Ref.v.x[3] = .0;
         Comp[1] = SphereCollecLocNew(&Ref,OctoSphereNew("Octo1",10,16));
         Sc0 = SphCollCompNew(Comp,Comp+2);
         SphereCollecIntersect(Sc0);
    } rClose
    printf("\n");
    rOpen
    printf("\nCubic arrangement: Disjoint,close.\n");
    Sc0 = OctoSphereNew("OctoSphere",40,65);
    SphereCollecIntersect(Sc0);
    rClose
    printf("\n");
    rOpen
    printf("\nCubic arrangement: Disjoint.\n");
    Sc0 = OctoSphereNew("OctoSphere",10,96);
    SphereCollecIntersect(Sc0);
    rClose
    printf("\n");
    rOpen
    printf("\nTetrahedric arrangement: overlap.\n");
    Sc0 = TetraSphereNew("TetraSphere",40,50);
    SphereCollecIntersect(Sc0);
    rClose
    printf("\n");
    rOpen
    printf("\nTetrahedric arrangement: no overlap.\n");
    Sc0 = TetraSphereNew("TetraSphere",10,100);
    SphereCollecIntersect(Sc0);
    rClose
    printf("\n");
    rOpen
    printf("\nTetrahedric arrangement: possible overlap.\n");
    Sc0 = TetraSphereNew("TetraSphere",34,70);
    SphereCollecIntersect(Sc0);
    rClose
    EnvClose();
}
