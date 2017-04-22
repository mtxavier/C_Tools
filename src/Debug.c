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

#include "Debug.h"

/*_____________________________________________________________________________
 |
 |  Debug
 |_____________________________________________________________________________
*/

char *DbgPutBaseUint(char *p,int b,int D) {
    int n;
    char *r;
    unsigned int d;
    static char bn[]="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    d=(unsigned int)D;
    r=p;
    n=0;
    while (d>=b) {
	*r++=bn[d%b];
	d=d/b;
	n++;
    }
    *r++=bn[d%b];
    *r=0;
    while (n>=1) {
	char x;
	x=p[0]; p[0]=p[n]; p[n]=x; p++;
	n-=2;
    }
    return r;
}

char *DbgPutBaseInt(char *p,int b,int d) {
    if (d<0) {
	*p++='-';
	d=-d;
    }
    return DbgPutBaseUint(p,b,d);
}

char *DbgPutString(char *p,char *s) {
    while (*p++=*s++);
    return p-1;
}

/*_______________________________________________
 |
 |_______________________________________________
*/

#include <stdio.h>
#include <StackEnv.h>
#include <Classes.h>

typedef struct {
    BuffText BuffText;
    BuffText *Child;
} DbgBTSpy;

static char *BuffTextSpyCheck0(BuffText *this,char *p) {
    char Buff[32];
    char *r,*e;
    ThisToThat(DbgBTSpy,BuffText);
    r = this->p;
    e = Buff+31; *e = 0;
    if (r && p) {
        while (r<p) {
	    char *q;
            q = Buff;
	    while(r<p && q<e) *q++ = *r++;
	    *q = 0;
	    printf(Buff);
	}
    }
    p = Call(that->Child,Check0,1(p));
    this->p = that->Child->p;
    return p;
}

static long SpySeek(BuffText *this,long dp) {
    ThisToThat(DbgBTSpy,BuffText);
	return Call(that->Child,Seek,1(dp));
}
static void SpyClose(BuffText *this) { 
    ThisToThat(DbgBTSpy,BuffText);
	return Call(that->Child,Close,0);
}

BuffText *DbgBuffTextSpy(BuffText *BT) {
    DbgBTSpy *R;
    static struct BuffText Static = {
		BuffTextSpyCheck0,SpySeek,SpyClose
	};
    rPush(R);
    R->BuffText.Static = &Static;
    R->Child = BT;
    R->BuffText.p = BT->p;
    return &R->BuffText;
}

