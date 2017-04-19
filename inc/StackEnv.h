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

#ifndef _StackEnv_
#define _StackEnv_

/*  _________________________________________________________________
 * |                                                                 |
 * | This file contains definition of the main environment symbols.  | 
 * |_________________________________________________________________|
 */

typedef struct {
    void *p,*Bottom,*Top;
} MemStack;


MemStack *MemStackOpen(int GrowthSize);
void MemStackClose(MemStack *S);
MemStack *MemStackFork(MemStack *Root,int Growth);
void *MemStackPush(MemStack *S,void *Last);
void mEnter(MemStack *S);
void mLeave(MemStack *S);
void MemStackPop(MemStack *S,void *p);

#define mPush(m,R)    { R=(m)->p; (m)->p=(R+1); if((m)->p>(m)->Top) R=MemStackPush(m,R); }
#define mnPush(m,R,n) { R=(m)->p; (m)->p=(R+n); if((m)->p>(m)->Top) R=MemStackPush(m,R); }
#define mOpen(m)      { void *_S_; _S_=(m)->p; {
#define mClose(m)     } if (_S_>(m)->Top || _S_<(m)->Bottom) MemStackPop(m,_S_); else (m)->p=_S_; }

/*  _________________________________________________________________
 * |                                                                 |
 * | As a rule, we use to stacks structures: the parameter stack and |
 * | the result stack. These two stacks may be swapped whenever the  |
 * | result of a function should be used as parameter of the calling |
 * | function.                                                       |
 * | Calling rules follow:                                           |
 * | Position of stacks pointers should be restored before returning.|
 * | Whenever a functions returns a result, its value should be the  |
 * | adress following.                                               |
 * |_________________________________________________________________|
 */

typedef struct _Env_Env Env_Env;
struct _Env_Env {
    MemStack *p;
    MemStack *r;
};

extern __thread Env_Env Env;

/*  _________________________________________________________________
 * |                                                                 |
 * |  EnvOpen returns false if it couldn't be performed.             |
 * |  EnvClose returns 0 whatever happens.                           |
 * |_________________________________________________________________|
 */

int EnvOpen(long ParamSize,long ResultSize);
int EnvClose(void);

int xEnvOpen(Env_Env *x,long ParamSize,long ResultSize);
int xEnvClose(Env_Env *x);

#define EnvSwap() { MemStack *xchg; xchg = Env.p; Env.p = Env.r; Env.r = xchg; }

#define rnPush(x,n) mnPush(Env.r,x,n)
#define pnPush(x,n) mnPush(Env.p,x,n)
#define rPush(x)    mPush(Env.r,x)
#define pPush(x)    mPush(Env.p,x)
#define xrPush(e,x) mPush(e.r,x)
#define xrnPush(e,x,n) mnPush(x.r,n,x)

#define rOpen mOpen(Env.r)
#define rClose mClose(Env.r)
#define pOpen  mOpen(Env.p)
#define pClose mClose(Env.p)

#define rFork(Growth) MemStackFork(Env.r,Growth)
#define pFork(Growth) MemStackFork(Env.p,Growth)

#define sOpen mOpen(Env.r) mOpen(Env.p)
#define sClose mClose(Env.p) mClose(Env.r)

#define sSwap { MemStack *xchg; xchg=Env.p; Env.p=Env.r; Env.r=xchg; }

#define mIn(Mem,Inst) {\
    MemStack *_s_;\
    _s_ = Env.r;\
    Env.r = Mem;\
    Inst;\
    Env.r = _s_;\
}

#define pIn(Inst) \
    EnvSwap();\
    Inst;\
    EnvSwap();

#endif
