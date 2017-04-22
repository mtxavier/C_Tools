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

#include <stdarg.h>
#include <StackEnv.h>
#include <Browser.h>
#include <Classes.h>
#include <List.h>
#include <RpgCore.h>

/*_________________________________________
 |
 | TlsAlloc
 |_________________________________________
*/

typedef struct { struct TlsdListAlloc *Static; } TlsdListAlloc;
struct TlsdListAlloc {
    dList *(*Alloc)(TlsdListAlloc *this);
    void (*Free)(TlsdListAlloc *this,dList *Item);
};


/*_________________________________________
 |
 | Core Val
 |_________________________________________
*/

static int DirectValVal(TlsIntVal *this) {
    ThisToThat(RpgCoreDirectVal,TlsIntVal);
    return that->Val;
}
TlsIntVal *RpgCoreDirectValInit(RpgCoreDirectVal *r,int v) {
    static struct TlsIntVal Static = {DirectValVal};
    r->TlsIntVal.Static = &Static;
    r->Val = v;
    return &r->TlsIntVal;
}
TlsIntVal *RpgCoreDirectValNew(int val) {
    RpgCoreDirectVal *r;
    rPush(r);
    return RpgCoreDirectValInit(r,val);
}

/*_________________________________________
 |
 | Objects 
 |_________________________________________
*/

static int RpgObjectNullGetAttr(RpgObject *this,int Id) { return 0; }
static struct RpgObject RpgObjectNullStatic = {RpgObjectNullGetAttr};
RpgObject RpgObjectNull = {&RpgObjectNullStatic};

    /*---------*/

typedef struct {
    RpgMappedObject RpgMappedObject;
    RpgObject RpgObject;
    TlsRange Ids;
    int *Vals;
} MappedObject;
static int MappedObjectGetVal(RpgObject *this,int Id) {
    int r,b;
    ThisToThat(MappedObject,RpgObject);
    b = that->Ids.b;
    if ((Id>=b)&&(Id<that->Ids.e)) {
        r = that->Vals[Id-b];
    } else {
        r = 0;
    }
    return r;
}
static RpgObject *MappedObjectObject(RpgMappedObject *this) {
    ThisToThat(MappedObject,RpgMappedObject);
    return &that->RpgObject;
}
static void MappedObjectSetVal(RpgMappedObject *this,int Id,int Val) {
    int b;
    ThisToThat(MappedObject,RpgMappedObject);
    b = that->Ids.b;
    if ((Id>=b)&&(Id<that->Ids.e)) {
        that->Vals[Id-b] = Val;
    }
}

RpgMappedObject *RpgMappedObjectMap(int bId,int eId,int *Vals) {
    MappedObject *r;
    static struct RpgMappedObject Static = {
        MappedObjectObject,MappedObjectSetVal
    };
    static struct RpgObject ObjStatic = {MappedObjectGetVal};
    rPush(r);
    r->RpgMappedObject.Static = &Static;
    r->RpgObject.Static = &ObjStatic;
    r->Ids.b = bId;
    r->Ids.e = eId;
    r->Vals = Vals;
    return &r->RpgMappedObject;
}

RpgMappedObject *RpgMappedObjectNew(int bId,int eId) {
    int *Vals;
    rnPush(Vals,eId-bId);
    return RpgMappedObjectMap(bId,eId,Vals);
}

/*_________________________________________
 |
 | Obj, Attributes
 |_________________________________________
*/

static int OANullVal(RpgObjAttribute *this,RpgObject *base) { return 0; }
static struct RpgObjAttribute OANullStatic = {OANullVal};
RpgObjAttribute RpgObjAttributeNull = {&OANullStatic};

typedef struct {
    RpgObjAttribute RpgObjAttribute;
    int Id;
} ObjAttributeDirect;
static int OADirectVal(RpgObjAttribute *this,RpgObject *base) {
    ThisToThat(ObjAttributeDirect,RpgObjAttribute);
    return Call(base,GetAttr,1(that->Id));
}
RpgObjAttribute *RpgObjDirectAttr(int Id) {
    ObjAttributeDirect *r;
    static struct RpgObjAttribute Static = {OADirectVal};
    rPush(r);
    r->RpgObjAttribute.Static = &Static;
    r->Id = Id;
    return &r->RpgObjAttribute;
}

static int OAConstVal(RpgObjAttribute *this,RpgObject *base) {
    ThisToThat(ObjAttributeDirect,RpgObjAttribute);
    return that->Id;
}
RpgObjAttribute *RpgObjConstAttr(int Id) {
    ObjAttributeDirect *r;
    static struct RpgObjAttribute Static = {OAConstVal};
    rPush(r);
    r->RpgObjAttribute.Static = &Static;
    r->Id = Id;
    return &r->RpgObjAttribute;
}

    /*--------*/

typedef struct {
    RpgObjAttribute RpgObjAttribute;
    int Id,scale;
} ObjAttributeScale;
static int OAScaleVal(RpgObjAttribute *this,RpgObject *base) {
    ThisToThat(ObjAttributeScale,RpgObjAttribute);
    return that->scale*(Call(base,GetAttr,1(that->Id)));
}
RpgObjAttribute *RpgObjScaleAttr(int scale,int Id) {
    ObjAttributeScale *r;
    static struct RpgObjAttribute Static = {OAScaleVal};
    rPush(r);
    r->RpgObjAttribute.Static = &Static;
    r->Id = Id;
    r->scale = scale;
    return &r->RpgObjAttribute;
}

typedef struct {
    RpgObjAttribute RpgObjAttribute;
    int Id,a,b;
} ObjAttributeDownscale;
static int OADownscaleVal(RpgObjAttribute *this,RpgObject *base) {
    ThisToThat(ObjAttributeDownscale,RpgObjAttribute);
    return ((Call(base,GetAttr,1(that->Id)))*that->a)/that->b;
}
RpgObjAttribute *RpgObjDownscaleAttr(int a,int b,int Id) {
    ObjAttributeDownscale *r;
    static struct RpgObjAttribute Static = {OADownscaleVal};
    if (!b) {b = 1;}
    rPush(r);
    r->RpgObjAttribute.Static = &Static;
    r->Id = Id; r->a = a; r->b = b;
    return &r->RpgObjAttribute;
}

   /*----------*/

typedef struct {
    RpgObjAttribute RpgObjAttribute;
    int (*Combine)(int a,int b);
    int Ida,Idb;
} OACombine;
static int OACombineVal(RpgObjAttribute *this,RpgObject *base) {
    int a,b;
    ThisToThat(OACombine,RpgObjAttribute);
    a = Call(base,GetAttr,1(that->Ida));
    b = Call(base,GetAttr,1(that->Idb));
    return that->Combine(a,b);
}
RpgObjAttribute *RpgObjAttrCombine(int (*Combine)(int a,int b),int Ida,int Idb) {
    OACombine *r;
    static struct RpgObjAttribute Static = {OACombineVal};
    rPush(r);
    r->RpgObjAttribute.Static = &Static;
    r->Combine = Combine;
    r->Ida = Ida;
    r->Idb = Idb;
    return &r->RpgObjAttribute;
}

   /*-------------*/

typedef struct {
    RpgObjAttribute RpgObjAttribute;
    int (*Combine)(int a,int b,int c);
    int Ida,Idb,Idc;
} OACombine3;
static int OACombine3Val(RpgObjAttribute *this,RpgObject *base) {
    int a,b,c;
    ThisToThat(OACombine3,RpgObjAttribute);
    a = Call(base,GetAttr,1(that->Ida));
    b = Call(base,GetAttr,1(that->Idb));
    c = Call(base,GetAttr,1(that->Idc));
    return that->Combine(a,b,c);
}
RpgObjAttribute *RpgObj3AttrCombine(
    int (*Combine)(int a,int b,int c),int Ida,int Idb,int Idc
) {
    OACombine3 *r;
    static struct RpgObjAttribute Static = {OACombine3Val};
    rPush(r);
    r->RpgObjAttribute.Static = &Static;
    r->Combine = Combine;
    r->Ida = Ida;
    r->Idb = Idb;
    r->Idc = Idc;
    return &r->RpgObjAttribute;
}

    /*------------*/

typedef struct {
    RpgObjAttribute RpgObjAttribute;
    int (*Combine)(int a,int b,int c,int d);
    int Ida,Idb,Idc,Idd;
} OACombine4;
static int OACombine4Val(RpgObjAttribute *this,RpgObject *base) {
    int a,b,c,d;
    ThisToThat(OACombine4,RpgObjAttribute);
    a = Call(base,GetAttr,1(that->Ida));
    b = Call(base,GetAttr,1(that->Idb));
    c = Call(base,GetAttr,1(that->Idc));
    d = Call(base,GetAttr,1(that->Idd));
    return that->Combine(a,b,c,d);
}
RpgObjAttribute *RpgObj4AttrCombine(
    int (*Combine)(int a,int b,int c,int d),int Ida,int Idb,int Idc,int Idd
) {
    OACombine4 *r;
    static struct RpgObjAttribute Static = {OACombine4Val};
    rPush(r);
    r->RpgObjAttribute.Static = &Static;
    r->Combine = Combine; 
    r->Ida = Ida; r->Idb = Idb; r->Idc = Idc; r->Idd = Idd;
    return &r->RpgObjAttribute;
}

/*_________________________________________
 |
 |
 |_________________________________________
*/

typedef struct {
    RpgDice RpgDice;
    TlsRange Range;
    BrwRSerial *Vals;
} nSidesDice;
static TlsRange nDiceRange(RpgDice *this) {
    ThisToThat(nSidesDice,RpgDice);
    return that->Range;
}
static void nDicenThrow(RpgDice *this,int *b,int *e) {
    ThisToThat(nSidesDice,RpgDice);
    Call(that->Vals,Read,2(b,e));
}
RpgDice *RpgUniformDiceNew(int b,int e) {
    nSidesDice *r;
    BrwRSerial *rand;
    unsigned char *bshuff,*eshuff,*p;
    int ns,i,nb;
    static struct RpgDice Static = {
        nDiceRange,nDicenThrow
    };
    rPush(r);
    r->RpgDice.Static = &Static;
    nb = e-b;
    r->Range.b = b; r->Range.e = e;
    if (nb>64) {
        rnPush(bshuff,nb);
        eshuff = bshuff+nb;
    } else {
        rnPush(bshuff,64);
        ns = 64/nb;
        eshuff = bshuff+(ns*nb);
    }
    p = bshuff; i = b;
    while (p<eshuff) { *p++ = i++; if (i>=e)  i=b;  }
    rand = BrwSerialRandom(BrwRandomSeed());
    r->Vals = BrwShuffledDice(bshuff,eshuff,256,rand);
    return &r->RpgDice;
}

/*---------------*/

typedef struct {
    RpgDice RpgDice;
    RpgDice *child;
    int nb;
} NDices;
static TlsRange NDicesRange(RpgDice *this) {
    TlsRange r;
    ThisToThat(NDices,RpgDice);
    r = Call(that->child,Range,0);
    r.b = that->nb*r.b;
    r.e = (that->nb*(r.e-1))+1;
    return r;
}
static void AddnThrow(RpgDice *dice,int nb,int *b,int *e) {
    int *p,c,part[8],*ep,*cp,nc,n;
    n = (e-b)*nb;
    p = b; nc = nb; c = *p; ep = part+8;
    while (n>0) {
        if (n<=8) {
            ep = part+n; n = 0;
        } else {
            n-=8;
        }
        Call(dice,nThrow,2(part,ep));
        cp  = part;
        while (cp<ep) {
            c += *cp; cp++; nc--;
            if (nc<=0) {
                *p++ = c;
                c = *p;	
                nc = nb;
            }
        }
    }
}
static void NDicesnThrow(RpgDice *this,int *b,int *e) {
    int *p;
    ThisToThat(NDices,RpgDice);
    p = b;
    while (p<e) { *p++ = 0; }
    AddnThrow(that->child,that->nb,b,e);
}
RpgDice *RpgNDices(int nb,RpgDice  *base) {
    NDices *r;
    static struct RpgDice Static = { NDicesRange, NDicesnThrow };
    rPush(r);
    r->RpgDice.Static = &Static;
    r->child = base;
    r->nb = nb;
    return &r->RpgDice;
}

/*-------------------*/

typedef struct {
    RpgDice RpgDice;
    RpgDice **b,**e;
} DiceSum;

static TlsRange DiceSumRange(RpgDice *this) {
    TlsRange r;
    RpgDice **p,**e;
    ThisToThat(DiceSum,RpgDice);
    r.b = r.e = 0;
    p = that->b; e = that->e;
    while (p<e) {
        TlsRange pr;
        pr = Call(*p,Range,0);
        p++;
        r.b += pr.b;
        r.e += pr.e-1;
    }
    r.e++;
    return r;
}
static void DiceSumnThrow(RpgDice *this,int *br,int *er) {
    int bPart[8],*ePart,*p,*ep;
    RpgDice **s,**es;
    ThisToThat(DiceSum,RpgDice);
    p = br;
    while (p<er) { *p++ = 0; }
    p = br;
    ePart = bPart+8;
    es = that->e;
    while (p<er) {
    ep = p+8;
        if (ep>er) { ep = er; ePart = bPart+(er-p); }
        s = that->b;
        while (s<es) {
            int *sp,*sq;
            Call(*s,nThrow,2(bPart,ePart));
            sq = bPart; sp = p;
            while (p<ep) {
                    *p += *sq; p++; sq++;
            }
            s++;
        }
        p = ep;
    }
}
static RpgDice *DiceSumNew(RpgDice **b,RpgDice **e) {
    DiceSum *r;
    static struct RpgDice Static = {DiceSumRange,DiceSumnThrow};
    rPush(r);
    r->RpgDice.Static = &Static;
    r->b = b; r->e = e;
    return &r->RpgDice;
}

RpgDice *RpgDiceSum(RpgDice *A,...) {
    RpgDice **b,**e,**p,*q;
    va_list l;
    int nb;
    q = A; nb = 0;
    va_start(l,A);
    while (q) {
        nb++; 
        q = va_arg(l,RpgDice *);
    }
    va_end(l);
    rnPush(b,nb);
    e=b+nb;
    p = b;
    q = A; va_start(l,A);
    while (p<e) {
        *p++ = q;
        q = va_arg(l,RpgDice *);
    }
    va_end(l);
    return DiceSumNew(b,e);
}

RpgDice *RpgLDiceSum(RpgDice **pb,RpgDice **pe) {
    RpgDice **b,**e,**p,**q;
    int nb;
    nb = pe-pb;
    rnPush(b,nb);
    e = b+nb;
    p = b; q = pb;
    while (p<e) { *p++ = *q++; }
    return DiceSumNew(b,e);
}

/*------------------------*/

typedef struct {
    RpgDice RpgDice;
    int val;
} ConstDice;
static TlsRange ConstDiceRange(RpgDice *this) {
    TlsRange r;
    ThisToThat(ConstDice,RpgDice);
    r.b = that->val; r.e = r.b+1;
    return r;
}
static void ConstDicenThrow(RpgDice *this,int *b,int *e) {
    int *p,v;
    ThisToThat(ConstDice,RpgDice);
    p = b; v = that->val;
    while (p<e) { *p++ = v; }
}
RpgDice *RpgDiceConst(int val) {
    ConstDice *r;
    static struct RpgDice Static= {ConstDiceRange,ConstDicenThrow};
    rPush(r);
    r->RpgDice.Static = &Static;
    r->val = val;
    return &r->RpgDice;
}

/*-----------------*/

typedef struct {
    RpgDice RpgDice;
    RpgDice *child;
    int mod;
} DiceMod;
static TlsRange DiceModRange(RpgDice *this) {
    TlsRange r;
    ThisToThat(DiceMod,RpgDice);
    r = Call(that->child,Range,0);
    if (r.e>that->mod) { 
        r.e = that->mod;
        r.b = 0;
    }
    return r;
}
static void DiceModnThrow(RpgDice *this,int *b,int *e) {
    int *p,mod;
    ThisToThat(DiceMod,RpgDice);
    Call(that->child,nThrow,2(b,e));
    p = b; mod = that->mod;
    while (p<e) {
        *p = *p%mod;
        p++;
    }
}
RpgDice *RpgDiceMod(RpgDice *d,int mod) {
    DiceMod *r;
    static struct RpgDice Static =  {DiceModRange,DiceModnThrow};
    rPush(r);
    r->RpgDice.Static = &Static;
    r->child = d;
    r->mod = mod;
    return &r->RpgDice;
}

/*_______________________________________________________
 |
 |
 |_______________________________________________________
*/

typedef struct {
    RpgDiceCollect RpgDiceCollect;
    RpgDice **b,**e;
    int DiceNb;
} DiceCollect;
typedef struct {
    RpgDiceVector RpgDiceVector;
    DiceCollect *Collect;
    int *b,*e;
} DiceVector;
static TlsRange DiceVectorRange(RpgDice *this){
    TlsRange r,r0;
    int *en,*n;
    RpgDice **D;
    ThisToThat(DiceVector,RpgDiceVector.RpgDice);
    r.b = 0;
    r.e = 0;
    n = that->b; en = that->e;
    D = that->Collect->e;
    while (n<en) {
        if (*n>0) {
            r0 = Call(*D,Range,0);
            r.b += (*n)*r0.b;
            r.e += (*n)*(r0.e-1);
        }
        n++; D++;
    }
    r.e++;
    return r;
}
static void DiceVectornThrow(RpgDice *this,int *b,int *e) {
    int *p,*ep;
    RpgDice **D;
    ThisToThat(DiceVector,RpgDiceVector.RpgDice);
    p = b;
    while (p<e) { *p++=0; }
    D = that->Collect->b;
    p = that->b; ep = that->e;
    while (p<e) {
        if (*p>0) {
            AddnThrow(*D,*p,b,e);
        }
        p++; D++;
    }
}
static RpgDiceCollect *DiceVectorCollection(RpgDiceVector *this) {
    ThisToThat(DiceVector,RpgDiceVector);
    return &that->Collect->RpgDiceCollect;
}
static void DiceVectorAddCmpnt(RpgDiceVector *this,int Id,int nb) {
    ThisToThat(DiceVector,RpgDiceVector);
    that->b[Id]+=nb;
}
static int DiceVectorCmpntPop(RpgDiceVector *this,int Id) {
    ThisToThat(DiceVector,RpgDiceVector);
    return that->b[Id];
}
static void DiceVectorAdd(RpgDiceVector *this,RpgDiceVector *a) {
    ThisToThat(DiceVector,RpgDiceVector);
    if (a->Static == this->Static) {
        DiceVector *A;
        A = CastBack(DiceVector,RpgDiceVector,a);
        if (A->Collect==that->Collect) {
            int *p,*q,*e;
            p=that->b; e=that->e;  q=A->b;
            while (p<e) { *p += *q; p++; q++; }
        }
    }
}
static void DiceVectorScale(RpgDiceVector *this,int n) {
    int *p,*e;
    ThisToThat(DiceVector,RpgDiceVector);
    p = that->b; e = that->e;
    while (p<e) { *p = *p*n; p++; }
}

static int DiceCollectCmpntNb(RpgDiceCollect *this) {
    ThisToThat(DiceCollect,RpgDiceCollect);
    return that->DiceNb;
}
static RpgDice *DiceCollectCmpnt(RpgDiceCollect *this,int Id) {
    ThisToThat(DiceCollect,RpgDiceCollect);
    return that->b[Id];
}
static RpgDiceVector *DiceCollectGetNDices(
    RpgDiceCollect *this,int Id,int nb
) {
    static struct RpgDice DiceStatic = {DiceVectorRange,DiceVectornThrow};
    static struct RpgDiceVector Static = {
        DiceVectorCollection,DiceVectorAddCmpnt,DiceVectorCmpntPop,
        DiceVectorAdd,DiceVectorScale
    };
    DiceVector *r;
    int *p,*e;
    ThisToThat(DiceCollect,RpgDiceCollect);
    rPush(r);
    r->RpgDiceVector.RpgDice.Static = &DiceStatic;
    r->RpgDiceVector.Static = &Static;
    r->Collect = that;
    rnPush(r->b,that->DiceNb);
    r->e = r->b+that->DiceNb;
    p = r->b; e = r->e; while (p<e) { *p++ = 0; }
    r->b[Id] = nb;
    return &r->RpgDiceVector;
}
RpgDiceCollect *RpgDiceCollectNew(RpgDice *d0,...) {
    DiceCollect *r;
    int nb;
    va_list l;
    static struct RpgDiceCollect Static = {
        DiceCollectCmpntNb,DiceCollectCmpnt,DiceCollectGetNDices
    };
    rPush(r);
    r->RpgDiceCollect.Static = &Static;
    {
        RpgDice *d;
        nb = 0;
        d = d0;
        va_start(l,d0);
        while (d) {
            d = va_arg(l,RpgDice *);
            nb++;
        }
        va_end(l);
    }
    r->DiceNb = nb;
    rnPush(r->b,nb);
    r->e = r->b+nb;
    {
        RpgDice *d,**p;
        d = d0; p = r->b;
        va_start(l,d0);
        while  (d) {
            *p++ = d;
            d = va_arg(l,RpgDice *);
        }
        va_end(l);
    }
    return &r->RpgDiceCollect;
}

/*_____________________________________________________________________________
 |
 | Modifiers
 |_____________________________________________________________________________
*/


RpgModCtx RpgModCtxNull;
static int ModifierNullVal(
    RpgModifier *this,RpgModCtx *Ctx
) { 
    return 0; 
}
static struct RpgModifier ModifierNullStatic  = {ModifierNullVal};
RpgModifier RpgModifierNull = {&ModifierNullStatic};

   /*-------------*/

typedef struct {
    RpgModifier RpgModifier;
    int Val;
} StraightMod;
static int StraightModVal(RpgModifier *this, RpgModCtx *Scene){
    ThisToThat(StraightMod,RpgModifier);
    return that->Val;
}
RpgModifier *RpgStraightModifier(int Type,int Val) {
    StraightMod *r;
    static struct RpgModifier Static = {StraightModVal};
    rPush(r);
    r->RpgModifier.Static = &Static;
    r->RpgModifier.Type = Type;
    r->Val = Val;
    return &r->RpgModifier;
}

/*______________________________________________
 |
 | Templates
 |______________________________________________
*/

typedef struct {
    RpgObjTemplate RpgObjTemplate;
    TlsIntTrie/*<IdBoni.TlsIntTrie>*/ Boni;
    TlsMemPool *PoolStat,*PoolBon;
} RpgObjTemplateRec;
typedef struct {
    TlsIntTrie TlsIntTrie;
    RpgObjTemplateRec *Rec;
    List/*<BonusList.List>*/ Boni;
} IdBoni;
typedef struct {
    RpgRemoveHandle RpgRemoveHandle;
    List/*<BonusList.List>*/ List;
    IdBoni *AttrId;
    RpgModifier *Mod;
} BonusList;

static void BonusRecRemoveRemove(RpgRemoveHandle *this) {
    List *p,*n,*f,*e;
    ThisToThat(BonusList,RpgRemoveHandle);
    f = &that->List;
    p = e = &(that->AttrId->Boni);
    n = p->n;
    while ((n!=e)&&(n!=f)) { p = n; n = p->n; }
    if (n!=e) {
        BonusList *F;
        RpgObjTemplateRec *Rec;
        Rec = that->AttrId->Rec;
        p->n = n->n;
        F = CastBack(BonusList,List,n);
        Call(Rec->PoolBon,Free,1(F));
        if (e->n==e) {
            IdBoni *B;
            B = CastBack(IdBoni,TlsIntTrie,that->AttrId);
            TlsIntTrieRemove(&Rec->Boni,that->AttrId->TlsIntTrie.Key);
            Call(Rec->PoolStat,Free,1(B));
        }
    }
}

static void  TmplRecAllBonus(RpgObjTemplate *this,
    int (*Bonus)(int StatId,RpgModifier *Value,void *clos),void *clos
) {
    struct {
        int (*Bonus)(int StatId,RpgModifier *Value,void *clos);
        void *clos;
    } Clos;
    int Brws(TlsIntTrie *b,void *clos) {
        int r,stat;
        IdBoni *B;
        List *l,*e;
        stat = b->Key;
        B = CastBack(IdBoni,TlsIntTrie,b);
        e = &B->Boni; l = e->n;
        r = (0!=0);
        while ((l!=e)&&(!r)) {
            BonusList *L;
            L = CastBack(BonusList,List,l);
            l = l->n;
            r = Clos.Bonus(stat,L->Mod,Clos.clos);
        }
        return r;
    }
    ThisToThat(RpgObjTemplateRec,RpgObjTemplate);
    Clos.clos = clos;
    Clos.Bonus = Bonus;
    TlsIntTrieForEach(&that->Boni,Brws,&Clos);
}
static void TmplRecStatBonus(RpgObjTemplate *this,int StatId,
	int (*Bonus)(RpgModifier *Value,void *clos),void *clos
) {
    TlsIntTrie *f;
    int r;
    ThisToThat(RpgObjTemplateRec,RpgObjTemplate);
    f = TlsIntTrieSeek(&that->Boni,StatId);
    if (f!=&that->Boni) {
        IdBoni *B;
        List *p,*e;
        B = CastBack(IdBoni,TlsIntTrie,f);
        e = &B->Boni; p = e->n;
        r = (0!=0);
        while ((p!=e)&&(!r)) {
            BonusList *P;
            P = CastBack(BonusList,List,p);
            p = p->n;
            r = Bonus(P->Mod,clos);
        }
    }
}
static RpgRemoveHandle *TmplRecAddModifier(RpgObjTemplate *this,
    int StatId,RpgModifier *Mod
) {
    TlsIntTrie *f;
    IdBoni *F;
    List *l;
    BonusList *n;
    static struct RpgRemoveHandle Static = {BonusRecRemoveRemove};
    ThisToThat(RpgObjTemplateRec,RpgObjTemplate);
    f = TlsIntTrieSeek(&that->Boni,StatId);
    if (f==&that->Boni) {
    F = Call(that->PoolStat,Alloc,0);
        F->Rec = that;
        l = &F->Boni; l->n = l;
        TlsIntTrieSeekOrInsert(&that->Boni,StatId,&F->TlsIntTrie);
    } else {
        F = CastBack(IdBoni,TlsIntTrie,f);
        l = &F->Boni;
    }
    n = Call(that->PoolBon,Alloc,0);
    n->RpgRemoveHandle.Static = &Static;
    n->AttrId = F;
    n->Mod = Mod;
    n->List.n = l->n; l->n = &n->List;
    return &n->RpgRemoveHandle;
}
static void TmplRecClear(RpgObjTemplate *this){
    TlsIntTrie *rm,*erm;
    ThisToThat(RpgObjTemplateRec,RpgObjTemplate);
    erm = &that->Boni;
    rm = erm->Children[0];
    while (rm!=erm) {
        List *l,*e;
        IdBoni *Rm;
        TlsIntTrieRemove(erm,rm->Key);
        Rm = CastBack(IdBoni,TlsIntTrie,rm);
        e = &Rm->Boni; l = e->n;
        while (l!=e) {
            BonusList *L;
            L = CastBack(BonusList,List,l);
            l = l->n;
            Call(that->PoolBon,Free,1(L));
        }
        Call(that->PoolStat,Free,1(Rm));
    }
}
RpgObjTemplate *RpgObjTemplateNew(TlsMemPool *Pool) {
    RpgObjTemplateRec *r;
    static struct RpgObjTemplate Static = {
        TmplRecAllBonus,TmplRecStatBonus,TmplRecAddModifier,TmplRecClear
    };
    rPush(r);
    r->RpgObjTemplate.Static = &Static;
    r->PoolStat = Call(Pool,SizeSelect,1(TlsAtomSize(IdBoni)));
    r->PoolBon = Call(Pool,SizeSelect,1(TlsAtomSize(BonusList)));
    TlsIntTrieInit(&r->Boni);
    return &r->RpgObjTemplate;
}

