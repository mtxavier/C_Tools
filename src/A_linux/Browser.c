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
#include <Browser.h>
#include <Patricia.h>


#include <../src/Browser.cmn.c>

/*_____________________________________________________________________________
 |
 | System V specific
 |_____________________________________________________________________________
*/

#define _LARGEFILE64_SOURCE
#include <sys/types.h>
#include <unistd.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <stddef.h>

#include <time.h>

/*---------------------------*/

unsigned int BrwRandomSeed(void) {
	struct timespec t;
	static int seed = 1;
    clock_gettime(CLOCK_MONOTONIC,&t);
	seed += t.tv_nsec;
	seed = ((1664525*seed)+1013904223); // Numerical receipe
	return seed;
}

/*---------------------------*/

typedef struct {
	BrwRSerial BrwRSerial;
	char *FileName;
	int FileId,Opened,Eof;
    off64_t Pos,Length;
} FileSerial;

static void FlOpen(FileSerial *t) {
	if ((!t->Opened)&&(!t->Eof)) {
		t->FileId = open(t->FileName,O_RDONLY);
		t->Opened = (t->FileId>=0);
		if (t->Opened) {
			t->Length = lseek64(t->FileId,0,SEEK_END);
			if (t->Pos<0) t->Pos = 0;
			if (t->Pos>=t->Length) {
				t->Pos = t->Length;
			} else {
			    t->Pos = lseek64(t->FileId,t->Pos,SEEK_SET);
			}
			t->Eof = (0!=0);
		}
	}
}
static void FlClose(FileSerial *t) {
	if (t->Opened) {
		close(t->FileId);
		t->FileId = -1;
		t->Opened = (0!=0);
	}
}

static BrwPos *FileGetPos(BrwRSerial *this,int From) {
	off64_t pos;
	BrwPos *r;
	unsigned int p0;
	int p1;
	ThisToThat(FileSerial,BrwRSerial);
	if (From==BrwPosCur) {
	    pos = that->Pos;
	} else {
		pos = (From==BrwPosEnd)?that->Length:0;
	}
	p0 = pos & 0xffffffff;
	r = BrwPosInt64(p0,0);
	Call(r,Move,2((pos>>32),32));
	return r;
}
static void FileSeek(BrwRSerial *this,BrwPos *Idx) {
	off64_t idx;
	unsigned int v0;
	int v1,l,i,shft;
	ThisToThat(FileSerial,BrwRSerial);
    idx = 0; shft = 0; i = 0;
	do { v0 = Call(Idx,PartVal,3(&l,(&l)+1,i)); idx = idx+(v0<<shft); shft+=32; i++; } while (l>0);
	if ((!that->Opened)&&(idx>0)) { FlOpen(that); }
	if (idx<0) idx = 0;
	if (idx>that->Length) idx = that->Length;
	that->Pos = idx;
	if (that->Opened) {
		that->Pos = lseek64(that->FileId,idx,SEEK_SET);
	}
}
static void *FileRead(BrwRSerial *this,void *q,void *eq) {
	void *r;
	ThisToThat(FileSerial,BrwRSerial);
	if (that->Eof) {
		r = BrwEof;
	} else {
		if ((eq>q)&&(that->Pos>=that->Length)) {
			r = BrwEof;
			if (that->Opened) { FlClose(that); }
			that->Eof = (0==0);
		} else {
			off64_t lft;
			if (!that->Opened) { FlOpen(that); }
			lft = that->Length-that->Pos;
			if (lft<(eq-q)) { eq = q+lft; }
	        r = q;
	        if ((that->Opened) && (eq>q)) {
		        int s;
		        s = read(that->FileId,q,eq-q);
		        r += s;
		        that->Pos += s;
	        }
			if (that->Pos>=that->Length) { FlClose(that); }
		}
	}
	return r;
}

BrwRSerial *BrwRSerialFile(char *Name) {
	FileSerial *r;
	static struct BrwRSerial Static = {
		FileGetPos,FileSeek,FileRead
	};
	rPush(r);
	r->BrwRSerial.Static = &Static;
	r->FileName = TlsStringDuplicate(Name);
	r->FileId = -1; 
	r->Opened = r->Eof = (0!=0);
	r->Pos = r->Length = 0; 
	return &r->BrwRSerial;
}

/*----------------------------------*/
// Experimental for now
#if 0
typedef struct {
	char *b,*e;
} DirLabelNod;
typedef struct {
	int Org;
	DirLabelNod *b,*e,*ee;
} DirLabel;

#define DirLabelRoot 0
#define DirLabelCurrent 1

#define DirLabelTypeCurrent 0
#define DirLabelTypePrevious 1
#define DirLabelTypeLabel 2

static DirLabel *DirLabelCpy(DirLabel *r,DirLabel *org) {
	DirLabelNod *p,*q,*e;
	r->Org = org->Org;
	q = r->b; p = org->b; e = org->e;
	while (p<e) { *q++ = *p++; }
	r->e = q;
}
static int DirLabelType(DirLabelNod *l) {
	int r;
	r = DirLabelTypeCurrent;
	if (l->e>l->b) {
		r = DirLabelTypeLabel;
		if (l->b[0]=='.') {
			r = DirLabelTypeCurrent;
			if (l->e>l->b+1) {
				r = DirLabelTypeLabel;
				if ((l->b[1]=='.')&&((l->e)==(l->b+2))) { r = DirLabelTypePrevious; }
			}
		}
	}
	return r;
}
static DirLabel *DirLabelAddNod(DirLabel *r,DirLabelNod *add) {
	int lt,lt1;
	lt = DirLabelType(add);
	switch (lt) {
	case DirLabelTypeLabel: {
		*(r->e) = *add; r->e++;
	} break;
	case DirLabelTypePrevious: {
		if (r->e>r->b) {
			lt1 = DirLabelType(r->e-1);
		} else {
			lt1 = DirLabelTypePrevious;
		}
		if (lt1==DirLabelTypePrevious) {
			*(r->e) = *add; r->e++;
		} else {
			r->e--;
		}
	} break;
	}
	return r;
}
static DirLabel *DirLabelCat(DirLabel *org,DirLabel *add){
	DirLabel *r;
	int nb;
	rPush(r);
	if (add->Org == DirLabelRoot) {
		nb = add->e-add->b;
		rnPush(r->b,nb); r->ee = r->b+nb; r->e = r->b;
		DirLabelCpy(r,add); 
	} else {
		DirLabelNod *p,*e;
		nb = (org->e-org->b)+(add->e-add->b);
		rnPush(r->b,nb); r->ee = r->b+nb; r->e = r->b;
		DirLabelCpy(r,org);
        p = add->b; e = add->e;
		while (p<e) { DirLabelAddNod(r,p); p++; }
	}
	return r;
}

/*----------------------------------*/

#include <dirent.h>

typedef struct {
	PatriciaNod Label;
	int idx;
} DirEntry;
typedef struct {
	BrwNod BrwNod;
	char *Label;
	int EntriesNb;
	PatriciaNod EntriesLabel;
	char **Entries;
} BrwDir;
static int DirEntriesNb(BrwNod *this) {
	ThisToThat(BrwDir,BrwNod);
	return that->EntriesNb;
}
static void DirBrowse(BrwNod *this,BrwNodEntries *Tgt,int b,int e) {
	int i,found;
	char **p;
	ThisToThat(BrwDir,BrwNod);
	if (b<0) b=0; if (e>that->EntriesNb) e=that->EntriesNb;
	i = b; found = (0!=0); p = that->Entries+i;
	while ((i<e)&&(!found)) {
		found = Call(Tgt,Elt,3(&BrwTypeNull,i,*p));
		i++; p++;
	}
}
BrwNod *BrwNewDir(char *Path);
static BrwNod *DirGetEntry(BrwNod *this,BrwType **type,char *Label) {
	char *Nlbl,*p,*q;
	int l,found;
	BrwNod *r;
	ThisToThat(BrwDir,BrwNod);
	r = &BrwNodNull;
	*type = &BrwTypeNull;
	Nlbl = p = that->Label; while (*p) p++; l = p - Nlbl;
	if (l>0) { if (p[-1]!='/') {l++;} }
	found = !Label[0];
	p = Label; while (*p) { l++; p++; }
	if (found) { r = this; }
	if ((l)&&(!found)) {
	    l++; l = TlsRoundUp(int,l);
	    pOpen
		pnPush(Nlbl,l);
		p = that->Label; q = Nlbl; while (*p) { *q++ = *p++; }
		if (q>Nlbl) { if (q[-1]!='/') *q++='/'; }
		p = Label; while (*p) { *q++ = *p++; }
		*q++ = 0;
        r = BrwNewDir(Nlbl);
	    pClose
	}
}
static BrwRSerial *DirLeaf(BrwNod *this,BrwType **type,char *Label) {
	ThisToThat(BrwDir,BrwNod);
}
static PatriciaNod *DirCount(PatriciaNod *Nod,void *Clos) {
	BrwDir *that;
	DirEntry *n;
	n = CastBack(DirEntry,Label,Nod);
	that = Clos;
	that->Entries[n->idx] = Nod->Key;
	return 0;
}
BrwNod *BrwNewDir(char *Path) {
	BrwNod *R;
	DIR *D;
	static struct BrwNod Static = {
		DirEntriesNb,DirBrowse,DirGetEntry,DirLeaf
	};
    D = opendir(Path);
	if (D!=NULL) {
	    BrwDir *r;
		struct dirent *p,*pp;
		int idx,l;
		DirEntry *N;
	    rPush(r);
	    r->BrwNod.Static = &Static;
        r->Label = TlsStringDuplicate(Path);
	    PatriciaInit(&r->EntriesLabel);
	    r->EntriesNb = 0;
	    r->Entries = 0;
		idx = 0;
		pOpen
		{   void *n; 
		    l = offsetof(struct dirent,d_name)+pathconf(Path,_PC_NAME_MAX)+1;
		    l = TlsRoundUp(int,l); pnPush(n,l); p = n;
	    }
		do {
			readdir_r(D,p,&pp);
			if (pp!=NULL) {
				char *lbl;
				lbl = TlsStringDuplicate(p->d_name);
				rPush(N); N->idx = idx++;
				PatriciaSeekOrInsert(&r->EntriesLabel,lbl,&N->Label);
			}
		} while(pp!=NULL);
		rPush(N); N->idx = idx++;
		PatriciaSeekOrInsert(&r->EntriesLabel,"..",&N->Label);
		r->EntriesNb = idx;
		rnPush(r->Entries,idx);
		PatriciaForEach(&r->EntriesLabel,DirCount,r);
		pClose
		closedir(D);
		R = &r->BrwNod;
	} else {
		R = &BrwNodNull;
	}
	return R;
}
#endif

