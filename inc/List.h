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

#ifndef _List_h_
#define _List_h_

#include <Classes.h>

/*_____________________________________________________________________________
 |
 | r node is not part of the list, neither the various ListEnd variables.
 | ForEach ends the first time a List returns (0==0) and it returns it to the 
 | caller.
 |_____________________________________________________________________________
*/

typedef struct _List_ List;
struct _List_ {
    List *n;
};

extern List _ListsEnd_;
#define ListEnd (&_ListsEnd_)
#define ListVoid(r) ((r)->n==ListEnd);
#define ListFirst(r) ((r)->n)
void ListInit(List *r);
void ListInsert(List *r,List *n);
int  ListUnchain(List *r,List *o);
List *ListForEach(List *r,int (*f)(List *n,void *closure),void *Closure);

/*_____________________________________________________________________________
 |
 | Fifo
 |_____________________________________________________________________________
*/

typedef struct {
    List Fifo,*Last;
} Fifo;
void FifoInit(Fifo *r);
void FifoInsert(Fifo *r,List *n);
#define FifoForEach(r,f,c) ListForEach(&r->Fifo,f,c);


/*_____________________________________________________________________________
 |
 | double list
 |_____________________________________________________________________________
*/

typedef struct _dList_ dList;
struct _dList_ {
    dList *n,*p;
};
extern dList _dListEnd_;
#define dListEnd (&_dListEnd_)
#define dListIsVoid(r) ((r)->n==ListEnd)
#define dListFirst(r) ((r)->n)
void dListInit(dList *r);
void dListInsert(dList *r,dList *n);
void dListUnchain(dList *n);
dList *dListForEach(dList *r,int (*f)(dList *n,void *closure),void *Closure);


/*_____________________________________________________________________________
 |
 | BinTree
 |_____________________________________________________________________________
*/

typedef struct _BinTree_ BinTree;
struct _BinTree_ {
    BinTree *Children[2];
    int Id;
};
extern BinTree _BinTreeEnd_;
#define BinTreeEnd (&_BinTreeEnd_)
void BinTreeInit(BinTree *root);
BinTree *BinTreeSeek(BinTree *Root,int Id); /* if not found, return BinTreeEnd */
#define BinTreeNotFound(f) ((f)==BinTreeEnd)
BinTree *BinTreeSeekOrInsert(BinTree *Root,int Id,BinTree *New);
BinTree *BinTreeRemove(BinTree *Root,int Id);
BinTree *BinTreeForEach(BinTree *Root,int (*f)(BinTree *n,void *clodure),void *Closure);

/*_____________________________________________________________________________
 |
 | The following keep the item organised in crescent order.
 |_____________________________________________________________________________
*/

BinTree *BinTreeOSeek(BinTree *Root,int Id);
BinTree *BinTreeOSeekOrInsert(BinTree *Root,int Id,BinTree *New);
BinTree *BinTreeORemove(BinTree *Root,int Id);
BinTree *BinTreeOForEach(BinTree *Root,int (*f)(BinTree *n,void *clodure),void *Closure);

/*_____________________________________________________________________________
 |
 | TlsIntTrie is used to partition the space in segments delimited by "Nods".
 |_____________________________________________________________________________
*/

typedef struct TlsIntTrie TlsIntTrie;
struct TlsIntTrie {
    TlsIntTrie *Children[2];
    unsigned int Key;
    int ToTest;
};
typedef struct { 
    TlsIntTrie *Begin,*End; // As usual, Begin included, End Excluded.
} TlsIntTrieSeg;
void TlsIntTrieInit(TlsIntTrie *Root);
int TlsIntTrieIsEmpty(TlsIntTrie *Root);
TlsIntTrie *TlsIntTrieSeek(TlsIntTrie *Root,int Nod);
TlsIntTrie *TlsIntTrieSeekOrInsert(TlsIntTrie *Root,int Nod,TlsIntTrie *New);
TlsIntTrie *TlsIntTrieRemove(TlsIntTrie *Root,int Nod);
TlsIntTrie *TlsIntTrieForEach(TlsIntTrie *Root,int (*f)(TlsIntTrie *b,void *closure),void *Closure);
TlsIntTrie *TlsIntTrieLowest(TlsIntTrie *root);
TlsIntTrie *TlsIntTrieHighest(TlsIntTrie *root);
TlsIntTrieSeg *TlsIntTrieBoundaries(TlsIntTrieSeg *r,TlsIntTrie *Root,int Val);
// For debugging, gives the actual tree structure.
TlsIntTrie *TlsIntTrieStruct(TlsIntTrie *Root,int (*nod)(TlsIntTrie *n,void *c),int (*leaf)(TlsIntTrie *n,void *c),void *Closure); 

/*_____________________________________________________________________________
 |
 |  Network
 |_____________________________________________________________________________
*/

typedef struct NetNode {
    List Links; /* List of NetLink */
    int Color;  /* Used for internal management only. */
} NetNode;
typedef struct {
    List List;
    NetNode *Node;
} NetLink;
typedef struct {
    List List;
    NetLink *Link;
} NetPath;

NetNode *NetNodeInit(NetNode *N);
NetNode *NetNodeTie(NetNode *N,...);
void NetNodeSetLink(NetNode *A,NetLink *N,NetNode *B);
int NetNodeLinked(NetNode *A,NetNode *B);
int NetNodeConnected(NetNode *A,NetNode *B); /* Connected (A,A) is not automatically true: you have to put a reflexive link.*/
List *NetNodePath(NetNode *A,NetNode *B);    /* NetNodePath(A,A)!=ListEnd only if A has a reflexive link  */
NetNode *NetNodeForEach(NetNode *r,int(*f)(NetNode *n,void *closure),void *Closure);

/*_____________________________________________________________________________
 |
 |  LabeledNet
 |_____________________________________________________________________________
*/

typedef struct _NetChar_ NetChar;
struct _NetChar_ {
    NetChar *o;
};
extern NetChar _NetCharNull_;
extern NetChar *NetWordNull[];
#define NetCharNull (&_NetCharNull_)
#define NetCharSet(A) (A)->o=(A)

typedef struct {
    NetLink NetLink;
    NetChar Label;
} NetLabeledLink;

List/*<NetPath>*/ *NetLabeledRecogniseWord(NetNode *Start,NetChar **Word);
#define NetLabeledLinkSet(L,S,D) NetNodeSetLink((S),&(L)->NetLink,(D))

#define NetTree BinTree
#define NetTreeInit BinTreeInit
#define NetTreeEnd (&_BinTreeEnd_)
#define NetTreeSeek(Root,Id) BinTreeSeek(Root,(int)(Id))
#define NetTreeSeekOrInsert(Root,Id,New) BinTreeSeekOrInsert(Root,(int)(Id),New)
#define NetTreeNotFound BinTreeNotFound
#define NetTreeRemove(Root,Id) BinTreeRemove(Root,(int)(Id))
#define NetTreeForEach(Root,f,Closure) BinTreeForEach(Root,f,Closure)
#define NetTreeKey(Nod) ((NetChar *)((Nod)->Id))
#define NetTreeIsKey(Nod) NetCharSet((NetChar *)(&Nod->Id))

/*____________________________________________________________________________
 |
 | PtrTree: record objects pointer in a BinTree
 |____________________________________________________________________________
*/

#define BinTreeKey(t,v) ((int)(v))
#define BinTreeKeyVal(t,Nod) ((t *)(Nod->Id))

#endif

