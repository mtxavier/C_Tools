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

#include <List.h>
#include <Tools.h>

List  _ListsEnd_={ListEnd};

void ListInit(List *r){
    r->n = ListEnd;
}

void ListInsert(List *r,List *n) {
    n->n = r->n;
    r->n = n;
}

int ListUnchain(List *r,List *o) {
    List *p;
	int result;
	p = r;
	while (p->n != o && p->n!=ListEnd) p++;
	result = (p->n == o);
	if (result) {
	    p->n = o->n;
	}
	return result;
}

List *ListForEach(List *r,int (*f)(List *n,void *closure),void *Closure){
    List *p;
	int stop;
	List *result;
	p = r->n;
	result = r;
	stop = (p==ListEnd);
	while (!stop) {
	    result = p;
		p = p->n;
	    stop = f(result,Closure) || (p==ListEnd);
	}
	return result;
}

/*_____________________________________________________________________________
 |
 | Fifo
 |_____________________________________________________________________________
*/

void FifoInit(Fifo *r) {
    r->Last = &r->Fifo;
    r->Fifo.n = ListEnd;
}

void FifoInsert(Fifo *r,List *n) {
    n->n = r->Last->n;
    r->Last->n = n;
    r->Last = r->Last->n;
}

/*_____________________________________________________________________________
 |
 | double list
 |_____________________________________________________________________________
*/

dList _dListEnd_;

void dListInit(dList *r) {
    r->n=dListEnd;
    r->p=dListEnd;
};

void dListInsert(dList *r,dList *n) {
    n->n=r->n;
    n->p=r;
    if (n->n!=dListEnd) n->n->p=n;
    r->n=n;
}

void dListUnchain(dList *n) {
    if (n->n!=dListEnd) {
	n->n->p=n->p;
    }
    if (n->p!=dListEnd) {
	n->p->n=n->n;
    }
    n->n=n->p=dListEnd;
}

dList *dListForEach(dList *r,int (*f)(dList *n,void *closure),void *Closure){
    dList *p;
	int stop;
	dList *result;
	p = r->n;
	result = r;
	stop = (p==dListEnd);
	while (!stop) {
	    result = p;
		p = p->n;
	    stop = f(result,Closure) || (p==dListEnd);
	}
	return result;
}


/*_____________________________________________________________________________
 |
 | BinTree
 |_____________________________________________________________________________
*/

BinTree _BinTreeEnd_ = {{BinTreeEnd,BinTreeEnd},0};
void BinTreeInit(BinTree *root){
    *root=*BinTreeEnd;
}

BinTree *BinTreeSeek(BinTree *Root,int Id){
    BinTree *r;
    int Path;
    r = Root->Children[0];
    Path = Id;
    while ((r->Id!=Id)&&(r!=BinTreeEnd)) {
	r=r->Children[Path & 1];
	Path=Path>>1;
    }
    return r;
}

BinTree *BinTreeSeekOrInsert(BinTree *Root,int Id,BinTree *New) {
    BinTree *r,*q;
    int qPath,Path;
    qPath=0;
    Path=Id;
    q=Root; r=q->Children[0];
    while ((r->Id!=Id)&&(r!=BinTreeEnd)) {
	qPath=Path;
	q=r;
	r=r->Children[Path & 1];
	Path=Path>>1;
    }
    if (r==BinTreeEnd) {
	New->Id=Id;
	New->Children[0]=New->Children[1]=BinTreeEnd;
	q->Children[qPath & 1] = New;
	r=New;
    }
    return r;
}

BinTree *BinTreeRemove(BinTree *Root,int Id) {
    BinTree *r,*q;
    int qPath,Path;
    qPath=0;
	q = Root;
    Path=Id;
    r = q->Children[0];
    while ((r->Id!=Id) && (r!=BinTreeEnd)) {
		q = r;
	    qPath=Path;
	    r=r->Children[Path & 1];
	    Path=Path>>1;
    }
    if (r!=BinTreeEnd) {
	if (r->Children[0]==BinTreeEnd && r->Children[1]==BinTreeEnd) {
	    q->Children[qPath & 1] = BinTreeEnd;
	} else {
        BinTree *p,*qp;
	    p=qp=r;
	    do {
		    qp=p;
		    if (p->Children[0]==BinTreeEnd) {
		        Path=1;
		        p=r->Children[1];
		    } else {
		        Path=0;
		        p=r->Children[0];
		    }
	    } while ((p->Children[0]!=BinTreeEnd)||(p->Children[1]!=BinTreeEnd));
	    qp->Children[Path]=BinTreeEnd;
        p->Children[0]=r->Children[0];
	    p->Children[1]=r->Children[1];
	    q->Children[qPath & 1]=p;
	}
    }
    return r;
}

typedef struct {
    void *Closure;
    BinTree *r;
    int (*f)(BinTree *n,void *closure);
} BTForEachClosure;

BinTree *BinTreeForEachRecurse(BinTree *n,BTForEachClosure *Closure) {
    if (Closure->f(n,Closure->Closure))  {
	Closure->r=n;
    } else {
	if (n->Children[0]!=BinTreeEnd) {
	    Closure->r=BinTreeForEachRecurse(n->Children[0],Closure);
	    if (Closure->r==BinTreeEnd && n->Children[1]!=BinTreeEnd) {
                Closure->r=BinTreeForEachRecurse(n->Children[1],Closure);
	    }
	} else {
	    if (n->Children[1]!=BinTreeEnd) {
		Closure->r=BinTreeForEachRecurse(n->Children[1],Closure);
	    }
	}
    }
    return Closure->r;
}

BinTree *BinTreeForEach(BinTree *Root,int (*f)(BinTree *n,void *closure),void *Closure){
    BinTree *r;
    if (Root->Children[0]!=BinTreeEnd) {
        BTForEachClosure C;
        C.Closure=Closure;
        C.r=BinTreeEnd;
        C.f=f;
        r=BinTreeForEachRecurse(Root->Children[0],&C);
    } else {
	r=BinTreeEnd;
    }
    return r;
}

/*_____________________________________________________________________________
 |
 | BinTree (semi ordered).
 | Additional constraint is that lower depth implies lower Id.
 | The result is that the tree is unique for a given set of Ids.
 | Also, running the tree orderly is easier.
 |_____________________________________________________________________________
*/

BinTree *BinTreeOSeek(BinTree *Root,int Id) {
    BinTree *r;
    int Path;
    r = Root->Children[0];
    Path = Id;
    while (((unsigned int)(r->Id)<(unsigned int)(Id))&&(r!=BinTreeEnd)) {
	r=r->Children[Path & 1];
	Path=Path>>1;
    }
    if (r->Id!=Id) r=BinTreeEnd;
    return r;
}

BinTree *BinTreeOSeekOrInsert(BinTree *Root,int Id,BinTree *New) {
    BinTree *R;
    R = BinTreeOSeek(Root,Id);
    if (R == BinTreeEnd) {
	BinTree *q,*p,*r;
	int qPath,pPath,Depth;
	r = R = New;
	R->Id = Id;
	q = Root; qPath = 0;
        p = Root->Children[0]; pPath=Id;
	Depth = 0;
	while (((unsigned int)(p->Id)<(unsigned int)(r->Id))&&(p!=BinTreeEnd)) {
	    qPath = pPath;
	    q = p;
	    p = p->Children[pPath & 1];
	    pPath = pPath>>1;
	    Depth++;
	}
	while (p!=BinTreeEnd) {
	    r->Children[0] = p->Children[0];
	    r->Children[1] = p->Children[1];
	    q->Children[qPath & 1] = r;
	    q = r;
	    qPath = (p->Id>>Depth); Depth++;
	    r = p;
	    p = p->Children[qPath & 1];
	}
	r->Children[0] = r->Children[1] = BinTreeEnd;
        q->Children[qPath & 1] = r;
    }
    return R;
}

BinTree *BinTreeORemove(BinTree *Root,int Id) {
    BinTree *R;
    R = BinTreeOSeek(Root,Id);
    if (R != BinTreeEnd) {
        BinTree s;
	BinTree *q,*p;
	int qPath,pPath;
	q = Root; qPath = 0;
	p = Root->Children[0]; pPath=Id;
	while (p!=R) {
	    qPath = pPath;
	    q = p;
	    p = p->Children[pPath & 1];
	    pPath = pPath>>1;
	}
	s = *R;
	while (q!=BinTreeEnd) {
	    BinTree *r;
	    if (s.Children[0]==BinTreeEnd) {
		r = s.Children[1];
	    } else {
		if (s.Children[1]==BinTreeEnd) {
		    r = s.Children[0];
		} else {
		    if ((unsigned int)(s.Children[0]->Id)<(unsigned int)(s.Children[1]->Id)) {
			r = s.Children[0];
		    } else {
			r = s.Children[1];
		    }
		}
	    }
	    q->Children[qPath & 1] = r;
	    q = r;
	    if (r!=BinTreeEnd) {
		s = *r;
		if (r==p->Children[0]) {
		    r->Children[1] = s.Children[1];
		    r->Children[0] = BinTreeEnd;
		    qPath = 0;
		} else {
                    r->Children[0] = s.Children[0];
		    r->Children[1] = BinTreeEnd;
		    qPath = 1;
		}
	    }
	}
    }
    return R;
}

typedef struct {
    int Max;
    int Min;
    int (*f)(BinTree *B,void *Closure);
    void *Closure;
} _BinTreeForEachNegPass;

int BinTreeOForEachNegPass(BinTree *B,void *Closure) {
    _BinTreeForEachNegPass *c;
    int R;
    c = Closure;
    if (B->Id>c->Max) c->Max=B->Id;
    R = B->Id<0;
    if (R) {
	R = c->f(B,c->Closure);
    }
    return R;
}

int BinTreeOForEachPosPass(BinTree *B,void *Closure) {
    _BinTreeForEachNegPass *c;
    int R;
    c = Closure;
    R = (B->Id>=c->Min)&&(B->Id<=c->Max);
    if (R) {
	R = c->f(B,c->Closure);
    }
    return R;
}

BinTree *BinTreeOForEach(BinTree *Root,int (*f)(BinTree *n,void *closure),void *Closure){
    _BinTreeForEachNegPass C;
    BinTree *R;
    int Max;
    C.Max = -1;
    C.Min = 0;
    C.f = f;
    C.Closure = Closure;
    R = BinTreeForEach(Root,BinTreeOForEachNegPass,&C);
    Max = C.Max;
    C.Max = 1;
    while (R==BinTreeEnd && C.Max<=Max && C.Max>0 && Max>0) {
	R = BinTreeForEach(Root,BinTreeOForEachPosPass,&C);
	C.Min = C.Max+1;
	C.Max = ((C.Max+1)<<1)-1;
    }
    return R;
}

/*_____________________________________________________________________________
 |
 | TlsIntTries
 |     Properties of the Trie:
 |    (1) * The 'ToTest' bit is strictly decreasing while we descend the tree. 
 |    (1')* We've reach a leaf when the 'ToTest' bit stay the same or increase.
 |    (2) * Every nodes of the same subtree share the same radix up to the
 |          (non included) 'ToTest' bit of the subtree root.
 |    (3) * On a non empty Trie, there is exactly one node '1stOne' with a 
 |          'ToTest' bit set to intLength(). That nod has also the following 
 |           properties:
 |               * 1stOne.Children[0] == &1stOne;
 |               * 1stOne.Children[1] == &1stOne; (by convention we take the root 
 |                 node as the NULL node).
 |    (4) * All the other nodes have a 'ToTest' field >=0. They also all point
 |          on valid nodes or leaf.
 |-----------------------------------------------------------------------------
 | Implementation note : it seems that the behaviour of (x<<s) might depend on
 | architecture when s is above or equal the x width. Therefore, we have to
 | make a special case when we think that might happen.
 |_____________________________________________________________________________
*/

#define intLength() (TlsAtomSize(int)<<3)

void TlsIntTrieInit(TlsIntTrie *Root) {
	Root->Children[0] = Root->Children[1] = Root;
	Root->Key = 0;
	Root->ToTest = intLength();
}
int TlsIntTrieIsEmpty(TlsIntTrie *Root) { return (Root->Children[0]==Root); }
TlsIntTrie *TlsIntTrieSeek(TlsIntTrie *Root,int Nod) {
	TlsIntTrie *r,*n;
	unsigned int nod;
	int cont;
	nod = Nod;
	n = Root;
	r = n->Children[0];
	while (n->ToTest>r->ToTest) {
		n = r;
		r = n->Children[(nod>>n->ToTest)&1];
	}
	return (r->Key==nod)?r:Root;
}
TlsIntTrie *TlsIntTrieLowest(TlsIntTrie *root) {
	TlsIntTrie *r,*n;
	n = root;
    r = n->Children[0];
	while (r->ToTest<n->ToTest) { n=r; r=n->Children[0]; }
	return r;
}
TlsIntTrie *TlsIntTrieHighest(TlsIntTrie *root) {
	TlsIntTrie *r,*n;
	n = root;
    r = n->Children[0];
	while (r->ToTest<n->ToTest) { n=r; r=n->Children[1]; }
	return r;
}

struct rBTSegSeek {
	TlsIntTrie *root,*min,*max;
	unsigned int val;
};
static void rBTSegSeek(TlsIntTrie *p,struct rBTSegSeek *c) {
	unsigned int min0,min1;
	int lstOne;
	TlsIntTrie *n;
	min0 = (p->ToTest>=(intLength()-1))?0:p->Key&(-1<<(p->ToTest+1));
	if (c->val>=min0) {
	    min1 = min0+(1<<p->ToTest);
		if (c->val>=min1) {
		    n = p->Children[1];
			if (p->ToTest<=n->ToTest) {
				if (n->Key<=c->val) { c->min = n; }
			} else {
			    rBTSegSeek(n,c);
			}
		}
		if (c->min==c->root) {
		    n = p->Children[0];
			if (p->ToTest<=n->ToTest) {
				if (n->Key<=c->val) { c->min = n; }
			} else {
                rBTSegSeek(n,c);
			}
			if ((c->min!=c->root)&&(c->max==c->root)) {
				n = p->Children[1];
				while (n->ToTest<p->ToTest) {
					p = n;
					n = p->Children[0];
				}
                c->max = n;
			}
		}
	}
}
TlsIntTrieSeg *TlsIntTrieBoundaries(TlsIntTrieSeg *r,TlsIntTrie *Root,int Val) {
	r->Begin = r->End = Root;
	if (Root->Children[0]->Key==intLength()) {
		r->Begin = r->End = Root->Children[0];
	} else {
		struct rBTSegSeek cl;
		cl.min = cl.max = cl.root = Root;
		cl.val = Val;
		rBTSegSeek(Root->Children[0],&cl);
		r->Begin = cl.min; r->End = cl.max;
		if (cl.min==Root) r->Begin = TlsIntTrieHighest(Root); 
		if (cl.max==Root) r->End = TlsIntTrieLowest(Root);
	}
	return r;
}

    /*----*/

static inline int firstOne(unsigned int v) {
	int r;
    TlsILg2(r,v);
	return r;
}
TlsIntTrie *TlsIntTrieSeekOrInsert(TlsIntTrie *Root,int Nod,TlsIntTrie *New) {
	TlsIntTrie *r,*p,**pp;
	int cont,tt;
	unsigned int nod,k;
	nod = Nod;
	p = Root;
	pp = p->Children+0;
	r = *pp;
	if (r==Root) { /* Empty Tree, we insert the 1stOne node */
		r = New;
		r->Key = nod;
		r->ToTest = intLength();
		r->Children[0] = r;
		r->Children[1] = r; // Root;
		*pp = r;
	} else {
		tt = r->ToTest;
	    cont = (p->ToTest>tt);
	    while ((cont)&&(r->Key!=nod)) {
		    unsigned int k,l;
		    k = nod>>tt;
		    l = r->Key>>tt;
		    cont = (((k^l)|1)==1);
				/* if !cont, the radix don't match. 
				   The new nod doesn't belong to the subtree and must be 
				   inserted here. */
			if (cont) {
				p = r;
				pp = r->Children+(k&1);
				r = *pp;
				tt = r->ToTest;
				cont = (p->ToTest>tt);
			}
	    }
		if (r->Key!=nod) {
			tt = firstOne(r->Key^nod);
			k = (nod>>tt)&1;
			New->Key = nod;
			New->ToTest = tt;
			New->Children[k]=New;
			New->Children[k^1]=r;
			r = *pp = New;
		} 
	}
	return r;
}

    /*------*/

TlsIntTrie *TlsIntTrieRemove(TlsIntTrie *Root,int Nod) {
	TlsIntTrie *r,*q,**pq,*p,**pp,**pr;
	unsigned int nod;
	int tt;
	q = r = Root;
	nod = Nod;
	pr = pp = pq = q->Children+0;
	p = *pp; tt = p->ToTest;
	while (q->ToTest>tt) {
		if (p->Key==nod) { pr = pp; }
        pq = pp; q = p;
        pp = p->Children+((nod>>tt)&1);
		p = *pp; tt = p->ToTest;
	}
	if ((p->Key==nod)&&(p!=Root)) {
		r = p;
		if (tt==intLength()) {
			/* We remove the 1stOne. */ 
			if (q==Root) { 
				/* It's the case with only one nod in the trie. */
                q->Children[0] = Root;
			} else { 
				/* q can and must become the new 1stOne. */
			    *pq = q->Children[((nod>>q->ToTest)&1)^1];
				q->ToTest = intLength();
				q->Children[0] = q;
                q->Children[1] = q; // Root;
			}
		} else {
		    if (q==p) {
		        *pq = q->Children[((nod>>q->ToTest)&1)^1];
	        } else {
			    *pq = q->Children[((nod>>q->ToTest)&1)^1];
			    q->ToTest = p->ToTest;
			    q->Children[0] = p->Children[0];
			    q->Children[1] = p->Children[1];
			    *pr = q;
		    }
		}
	}
	return r;
}
 
   /*------*/

struct BTSegForEach {
	int (*f)(TlsIntTrie *b,void *closure);
	void *Closure;
	TlsIntTrie *Found;
};
static int rBinTreeSegNodForEach(TlsIntTrie *n,struct BTSegForEach *c) {
	int r;
	TlsIntTrie **m,**e,*f;
	r = (0!=0);
    m = n->Children;
	e = (n->ToTest==intLength())?m+1:m+2; // FirstOne has two children pointing to itself.
	while ((m<e)&&(!r)) {
		f = *m; m++;
	    if (n->ToTest>f->ToTest) {
		    r = rBinTreeSegNodForEach(f,c);
	    } else {
	        r = c->f(f,c->Closure);
			if (r) {
				c->Found = f;
			}
	    }
	}
	return r;
}
TlsIntTrie *TlsIntTrieForEach(TlsIntTrie *Root,
	int (*f)(TlsIntTrie *b,void *closure),void *Closure
) {
	struct BTSegForEach c;
	TlsIntTrie *fst;
	fst = Root->Children[0];
	c.Found = Root;
	if (fst->ToTest==intLength()) {
		if (fst!=Root) {
			if (f(fst,Closure)) {
				c.Found = fst;
			}
		}
	} else {
	    c.f = f;
	    c.Closure = Closure;
        rBinTreeSegNodForEach(Root->Children[0],&c);
	}
	return c.Found;
}

    /*----*/

struct btSegStruct {
	TlsIntTrie *Root,*Found;
	int (*nod)(TlsIntTrie *n,void *c);
	int (*leaf)(TlsIntTrie *n,void *c);
	void *Closure;
};
static void rBinTreeSegStruct(TlsIntTrie *n,struct btSegStruct *Cls) {
	int cont;
	if (n!=Cls->Root) {
		if (Cls->nod(n,Cls->Closure)) {
			Cls->Found = n;
		} else {
			if (n->ToTest>n->Children[0]->ToTest) {
                rBinTreeSegStruct(n->Children[0],Cls);
			} else {
				if (Cls->leaf(n->Children[0],Cls->Closure)) { 
					Cls->Found = n->Children[0]; 
				}
			}
			if (Cls->Found==Cls->Root) {
				if (n->ToTest>n->Children[1]->ToTest) {
					rBinTreeSegStruct(n->Children[1],Cls);
				} else { 
					if (Cls->leaf(n->Children[1],Cls->Closure)) { 
					    Cls->Found = n->Children[1]; 
					} 
				}
			}
		}
	}
}	
TlsIntTrie *TlsIntTrieStruct(TlsIntTrie *Root,int (*nod)(TlsIntTrie *n,void *C),int (*leaf)(TlsIntTrie *n,void *C),void *c) {
	struct btSegStruct prm;
	prm.nod = nod; prm.leaf=leaf; prm.Closure = c; prm.Found = prm.Root = Root;
	rBinTreeSegStruct(Root->Children[0],&prm);
	return prm.Found;
}

/*_____________________________________________________________________________
 |
 | Network;
 |_____________________________________________________________________________
*/

#include "StackEnv.h"
#include <stdarg.h>

NetNode *NetNodeInit(NetNode *N) {
    N->Color = (0!=0);
    N->Links.n = ListEnd;
    return N;
}

void NetNodeSetLink(NetNode *A,NetLink *L,NetNode *B) {
    L->Node = B;
    L->List.n = A->Links.n;
    A->Links.n = &L->List;
}

NetNode *NetNodeTie(NetNode *N,...) {
    va_list p;
    NetNode *P;
    N->Color = (0!=0);
    va_start(p,N);
    P = va_arg(p,NetNode *);
    while (P!=0) {
	NetLink *L;
	rPush(L);
	L->Node = P;
	L->List.n = N->Links.n;
	N->Links.n = &L->List;
        P = va_arg(p,NetNode *);
    }
    va_end(p);
    return N;
}

int NetNodeLinked(NetNode *A,NetNode *B) {
    List *p;
    int R;
    p = A->Links.n;
    R = (0!=0);
    while ((!R)&&(p!=ListEnd)) {
	NetLink *P;
	P = CastBack(NetLink,List,p);
	R = (P->Node==B);
	p = p->n;
    }
    return R;
}

void NetNodeColor(NetNode *A,int C0) {
    List *p;
    A->Color = C0;
    p = A->Links.n;
    while (p!=ListEnd) {
        NetLink *P;
	P = CastBack(NetLink,List,p);
	p = p->n;
	if (P->Node->Color!=C0) NetNodeColor(P->Node,C0);
    }
}

int rxNetNodeConnected(NetNode *A,NetNode *B) {
    int R;
    R = (0!=0);
    if (!A->Color) {
        List *p;
        A->Color = (0==0);
        p = A->Links.n;
	while ((p!=ListEnd)&&(!R)) {
	    NetLink *P;
	    P = CastBack(NetLink,List,p);
            p = p->n;
	    R = (P->Node==B);
	    if (!R) R = rxNetNodeConnected(P->Node,B);
	}
    } 
    return R;
}

int NetNodeConnected(NetNode *A,NetNode *B) {
    int R;
    R = rxNetNodeConnected(A,B);
    NetNodeColor(A,0!=0);
    return R;
}

List *rxNetNodePath(NetNode *A,NetNode *B) {
    List *R;
    R = ListEnd;
    if (!A->Color) {
	List *p;
	A->Color = (0==0);
	p = A->Links.n;
	while ((p!=ListEnd)&&(R==ListEnd)) {
	    NetLink *P;
	    P = CastBack(NetLink,List,p);
	    p = p->n;
	    if (P->Node!=B) R=rxNetNodePath(P->Node,B);
	    if ((R!=ListEnd)||(P->Node==B)) {
		NetPath *Q;
	        rPush(Q);
	        Q->List.n=R;
	        Q->Link=P;
		R=&Q->List;
	    }
	}
    }
    return R;
}

List *NetNodePath(NetNode *A,NetNode *B) {
    List *R;
    R = rxNetNodePath(A,B);
    NetNodeColor(A,(0!=0));
    return R;
}

typedef struct {
    void *Closure;
    NetNode *r;
    int (*f)(NetNode *n,void *closure);
} NNForEachClosure;

NetNode *rxNetNodeForEach(NetNode *r,NNForEachClosure *C) {
    if (!r->Color) {
        List *p;
        r->Color = (0==0);
	p = r->Links.n;
	while ((p!=ListEnd)&&(C->r==0)) {
	    NetLink *P;
	    P = CastBack(NetLink,List,p);
	    p = p->n;
	    if (!P->Node->Color) rxNetNodeForEach(P->Node,C);
	}
	if (C->r==0) {
	    if (C->f(r,C->Closure)) {C->r=r;}
	}
    }
    return C->r;
}

NetNode *NetNodeForEach(NetNode *r,int(*f)(NetNode *n,void *closure),void *Closure) {
    NNForEachClosure Param;
    Param.Closure = Closure;
    Param.r = 0;
    Param.f = f;
    rxNetNodeForEach(r,&Param);
    NetNodeColor(r,(0!=0));
    return Param.r;
}

/*_________________________________________________________________________
 |
 |_________________________________________________________________________
*/

NetChar _NetCharNull_ = {NetCharNull};
NetChar *NetWordNull[] = {NetCharNull};

void rxNetLabeledRecogniseWord(NetNode *Start,NetChar **Word,List *R) {
    if (*Word!=NetCharNull) {
	List *p;
	p = Start->Links.n;
	while ((p!=ListEnd)&&(R->n==ListEnd)) {
	    NetLabeledLink *P;
	    P = CastBack(NetLabeledLink,NetLink.List,p);
	    p = p->n;
	    if (P->Label.o==Word[0]->o) {
                if (Word[1]!=NetCharNull) {
		    NetNode *S;
		    rxNetLabeledRecogniseWord(P->NetLink.Node,Word+1,R);
		}
		if ((Word[1]==NetCharNull)||(R->n!=ListEnd)) {
		    NetPath *NP;
		    rPush(NP);
		    NP->List.n = R->n;
		    NP->Link = &P->NetLink;
		    R->n = &NP->List;
                }
	    }
	}
    }
}

List *NetLabeledRecogniseWord(NetNode *Start,NetChar **Word) {
    List R;
    R.n = ListEnd;
    rxNetLabeledRecogniseWord(Start,Word,&R);
    return R.n;
}

