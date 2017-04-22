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

#include "Key.h"

char *K256Tail(char *Key) {
    char *r;
    r=Key;
    if (!(*r&0x80)) {
        int c0;
        c0 = 1;
        r++;
        while(c0) {
            if (*r&0x80) {
                c0++;
            } else {
                c0--;
            }
            r++;
        }
    } 
    return r;
}

char *K256Head(char *Key) {
    char *r;
    r = (*Key&0x80) ? Key+1:Key;
    return r;
}

int K256Cmp(const char *ref,const char *Key) {
   int r,c0;
   r = 0;
   c0 = (*ref&0x80)?1:0;
   while (*ref==*Key && c0) {
       r++; ref++; Key++;
       if (*ref&0x80) {
           c0++;
       } else {
           c0--;
       }
   }
   return r;
}

char *K256Cpy(char *dst,const char *Key) {
    int c0;
    c0 = 1;
    while (c0) {
        *dst=*Key;
        if (*dst&0x80) {
            c0++;
        } else {
            c0--;
        }
        dst++; Key++;
    }
    return dst;
}

char *K2String(char *S,char *Key) {
    int c0;
    c0=1;
    while (c0) {
        *S=*Key&0x7f;
        if (!*S) *S=-128;
        if (*Key&0x80) {
            c0++;
        } else {
            c0--;
        }
        S++; Key++;
    }
    if (S[-1]==-128) {
        S--;
    }
    *S=0;
    return S;
}

char *String2K(char *Key,char *S) {
unsigned char Msk;
    Msk=0x80;
    while (*S) {
        *Key=(*S&0x7f)|Msk;
        Msk=Msk^0x80;
        Key++; S++;
    }
    if (Msk) {
        *Key++=0;
    } else {
        Key[-1]=Key[-1]^0x80;
    }
    return Key;
}

int K256Length(const char *K) {
    const char *p;
    int c0;
    p = K;
    c0 = 1;
    while (c0) {
        if (*p&0x80) {
            c0++;
        } else {
            c0--;
        }
        p++;
    }
    return p-K;
}

char *QuoteKey(char *S,char *Key) {
    int c0;
    c0 = 1;
    while (c0) {
        *S++='0'+((*Key>>4)&0x7);
        *S++=((*Key&0xf)>9)?(*Key&0xf)+'a'-10:(*Key&0xf)+'0';
        if (*Key&0x80) {
            c0++;
            *S++ = KeyQuoteChain;
        } else {
            c0--;
            *S++ = KeyQuoteAtom;
        }
        Key++;
    }
    *S=0;
    return S;
}

char *UnquoteKey(char *Key,char *S) {
    int c0,c;
    c0 = 1;
    if (*S==KeyQuoteChain) S++;
    c = 0;
    while (c0 && *S) {
        if (*S>='0'&&*S<='9') c=(c<<4)+(*S)-'0';
        if (*S>='a'&&*S<='f') c=(c<<4)+(*S)+10-'a';
        if (*S>='A'&&*S<='F') c=(c<<4)+(*S)+10-'A';
        if (*S==KeyQuoteAtom) {
            *Key++=(c & 0x7f); c0--; c = 0;
        }
        if (*S==KeyQuoteChain) {
            *Key++=(c | 0x80); c++; c = 0;
        }
        S++;
    }
    while (c0) { *Key++=0; c0--; }
    return Key;
}

char *P2K(char *Key,void *P) {
    unsigned int j,i;
    unsigned char *p,*e;
    unsigned char Msk;
    p = (unsigned char *)(&P); e = p+sizeof(void *);
    Msk = 0x80; j=*p++; i = 1;
    while (p!=e) {
        *Key++=(j&0x7f)|Msk;
        Msk = Msk^0x80;
        j>>=7;
        if (i<7) { j=j+((*p)<<i); i++; p++; } else {i=0;}
    }
    while (j>=0x80) {
        *Key++=(j&0x7f)|Msk;
        Msk = Msk^0x80;
        j>>=7;
    }
    *Key++=j;
    if (!Msk) {
        *Key++=0;
    }
    return Key;
}

void *K2P(char *K) {
    int c0,dc;
    void *r;
    unsigned char *p,*e;
    unsigned int j;
    p = (unsigned char *)(&r); e = p+sizeof(void *);
    c0=1; dc=0; j=0;
    while (c0&&(p!=e)) {
        j|=(*K&0x7f)<<dc;
        if (*K&0x80) {
            c0++;
        } else {
            c0--;
        }
        if (dc>=1) { *p++=j; j=j>>8; dc--; } else { dc+=7; }
        K++;
    }
    while (p!=e) { *p++=j; j=j>>8; }
    return r;
}

char *Int2K(char *Key,int i) {
    unsigned int j;
    unsigned char Msk;
    j = (unsigned int)i;
    Msk = 0x80;
    while (j>=0x80) {
        *Key++=(j&0x7f)|Msk;
        Msk = Msk^0x80;
        j>>=7;
    }
    *Key++=j;
    if (!Msk) {
        *Key++=0;
    }
    return Key;
}

int K2Int(char *K) {
    int c0,dc;
    unsigned int j;
    c0=1; dc=0; j=0;
    while (c0&&dc<32) {
        j|=(*K&0x7f)<<dc;
        if (*K&0x80) {
            c0++;
        } else {
            c0--;
        }
        dc+=7; K++;
    }
    return (int)j;
}

#include <Classes.h>
typedef struct {
    PatriciaCtx PatriciaCtx;
    int c0;
} KPatCtx;


#define uCharFirst1(r,vl) { \
    unsigned int _v;\
    _v = vl;\
    if (_v&0xf0){\
        r=(_v&0xc0)?((_v&0x80)?0:1):((_v&0x20)?2:3);\
    } else {\
        r=(_v&0x0c)?((_v&0x08)?4:5):((_v&0x02)?6:(_v?7:8));\
    }\
}

static int K256GetBitVal(const char *Key,int n) {
    return Key[n>>3]>>((n&7)^7);
}

static int K256CheckRadixNext(PatriciaCtx *ctx,const char *Ref,int num) {
    KPatCtx *Ctx;
    int c0,r;
    const unsigned char *p,*q,*lst,*Key;
    unsigned char pv,df;
    r = num;
    Ctx = CastBack(KPatCtx,PatriciaCtx,ctx);
    c0 = Ctx->c0;
    Key = ctx->Key; p = ctx->c; q = Ref+(p-Key);
    lst = Key+(num>>3);
    pv = *p;
    while (c0&&(pv==*q)&&(p<lst)) {
        if (pv&0x80) {
            c0++;
        } else {
            c0--;
        }
        if (c0) { p++; q++; pv = *p; }
    }
    if (p==lst) {
        df = (pv^(*q))&(0xfe<<((num&7)^7));
            if (!df) {
                r = num; ctx->BitVal = (pv>>((num&7)^7))&1;
            }
    } else {
        df = (pv^(*q));
        if (!df) {
            r = num; ctx->BitVal = 0;
        }
    }
    if (df) {
        int m;
        uCharFirst1(m,df);
        r = ((p-Key)<<3)+m;
        ctx->BitVal = (pv>>(7^m))&1;
    }
    ctx->c = p;
    Ctx->c0 = c0;
    return r;
}

void K256NodeInit(PatriciaNod *Root){
    static char NullString[4];
    NullString[0] = 0;
    Root -> BitToTest = -1;
    Root -> Key = NullString;
    Root -> Next[0] = Root -> Next[1] = Root;
}

PatriciaNod *K256NodeSeek(PatriciaNod *Root,char *s){
    KPatCtx Ctx;
    Ctx.c0 = 1;
    Ctx.PatriciaCtx.Key = Ctx.PatriciaCtx.c = s;
    Ctx.PatriciaCtx.CheckRadixNext = K256CheckRadixNext;
    return cPatriciaSeek(&Ctx.PatriciaCtx,Root,s);
}

PatriciaNod *K256NodeSeekOrInsert(PatriciaNod *Root,char *s,PatriciaNod *NewNod){
    KPatCtx Ctx;
    Ctx.c0 = 1;
    Ctx.PatriciaCtx.Key = Ctx.PatriciaCtx.c = s;
    Ctx.PatriciaCtx.CheckRadixNext = K256CheckRadixNext;
    return cPatriciaSeekOrInsert(&Ctx.PatriciaCtx,Root,s,NewNod);
}

PatriciaNod *K256NodeRemove(PatriciaNod *Root,char *s){
    KPatCtx Ctx;
    Ctx.c0 = 1;
    Ctx.PatriciaCtx.Key = Ctx.PatriciaCtx.c = s;
    Ctx.PatriciaCtx.CheckRadixNext = K256CheckRadixNext;
    Ctx.PatriciaCtx.GetBitVal = K256GetBitVal;
    return cPatriciaRemove(&Ctx.PatriciaCtx,Root,s);
}

