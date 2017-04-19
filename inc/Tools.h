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

#ifndef _Tools_h_
#define _Tools_h_

#define TlsMaxVal(Atom) ((unsigned Atom)-1)
#define TlsMaxSignedVal(Atom) (((unsigned Atom)-1)>>1)
#define TlsMinSignedVal(Atom) ((Atom)(((unsigned Atom)-1)^TlsMaxSignedVal(Atom)))
#define TlsAtomSize(Atom) (((char *)(((Atom *)0)+1))-((char *)((Atom *)0)))
#define TlsAlign(Atom,p) ((void *)((((void *)p)-((void *)0)+TlsAtomSize(Atom)-1)&(-TlsAtomSize(Atom))))
#define TlsRoundUp(Atom,p) ((p+TlsAtomSize(Atom)-1)&(-TlsAtomSize(Atom)))
#define TlsFieldOffset(Atom,Field) ((char *)(&(((Atom *)0)->Field))-((char *)(0)))

// TlsILg2 returns the highest bit set in A
// a more efficient way would be to convert into floating point and take the exponant

#define TlsiLg2(Atom,R,x) {\
    unsigned Atom _i_,_msk_;\
    int _r_,_di_;\
    _i_=(x); \
    if (_i_) {\
        _r_=0;\
        _di_=TlsAtomSize(Atom)<<2;\
        _msk_=TlsMaxVal(Atom)<<(TlsAtomSize(Atom)<<2);\
        do {\
            if (_i_&_msk_) {\
                _r_+=_di_; _di_=_di_>>1; _i_=_i_>>_di_;\
            } else {\
                _di_=_di_>>1; _i_=_i_<<_di_;\
            }\
        } while(_di_>0);\
        (R)=_r_;\
    } else {\
        (R)=-1;\
    }\
}

#define TlsILg2(R,x) TlsiLg2(int,R,x)

/*____________________________________
 |
 | Bit Fields manipulation
 |____________________________________
*/

int TlsBitFieldGetField(char *buf,int bAttr,int eAttr);
void TlsBitFieldSetField(char *buf,int bField,int eField,int Value);
void TlsBitFieldFillField(char *buf,int bField,int eField,int val); // val is 0 or 1.

/*____________________________________
 |
 | Basic Number/String conversions
 |____________________________________
*/

int TlsStringToInt(char *s);
char *TlsStringDuplicate(char *s);
char *TlsStringCat(char *s0,...);

/*______________________________________________________________
 |
 | MemPools: For locals small elements that tends to be often 
 | reused. MemPools is only 'unallocated' as a whole, that
 | is, it can only grow. Individual elements, however, must be 
 | allocated and freed as needed.
 | MemPools should be used when the data structure growth in
 | an anarchic pattern, usually on the whim of an irresponsable
 | user.
 | It can also be used for elements that have capped number with
 | the cap unknown or data dependant.
 |______________________________________________________________
*/

typedef struct { struct TlsMemPool *Static; } TlsMemPool;
struct TlsMemPool {
    TlsMemPool *(*SizeSelect)(TlsMemPool *this,int Size);
    void *(*Alloc)(TlsMemPool *this);
    void (*Free)(TlsMemPool *this,void *Data);
};

TlsMemPool *TlsMemPoolNew(int Growth);

/*___________________________________________________________________
 |
 | Dynamical structures are often short lived and/or created on the
 | spots. 
 | The overall structure is usually held entirely in a MemPool, so
 | strict Alloc/Free discipline must be enforced as long as you 
 | don't wipe the whole MemPool entirely.
 |
 |___________________________________________________________________
*/
/*___________________________________________________________________
 |
 |    * dList are double lists and can be used to structure datas in
 | LIFO or FIFO pattern.
 |    * Use Push/Pop for LIFO use
 |    * Use Queue/Pop for FIFO use
 |    * Queue/Unqueue could be an alternative for LIFO, as 
 | Push/Unqueue would for FIFO
 |    * Remove might be used to get and remove an item in a middle of
 | the list, provided you have kept an handle on it. Use GetItem 
 | instead if you don't want to actually remove the Item.
 |___________________________________________________________________
*/

typedef struct TlsDyndListNod TlsDyndListNod;
typedef struct TlsDyndList TlsDyndList;

void *TlsDyndListGetItem(TlsDyndListNod *n);
TlsDyndListNod *TlsDyndListNext(TlsDyndList *L,TlsDyndListNod *N);
TlsDyndListNod *TlsDyndListPrevious(TlsDyndList *L,TlsDyndListNod *N);

int TlsDyndListIsEmpty(TlsDyndList *L);

TlsDyndListNod *TlsDyndListQueue(TlsDyndList *L,void *Item);
TlsDyndListNod *TlsDyndListPush(TlsDyndList *L,void *Item);
TlsDyndListNod *TlsDyndListChainBefore(TlsDyndList *L,TlsDyndListNod *O,void *Item);
TlsDyndListNod *TlsDyndListChainAfter(TlsDyndList *L,TlsDyndListNod *O,void *Item);

void *TlsDyndListRemove(TlsDyndList *L,TlsDyndListNod *N);
void *TlsDyndListPop(TlsDyndList *L);
void *TlsDyndListUnqueue(TlsDyndList *L);

TlsDyndList *TlsDyndListAlloc(TlsMemPool *Mem);
void TlsDyndListFree(TlsDyndList *L); /* Frees all nods then the root */
void *TlsDyndListForEach(TlsDyndList *L,int (*fnd)(void *item,void *clos),void *Clos);
void *TlsDynRvsdListForEach(TlsDyndList *L,int (*fnd)(void *item,void *clos),void *Clos);

/*___________________________________________________________________
 |
 | Circle Buffer
 | - the actual storage space is between 'bBuff' and 'eBuff'
 | - By convention:
 |     - bElt is never and should never be set to eBuff;
 |     - eElt is set to eBuff if and only if the buffer is empty.
 |     - the buffer is full when (bElt==eElt)
 | - TlsCBfWrite(): writes as much data as possible. Returns
 |   pointer on the data following the last data written (in the src 
 |   buffer). The 'eElt' pointer of the circle buffer is updated.
 | - TlsCBfRead(): reads as much data as possible. Returns 
 |   pointer on the data following the last data read (in the dst
 |   buffer). The 'bElt' pointer of the circle buffer is updated.
 |
 | - The following aren't part of the circle buffer formalism:
 |    - TlsCBfInsert(): starting at 'pos' from the first 
 |    element of the buffer, insert 'nb' spaces. The content of those
 |    added spaces is left unspecified. Returns nb of spaces that
 |    couldn't be inserted.
 |    - TlsCBfRemove(): starting at 'pos' from the first 
 |    element of the buffer, remove 'nb' elt. Returns nb of elt that
 |    couldn't be removed.
 |___________________________________________________________________
*/

typedef struct {
    char *bBuff,*eBuff;
    void *bElt,*eElt;
} TlsCircleBuffer;

#define TlsCBfClear(buf) { (buf)->bElt=(buf)->bBuff; (buf)->eElt=(buf)->eBuff; }
#define TlsCBfMaxContent(buf) ((buf)->eBuff-(buf)->bBuff)
#define TlsCBfOccupied(buf)\
  (((buf)->eElt==(buf)->eBuff)?0:(((buf)->eElt-(buf)->bElt)+(((buf)->eElt>(buf)->bElt)?0:((buf)->eBuff-(buf)->bBuff))))

void *TlsCBfWrite(TlsCircleBuffer *dst,void *sb,void *se);
void *TlsCBfRead(void *db,void *de,TlsCircleBuffer *src);

int TlsCBfInsert(TlsCircleBuffer *src,int pos,int nb);
int TlsCBfRemove(TlsCircleBuffer *src,int pos,int nb);

void *TlsCBfOverwrite(TlsCircleBuffer *dst,int pos,void *sb,void *se);
int TlsCBfFill(TlsCircleBuffer *dst,int bpos,int epos,void *fb,void *fe);
void *TlsCBfGet(void *db,void *de,TlsCircleBuffer *src,int pos);

void TlsCBfCpy(TlsCircleBuffer *dst,int db,int de,TlsCircleBuffer *src,int sb);


void TlsCBfAlloc(TlsCircleBuffer *dst,int Size);
TlsCircleBuffer *TlsCBfNew(int Size);

int TlsCBf_RequiredMem(int Size);
TlsCircleBuffer *TlsCBf_MemFormat(void *chunk,int Size);

/*____________________________________
 |
 | Id & Lexic management.
 |____________________________________
*/

/*____________________________________________________
 | 
 | Lexic: The idead behind lexics is to replace all
 | those pesky string comparisons/affectations with 
 | pointer comparison/affectation. 
 | We store each found word in the lexic that will
 | then provide a unique reference pointer to that 
 | word. The space occupied by the new word can then
 | be disposed of, which is a boon and might very
 | well save space in the  long run.
 | Slang provides additional recorded words on top of 
 | a standard lexic. When you reach the end of the 
 | scope wher Slang is relevant, simply dump the Slang
 | object to recover memory space.
 |____________________________________________________
*/

typedef struct { struct TlsLexic *Static; } TlsLexic;
struct TlsLexic {
    char *(*GetLabel)(TlsLexic *this,char *Label);
    char *(*CheckLabel)(TlsLexic *this,char *Label);
};
extern TlsLexic TlsLexicNull;
TlsLexic *TlsLexicNew(void);
TlsLexic *TlsSlangNew(TlsLexic *Base);

/*____________________________________
 |
 | Dynamic structures
 |____________________________________
*/

/*_________________________________________________
 |
 | Idx List: A List whose element may be addressed
 | via their index. NOT a dynamic array since elts
 | index will vary when elements are inserted or
 | removed. See it as a list with random access
 | facilities.
 |_________________________________________________
*/

typedef struct {struct TlsIdxListIterator *Static;} TlsIdxListIterator;
typedef struct { struct TlsIdxList *Static; } TlsIdxList;

struct TlsIdxListIterator {
    int (*Found)(TlsIdxListIterator *this,int Idx,void *Elt); // returns (0==0) if the iteration is at an end.
};

struct TlsIdxList {
    int (*EltNb)(TlsIdxList *this);
    void *(*EltGet)(TlsIdxList *this,void *Dst,int idx,int nb);
    void *(*EltSet)(TlsIdxList *this,int idx,void *Src,int nb);
    int (*Insert)(TlsIdxList *this,int idx,int nb); // returns actual inserted
    void (*Remove)(TlsIdxList *this,int idx,int nb);
    void (*ForEach)(TlsIdxList *this,int bRange,int eRange,int Step,TlsIdxListIterator *Itr);
    TlsIdxList *(*Clear)(TlsIdxList *this);
};

typedef struct { struct TlsIdxListPool *Static; } TlsIdxListPool;
struct TlsIdxListPool {
    TlsIdxList *(*NewEmpty)(TlsIdxListPool *this,void *bDefault,void *eDefault);
};

extern TlsIdxList TlsIdxListNull;

TlsIdxList *TlsBigIdxListNew(int EltSize,int Growth);

/*_____________________________________________________________________________
 |
 | Generic structures.
 |_____________________________________________________________________________
*/

typedef struct {
    unsigned char *b,*e;
} TlsCharString;

    /*--------------*/

typedef struct {
    int b,e;
} TlsRange;

#define TlsRangeClip(r,A,B) \
    (r).b = ((A).b<(B).b)?(B).b:(A).b; \
    (r).e = ((A).e<(B).e)?(A).e:(B).e; \
    if ((r).e<(r).b) (r).e=(r).b;

#define TlsRangeExtend(r,A,B) \
    (r).b = ((A).b<(B).b)?(A).b:(B).b; \
    (r).e = ((A).e>(B).e)?(A).e:(B).e;

    /*--------------*/

typedef struct {
    TlsRange w,l;
} Tls2dArea;

/*__________________________________________
 |
 | Reflexive structures
 |__________________________________________
*/

typedef int (TlsIntTranslator)(int a);
TlsIntTranslator TlsIntTransNeg,TlsIntTransNot,TlsIntTransBool,TlsIntTranCmpl0
    ,TlsIntTransAbs;

typedef int (TlsIntOperator)(int a,int b);

TlsIntOperator TlsIntOpSum,TlsIntOpProd,TlsIntOpDiff,TlsIntOpMin,TlsIntOpMax;
TlsIntOperator TlsIntOpDiv,TlsIntOpMod;
TlsIntOperator TlsIntOpAnd,TlsIntOpOr,TlsIntOpXor;
TlsIntOperator TlsBoolOpAnd,TlsBoolOpOr,TlsBoolOpXor,TlsBoolOpEq;
TlsIntOperator TlsIntOpSar,TlsIntOpSal,TlsIntOpSlr;
TlsIntOperator TlsIntOpInf,TlsIntOpInfEq,TlsIntOpSup,TlsIntOpSupEq;

    /*-------------*/

typedef struct { void **b,**e; } TlsVoidVec;
TlsVoidVec *TlsVoidVecNew(TlsVoidVec *r,void *a0,...);


    /*-------------*/

typedef struct { struct TlsIntVal *Static; } TlsIntVal;
struct TlsIntVal {
    int (*Val)(TlsIntVal *this);
};

extern TlsIntVal TlsIntValNull;
TlsIntVal *TlsConstIntVal(int n);
TlsIntVal *TlsDirectIntVal(int *n);

TlsIntVal *TlsIntValTrans(TlsIntTranslator *op,TlsIntVal *x);

TlsIntVal *TlsIntValSum(TlsIntVal *a,TlsIntVal *b);
TlsIntVal *TlsIntValDiff(TlsIntVal *a,TlsIntVal *b);
TlsIntVal *TlsIntValProduct(TlsIntVal *a,TlsIntVal *b);
TlsIntVal *TlsIntValCmbnd(TlsIntOperator *Op,TlsIntVal *a,TlsIntVal *b);

typedef struct { TlsIntVal **b,**e; } TlsIntValVec;
TlsIntValVec *TlsIntValVecNew(TlsIntValVec *r,TlsIntVal *a0,...);

TlsIntVal *TlsIntValVecMin(TlsIntValVec *v);
TlsIntVal *TlsIntValVecMax(TlsIntValVec *v);
TlsIntVal *TlsIntValVecSum(TlsIntValVec *v);
TlsIntVal *TlsIntValVecProduct(TlsIntValVec *v);

/* (a0 x (a1 x ... ( am x an))) */
TlsIntVal *TlsIntValVecRCmbnd(TlsIntOperator *x,TlsIntValVec *a0);
TlsIntVal *TlsIntValdListRCmbnd(TlsIntOperator *x,TlsDyndList/*<TlsIntVal *>*/ *v);
/* ((((a0 x a1) x ... ) x am) x an) */
TlsIntVal *TlsIntValVecLCmbnd(TlsIntOperator *x,TlsIntValVec *a0);
TlsIntVal *TlsIntValdListLCmbnd(TlsIntOperator *x,TlsDyndList/*<TlsIntVal *>*/ *v);

/*_________________________
 |
 | IntFn 
 |_________________________
*/

typedef struct {struct TlsIntFn *Static; } TlsIntFn;
struct TlsIntFn {
    int (*Val)(TlsIntFn *this,int x);
};

typedef struct {
    TlsIntFn **b,**e;
} TlsIntFnVec;
TlsIntFnVec *TlsIntFnVecNew(TlsIntFnVec *r,TlsIntFn *a0,...);

TlsIntFn *TlsIntFnAdd(TlsIntVal *A);
TlsIntFn *TlsIntFnMax(TlsIntVal *A);
TlsIntFn *TlsIntFnMin(TlsIntVal *A);

TlsIntVal *TlsIntClosedFn(TlsIntFn *f,TlsIntVal *x);
TlsIntFn *TlsIntFnCmpsd(TlsIntFn *f,TlsIntFn *g);

/* y=v0(v1(...vn(x))) */
TlsIntFn *TlsIntFnVecRCmpsd(TlsIntFnVec *v);
TlsIntFn *TlsIntFndListRCmpsd(TlsDyndList/*<TlsIntFn *>*/ *L);
/* y=vn(...v1(v0(x))) */
TlsIntFn *TlsIntFnVecLCmpsd(TlsIntFnVec *v);
TlsIntFn *TlsIntFndListLCmpsd(TlsDyndList/*<TlsIntFn *>*/ *L);


/*__________________________________________________________
 |
 | Evaluation in context.
 | Evaluators perform actions.
 | Each evaluator is tied to a specific context which 
 | contains the parameters of the performed action and/or
 | storage space for the result.
 | Evaluators are often chained and/or nested. Nesting 
 | evaluator allow nested evaluator to perform in
 | context altered by the nesting evaluators.
 |__________________________________________________________
*/

typedef struct { struct TlsEvaluator *Static;} TlsEvaluator;
struct TlsEvaluator {
    void (*Eval)(TlsEvaluator *this,void *Ctx);
};
extern TlsEvaluator TlsEvaluatorNull;

TlsEvaluator *TlsEvalWithSetIntConst(int FieldOff,int Val,TlsEvaluator *Expr);
TlsEvaluator *TlsEvalWithSetInt(int FldOrg,int *Val,TlsEvaluator *Expr);
TlsEvaluator *TlsEvalWithSetIntVal(int FldOrg,TlsIntVal *Val,TlsEvaluator *Expr);

TlsEvaluator *TlsEvalWithSetBlock(int bFld,int eFld,void *Val,TlsEvaluator *Expr);

/*_________________________
 |
 | StringVal
 |_________________________
*/

typedef struct { struct TlsStringVal *Static; } TlsStringVal;
struct TlsStringVal {
    TlsCharString (*Val)(TlsStringVal *this);
};

extern TlsStringVal TlsStringValNull;

/*_____________________________________________________________________________
 |
 | Dynamic arrays.
 | Mem space are the fundations for dynamic arrays.
 | Fixed sized memory block are stored int Memspace.
 | Each array type is responsible for making sense of what exactly those blocks
 | means. 
 | For small data type, a block will typically contains a fixed amount of those
 | datas (usually a power of 2). As a rule, it is ill advised to have blocks
 | of too small size. Blocks of size 0x80 or 0x100 seem about right.
 |_____________________________________________________________________________
*/

typedef struct { struct Tls1dMemSpace *Static; } Tls1dMemSpace;
struct Tls1dMemSpace {
    void *(*GetOrAdd)(Tls1dMemSpace *this,int idx);
    void *(*Get)(Tls1dMemSpace *this,int idx);
    TlsRange *(*Scope)(Tls1dMemSpace *this,TlsRange *rScope);
    void (*ForEach)(Tls1dMemSpace *this,TlsRange *Scope,int (*fn)(int idx,void *Data,void *Closure),void *closure);
    void (*Release)(Tls1dMemSpace *this,TlsRange *range);
};

extern Tls1dMemSpace Tls1dMemSpaceNull;
Tls1dMemSpace *Tls1dMemSpaceNew(int ChunkSize);
Tls1dMemSpace *Tls1dMemSpaceMount(int ChunkSize,TlsMemPool *Pool);

   /*--------*/

typedef struct { struct Tls1dIntSpace *Static;} Tls1dIntSpace;
struct Tls1dIntSpace {
    void (*Set)(Tls1dIntSpace *this,int idx,int v);
    int (*Get)(Tls1dIntSpace *this,int idx);
    TlsRange *(*Scope)(Tls1dIntSpace *this,TlsRange *rScope);
    void (*ForEach)(Tls1dIntSpace *this,TlsRange *Scope,int (*fn)(int idx,int *b,int *e,void *Clos),void *closure);
    void (*Release)(Tls1dIntSpace *this,TlsRange *range);
};
Tls1dIntSpace *Tls1dIntSpaceNew(int Default);

   /*---------*/

typedef struct { struct Tls2dMemSpace *Static; } Tls2dMemSpace;
struct Tls2dMemSpace  {
    void *(*GetOrAdd)(Tls2dMemSpace *this,int x,int y);
    void *(*Get)(Tls2dMemSpace *this,int x,int y);
    void (*ForEach)(Tls2dMemSpace *this,Tls2dArea *Scope,int (*fn)(int x,int y,void *Data,void *Closure),void *clos);
    void (*Release)(Tls2dMemSpace *this,Tls2dArea *Scope);
};

Tls2dMemSpace *TlsSpace2dNew(int LeafSz);

/*_____________________________________________________________
 |
 | UTF8 Encoding/Decoding
 |_____________________________________________________________
*/

int TlsUTF8CodeLength(char *s); // Nb of UTF8 characters in the string.
int TlsUTF8CodeSeek(char *s,int pos); // Gives the index of the 'pos'th UTF8 char in the string.
int TlsUTF8CodingLength(int *s);  // For a string of ints, gives the length of the associated char string. The null string will give you length 0.
char *TlsIntToUTF8(char *d,int *s); // Given a int string, get the associated char string. destination should be long enough. Returns the end of the string (after the end 0).
int *TlsUTF8ToInt(int *d,char *s); // Given a char string, get the associated int string. destination should be long enough. Returns the end of the string (after the end 0).

#endif
