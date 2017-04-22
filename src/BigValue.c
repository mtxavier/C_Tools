/*  The code for BigValue. 
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

#include <StackEnv.h>
#include <Classes.h>
#include <List.h>
#include <BigValue.h>

typedef struct {
	List/*<BigValue>*/ List;
	unsigned int Value;
} BigValue;

struct BigInt {
	unsigned int Sign;
	List/*<BigValue*/ Val;
};

static BigValue BigValueOne = {{ListEnd},1};
static BigValue BigValueTwo = {{ListEnd},2};
static BigValue BigValueEight = {{ListEnd},8};
static BigValue BigValueTen = {{ListEnd},10};
static BigValue BigValueSixteen = {{ListEnd},16};
BigInt BigIntZero = {0,{ListEnd}};
BigInt BigIntMinusOne = {(unsigned int)(-1),{ListEnd}};
BigInt BigIntOne = {0,{&BigValueOne.List}};
BigInt BigIntTwo = {0,{&BigValueTwo.List}};
static BigInt BigIntEight = {0,{&BigValueEight.List}};
static BigInt BigIntTen = {0,{&BigValueTen.List}};
static BigInt BigIntSixteen = {0,{&BigValueSixteen.List}};

#define Debug
#ifdef Debug
#include <stdio.h>
/*Debug*/ void BigValueShow(List/*BigValue*/ *p) {
	BigValue *P;
	unsigned int v,m;
	char Word[16],*b,*e;
	if (p->n!=ListEnd) {
		BigValueShow(p->n);
		printf(":");
	}
	P = CastBack(BigValue,List,p);
    v = P->Value;
	m = -1;
    e = b = Word;
	while (m) {
        char c;
		c = v & 15;
		v = v>>4;
		m = m>>4;
		if (c<10) c+='0'; else c+='A'-10;
		*e++ = c;
	}
	*e-- = 0;
	while (b<e) {char x; x=*e; *e=*b; *b=x; e--; b++;}
	printf(Word);
}

/*Debug*/ void BigIntShow(char *Before,BigInt *O){
	 List/*<BigValue>*/ *p;
	 printf(Before);
	 if (O->Sign) printf("[-]");
	 p = O->Val.n;
	 if (p==ListEnd) {
		 if (O->Sign) printf("FFFFFFFF"); else printf("0");
	 } else {
	     BigValueShow(p);
	 }
}
#endif

BigInt *BigIntNew(int n) {
	BigInt *R;
	if (!n) {
	    R = &BigIntZero;
	} else {
		BigValue *vR;
		rPush(R);
        rPush(vR);
		R->Val.n = &vR->List;
		vR->List.n = ListEnd;
	    vR->Value = n;
		R->Sign = (n>=0)?0:-1;
	}
	return R;
}

static List/*<BigValue>*/ *BigIntCutPos(BigInt *C) {
    List/*<BigValue>*/ *p,*R;
    p = R = &C->Val;
    while (p->n!=ListEnd) {
	BigValue *P;
	p = p->n;
	P = CastBack(BigValue,List,p);
	if (P->Value!=C->Sign) R = p;
    }
    return R;
}

static BigInt *BigIntCut(BigInt *C) {
    List *CutPos;
    CutPos = BigIntCutPos(C);
    CutPos->n = ListEnd;
    return C;
}

BigInt *BigIntSum(BigInt *A,BigInt *B){
    BigInt *R;
    List/*<BigValue>*/ *p,*a,*b;
    unsigned int Carry;
    rPush(R);
    R->Sign = 0;
    p = &R->Val;
    a = A->Val.n;
    b = B->Val.n;
    Carry = 0;
    while (a!=ListEnd || b!=ListEnd) {
        unsigned int va,vb;
        BigValue *P;
	if (a!=ListEnd) {
            P = CastBack(BigValue,List,a);
	    a = a->n;
	    va = P->Value;
	} else {
	    va = A->Sign;
	}
	if (b!=ListEnd) {
	    P = CastBack(BigValue,List,b);
	    b = b->n;
	    vb = P->Value;
	} else {
	    vb = B->Sign;
	}
	rPush(P);
	p->n = &P->List;
	p = p->n;
	P = CastBack(BigValue,List,p);
	P->Value = va+vb+Carry;
	va+=Carry;
	if ((va!=0)&&(Carry!=0)) Carry = 0;
	vb+=va;
	if (vb<va) Carry = 1;
    }
    Carry += B->Sign+A->Sign;
    if ((Carry==0)||(Carry==1)) {
	R->Sign = 0;
        if (Carry==1) {
	    BigValue *P;
            rPush(P);
	    P->Value = Carry;
	    p->n = &P->List;
	    p = p->n;
	}
    } else {
	R->Sign = -1;
	if ((Carry&1)==0) {
	    BigValue *P;
            rPush(P);
	    P->Value = Carry;
	    p->n = &P->List;
	    p = p->n;
        }
    }
    p->n = ListEnd;
    return BigIntCut(R);
}

BigInt *BigIntNot(BigInt *A) {
    BigInt *R;
    List/*<BigValue>*/ *a,*p;
    rPush(R);
    p = &R->Val;
    a = A->Val.n;
    R->Sign = ~A->Sign;
	do {
	    BigValue *P;
	    unsigned int va;
		if (a!=ListEnd) {
	        P = CastBack(BigValue,List,a);
	        a = a->n;
	        va = P->Value;
		} else {
			va = A->Sign;
		}
	    rPush(P);
        p->n = &P->List;
	    p = p->n;
	    P->Value = ~va;
    } while (a!=ListEnd);
    p->n = ListEnd;
    return BigIntCut(R);
}

BigInt *BigIntAnd(BigInt *A,BigInt *B) {
    BigInt *R;
    List/*<BigValue>*/ *a,*b,*p;
    rPush(R);
    p = &R->Val;
    a = A->Val.n;
    b = B->Val.n;
    R->Sign = A->Sign & B->Sign;
    while (a!=ListEnd || b!=ListEnd) {
	BigValue *P;
	unsigned int va,vb;
	if (a!=ListEnd) {
	    P = CastBack(BigValue,List,a);
	    va = P->Value;
	} else {
            va = A->Sign;
	}
	if (b!=ListEnd) {
	    P = CastBack(BigValue,List,b);
	    vb = P->Value;
	} else {
	    vb = B->Sign;
	}
	rPush(P);
	p->n = &P->List;
        p = p->n;
	P->Value = va & vb;
    }
    p->n = ListEnd;
    return BigIntCut(R);
}

BigInt *BigIntMinus(BigInt *A) {
    BigInt *R;
    pOpen {
        BigInt *nA;
	pIn(nA = BigIntNot(A));
	R = BigIntSum(nA,&BigIntOne);
    } pClose
    return R;
}

BigInt *BigIntDiff(BigInt *A,BigInt *B) {
    BigInt *R;
    pOpen {
	BigInt *nB;
	pIn(nB = BigIntMinus(B));
	R = BigIntSum(A,nB);
    } pClose
    return R;
}

BigInt *BigIntOr(BigInt *A,BigInt *B) {
    BigInt *R;
    pOpen {
	BigInt *nA,*nB,*nAnd;
	EnvSwap();
	    nA = BigIntNot(A);
	    nB = BigIntNot(B);
	    nAnd = BigIntAnd(nA,nB);
	EnvSwap();
	R = BigIntNot(nAnd);
    } pClose
    return R;
}

BigInt *BigIntXor(BigInt *A,BigInt *B) {
    BigInt *R;
    pOpen {
	BigInt *nA,*nB,*xA,*xB;
	EnvSwap();
	    nA = BigIntNot(A);
	    nB = BigIntNot(B);
	    xA = BigIntAnd(A,nB);
	    xB = BigIntAnd(B,nA);
	EnvSwap();
	R = BigIntOr(xA,xB);
    } pClose
    return R;
}

static int BigIntGetIsZero(BigInt *Z) {
    int R;
    List/*<BigValue>*/ *p;
    R = !(Z->Sign);
    p = Z->Val.n;
    while (R && p!=ListEnd) {
	BigValue *P;
	P = CastBack(BigValue,List,p);
	R = !(P->Value);
	p = p->n;
    }
    return R;
}
int BigIntGetSign(BigInt *Z) {
    int R;
    if (!BigIntGetIsZero(Z)) {
	if (!Z->Sign) {
	    R = 1;
	} else {
	    R = -1;
	}
    } else {
	R = 0;
    }
    return R;
}

BigInt *BigIntBoolify(int B) {
	return B ? &BigIntOne:&BigIntZero;
}

/*____________________________________________
 |
 | Tie with shift, product and division
 |____________________________________________
*/

static int IntBitCount(void) {
    static int init=(0!=0);
    static int bc;
    if (!init) {
	    unsigned int Full;
	    Full = -1;
    	bc = 0;
    	while (Full) { bc++; Full = Full>>1; }
    	init = (0==0);
    }
    return bc;
}
static unsigned int IntLowHalf(unsigned int n) {
    static int init=(0!=0);
    static unsigned int Mask;
    if (!init) {
    	Mask = -1;
    	Mask = Mask>>(IntBitCount()>>1);
    	init = (0==0);
    }
    return n&Mask;
}
static unsigned int IntHighHalf(unsigned int n) {
    static init=(0!=0);
    static int bc;
    if (!init) {
    	bc = IntBitCount()>>1;
    	init = (0==0);
    }
    return n>>bc;
}
static unsigned int IntHalfMax(void) {
    return (((unsigned int)(-1))^(((unsigned int)(-1))>>1));
}

BigInt *BigIntWindow(BigInt *B,int Window) {
    BigInt *R;
    List/*<BigValue>*/ *p,*b;
    int bc;
    bc = IntBitCount();
    rPush(R);
    R->Sign = B->Sign;
    p = &R->Val;
    b = B->Val.n;
    while (Window>=bc) {
	    BigValue *P;
	    unsigned int vb;
	    Window-=bc;
	    if (b!=ListEnd) {
	        P = CastBack(BigValue,List,b);
	        vb = P->Value;
	        b = b->n;
	    } else {
	        vb = B->Sign;
	    }
	    rPush(P);
	    p->n = &P->List;
	    p = p->n;
	    P->Value = vb;
    }
    if (Window) {
	    BigValue *P;
	    unsigned int SignExtent,vb,vm;
	    SignExtent = B->Sign<<Window;
	    vm = (-1)^SignExtent;
	    if (b!=ListEnd) {
	        P = CastBack(BigValue,List,b);
	        vb = P->Value;
	    } else {
	        vb = B->Sign;
	    }
	    rPush(R);
	    p->n = &P->List;
	    p = p->n;
	    P->Value = (vb & vm) | SignExtent;
    }
    p->n = ListEnd;
    return R;
}

BigInt *BigIntSAR(BigInt *B,int S) {
    BigInt *R;
    List/*<BigValue>*/ *p,*b;
    int bc;
    unsigned int Carry;
    bc = IntBitCount();
    rPush(R);
    R->Sign = B->Sign;
    p = &R->Val;
    b = B->Val.n;
    while (bc<=S) { S-=bc; if (b!=ListEnd) b=b->n;}
    if (b!=ListEnd) {
	    BigValue *P;
	    P = CastBack(BigValue,List,b);
	    Carry = P->Value;
	    b = b->n;
    } else {
        Carry = B->Sign;
    }
    Carry = Carry>>S;
    while (b!=ListEnd) {
	    unsigned int vb;
    	BigValue *P;
    	P = CastBack(BigValue,List,b);
    	vb = P->Value;
    	b = b->n;
    	rPush(P);
    	p->n = &P->List;
    	p = p->n;
	    P->Value = S?Carry|(vb<<(bc-S)):Carry;
    	Carry = vb>>S;
    }
    /* Add Sign Extension */{
	    BigValue *P;
	    rPush(P);
	    p->n = &P->List;
	    p = p->n;
	    P->Value = S?(Carry|(R->Sign<<(bc-S))):Carry;
    }
    p->n = ListEnd;
    return R;
}

BigInt *BigIntSAL(BigInt *B,int S) {
    BigInt *R;
    List /*<BigValue>*/ *p,*b;
    int bc;
    unsigned int Carry;
    rPush(R);
    p = &R->Val;
    b = B->Val.n;
    bc = IntBitCount();
    R->Sign = B->Sign;
    while (bc<=S) {
	    BigValue *P;
	    rPush(P);
	    P->Value = 0;
	    S-=bc;
	    p->n = &P->List;
	    p = p->n;
    }
    Carry = 0;
    while (b!=ListEnd) {
	    unsigned int vb;
	    BigValue *P;
	    P = CastBack(BigValue,List,b);
	    vb = P->Value;
	    b = b->n;
	    rPush(P);
	    p->n = &P->List;
	    p = p->n;
	    P->Value = Carry | (vb<<S);
	    Carry = S?vb>>(bc-S):0;
    }
	Carry = Carry|(R->Sign<<S);
    if (Carry!=R->Sign) {
	    BigValue *P;
	    rPush(P);
	    p->n = &P->List;
	    p = p->n;
	    P->Value = Carry;
    }
    p->n = ListEnd;
    return R;
}

BigInt *BigIntSHR(BigInt *B,int S,int Window) {
    BigInt *R;
    pOpen {
	    BigInt *P;
	    unsigned int bs;
	    bs = B->Sign;
	    B->Sign = 0;
	    pIn(P = BigIntWindow(B,Window));
	    B->Sign = bs;
	    R = BigIntSAR(B,S);
    } pClose
    return R;
}

int BigIntGetRawValue(BigInt *V) {
	int s;
	if (V->Val.n==ListEnd) {
        s = 0;
	} else {
	    BigValue *v;
		v = CastBack(BigValue,List,V->Val.n);
		s = v->Value;
	}
    return s;
}

BigInt *BigIntBSAL(BigInt *B,BigInt *S) {
	int s;
	s = BigIntGetRawValue(S);
#define BigIntSecureShift 4096
	if (s<0) s = 0;
	if (s>BigIntSecureShift) s = BigIntSecureShift;
	return BigIntSAL(B,s);
}
BigInt *BigIntBSAR(BigInt *B,BigInt *S) {
	int s;
	s = BigIntGetRawValue(S);
	if (s<0) s = 0;
	return BigIntSAR(B,s);
}

BigInt *BigIntnlMul(unsigned int a,BigInt *B) {
    BigInt *R;
    pOpen {
	    BigInt *HR,*LR;
        List /*<BigValue>*/ *hp,*lp,*b;
	    int bc;
	    bc = IntBitCount();
        b = B->Val.n;
	    pPush(HR);
	    hp = &HR->Val;
	    HR->Sign = 0;
	    pPush(LR);
	    lp = &LR->Val;
	    LR->Sign = 0;
	    while (b!=ListEnd) {
	        unsigned int vb;
	        BigValue *R;
	        R = CastBack(BigValue,List,b);
	        b = b->n;
	        vb = R->Value;
	        pPush(R);
	        lp->n = &R->List;
	        R->Value = a*IntLowHalf(vb);
	        lp = lp->n;
	        pPush(R);
	        hp->n = &R->List;
	        R->Value = a*IntHighHalf(vb);
	        hp = hp->n;
    	}
    	hp->n = ListEnd;
    	lp->n = ListEnd;
    	pIn(HR = BigIntSAL(HR,bc>>1));
        R = BigIntSum(LR,HR);
    } pClose
    return R;
}

BigInt *BigIntnMul(unsigned int a,BigInt *B) {
    BigInt *R;
    List/*<BigValue>*/ *b;
    unsigned int la,ha;
    b = B->Val.n;
    la = IntLowHalf(a);
    ha = IntHighHalf(a);
    pOpen {
        BigInt *HR,*LR;
	int bc;
	bc = IntBitCount();
	pIn(
	    LR = BigIntnlMul(la,B);
	    HR = BigIntSAL(BigIntnlMul(ha,B),bc>>1);
	);
	R = BigIntSum(LR,HR);
    } pClose
    return R;
}

BigInt *BigIntuMul(BigInt *A,BigInt *B) {
    BigInt *R;
    int bc,Shift;
    List/*<BigValue>*/ *a;
    a = A->Val.n;
    R = &BigIntZero;
    bc = IntBitCount();
    Shift = 0;
    pOpen {
    EnvSwap()
    while (a!=ListEnd) {
	BigValue *P;
	unsigned int va;
	P = CastBack(BigValue,List,a);
	va = P->Value;
	a = a->n;
	pOpen {
	    BigInt *S;
	    pIn(S = BigIntSAL(BigIntnMul(va,B),Shift););
	    Shift += bc;
	    R = BigIntSum(R,S);
	} pClose
    }
    EnvSwap()
    R = BigIntSum(R,&BigIntZero); /* Copy the result in result stack */
    } pClose
    return R;
}

BigInt *BigIntMul(BigInt *A,BigInt *B) {
    BigInt *R;
	pOpen {
		BigInt *uA,*uB;
	    int sign;
	    sign = (0!=0);
	    if (A->Sign) {
			pIn(uA = BigIntMinus(A));
		    sign = !sign;
	    } else {
			uA = A;
		}
		if (B->Sign) {
			pIn(uB = BigIntMinus(B));
			sign = !sign;
		} else {
			uB = B;
		}
		if (sign) {
			BigInt *uR;
			pIn(uR = BigIntuMul(uA,uB));
			R = BigIntMinus(uR);
		} else {
			R = BigIntuMul(uA,uB);
		}
	} pClose
	return R;
}

/*____________________________________________
 |
 | Tie with Boolean Expression
 |____________________________________________
*/


int BigIntInfEq(BigInt *A,BigInt *B) {
    static int R[3]={(0==0),(0==0),(0!=0)};
    int d;
    pOpen {
	BigInt *D;
	pIn(D = BigIntDiff(A,B));
        d = BigIntGetSign(D);
    } pClose
    return R[d+1];
}

int BigIntInf(BigInt *A,BigInt *B) {
    static int R[3]={(0==0),(0!=0),(0!=0)};
    int d;
    pOpen {
	BigInt *D;
	pIn(D = BigIntDiff(A,B));
	d = BigIntGetSign(D);
    } pClose
    return R[d+1];
}

int BigIntSup(BigInt *A,BigInt *B) {return BigIntInf(B,A);}
int BigIntSupEq(BigInt *A,BigInt *B) {return BigIntInfEq(B,A);}

int BigIntEq(BigInt *A,BigInt *B) {
    static int R[3]={(0!=0),(0==0),(0!=0)};
    int d;
    pOpen {
	BigInt *D;
	pIn(D = BigIntDiff(A,B));
	d = BigIntGetSign(D);
    } pClose
    return R[d+1];
}

int BigIntNotEq(BigInt *A,BigInt *B) {
    return !BigIntEq(A,B);
}

/*______________________________________________
 |
 | Tie with Division
 |______________________________________________
*/

static int BigIntUBitCount(BigInt *A) {
	int R,bc;
	List/*<BigValue>*/ *p;
	R = 0;
    p = A->Val.n;
	bc = IntBitCount();
	while (p->n!=ListEnd) {
		R += bc;
		p = p->n;
	}
	if (p!=ListEnd) {
	    BigValue *P;
		unsigned int v;
		P = CastBack(BigValue,List,p);
		v = P->Value;
		while(v) { v=v>>1; R++;}
	}
	return R;
}

static BigIntDivResult *BigIntuDiv(BigIntDivResult *Res,BigInt *A,BigInt *B) {
	int c,bc,zero;
	bc = IntBitCount();
	zero = BigIntUBitCount(B);
	c = BigIntUBitCount(A)-zero;
	if (!zero) {
        Res->R = A;
		Res->Q = &BigIntZero;
	} else {
    pOpen {
		unsigned int v,vs;
		BigInt *R;
		List/*<BigValue>*/ *q;
	    rPush(Res->Q);
		Res->Q->Sign = 0;
		q = &Res->Q->Val;
		q->n = ListEnd;
        R = A;
		vs = c%bc; 
		v = 0;
		while (c>=0) {
		    rOpen {
				BigInt *BS;
		        BS = BigIntSAL(B,c);
			    v = v<<1;
				if (BigIntInfEq(BS,R)) {
					v = v+1;
					pIn(R=BigIntDiff(R,BS));
				}
			} rClose
			if (!vs) {
				BigValue *V;
                rPush(V);
				V->Value = v;
				V->List.n = q->n;
				q->n = &V->List;
				vs = bc;
				v = 0;
			}
			vs--;
			c--;
		}
		Res->R = BigIntSum(R,&BigIntZero);
	} pClose
	}
	return Res;
}

BigIntDivResult *BigIntDivMod(BigIntDivResult *Res,BigInt *A,BigInt *B) {
	pOpen {
	    BigInt *uA,*uB;
	    int Sign;
		Sign = ((A->Sign)!=(B->Sign));
		if (A->Sign) {
			pIn(uA=BigIntMinus(A));
	    } else {
			uA = A;
	    }
		if (B->Sign) {
			pIn(uB=BigIntMinus(B));
		} else {
			uB = B;
		}
		if (Sign) {
            pIn(Res=BigIntuDiv(Res,uA,uB));
			if (BigIntGetSign(Res->R)==0) {
				Res->R = &BigIntZero;
				Res->Q = BigIntMinus(Res->Q);
			} else {
			    BigInt *Q;
				Res->R = BigIntDiff(uB,Res->R);
				pIn(Q = BigIntMinus(Res->Q));
				Res->Q = BigIntDiff(Q,&BigIntOne);
			}
		} else {
		    Res = BigIntuDiv(Res,uA,uB);
		}
	} pClose
	return Res;
}

BigInt *BigIntDiv(BigInt *A,BigInt *B) {
	BigInt *R;
	pOpen {
		BigIntDivResult DR;
		pIn(BigIntDivMod(&DR,A,B));
		R = BigIntSum(DR.Q,&BigIntZero);
	} pClose
	return R;
}

BigInt *BigIntMod(BigInt *A,BigInt *B) {
	BigInt *R;
	pOpen {
	    BigIntDivResult DR;
		pIn(BigIntDivMod(&DR,A,B));
		R = BigIntSum(DR.R,&BigIntZero);
	} pClose
	return R;
}

/*________________________________________________
 |
 | Tie with ascii form.
 |________________________________________________
*/

BigInt *BigIntGet(char *Src) {
    BigInt *R;
	BigInt *Base;
	int Sign;
	char *p;
	Base = &BigIntTen;
	p = Src;
	Sign = (*p=='-');
	if (Sign) p++;
	if (*p=='0') {
		p++;
		if (*p=='x' || *p=='X') {
			p++;
			Base = &BigIntSixteen;
		} else {
			Base = &BigIntEight;
		}
	}
	R = &BigIntZero;
	pOpen {
		BigInt V;
		BigValue v;
        v.List.n = ListEnd;
		v.Value = 0;
		V.Sign = 0;
		V.Val.n = &v.List;
		EnvSwap();
		do {
		    char c;
		    c = *p++;
		    if (c>='0' && c<='9') {
			    v.Value = c-'0';
		    } else {
			    if (c>='a' && c<='f') {
					v.Value = 10 + c-'a';
			    } else {
				    if (c>='A' && c<='F') {
						v.Value = 10 + c-'A';
				    } else {
					    v.Value = 16;
				    }
			    }
		    }
			if (v.Value<=15) R = BigIntSum(&V,BigIntMul(R,Base));
		} while (v.Value<=15);
		EnvSwap();
		if (Sign) {
			R = BigIntMinus(R);
		} else {
		    R = BigIntSum(R,&BigIntZero);
		}
	} pClose
	return R;
}

char *BigIntPut(BigInt *Src) {
	char *R,*p;
	int Size,bc;
	bc = BigIntUBitCount(Src);
    Size = 3 + (bc/3);
	rnPush(R,Size);
	p = R;
	rOpen {
		char *q;
		BigInt *Q;
	    if (Src->Sign) {
			*p++='-';
			Q = BigIntMinus(Src);
		} else {
			Q = Src;
		}
		q = p;
		do {
			BigIntDivResult DR;
			BigIntDivMod(&DR,Q,&BigIntTen);
			Q = DR.Q;
			if (DR.R->Val.n == ListEnd) {
				*p++ = '0';
			} else {
				BigValue *V;
				V = CastBack(BigValue,List,DR.R->Val.n);
				*p++ = '0'+V->Value;
			}
		} while(BigIntGetSign(Q)!=0);
	    *p-- = 0;
		while (q<p) {
			char x;
			x = *p; *p = *q; *q=x; 
			p--; q++;
		}
	} rClose
	return R;
}


