#include <StackEnv.h>
#include <Classes.h>
#include <List.h>
#include <stdio.h>

typedef struct {
	struct {
	    TlsIntTrie n[128],*c,*e;
	} pool;
	unsigned char Set[0x2000];
	TlsIntTrie root;
	int Errors,Inserted,Min,Max,Org;
} BTSegEnv;

static BTSegEnv *BTSegEnvReset(BTSegEnv *r,int Org) {
	TlsIntTrieInit(&r->root);
	r->pool.c = r->pool.n;
	r->pool.e = r->pool.c+128;
	{
		unsigned char *p,*e;
		p = r->Set; e = p+0x2000; while (p<e) *p++=0;
	}
	r->Errors = 0;
	r->Inserted = 0;
	r->Min = 0x10000; r->Max=-1;
    r->Org = Org;
	return r;
}

static void BTSegInsert(BTSegEnv *e,int Id) {
	TlsIntTrie *f;
	Id = Id&0xffff;
	f = TlsIntTrieSeek(&e->root,Id);
	if ((e->Set[Id>>3]>>(Id&7))&1) {
		if (f==&e->root) {
			e->Errors++;
		}
	} else {
		if (f!=&e->root) {
			e->Errors++;
		}
		f = TlsIntTrieSeekOrInsert(&e->root,Id,e->pool.c);
		if (f!=e->pool.c) {
            e->Errors++;
		}
		f = TlsIntTrieSeek(&e->root,Id);
		if (f!=e->pool.c) {
			e->Errors++;
		}
		e->pool.c++;
        e->Inserted++;
		if (Id<e->Min) {e->Min=Id;}
		if (Id>e->Max) {e->Max=Id;}
		e->Set[Id>>3]|=(1<<(Id&7));
	}
}

static void BTSegRemove(BTSegEnv *e,int Id) {
	TlsIntTrie *f;
	int f0;
	Id = Id&0xffff;
	f = TlsIntTrieSeek(&e->root,Id);
    f0 = (e->Set[Id>>3]>>(Id&7))&1;
	if ((f0==1)!=(f!=&e->root)) {
		e->Errors++;
	} else {
		if (f0) {
			TlsIntTrieRemove(&e->root,Id);
			e->Set[Id>>3] &= (0xff-(1<<(Id&7)));
			e->Inserted--;
			f = TlsIntTrieSeek(&e->root,Id);
			if (f!=&e->root) {
				e->Errors++;
			}
		}
	}
}

static BTSegEnv *BTSegEnvNew(void) {
	BTSegEnv *r;
	rPush(r);
	return BTSegEnvReset(r,0);
}

struct NodList {
	List List;
	int Key;
};
struct fCheckNodOrder {
	BTSegEnv *e;
    List/*<NodList.List>*/ enm,*lst;
	int count,errors;
};
static int fCheckNodOrder(TlsIntTrie *b,void *Cl) {
	struct fCheckNodOrder *c;
	TlsIntTrie *chk;
	struct NodList *n;
	c =  Cl;
	if (((((c->e->Set[b->Key>>3])>>(b->Key&7))&1)==0)) {
		c->errors++;
	}
	c->count++;
	rPush(n);
	n->Key = b->Key;
	n->List.n = c->lst->n;
	c->lst->n = &n->List;
	c->lst = c->lst->n;
	return (0!=0);
}

static void BTSegContentCheck(BTSegEnv *t) {
	rOpen
	{
		struct fCheckNodOrder no;
		List *b,*e;
		no.enm.n = ListEnd;
		no.lst = &no.enm;
		no.e = t;
		no.count = 0;
		no.errors = 0;
		TlsIntTrieForEach(&t->root,fCheckNodOrder,&no);
		t->Errors += no.errors;
		if (no.count!=t->Inserted) {
			t->Errors++;
		}
        b = no.enm.n;
		e = b->n;
		while (e!=ListEnd) {
			struct NodList *B,*E;
			TlsIntTrie *chk;
			TlsIntTrieSeg chk0;
			B = CastBack(struct NodList,List,b);
			E = CastBack(struct NodList,List,e);
            chk = TlsIntTrieSeek(&t->root,B->Key);
			if (chk==&t->root) {
				t->Errors++;
			} else {
				TlsIntTrieBoundaries(&chk0,&t->root,B->Key);
				if ((chk0.Begin->Key!=B->Key)||(chk0.End->Key!=E->Key)) {
					t->Errors++;
				}
				TlsIntTrieBoundaries(&chk0,&t->root,(B->Key+E->Key)>>1);
				if ((chk0.Begin->Key!=B->Key)||(chk0.End->Key!=E->Key)) {
					t->Errors++;
				}
				TlsIntTrieBoundaries(&chk0,&t->root,E->Key-1);
				if ((chk0.Begin->Key!=B->Key)||(chk0.End->Key!=E->Key)) {
					t->Errors++;
				}
			}
			b = e; e = e->n;
		}
	}
	rClose
}

static void BTSegInsertTest(BTSegEnv *t,int *b,int *e) {
	TlsIntTrie *f;
	int *c;
	c = b;
	while (c<e) {
        BTSegInsert(t,*c);
		c++;
	}
	f = TlsIntTrieLowest(&t->root);
	if (f!=&t->root) {
		if (f->Key!=t->Min) {
			t->Errors++;
		}
	}
	f = TlsIntTrieHighest(&t->root);
	if (f!=&t->root) {
		if (f->Key!=t->Max) {
			t->Errors++;
		}
	}
	BTSegContentCheck(t);
}

static void BTSegRemoveTest(BTSegEnv *t,int *b,int *e) {
	int *c;
	TlsIntTrie *f;
	while (c<e) {
		BTSegRemove(t,*c);
        BTSegInsert(t,*c);
		BTSegRemove(t,*c);
		c++;
	}
	BTSegContentCheck(t);
}

/*--------------------------------*/

typedef struct {
	struct testSeq *Static;
} testSeq;
struct testSeq {
	int (*Start)(testSeq *this);
	int (*Next)(testSeq *this,int **p);
};

/*--------*/

static int *rPermEnum(int *b,int i,int k) {
	int *n,c;
	n = b+k;
	if (i<(k-1)) {
	    c = i+1;
		while (c<k) {
			int xchg,*p,*q,*e;
			p = b; q = n; e = q+k;
			while (q<e) { *q++=*p++; }
			xchg = n[i]; n[i] = n[c]; n[c] = xchg;
		    n = rPermEnum(n,i+1,k);
			c++;
		}
	}
	return n;
}

typedef struct {
	testSeq testSeq;
	int *Keys,nKeys;
	int *Dist;
	int Num,eNum;
} testSeqSimple;
static int testSeqSimpleStart(testSeq *this) {
	ThisToThat(testSeqSimple,testSeq);
	that->Num = 0;
	return that->nKeys;
}
static int testSeqSimpleNext(testSeq *this,int **P) {
	int r,*p;
	ThisToThat(testSeqSimple,testSeq);
    r = (that->Num<that->eNum);
	if (r) {
		int *B,*E,*q;
		B = that->Dist + (that->Num*that->nKeys);
		E = B+that->nKeys;
		q = B;
	    p = *P;
		while (q<E) {
			*p = that->Keys[*q];
			p++; q++;
		}
		*P = p;
		that->Num++;
	}
	return r;
}

static testSeq *testSeqSimpleNew(int *bKey,int *eKey) {
	testSeqSimple *r;
	static struct testSeq Static = {testSeqSimpleStart,testSeqSimpleNext};
	rPush(r);
	r->testSeq.Static = &Static;
	rnPush(r->Keys,(eKey-bKey));
	r->nKeys = eKey-bKey;
	{ 
		int *p,*q;
	    p = bKey; q = r->Keys;
		while (p<eKey) { *q++ = *p++; }
	}
	{
		int nb,n,*b,*e;
		nb = 1; n = r->nKeys;
		while (n>1) { nb = nb*n; n--; }
		r->eNum = nb; r->Num = 0;
		rnPush(r->Dist,nb);
        n = 0; b = r->Dist; e = r->Dist+r->nKeys;
		while (b<e) { *b++ = n++; }
		rPermEnum(r->Dist,0,r->nKeys);
	}
	return &r->testSeq;
}


/*--------*/

typedef struct {
	testSeq testSeq;
	testSeq **Mod;
	int nMod,nKeys;
	int *Dist;
	int cNum,eNum;
	int *bKeys;
} testSeqCmplx;

static int testSeqCmplxStart(testSeq *this) {
	int i,j,k,*p;
	ThisToThat(testSeqCmplx,testSeq);
	that->cNum = 0;
	p = that->bKeys;
	for (i=0;i<that->nMod;i++) {
		j = that->Dist[i];
		Call(that->Mod[j],Start,0);
		Call(that->Mod[j],Next,1(&p));
	}
	return that->nKeys;
}
static int testSeqCmplxNext(testSeq *this,int **P) {
	int r,i,j,*dist,*p,*q,*e,carry;
	ThisToThat(testSeqCmplx,testSeq);
	r = (that->cNum<that->eNum);
    if (r) {
		p = that->bKeys; e = p+that->nKeys; q = *P;
        while (p<e) { *q++ = *p++; }
		*P = q;
		i = 0; dist = that->Dist+that->cNum*that->nMod;
		p = that->bKeys;
		do {
            j = dist[i];
			q = p;
		    carry = !(Call(that->Mod[j],Next,1(&p)));
			if (carry) {
				p = q;
				Call(that->Mod[j],Start,0);
				Call(that->Mod[j],Next,1(&p));
				i++;
			}
		} while ((carry)&&(i<that->nMod));
        if (i>=that->nMod) {
			that->cNum++;
			if (that->cNum<that->eNum) {
				dist += that->nMod;
				p = that->bKeys;
				for (i=0;i<that->nMod;i++) {
					j = dist[i];
					Call(that->Mod[j],Start,0);
					Call(that->Mod[j],Next,1(&p));
				}
			}
		}
	}
	return r;
}

static testSeq *testSeqCmplxNew(testSeq *b,testSeq *e) {
	static struct testSeq Static = {
		testSeqCmplxStart,testSeqCmplxNext
	};
	testSeqCmplx *r;
	testSeq *p,*q;
	int nK;
    rPush(r);
	r->testSeq.Static = &Static;
	r->nMod = e-b;
	rnPush(r->Mod,(r->nMod));
	q = *r->Mod; p = b; while (p<e) { *q++=*p++; }
    nK = 0;
	p = *r->Mod; e = p+r->nMod;
	while (p<e) { nK += Call(p,Start,0); p++; }
	r->nKeys = nK;
	rnPush(r->bKeys,nK);
    {
		int nb,n,*b,*e;
		nb = 1; n=r->nMod; while (n>1) { nb = nb*n; n--; }
		r->eNum = nb; r->cNum  = 0;
		rnPush(r->Dist,nb);
		n = 0; b = r->Dist; e = b+r->nMod; while (b<e) { *b++ = n++; }
		rPermEnum(r->Dist,0,r->nMod);
	}
	return &r->testSeq;
}


/*--------*/

main() {
	BTSegEnv *MyEnv;
	int Errors,i,j,org;
	static int Vector0[] = {0,1,1,0,1,2,5,4,9,5,0,11,6,7,17};
	EnvOpen(4096,4096);
	MyEnv = BTSegEnvNew();
	Errors = 0;
    org = 0;
    for (i=0;i<2;i++) {
	BTSegEnvReset(MyEnv,org);
    BTSegInsertTest(MyEnv,Vector0,Vector0+2);
	BTSegRemoveTest(MyEnv,Vector0,Vector0+2);
	Errors += MyEnv->Errors;
	BTSegEnvReset(MyEnv,org);
	BTSegInsertTest(MyEnv,Vector0,Vector0+2);
	BTSegRemoveTest(MyEnv,Vector0+2,Vector0+4);
	Errors += MyEnv->Errors;
	BTSegEnvReset(MyEnv,org);
    BTSegInsertTest(MyEnv,Vector0+2,Vector0+4);
	Errors += MyEnv->Errors;
	BTSegEnvReset(MyEnv,org);
    BTSegInsertTest(MyEnv,Vector0+1,Vector0+4);
	Errors += MyEnv->Errors;
	BTSegEnvReset(MyEnv,org);
    BTSegInsertTest(MyEnv,Vector0+0,Vector0+4);
	Errors += MyEnv->Errors;
	BTSegEnvReset(MyEnv,org);
    BTSegInsertTest(MyEnv,Vector0+5,Vector0+12);
	Errors += MyEnv->Errors;
	if (Errors==0) {
		printf("\n Tests Ok.\n");
	} else {
		printf("\n %d errors have been found...\n",Errors);
	}
    for (j=0;j<12;j++) { Vector0[j]=(Vector0[j]-5)&0xffff;}
    // TO DO : trying with a mix of positive and negative values.
    // There were bugs in the library where tries didn't get along with values with
    // the most significant bit set.
    // Also : checking the behaviour around the value 0x7fffffff (or 64/128 bit equivalent
    // should be done.
    }
	EnvClose();
}

