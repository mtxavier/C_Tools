#include <Classes.h>
#include <StackEnv.h>
#include <Tools.h>
#include <../src/Tools.1.c>
#include <../src/Tools.2.c>
#include <../src/Tools.4.c>
#include <stdio.h>

/*_____________________________________________________________________________
 |
 |_____________________________________________________________________________
*/

static IdxNod **idxLeafThoroughPack(IdxDesc *d,IdxNod **b,IdxNod **e);

IdxDesc *IdxDescNew(void) {
	IdxDesc *r;
	rPush(r);
	r->Pool = TlsMemPoolNew((64+sizeof(IdxNod))*16);
	r->Pool = Call(r->Pool,SizeSelect,1(64+sizeof(IdxNod)));
	r->LeafSz = 64;
	r->LinkSz = 64/sizeof(IdxNod *);
	r->Lvl = 0;
	r->Root = 0;
	return r;
}

char b64(char c) {
	if (c>=10) {
        c-=10;
		if (c>=26) {
			c-=26;
			if (c>=26) {
				c = (c==26)?'&':'$';
			} else {
				c = 'a'+c;
			}
		} else {
			c = 'A'+c;
		}
	} else {
		c = '0'+c;
	}
    return c;
}
char *DataInit(void) {
	char *r,c;
	int i;
	rnPush(r,0x2000);
	for (i=0;i<0x2000;i+=2) {
		c = (i>>7)&0x3f;
		r[i] = b64(c);
		c = (i>>1)&0x3f;
		r[i+1] = b64(c);
	}
	return r;
}

void NodArrayCheck(IdxDesc *d,IdxNod **br,IdxNod **er,char *bData,char *eData) {
	char Str[84],*ps,*bs,*es;
	IdxNod **p;
	bs = Str; es = Str+80; *es = '\0'; ps = bs;
	p = br;
	printf("\n[");
	while (p<er) {
		int l,dl,idx;
        LeafNod P;
		LeafNodGet(P,*p,d->LeafSz);
		idxLeafGetLength(l,*p,d->LeafSz);
        idx = 0;
		while (idx<l) {
		    dl = (l-idx); 
			if ((ps+dl)>es) { dl = (es-ps);}
			TlsCBfGet(ps,ps+dl,&P.Data,idx);
			idx += dl; ps+=dl;
			if (ps>=es) { printf(Str); ps = bs; }
		}
        *ps = '\0'; printf(Str); ps = bs;
		printf(".");
		p++;
	}
	*ps = '\0'; printf(Str);
	printf("]\n");
}

void NodArrayClean(IdxDesc *d,IdxNod **br,IdxNod **er) {
	IdxNod **p;
	p = br; while (p<er) { Call(d->Pool,Free,1(*p)); p++; }
}

IdxNod **IdxNodArrayNew(IdxDesc *d,IdxNod **br,IdxNod **er,int *bNb,int *eNb,char *bData,char *eData) {
	IdxNod **r,**e,*c;
	char *pData;
	int *pNb,l,idx,end,dl;
	pNb = bNb; pData = bData; r = br; end = 0;
	while ((pNb<eNb)&&(r<er)) {
		LeafNod C;
		c = *r = Call(d->Pool,Alloc,0);
		l = *pNb; end += l; c->End = end;
		c->Data.Leaf.b = c+1;
		if (l>0) { 
			if (l>=d->LeafSz) {
				l = d->LeafSz; c->Data.Leaf.e = c->Data.Leaf.b; 
			} else {
				c->Data.Leaf.e = c->Data.Leaf.b+l; 
			}
		} else {
			l=0; c->Data.Leaf.e = c->Data.Leaf.b + d->LeafSz;
		}
		LeafNodGet(C,c,d->LeafSz);
		idx = 0;
		while (idx<l) {
			dl = l-idx; if (pData+dl>eData) { dl = eData-pData; }
			TlsCBfOverwrite(&C.Data,idx,pData,pData+dl);
			idx+=dl; pData+=dl; if (pData>=eData) pData=bData;
		}
		LeafNodUpdate(c,C);
		r++; pNb++;
	}
	return r;
}

void LeafPackTest(IdxDesc *d,IdxNod *Lvl1,int *bd,int *ed,char *bData,char *eData) {
	IdxNod **bNod,**eNod;
	bNod = Lvl1->Data.Link.b;
	eNod = bNod+d->LinkSz;
    Lvl1->Data.Link.e = eNod = IdxNodArrayNew(d,bNod,eNod,bd,ed,bData,eData);
	printf("\nInitial:");
    NodArrayCheck(d,bNod,eNod,bData,eData);
	Lvl1->Data.Link.e = eNod = idxLeafThoroughPack(d,bNod,eNod-1)+1;
	printf("\nPacked:");
    NodArrayCheck(d,bNod,eNod,bData,eData);
    NodArrayClean(d,bNod,eNod);
	bNod = Lvl1->Data.Link.b;
	eNod = bNod+d->LinkSz;
    Lvl1->Data.Link.e = eNod = IdxNodArrayNew(d,bNod,eNod,bd,ed,bData,eData);
	printf("\nShallow Packed:");
    Lvl1 = idxLeafShallowPack(d,Lvl1);
    NodArrayCheck(d,Lvl1->Data.Link.b,Lvl1->Data.Link.e,bData,eData);
	NodArrayClean(d,Lvl1->Data.Link.b,Lvl1->Data.Link.e); 
	Lvl1->Data.Link.e = Lvl1->Data.Link.b;
}

main() {
	IdxDesc *d;
	IdxNod *Lvl1;
	char *bData,*eData;
	int *bd,*ed;
	EnvOpen(4096,4096);
	bData = DataInit(); eData = bData+0x2000;
	d = IdxDescNew(); 
	Lvl1 = idxEmptyNodAlloc(d,0);
	{
	    int distr[]={
		    64, 2, 6,58, 40,40,40,20,
		    36,36,64,64, 64, 0,44,12
	    };
        LeafPackTest(d,Lvl1,distr,distr+16,bData,eData);
	}
	{
	    int distr[]={
		    60,50,60,62, 64,56,60,64,
		    62,60,60,64, 50,22,44,12
	    };
        LeafPackTest(d,Lvl1,distr,distr+16,bData,eData);
	}
	printf("\n");
	EnvClose();
}
