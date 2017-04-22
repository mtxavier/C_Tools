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

#include <Thd.h>
#include <Classes.h>
#include <StackEnv.h>
#include <stdio.h>

typedef struct {
    ThdItf ThdItf;
    int Num,End,Blocked;
    char *Msg;
} myMainParm;

static void myMain(ThdItf *this,ThdThread *Sync) {
    char Tab[80],Msg[200],*b,*e;
    int Count,Num,MsgPtr;
    ThisToThat(myMainParm,ThdItf);
    Num = that->Num;
    MsgPtr = 0;
    that->Msg = Msg+MsgPtr; MsgPtr = 100-MsgPtr;
    b = Tab; e = b+Num*3;
    while (b<e) { *b++=' '; }
    *b = 0;
    Count = 0;
    sprintf(that->Msg,"%sThread %d Starting\n",Tab,Num);
    while (Count<(15-Num)) {
        that->Blocked = (0==0);
        Call(Sync,Wait,0);
        that ->Blocked = (0!=0);
        that->Msg = Msg+MsgPtr; MsgPtr = 100-MsgPtr;
        sprintf(that->Msg,"%sThread %d --> %d\n",Tab,Num,Count);
        Count++;
    }
    that->Blocked = (0==0);
    Call(Sync,Wait,0);
    that ->Blocked = (0!=0);
    that->End = (0==0);
    that->Blocked = (0==0);
}

myMainParm *MyMainParm(int Num) {
    myMainParm *r;
    static struct ThdItf Static = {myMain};
    rPush(r);
    r->ThdItf.Static = &Static;
    r->Num = Num;
    r->Blocked = (0!=0);
    r->End = (0!=0);
    return r;
}

main() {
    myMainParm *p[5];
    ThdThread *Children[5];
    int i,j;
    int alived;
    EnvOpen(1024,4096);
    printf("\n");
    for (i=0;i<5;i++) {
        p[i] = MyMainParm(i);
        // Children[i] = ThdEnvFiberLaunch(&p[i]->ThdItf,4096,1024,1024);
        // Children[i] = ThdThreadLaunch(&p[i]->ThdItf,4096);
        Children[i] = ThdFiberLaunch(&p[i]->ThdItf,4096);
        // Children[i] = ThdNativeFiberLaunch(&p[i]->ThdItf,4096,4096,1024,1024);
    }
    alived = 0x1f;
    while (alived) {
        int mask,check;
        check = alived;
        printf("...\n");
        while (check) {
            mask = 1;
            for (j=0;j<5;j++) {
                if ((check & mask)&&(p[j]->Blocked)) {
                    check = check & (~mask);
                    if (!p[j]->End) printf(p[j]->Msg);
                    if (!Call(Children[j],Ack,0)) {
                        alived = alived & (~mask);
                        printf("%d RIP.\n",j); 
                    }
                }
                mask = mask<<1;
            }
        }
    }
    printf("\n");
    EnvClose();
}

