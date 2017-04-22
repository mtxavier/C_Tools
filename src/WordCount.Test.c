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
#include <List.h>
#include <Patricia.h>
#include <Tools.h>

#include <stdio.h>
#include <string.h>

static int Blanks[256];
static int Charac[256];

void InitCharac(void) {
	int i;
	char *p;
	static char theblanks[] = " \t\b\r\n";
	for (i=0;i<256;i++) { Blanks[i] = Charac[i] = (0!=0); }
    p = theblanks;
	while (*p) { Blanks[*p]=(0==0); p++; }
	Blanks[*p] = (0==0);
	for (i='a';i<='z';i++) {
		Charac[i] = (0==0);
		Charac[i+'A'-'a'] = (0==0);
	}
	for (i='0';i<='9';i++) { Charac[i] = (0==0);}
	Charac['_']=(0==0);
}

typedef struct {
	PatriciaNod PatriciaNod;
	dList dList;
	int Count;
} WordCount;

typedef struct {
	FILE *f;
	PatriciaNod Words,Words1;
	dList L,L1;
	char fwd[8],*p,*e;
} FileParseCtx;

FileParseCtx *FileParseCtxNew(char *Name) {
	FileParseCtx *r;
	dList *d;
	rPush(r);
	r->e = r->p = r->fwd;
	PatriciaInit(&r->Words);
	PatriciaInit(&r->Words1);
	d = &r->L; d->p = d->n = d;
	d = &r->L1; d->p = d->n = d;
    r-> f = fopen(Name,"r");
	if (r->f==NULL) {
		printf("Erreur d'ouverture du fichier %s\n",Name);
	}
	return r;
}

void FileParseCtxClose(FileParseCtx *t) {
	if (t->f) {
		fclose(t->f);
		t->f = 0;
	}
}

inline int FileParseGetChar(FileParseCtx *pc) {
	int c;
    if (pc->p<pc->e) {
		c = *pc->p;
		pc->p++; 
		if (pc->p>=pc->e) {pc->p = pc->e = pc->fwd; }
	} else {
		c = fgetc(pc->f);
	}
    return c;
}
int FileParseSkipBlank(FileParseCtx *pc) {
	int c,cont,r;
	r = (0==0);
	do {
		c = FileParseGetChar(pc);
		r = (c!=EOF);
		if (!r) c =' ';
		cont = Blanks[c];
	} while(r&&cont);
    if (r) { *pc->e = c; pc->e++; }
	return r;
}
int FileParseCtxNextWord(FileParseCtx *pc) {
	int r,c,cont;
	char cword[256],*q;
	r = (pc->f!=NULL);
	if (r) {
		q = cword;
        r = FileParseSkipBlank(pc);
		c = FileParseGetChar(pc);
		r = (c!=EOF);
		if (!r) c=' ';
        cont = Charac[c];
		if (!cont) {
			if (!Blanks[c]) {
				*q++=c;
			}
		} else {
			*q++ = c;
		    while (r&&cont) {
		        c = FileParseGetChar(pc);
			    r = (c!=EOF);
			    if (!r) c=' ';
                cont = Charac[c];
				if (cont) { *q++ = c; }
		    }
			if (!cont) {
				*pc->e = c; pc->e++;
			}
		}
		*q++ = 0;
		if (cword[0]!=0) {
			WordCount *F;
			PatriciaNod *f;
			f = PatriciaSeek(&pc->Words,cword);
			if (f==&pc->Words) {
				int sz;
				char *nn;
				sz = ((q-cword)+3)&(-4);
				rnPush(nn,sz);
				rPush(F);
				F->Count = 0;
				strcpy(nn,cword);
				PatriciaSeekOrInsert(&pc->Words,nn,&F->PatriciaNod);
			} else {
				PatriciaNod *f1;
				PatriciaRemove(&pc->Words,cword);
				f1 = PatriciaSeek(&pc->Words,cword);
				if (f1!=&pc->Words) {
					printf("Error: node not removed.\n");
				}
				PatriciaSeekOrInsert(&pc->Words,f->Key,f);
				f1 = PatriciaSeek(&pc->Words,cword);
                if (f==&pc->Words) {
					printf("Error: node not reinserted.\n");
				}
				F = CastBack(WordCount,PatriciaNod,f);
            }
		    F->Count++;
		}
	}
	return r; 
}

PatriciaNod *PrintCount(PatriciaNod *k,void *clos) {
	WordCount *N;
	FileParseCtx *C;
	dList *p,*n;
	C = clos;
	N = CastBack(WordCount,PatriciaNod,k);
	printf("%s : %d\n",k->Key,N->Count);
    p = &C->L;
	n = &N->dList;
	n->n = p; n->p = p->p; 
	n->p->n = n; n->n->p = n;
	return 0;
}

typedef struct {
	dList *d;
	int ErrCount;
} CCClos;
PatriciaNod *fCountCheck(PatriciaNod *n,void *Clos) {
    CCClos *c;
	WordCount *D;
	c = Clos;
	D = CastBack(WordCount,dList,c->d);
	c->d = c->d->n;
	if (strcmp(n->Key,D->PatriciaNod.Key)) {
		c->ErrCount++;
	}
	return 0;
}
void CountCheck(FileParseCtx *p) {
	CCClos cl;
	cl.d = p->L.n;
	cl.ErrCount = 0;
	PatriciaForEach(&p->Words,fCountCheck,&cl);
	if (cl.ErrCount) {
		printf("CountCheck: %d error(s) found.\n",cl.ErrCount);
	}
}

int main(int argc,char **argv) {
	FileParseCtx *pc;
	char *name;
	static char *ZeroStr="WordCount.txt";
	EnvOpen(4096,4096);
	InitCharac();
	if (argc>1) {
	    name = argv[1];
	} else {
		name = ZeroStr;
	}
	printf("First arg: \"%s\"\n",name);
    pc = FileParseCtxNew(name);
	if (pc->f!=NULL) {
		while (FileParseCtxNextWord(pc)) {}
		FileParseCtxClose(pc);
		PatriciaForEach(&pc->Words,PrintCount,pc);
		CountCheck(pc);
	}
	EnvClose();
	return 0;
}

