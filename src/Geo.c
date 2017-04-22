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


/*___________________________________________________________________________
 |
 | Intersections functions:
 |___________________________________________________________________________
*/

int TlsI3dCellCellIntersect(TlsISphere *Cell,TlsISphere *Ob) {
    int r,rc,ro,dr;
    TlsI3dPoint O;
    rc = Cell->r; ro = Ob->r;
    dr = ro-rc;
    TlsI3dVecDiff(O,Ob->O,Cell->O);
    if ((O.x[0]>=0)&&(O.x[1]>=0)&&(O.x[2]>=0)&&(O.x[0]<=dr)&&(O.x[1]<=dr)&&(O.x[2]<=dr)) {
        r=2;
    } else {
        int nrc; 
        nrc = -rc;
        if ((O.x[0]<nrc)||(O.x[1]<nrc)||(O.x[2]<nrc)||(O.x[0]>ro)||(O.x[1]>ro)||(O.x[2]>ro)) {
            r=0;
        } else {
            r=1;
        }
    }
    return r;
}
int TlsI3dCellSphereIntersect(TlsISphere *Cell,TlsISphere *Ob) {
    int r,rc,hrc,ro;
    long long ro2,x2[3],lx;
    TlsI3dPoint O;
    rc = Cell->r; lx = ro = Ob->r; hrc=rc>>1;
    ro2=lx*lx;
    O.x[0] = (Cell->O.x[0]+hrc)-Ob->O.x[0];
    if (O.x[0]<0) { O.x[0]==-1-O.x[0]; } // !!! might not be the exact formula here (maybe -O.x[0] ? ) !!! 
    O.x[1] = (Cell->O.x[1]+hrc)-Ob->O.x[1];
    if (O.x[1]<0) { O.x[1]==-1-O.x[1]; }
    O.x[2] = (Cell->O.x[2]+hrc)-Ob->O.x[2];
    if (O.x[2]<0) { O.x[2]==-1-O.x[2]; }
    lx = O.x[0]+hrc; x2[0]=lx*lx;
    lx = O.x[1]+hrc; x2[1]=lx*lx;
    lx = O.x[2]+hrc; x2[2]=lx*lx;
    if ((x2[0]+x2[1]+x2[2])<=ro2) {
        r = 2;
    } else {
        if (O.x[0]<=hrc) { x2[0]=0; } else { lx = O.x[0]-hrc; x2[0]=lx*lx; }
        if (O.x[1]<=hrc) { x2[1]=0; } else { lx = O.x[1]-hrc; x2[1]=lx*lx; }
        if (O.x[2]<=hrc) { x2[2]=0; } else { lx = O.x[2]-hrc; x2[2]=lx*lx; }
        r = ((x2[0]+x2[1]+x2[2])<=ro2)?1:0;
    }
    return r;
}
int TlsI3dCellRoundCylinderIntersect(TlsISphere *Cell,TlsICylinder *Ob) {
    /* First step: problem orientation: we want the cylinder orientation on the positive axes, 
     * and the cell at the origin along the positive axes. */
    TlsICylinder ob;
    int i,cr,hcr,px,dx,r;
    long long x,rc2,rob2;
    cr = Cell->r;
    x = ob.Base.r = Ob->Base.r;
    rob2 = x*x;
    for (i=0;i<3;i++) {
        if (Ob->d.x[i]<0) {
            ob.d.x[i]=-Ob->d.x[i];
            ob.Base.O.x[i]=-1-(Ob->Base.O.x[i]-(Cell->O.x[i]+cr-1));
        } else { 
            ob.d.x[i]=Ob->d.x[i]; 
            ob.Base.O.x[i]=Ob->Base.O.x[i]-Cell->O.x[i];
        }
    }
    /* second: deals with the deplacement: the space is divided in three by 2 plans 
     * with a normal set to (1,1,1) and passing thru the points (rc,0,0) and (rc-dx,-dy,-dz), respectively. */
    px = ob.Base.O.x[0]+ob.Base.O.x[1]+ob.Base.O.x[2];
    dx = ob.d.x[0]+ob.d.x[1]+ob.d.x[2];
    if (px<cr) {
        if (px<=(cr-dx)) {
            ob.Base.O.x[0]+=ob.d.x[0];
            ob.Base.O.x[1]+=ob.d.x[1];
            ob.Base.O.x[2]+=ob.d.x[2];
        } else {
            double lmbd;
            lmbd = (cr-px); lmbd = lmbd/dx;
            ob.Base.O.x[0]+=lmbd*ob.d.x[0];
            ob.Base.O.x[1]+=lmbd*ob.d.x[1];
            ob.Base.O.x[2]+=lmbd*ob.d.x[2];
        }
    }
    /* Third: space is divided in 9 by the cell walls*/
    if (ob.Base.O.x[0]>0) { if (ob.Base.O.x[0]<=cr) { ob.Base.O.x[0]=0; } else { ob.Base.O.x[0]-=cr;} }
    if (ob.Base.O.x[1]>0) { if (ob.Base.O.x[1]<=cr) { ob.Base.O.x[1]=0; } else { ob.Base.O.x[1]-=cr;} }
    if (ob.Base.O.x[2]>0) { if (ob.Base.O.x[2]<=cr) { ob.Base.O.x[2]=0; } else { ob.Base.O.x[2]-=cr;} }
    x = ob.Base.O.x[0]; rc2 = x*x;
    x = ob.Base.O.x[1]; rc2+= x*x;
    x = ob.Base.O.x[2]; rc2+= x*x;
    if (rc2>rob2) {
        r=0;
    } else {
        hcr = cr>>1;
        if (ob.Base.O.x[0]>=0) {ob.Base.O.x[0]+=hcr; } else { ob.Base.O.x[0]-=hcr;}
        if (ob.Base.O.x[1]>=0) {ob.Base.O.x[1]+=hcr; } else { ob.Base.O.x[1]-=hcr;}
        if (ob.Base.O.x[2]>=0) {ob.Base.O.x[2]+=hcr; } else { ob.Base.O.x[2]-=hcr;}
        rc2 = (ob.Base.O.x[0]*ob.Base.O.x[0])+(ob.Base.O.x[1]*ob.Base.O.x[1])+(ob.Base.O.x[2]*ob.Base.O.x[2]);
        if (rc2<=rob2) {
            r=2;
        } else {
            r=1;
        }
    }
    return r;
}
int TlsI3dCellSegmentIntersect(TlsISphere *Cell,TlsI3dSegment *Ob) {
    int r,i,cr,px,dx;
    TlsI3dSegment ob;
    cr = Cell->r;
    for (i=0;i<3;i++) {
        if (Ob->n.x[0]<0) {
            ob.n.x[i]=-Ob->n.x[i];
            ob.O.x[i]=-1-(Ob->O.x[i]-(Cell->O.x[i]+cr-1));
        } else {
            ob.n.x[i]=Ob->n.x[i];
            ob.O.x[i]=Ob->O.x[i]-Cell->O.x[i];
        }
    }
    px=ob.O.x[0]+ob.O.x[1]+ob.O.x[2];
    dx = ob.n.x[0]+ob.n.x[1]+ob.n.x[2];
    if (px<cr) {
        if (px<=(cr-dx)) {
            ob.O.x[0]+=ob.n.x[0];
            ob.O.x[1]+=ob.n.x[1];
            ob.O.x[2]+=ob.n.x[2];
        } else {
            double lmbd;
            lmbd = (cr-px); lmbd = lmbd/dx;
            ob.O.x[0]+=lmbd*ob.n.x[0];
            ob.O.x[1]+=lmbd*ob.n.x[1];
            ob.O.x[2]+=lmbd*ob.n.x[2];
        }
    }
    if ((ob.O.x[0]>=0)&&(ob.O.x[0]<=cr)&&(ob.O.x[1]>=0)&&(ob.O.x[1]<=cr)&&(ob.O.x[2]>=0)&&(ob.O.x[1]<=cr)) {
        r=1;
    } else {
        r=0;
    }
    return r;
}

typedef struct {
    TlsI3dPoint *b,*e;
} I3dPolygon;
static void ClipPolyAAPlane(I3dPolygon *r,I3dPolygon *p,int axis,int Ox,int outsd) {
    /* TODO: we should absolutely do the clipping strip by strip instead of plan by plan.
      as it is, the computing error will accumulate over 4 dodgy formulas instead of 2. */
    TlsI3dPoint a,*b,*e,*q;
    int c1,c2;
    c1 = axis+1; if (c1>=3) { c1-=3;}
    c2 = axis+2; if (c2>=3) { c2-=3;}
    a = p->e[-1]; e = p->e; b = p->b;
    q = r->b;
    while (b<e) {
        int tst;
        tst=(a.x[axis]<Ox);
        if (tst==(b->x[axis]<Ox)) {
            if (outsd!=tst) { *q++=*b; }
        } else {
            int num,den,c1,c2;
            long long dm;
            num = (Ox-a.x[axis]); den=(b->x[axis]-a.x[axis]);
            q->x[axis]=Ox;
            dm = (b->x[c1]-a.x[c1]); dm*=num;
            q->x[c1]=a.x[c1]+(dm/den);
            dm = (b->x[c2]-a.x[c2]); dm*=num;
            q->x[c2]=a.x[c2]+(dm/den);
            q++;
            if (outsd==tst) {
                *q++=*b;
            }
        }
        a=*b;
        b++;
    }
    r->e=q;
}
int TlsI3dCellTriangleIntersect(TlsISphere *Cell,TlsI3dTriangle *Ob) {
    int r,insd[3],i,cr;
    TlsI3dPoint A[3];
    cr = Cell->r;
    r=0; i=0;
    while ((i<3)&&(r==0)) {
        TlsI3dVecDiff(A[i],Cell->O,Ob->A[i]);
        if ((A[i].x[0]>=0)&&(A[i].x[0]<=cr)&&(A[i].x[1]>=0)&&(A[i].x[1]<=cr)&&(A[i].x[2]>=0)&&(A[i].x[2]<=cr)) { r=1;}
    }
    if (r==0) { /* We have to play the clipping game, which is lame. We hate the clipping game. */
        struct {TlsI3dPoint b[11];} data[2];
        I3dPolygon old,nw;
        TlsI3dPoint *p,*e;
        old.e = old.b = data[0].b+0; nw.e = nw.b = data[1].b+0;
        *old.e= A[0]; old.e++;
        *old.e= A[1]; old.e++;
        *old.e= A[2]; old.e++;
        /* we should cut widely because math imprecision can be nasty in this case. */
        cr+=1;
        ClipPolyAAPlane(&nw,&old,0,-1,(0==0));
        if ((nw.e-nw.b)>1) { ClipPolyAAPlane(&old,&nw,0,cr,(0!=0)); }
        if ((old.e-old.b)>1) { ClipPolyAAPlane(&nw,&old,1,-1,(0==0)); }
        if ((nw.e-nw.b)>1) { ClipPolyAAPlane(&old,&nw,1,cr,(0!=0)); }
        if ((old.e-old.b)>1) {
            int oside,side;
            p=old.e-1;
            oside=(p->x[2]<-1)?0:(p->x[2]<=cr)?1:2;
            p=old.b+0;
            if (oside==1) r=1;
            while ((r==0)&&(p!=old.e)) {
                side = (p->x[2]<-1)?0:(p->x[2]<=cr)?1:2;
                if (side!=oside) { r=1; }
                p++;
            }
        }
    }
    return r;
}

int TlsI3dSubCellIntersect(TlsISphere *Cell,int (*Intersect)(TlsISphere *cel,void *Clos),void *clos) {
    int r;
    TlsISphere sub;
    sub.O = Cell->O;
    sub.r = (Cell->r>>1);
    r = Intersect(&sub,clos);
    sub.O.x[0]+=sub.r; r=r+((Intersect(&sub,clos))<<2);
    sub.O.x[1]+=sub.r; r=r+((Intersect(&sub,clos))<<6);
    sub.O.x[0]-=sub.r; r=r+((Intersect(&sub,clos))<<4);
    sub.O.x[2]+=sub.r; r=r+((Intersect(&sub,clos))<<12);
    sub.O.x[0]+=sub.r; r=r+((Intersect(&sub,clos))<<14);
    sub.O.x[1]-=sub.r; r=r+((Intersect(&sub,clos))<<10);
    sub.O.x[0]-=sub.r; r=r+((Intersect(&sub,clos))<<8);
    return r;
}

/*___________________________________________________________________________
 |
 | The default neigbourhood is stored in a dynamic scaled 3d array.
 | * The array should only keep track of cells that actually intersect with 
 | objects.
 | * The array is scaled, which means multiple copies are maintained each one
 | with a scale factor. At the highest level (the entry point), the cells have
 | the highest diameter, with the diameter halving each level down.
 | * Objects are stored in the minimum level where they might wholly fit in a 
 | cell. At that level, the object can intersect with at most 8 cells. 
 | But be careful: objects from the 26 surroundings cells can intersects with
 | the object. That's because even if an object B is stored in a cell that
 | the object A doesn't intersect, B can still intersect with the cell where
 | A is stored  or a cell A can intersect with.
 |___________________________________________________________________________
*/

#define ILg2Maj(R,X) {int _ilg,_r,_x; _x=(X); TlsILg2(_ilg,_x); _r=1<<_ilg; if (_r<_x) _r=_r<<1; (R)=_r; }

/*______________________________________________________
 |
 |______________________________________________________
*/

typedef struct Neighbourhood Neighbourhood;
typedef struct Neigh3dCell Neigh3dCell;

typedef struct {
    Neigh3dCell *Cell;
    TlsI3dPoint O;
    int Scale;
} NeighCellLoc;

typedef struct {
    Neigh3dCell *Parent;
    int CellNum;
} ParentCell;
struct Neigh3dCell {
    Neighbourhood *Global;
    Neigh3dCell *Previous;
    dList/*<NeighHandle.CellPop>*/ Handles;
    Neigh3dCell *Children[8];
};

struct Neighbourhood {
    TlsI3dNeighbourhood TlsI3dNeighbourhood;
    TlsMemPool *MemPool,*CellPool,*HandlePool;
    NeighCellLoc Root;
};
typedef struct {
    TlsI3dNeighHandle TlsI3dNeighHandle;
    dList/*<NeighHandle.CellPop>*/ CellPop;
    TlsISphere BoundBox; /* O at a corner, r is the vertex length. */
    Neigh3dCell *Cell;
} NeighHandle;

/*--------*/

static Neigh3dCell *Neigh3dCellAlloc(Neighbourhood *N,Neigh3dCell *Previous) {
    Neigh3dCell *R;
    R=Call(N->CellPool,Alloc,0);
    R->Global = N;
    R->Previous = Previous;
    { dList *L; L=&R->Handles; L->p=L->n=L; }
    {Neigh3dCell **p,**e; p=R->Children; e=p+8; while (p<e) {*p++=0;}}
    return R;
}
static void rNeigh3dEmptyCellFree(Neigh3dCell *o) {
    /* since the cell is supposed to be empty, handles aren't released*/
    Neigh3dCell **p,**e;
    p = o->Children; e = p+8;
    while (p<e) { if (*p) {rNeigh3dEmptyCellFree(*p);} *p=0; p++;}
    { dList *L; L=&o->Handles; L->p=L->n=L; }
    Call(o->Global->CellPool,Free,1(o));
}
static void NeighGetOrInsertParentCell(ParentCell *r_r,Neighbourhood *N,NeighCellLoc *nw) {
    ParentCell r;
    NeighCellLoc p;
    int pScale;
    TlsI3dVecDiff(p.O,N->Root.O,nw->O);
    r.CellNum = -1;
    r.Parent = 0;
    pScale = nw->Scale<<1;
    p.Scale=N->Root.Scale;
    p.Cell=N->Root.Cell;
    while (p.Scale>=pScale) {
        p.Scale=p.Scale>>1;
        r.CellNum = (
            ((p.O.x[2]&p.Scale)?4:0)+
            ((p.O.x[1]&p.Scale)?2:0)+
            ((p.O.x[0]&p.Scale)?1:0)
        );
        r.Parent=p.Cell;
        p.Cell=r.Parent->Children[r.CellNum];
        if ((!p.Cell)&&(p.Scale>=pScale)) {
            p.Cell=r.Parent->Children[r.CellNum]=Neigh3dCellAlloc(N,r.Parent);
        }
    }
    *r_r=r;
}
static void NeighExtendDomain(Neighbourhood *N,TlsISphere *Bound) {
    struct { TlsI3dVec O; int l; } A,B;
    int xtnd;
    B.O = Bound->O; B.l = Bound->r;
    if (!N->Root.Cell) {
        int scl;
        N->Root.O = B.O;
        ILg2Maj(scl,B.l);
        N->Root.Scale = scl;
        N->Root.Cell = Neigh3dCellAlloc(N,0);
    }
    A.O = N->Root.O; A.l = N->Root.Scale;
    TlsI3dVecDiff(B.O,A.O,B.O);
    xtnd=(B.l>A.l); 
    if (!xtnd) { xtnd=(B.O.x[0]<0)||(B.O.x[0]>=A.l);}
    if (!xtnd) { xtnd=(B.O.x[1]<0)||(B.O.x[1]>=A.l);}
    if (!xtnd) { xtnd=(B.O.x[2]<0)||(B.O.x[2]>=A.l);}
    if (xtnd) {
        int i;
        int d,dm;
        NeighCellLoc old;
        old = N->Root;
        TlsRange rng,rg[3];
        dm = d = A.l;
        for (i=0;i<3;i++) {
            rng.b=B.O.x[i]; rng.e=rng.b+B.l;
            if (rng.b>0) { 
                rng.b=0;
            } else {
                rng.b= -(((A.l-(rng.b+1))/A.l)*A.l);
            }
            if (rng.e<A.l) rng.e = A.l;
            rng.e=((rng.e+A.l-1)/A.l)*A.l;
            rg[i] = rng;
            d = rng.e-rng.b;
            if (dm<d) dm=d;
            N->Root.O.x[i]=A.O.x[i]+rng.b;
        }
        ILg2Maj(d,dm);
        N->Root.Scale=d;
        N->Root.Cell=Neigh3dCellAlloc(N,0);
        {
            ParentCell pc;
            NeighGetOrInsertParentCell(&pc,N,&old);
            pc.Parent->Children[pc.CellNum]=old.Cell;
        }
    }
}
static void NeighGetOrInsertCell(NeighCellLoc *R,Neighbourhood *N,TlsISphere *Bound) {
    if (Bound->r<=0) {
        R->Cell = 0;
        R->Scale = 0;
        R->O = Bound->O;
    } else {
        int d,ld;
        NeighCellLoc r;
        ParentCell pc;
        NeighExtendDomain(N,Bound);
        d = Bound->r;
        r.O = Bound->O;
        ILg2Maj(r.Scale,d);
        r.Cell = 0;
	NeighGetOrInsertParentCell(&pc,N,&r);
        if (pc.CellNum<0) {
            *R = N->Root;
        } else {
            r.Cell = pc.Parent->Children[pc.CellNum];
            if (!r.Cell) {
                r.Cell=pc.Parent->Children[pc.CellNum]=Neigh3dCellAlloc(N,pc.Parent);
            }
            *R=r;
        }
    }
}

/*-------*/

static struct idx_mov {int tgt,coord,fwd,subtgt;} 
    NeighMovToNext[8]={
        {0,0,(0==0),0},{1,1,(0==0),1},{3,2,(0==0),5},{7,1,(0!=0),21},{5,0,(0!=0),17},{4,1,(0==0),16},{6,2,(0!=0),20},{2,0,(0!=0),4}
    }
   ,CurrentNeighBrowse[13]={
        {5,1,2,0},{8,0,0,0},{7,0,0,0},{6,2,2,0},{15,0,2,0},{16,0,2,0},{17,1,0,0},{14,1,0,0},{11,0,1,0},{10,0,1,0},
        {9,1,2,0},{12,0,2,0},{13,2,0,0}
    };

static void CellGetNextNeigh(Neigh3dCell **r,Neigh3dCell **p) {
    // copy a 3x3x2 area from a 4x4x3 area to a 3x3x2 area.
    r[0]=p[0]; r[1]=p[1]; r[2]=p[2]; r[3]=p[4]; r[4]=p[5]; r[5]=p[6]; r[6]=p[8]; r[7]=p[9]; r[8]=p[10];
    r[9]=p[16]; r[10]=p[17]; r[11]=p[18]; r[12]=p[20]; r[13]=p[21]; r[14]=p[22]; r[15]=p[24]; r[16]=p[25]; r[17]=p[26];
}

static void GetSubLocal(Neigh3dCell **r,Neigh3dCell **pp) {
    // copy the centered 4x4x3 subcells of a 3x3x2 cells to a 4x4x3 cells
    static Neigh3dCell *Zero[8]={0,0,0,0,0,0,0,0};
    Neigh3dCell **p,**pc,**q,**s;
    int idx,i,j,k;
    s = pp; idx=3;
    for (k=0;k<3;k++) {
         q=s;
         for (j=0;j<4;j++) {
              p=q; if (*p) {pc=(*p)->Children+idx;} else {pc=Zero+idx;}
              for (i=0;i<4;i++) {
                 *r++=*pc;
                 if (idx&1) {idx=idx^1; p++; if (*p) {pc=(*p)->Children+idx;} else {pc=Zero+idx;}} else {idx=idx^1; pc++;}
              }
              if (idx&2) {q+=3;}
              idx=idx^2;
         }
         if (idx&4) {s+=9;}
         idx=idx^4;
    }
}

/*-------*/

typedef struct {
    int (*Cont)(TlsI3dNeighHandle *o,void *clos);
    void *Clos;
} HandleInteractListCont;
static int HandleInteractListContinue(TlsI3dNeighHandle *o0,TlsI3dNeighHandle *o1,void *clos) {
    HandleInteractListCont *Clos;
    Clos=clos;
    return Clos->Cont(o1,Clos->Clos);
}

/*-------*/

typedef struct {
    NeighCellLoc Loc;
    int Embeded; /* 0:disjoint, 1:partial, 2:fully. */
    int (*Cont)(TlsI3dNeighHandle *o0,TlsI3dNeighHandle *o1,void *clos);
    void *Clos;
} InteractListCont;
typedef struct {
    InteractListCont Norg;
    Neigh3dCell *N[18];
} NeighIntrctListCont;

static int HandleSubCellInteractionList(NeighHandle *P,Neigh3dCell *c,InteractListCont *CC){
    int cont,Embeded;
    Neigh3dCell **cn;
    struct idx_mov *m,*me;
    InteractListCont cc;
    cc=*CC;
    if (cc.Embeded==0) { return (0==0); }
    if (cc.Embeded==1) {
        TlsNeighObject *Ob;
        Ob=P->TlsI3dNeighHandle.Obj;
        Embeded = Call(Ob,SubCellIntersect,2(&cc.Loc.O,cc.Loc.Scale));
    } else {
        Embeded = 0x6666;
    }
    cc.Loc.Scale = cc.Loc.Scale>>1;
    m = NeighMovToNext; me=m+8;
    cn = c->Children;
    cont = (0==0);
    while ((m!=me)&&(cont)) {
        c=cn[m->tgt];
        if (c) {
            dList *q,*e;
            NeighHandle *Q;
            e=&c->Handles; q=e->n;
            while ((q!=e)&&(cont)) {
                Q=CastBack(NeighHandle,CellPop,q);
                cont=cc.Cont(&P->TlsI3dNeighHandle,&Q->TlsI3dNeighHandle,cc.Clos);
                q=q->n;
            }
            if (cont) {
                cc.Embeded=(Embeded>>(m->tgt<<1))&3;
                if (cc.Embeded!=0) {
                    cont = HandleSubCellInteractionList(P,c,&cc);
                }
            }
        }
        cc.Loc.O.x[m->coord]+=(m->fwd)?cc.Loc.Scale:-cc.Loc.Scale;
        m++;
    }
    return cont;
}
static int HandleCellInteractionList(NeighHandle *P,Neigh3dCell *c,InteractListCont *CC){
    int cont;
    dList *e,*q;
    InteractListCont cc;
    cc.Cont = CC->Cont; cc.Clos==CC->Clos;
    cont=(0==0);
    e = &c->Handles; q=e->n;
    while ((q!=e)&&(cont)) {
        NeighHandle *Q;
        Q = CastBack(NeighHandle,CellPop,q);
        cont = cc.Cont(&P->TlsI3dNeighHandle,&Q->TlsI3dNeighHandle,cc.Clos);
        q = q->n;
    }
    return cont;
}
static int HandleSameCellInteractionList(NeighHandle *P,Neigh3dCell *c,InteractListCont *CC){
    dList *e,*q;
    int cont;
    InteractListCont cc;
    cc.Cont = CC->Cont; cc.Clos==CC->Clos;
    cont = (0==0);
    e = &c->Handles; q=e->n;
    while ((q!=e)&&(cont)) {
        NeighHandle *Q;
        Q = CastBack(NeighHandle,CellPop,q);
        if (P!=Q) cont = cc.Cont(&P->TlsI3dNeighHandle,&Q->TlsI3dNeighHandle,cc.Clos);
        q = q->n;
    }
    return cont;
}

/*-------*/
static void NeighHandleGetLoc(TlsI3dNeighHandle *this,TlsISphere *Loc) {
    int r;
    ThisToThat(NeighHandle,TlsI3dNeighHandle);
    Loc->r = r=that->BoundBox.r>>1;
    Loc->O.x[0] = that->BoundBox.O.x[0]+r;
    Loc->O.x[1] = that->BoundBox.O.x[1]+r;
    Loc->O.x[2] = that->BoundBox.O.x[2]+r;
}
static void NeighHandleRelocate(TlsI3dNeighHandle *this,TlsISphere *nLoc) {
    TlsISphere bndbox,orgC,newC;
    Neighbourhood *glb;
    ThisToThat(NeighHandle,TlsI3dNeighHandle);
    glb = that->Cell->Global;
    bndbox.r = nLoc->r; if (bndbox.r<=0) bndbox.r=1;
    bndbox.O.x[0] = nLoc->O.x[0]-bndbox.r;
    bndbox.O.x[1] = nLoc->O.x[1]-bndbox.r;
    bndbox.O.x[2] = nLoc->O.x[2]-bndbox.r;
    bndbox.r = bndbox.r<<1;
    TlsI3dVecDiff(orgC.O,glb->Root.O,that->BoundBox.O);
    ILg2Maj(orgC.r,that->BoundBox.r);
    TlsI3dVecDiff(newC.O,glb->Root.O,bndbox.O);
    ILg2Maj(newC.r,bndbox.r);
    if ((newC.r==orgC.r)
        &&(!((newC.O.x[0]^orgC.O.x[0])&(-newC.r)))
        &&(!((newC.O.x[1]^orgC.O.x[1])&(-newC.r)))
        &&(!((newC.O.x[2]^orgC.O.x[2])&(-newC.r)))
    ) { /* We didn't change cell, just update the position. */
        that->BoundBox=bndbox;
    } else {
        dList *p,*h;
        NeighCellLoc N;
        that->BoundBox=bndbox;
        p = &that->CellPop;
        { p->n->p=p->p; p->p->n=p->n; p->p=p->n=p; }
        NeighGetOrInsertCell(&N,glb,&bndbox);
        h=&N.Cell->Handles;
        { p->p=h; p->n=h->n; h->n=p->n->p=p; }
    }
}
static void rCleanEmptyCell(Neigh3dCell *c) {
    Neighbourhood *glb;
    Neigh3dCell *prev,**p,**e;
    glb=c->Global;
    prev=c->Previous;
    if (!prev) {
        glb->Root.Cell=0;
        glb->Root.Scale=1;
        glb->Root.O.x[0] = glb->Root.O.x[1] = glb->Root.O.x[2] = 0;
        Call(glb->CellPool,Free,1(c));
    } else {
        int clear,unset;
        clear=(0==0); unset=(0!=0);
        p = prev->Children; e=p+8;
        while ((p!=e)&&(clear||(!unset))) {
            if ((*p)==c) {
                *p=0;
                unset=(0==0);
            }
            clear=clear&&(!*p);
        }
        Call(glb->CellPool,Free,1(c));
        if (clear) {
            dList *p;
            p = &prev->Handles;
            if (p->n==p) {rCleanEmptyCell(prev);} // Typical terminal recursion; try iterative method
        }
    }
}
static void NeighHandleRemove(TlsI3dNeighHandle *this) {
    Neighbourhood *glb;
    dList *p;
    Neigh3dCell *here;
    ThisToThat(NeighHandle,TlsI3dNeighHandle);
    here = that->Cell;
    glb = here->Global;
    p = &that->CellPop;
    {p->n->p=p->p; p->p->n=p->n; p->n=p->p=p; }
    Call(glb->HandlePool,Free,1(that));
    p = &here->Handles;
    if (p->n==p) {
        int clear;
        Neigh3dCell **c,**e;
        clear = (0==0);
        c = that->Cell->Children; e=c+8;
        while (clear&&(c!=e)) { clear=!(*c); c++; }
        if (clear) {rCleanEmptyCell(here);}
    }
}

typedef struct {
    Neigh3dCell *N[27];
    NeighHandle *P;
    TlsISphere PCellLoc;
    InteractListCont Norg;
} HandleIntrctListCont;

static void GetSubCell333(int O,Neigh3dCell **r,Neigh3dCell **p) {
    /* Get the subcells centered on the O subcell of the central cell */
    static Neigh3dCell *Zero[8]={0,0,0,0,0,0,0,0};
    Neigh3dCell **q,**qc,**qk,**s,**e;
    int i,j,k,idx;
    e = r+27;
    s = p; idx=7-O;
    if (O&1) { s++; }
    if (O&2) { s+=3; }
    if (O&4) { s+=9; }
    for (k=0;k<3;k++) {
        qk = s;
        for (j=0;j<3;j++) {
            q = qk;
            if (*q) {qc=q[0]->Children+idx;} else {qc=Zero+idx;}
            for (i=0;i<3;i++) {
                *r++=*qc;
                if (idx&1) {
                    idx=idx^1; q++; 
                    if(*q){
                        qc=q[0]->Children+idx;
                    } else {
                        qc=Zero+idx; 
                    }
                } else {
                    idx=idx^1; qc++;
                }
            }
            idx = idx^1;
            if (idx&2) { qk=qk+3; idx=idx^2; } else { idx=idx|2; }
        }
        idx = idx^2;
        if (idx&4) { s=s+9; idx=idx^4; } else { idx=idx|4; }
    }
}
static void rNeighHandleForEachNeighbour(HandleIntrctListCont *CC) {
    int cont;
    InteractListCont cc;
    NeighHandle *P;
    TlsISphere Tgt;
    Neigh3dCell *N[27],**p,**pc,**pp,**pe;
    P=CC->P;
    cc = CC->Norg;
    cont = (0==0);
    p = CC->N; pc = N;
    Tgt = CC->PCellLoc;
    while ((Tgt.r<cc.Loc.Scale)&&(cont)) {
        int idx;
        pp=p; pe=pp+27;
        while ((cont)&&(pp!=pe)) {
            if(*pp) cont=HandleCellInteractionList(P,*pp,&cc);
            pp++;
        }
        idx = 0;
        cc.Loc.Scale = cc.Loc.Scale>>1;
        if (Tgt.O.x[0]&cc.Loc.Scale) { idx+=1; cc.Loc.O.x[0]+=cc.Loc.Scale; }
        if (Tgt.O.x[1]&cc.Loc.Scale) { idx+=2; cc.Loc.O.x[1]+=cc.Loc.Scale; }
        if (Tgt.O.x[2]&cc.Loc.Scale) { idx+=4; cc.Loc.O.x[2]+=cc.Loc.Scale; }
        GetSubCell333(idx,pc,p);
        {Neigh3dCell **xcg; xcg=pc; pc=p; p=xcg; }
    }
    pp=p; pe=pp+13;
    while ((cont)&&(pp!=pe)) {
        if (*pp) {
            cont=HandleCellInteractionList(P,*pp,&cc);
            if (cont) cont = HandleSubCellInteractionList(P,*pp,&cc);
        }
        pp++;
    }
    if (cont) {
        if (*pp) {
            cont=HandleSameCellInteractionList(P,*pp,&cc);
            if (cont) cont= HandleSubCellInteractionList(P,*pp,&cc);
        }
        pp++;
    }
    pe = pp+13;
    while ((cont)&&(pp!=pe)) {
        if (*pp) {
            cont=HandleCellInteractionList(P,*pp,&cc);
            if (cont) cont = HandleSubCellInteractionList(P,*pp,&cc);
        }
        p++;
    }
}
static void NeighHandleForEachNeighbour(TlsI3dNeighHandle *this,
    int (*Intersect)(TlsI3dNeighHandle *Obj,void *Clos),void *Clos) {
    HandleInteractListCont hcont;
    HandleIntrctListCont cont;
    Neighbourhood *Neigh;
    ThisToThat(NeighHandle,TlsI3dNeighHandle);
    Neigh=that->Cell->Global;
    if (Neigh->Root.Cell) {
        Neigh3dCell **p,**e;
        hcont.Cont = Intersect;
        hcont.Clos = Clos;
        cont.PCellLoc.r = that->BoundBox.r;
        ILg2Maj(cont.PCellLoc.r,that->BoundBox.r);
        TlsI3dVecDiff(cont.PCellLoc.O,Neigh->Root.O,that->BoundBox.O);
        cont.Norg.Cont=HandleInteractListContinue;
        cont.Norg.Clos=&hcont;
        cont.Norg.Loc=Neigh->Root;
        cont.Norg.Embeded=0x1;
        cont.P=that;
        p = cont.N; e=p+27; while (p!=e) { *p++=0; }
        cont.N[13] = Neigh->Root.Cell;
        rNeighHandleForEachNeighbour(&cont);
    }
}
static void NeighHandlePeekCell(TlsI3dNeighHandle *this,TlsISphere *rCell,TlsISphere *rBound) {
    int msk;
    ThisToThat(NeighHandle,TlsI3dNeighHandle);
    *rBound = that->BoundBox;
    ILg2Maj(rCell->r,rBound->r);
    msk = -rBound->r;
    rCell->O.x[0] = rBound->O.x[0]&msk;
    rCell->O.x[1] = rBound->O.x[1]&msk;
    rCell->O.x[2] = rBound->O.x[2]&msk;
}

/*-------*/

static TlsI3dNeighHandle *NeighAddObject(TlsI3dNeighbourhood *this,TlsISphere *Loc,TlsNeighObject *O) {
    static struct TlsI3dNeighHandle Static = {
        NeighHandleGetLoc,NeighHandleRelocate,NeighHandleRemove,NeighHandleForEachNeighbour,NeighHandlePeekCell
    };
    NeighHandle *R;
    NeighCellLoc Cell;
    TlsISphere b;
    ThisToThat(Neighbourhood,TlsI3dNeighbourhood);
    R = Call(that->HandlePool,Alloc,0);
    R->TlsI3dNeighHandle.Static = &Static;
    R->TlsI3dNeighHandle.Obj = O;
    b=*Loc;
    b.O.x[0]-=b.r;
    b.O.x[1]-=b.r;
    b.O.x[2]-=b.r;
    b.r=b.r<<1;
    if (b.r<=0)  b.r = 1;
    R->BoundBox = b;
    NeighGetOrInsertCell(&Cell,that,&b);
    {    dList *r,*n;
         r = &Cell.Cell->Handles;
         n = &R->CellPop;
         n->n=r->n; n->p=r; r->n=n->n->p=n;
    }
    R->Cell=Cell.Cell;
    return &R->TlsI3dNeighHandle;
}
static int rCellInteractionList(Neigh3dCell *c,NeighIntrctListCont *CC) {
    int cont;
    NeighIntrctListCont cc;
    dList *p,*q,*e;
    NeighHandle *P,*Q;
    cc.Norg = CC->Norg;
    e = &c->Handles;
    p = e->n; 
    cont = (0==0);
    while ((p!=e)&&cont) {
        P=CastBack(NeighHandle,CellPop,p);
        q=p->n;
        while ((q!=e)&&cont) {
            Q=CastBack(NeighHandle,CellPop,q);
            cont = cc.Norg.Cont(&P->TlsI3dNeighHandle,&Q->TlsI3dNeighHandle,cc.Norg.Clos);
            q=q->n;
        }
        p=p->n;
        if (cont) cont=HandleSubCellInteractionList(P,c,&cc.Norg);
    }
    if ((cont)&&(e->n!=e)) { /* Check with the neighbour cells of the same scale */
        struct idx_mov *m,*me;
        int LocAdj[3];
        Neigh3dCell **N;
        N=CC->N;
        m = CurrentNeighBrowse; me=m+13;
        LocAdj[0]=-cc.Norg.Loc.Scale; LocAdj[1]=0; LocAdj[2]=cc.Norg.Loc.Scale;
        cc.Norg.Loc.O.x[0]+=cc.Norg.Loc.Scale;
        while ((cont)&&(m!=me)) {
            NeighCellLoc nc;
            nc.Cell=N[m->tgt];
            if (nc.Cell) {
                p = e->n;
                while ((cont)&&(p!=e)) {
                    P=CastBack(NeighHandle,CellPop,p);
                    cont=HandleCellInteractionList(P,nc.Cell,&cc.Norg);
                    if (cont) cont=HandleSubCellInteractionList(P,nc.Cell,&cc.Norg);
                    p=p->n;
                }
            }
            cc.Norg.Loc.O.x[m->coord]+=LocAdj[m->fwd];
            m++;
        }
    }
    if (cont) { /* Childrens */
        Neigh3dCell **p,**pp,*q,*area[48];
        struct idx_mov *m,*me;
        p = c->Children; pp=CC->N;
        GetSubLocal(area,pp);
        cc.Norg.Loc.Scale = cc.Norg.Loc.Scale>>1;
        m=NeighMovToNext; me=m+8;
        while ((cont)&&(m!=me)) {
            if (p[m->tgt]) {
                CellGetNextNeigh(cc.N,area+m->subtgt);
                cont=rCellInteractionList(p[m->tgt],&cc); 
            }
            cc.Norg.Loc.O.x[m->coord]+=(m->fwd)?cc.Norg.Loc.Scale:-cc.Norg.Loc.Scale;
            m++;
        }
    }
    return cont;
}
static void NeighForEachInteract(TlsI3dNeighbourhood *this,int (*Cont)(TlsI3dNeighHandle *o0,TlsI3dNeighHandle *o1,void *clos),void *Clos) {
    NeighIntrctListCont cont;
    ThisToThat(Neighbourhood,TlsI3dNeighbourhood);
    if (that->Root.Cell) {
        Neigh3dCell **p,**e;
        cont.Norg.Cont=Cont;
        cont.Norg.Clos=Clos;
        cont.Norg.Loc=that->Root;
        cont.Norg.Embeded=0x1;
        p=cont.N; e=p+18; while (p!=e) { *p++=0; }
        cont.N[4]=that->Root.Cell;
        rCellInteractionList(that->Root.Cell,&cont);
    }
}

TlsI3dNeighbourhood *TlsI3dNeighbourhoodNew(void) {
    Neighbourhood *R;
    static struct TlsI3dNeighbourhood Static = {
        NeighAddObject,NeighForEachInteract
    };
    rPush(R);
    R->TlsI3dNeighbourhood.Static = &Static;
    R->MemPool=TlsMemPoolNew(0x200);
    R->CellPool=Call(R->MemPool,SizeSelect,1(TlsAtomSize(Neigh3dCell)));
    R->HandlePool=Call(R->MemPool,SizeSelect,1(TlsAtomSize(NeighHandle)));
    R->Root.Scale=1;
    R->Root.O.x[0] = R->Root.O.x[1] = R->Root.O.x[2] = 0;
    R->Root.Cell = 0;
    return &R->TlsI3dNeighbourhood;
}

