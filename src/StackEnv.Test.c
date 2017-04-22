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

#include "Classes.h"
#include "StackEnv.h"
#include <stdio.h>

MemStack *Mem;

typedef struct _Data_ Data;
typedef struct {
    int (*CatSize)(Data *this);
    char *(*Cat)(Data *this,char *dst);
} DataStatic;

struct _Data_ {
    DataStatic *Static;
};

typedef struct {
    Data Data;
    int n;
} DataValue;

char *DataValueCat(Data *this,char *dst) {
    char *p;
    int v;
    ThisToThat(DataValue,Data);
    v = that->n;
    if (v<0) { *dst++='-'; v=-v; }
    p = dst;
    if (!v) {
	*p++ = '0';
    } else {
	char *q;
	while (v) {
	    *p++ = '0'+(v%10);
	    v = v/10;
	}
	q = p-1;
	while (q>dst) {
	    char x;
	    x=*q; *q=*dst; *dst=x;
	    dst++; q--;
	}
    }
    return p;
}

int DataValueSize(Data *this) {
    char b[16];
    return DataValueCat(this,b)-b;
}

Data *DataValueNew(int n) {
    DataValue *R;
    static DataStatic Static = {DataValueSize,DataValueCat};
    mPush(Mem,R);
    R->Data.Static = &Static;
    R->n = n;
    return &R->Data;
}

typedef struct {
    Data Data;
    char *b,*e;
} DataString;

char *DataStringCat(Data *this,char *dst) {
    char *p;
    ThisToThat(DataString,Data);
    p = that->b;
    while (p<that->e) {
	*dst++ = *p++;
    }
    return dst;
}

int DataStringSize(Data *this) {
    ThisToThat(DataString,Data);
    return that->e-that->b;
}

Data *DataStringNew(char *S) {
    static DataStatic Static = {DataStringSize,DataStringCat};
    DataString *R;
    mPush(Mem,R);
    R->Data.Static = &Static;
    R->b = S;
    while (*S) S++;
    R->e = S;
    return &R->Data;
}

typedef struct {
    Data Data;
    Data *Children[2];
} DataPair;

int DataPairSize(Data *this) {
    int S;
    ThisToThat(DataPair,Data);
    S = 3;
    S += Call(that->Children[0],CatSize,0);
    S += Call(that->Children[1],CatSize,0);
    return S;
}

char *DataPairCat(Data *this,char *dst) {
    ThisToThat(DataPair,Data);
    *dst++='(';
    dst = Call(that->Children[0],Cat,1(dst));
    *dst++=',';
    dst = Call(that->Children[1],Cat,1(dst));
    *dst++=')';
    return dst;
}

Data *DataPairNew(Data *A,Data *B) {
    DataPair *R;
    static DataStatic Static = {DataPairSize,DataPairCat};
    mPush(Mem,R);
    R->Data.Static = &Static;
    R->Children[0] = A;
    R->Children[1] = B;
    return &R->Data;
}

main() {
    Data *D;
    MemStack *M1,*M2;
    int i;
    M1 = Mem = MemStackOpen(0x1000);
    M2 = MemStackFork(M1,0x100);
    Mem = M2;
    D = DataPairNew(DataStringNew("Bienvenue"),DataValueNew(54));
    Mem = M1;
    printf("\n");
    for (i=0;i<4;i++) {
        mOpen(Mem)
	int S;
        char *b,*e;
        S = Call(D,CatSize,0);
	mnPush(Mem,b,S+1);
	e = Call(D,Cat,1(b));
	*e = 0;
	printf(b);
	printf("\n");
	Mem = M2;
	D = DataPairNew(DataStringNew("Et puis"),D);
	Mem = M1;
        mClose(Mem)
    }
    MemStackClose(Mem);
}

