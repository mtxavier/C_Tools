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
#include <C_Util.h>
#include <CPrepMacro.h>
#include <stdarg.h>
#include <Debug.h>

/*--------------------------*/

/*___________________________________________________________
 |
 [ BuffTextNull
 |___________________________________________________________
*/

static char *BuffTextNullCheck0(BuffText *this,char *p) {
    static char StNull[]="";
    return StNull;
}

static long BuffTextNullSeek(BuffText *this,long dp) { return 0; }
static void BuffTextNullClose(BuffText *this) {}

BuffText *BuffTextNull(void) {
    static struct BuffText Static = {BuffTextNullCheck0,BuffTextNullSeek,BuffTextNullClose};
    static BuffText BTNull = {&Static,0};
    BTNull.p = BuffTextNullCheck0(&BTNull,0);
    return &BTNull;
}

/*___________________________________________________________
 |
 |  BTFileAutoClose
 |___________________________________________________________
*/

typedef struct {
    BuffText BuffText;
    BuffText *Child;
} BTFileAutoClose;

static char *BTFileAutoCloseCheck0(BuffText *this,char *p) {
    ThisToThat(BTFileAutoClose,BuffText);
    this->p = Call(that->Child,Check0,1(p));
    if (!this->p) {
	    that->Child = BuffTextNull();
	    this->p = Call(that->Child,Check0,1(this->p));
		Call(that->Child,Close,0);
    }
    return this->p;
}

static long BTFileAutoCloseSeek(BuffText *this,long dp) {
    long l;
    ThisToThat(BTFileAutoClose,BuffText);
    l = Call(that->Child,Seek,1(dp));
    this->p = that->Child->p;
    return l;
}

static void BTFileAutoCloseClose(BuffText *this) {
	ThisToThat(BTFileAutoClose,BuffText);
	Call(that->Child,Close,0);
	this->p = that->Child->p;
}

BuffText *BTFileAutoCloseNew(char *Name) {
    static struct BuffText Static = {BTFileAutoCloseCheck0,BTFileAutoCloseSeek,BTFileAutoCloseClose};
    BTFileAutoClose *R;
    rPush(R);
    R->BuffText.Static = &Static;
    R->Child = BTFileNew(Name,2048);
    R->BuffText.p = R->Child->p;
    return &R->BuffText;
}

/*__________________________________________________
 |
 | EoLinePosRecord
 |__________________________________________________
*/
typedef struct {
	List/*<NewLinePos>*/ List;
	char *p;
	int EndPos;
    int nb;
} EoLinePos;
typedef struct {
	MemStack *Mem;
	EoxFeedback *Feedback;
	List/*<NewLinePos>*/ Record;
	List/*<NewLinePos>*/ Free;
	List *EndRecord;
	char *b,*e,*read,*write;
	int Line,Pos;
} EoLinePosRecord;
static EoLinePos *EoLinePosAlloc(EoLinePosRecord *this) {
	List *r;
	EoLinePos *R;
	r = this->Free.n;
	if (r==ListEnd) {
		mPush(this->Mem,R);
	} else {
		this->Free.n = r->n;
		R = CastBack(EoLinePos,List,r); 
	}
	return  R;
}
#define EoLinePosRecordWriteAt(this,q) this->write = q
static void EoLinePosRecordWriteLine(EoLinePosRecord *this,int filepos) {
	EoLinePos *F;
	F = 0;
	if (this->EndRecord!=&this->Record) {
        F = CastBack(EoLinePos,List,this->EndRecord);
		if (F->p!=this->write) F = 0;
	} 
	if (!F) {
		F = EoLinePosAlloc(this);
		F->p = this->write;
		F->nb = 0;
		F->List.n = ListEnd;
		this->EndRecord->n = &F->List;
		this->EndRecord = this->EndRecord->n;
	}
	F->EndPos = filepos;
	F->nb++;
}
static void EoLinePosRecordReadAt(EoLinePosRecord *this,char *E) {
	List *l;
	char *e,*p;
	p = this->read;
    l = this->Record.n;
	e = (E>=p)? E:this->e;
	while (l!=ListEnd && p!=E) {
	    EoLinePos *L;
		L = CastBack(EoLinePos,List,l);
		if ((p<=L->p)&&(L->p<e)) {
			List *rm;
			rm = l;
			p = L->p;
			this->Pos = L->EndPos;
			while (L->nb>0) {
			    L->nb--;
				Call(this->Feedback,EndOfLine,2(this->Line,this->Pos));
				this->Line++;
			}
			l=l->n;
			if (rm==this->EndRecord) this->EndRecord = &this->Record;
			this->Record.n = l;
			rm->n = this->Free.n;
			this->Free.n = rm;
	    } else {
			this->Pos+=e-p;
			p = e;
		}
		if ((p!=E)&&(p==e)) {
			p = this->b;
			e = E;
		}
	}
    if (p!=E) {
		if (p<E) {
			this->Pos+=E-p;
		} else {
			this->Pos+=(E-this->b)+(this->e-p);
		}
	}
	this->read = E;
}
static EoLinePosRecord *EoLinePosRecordNew(char *b,char *e,EoxFeedback *Feedback) {
	EoLinePosRecord *R;
	MemStack *Mem;
	Mem = rFork(256);
	mPush(Mem,R);
    R->Mem = Mem;
	R->b = b; R->e = e; R->write = R->read = R->b;
	R->Line = 0; R->Pos = 0;
	R->Free.n = ListEnd;
	R->Record.n = ListEnd;
	R->EndRecord = &R->Record;
	R->Feedback = Feedback;
	return R;
}

/*__________________________________________________
 |
 | BuffTextList
 |__________________________________________________
*/

typedef struct {
    BuffText *BuffText;
    List List;
} BuffTextList;

struct _BTList_ {
    BuffText BuffText;
    List *Current,*Last;
};

BuffText *BTListGetBT(BTList *L) {
    return &L->BuffText;
}

static char *BTListCheck0(BuffText *this,char *p) {
    List *L;
    BuffTextList *B;
    char *pp;
    ThisToThat(BTList,BuffText);
    L = that->Current;
    B = CastBack(BuffTextList,List,L);
    pp = Call(B->BuffText,Check0,1(p));
    if (pp) p = pp;
    L = L->n;
    while (!*p && L!=ListEnd) {
	    B = CastBack(BuffTextList,List,L);
	    p = B->BuffText->p;
	    pp = Call(B->BuffText,Check0,1(p));
	    if (pp) p = pp;
	    that->Current = L;
	    L = L->n;
    }
    this->p = p;
    return this->p;
}


static void BTListClose(BuffText *this) {
	List *L;
	BuffTextList *B;
	ThisToThat(BTList,BuffText);
    L = that->Current;
	B = CastBack(BuffTextList,List,L);
	Call(B->BuffText,Close,0);
	this->p = B->BuffText->p;
	L = L->n;
	while (L!=ListEnd) {
		B = CastBack(BuffTextList,List,L);
		Call(B->BuffText,Close,0);
		L = L->n;
	}
}

BTList *BTListNew(BuffText *B,...) {
    BTList *R;
    static struct BuffText Static = {BTListCheck0,BuffTextNullSeek,BTListClose};
    rPush(R);
    R->BuffText.Static = &Static;
    if (!B) {
        BuffTextList *Begin;
        rPush(Begin);
        R->Last = R->Current = &Begin->List;
        Begin->List.n = ListEnd;
        Begin->BuffText = BuffTextNull();
        R->BuffText.p = Begin->BuffText->p;
    } else {
        va_list C;
	    BuffTextList *L;
	    R->BuffText.p = B->p;
        rPush(L);
	    L->List.n = ListEnd;
	    L->BuffText = B;
	    R->Current = &L->List;
	    va_start(C,B);
	    B = va_arg(C,BuffText *);
	    while (B) {
	        BuffTextList *N;
	        rPush(N);
	        N->List.n = ListEnd;
	        N->BuffText = B;
	        L->List.n = &N->List;
	        L = N;
	        B = va_arg(C,BuffText *);
	    }
	    va_end(C);
        R->Last = &L->List;
    }
    return R;
}

BTList *BTListCatBuffer(BTList *this,BuffText *B) {
    BuffTextList *N;
    rPush(N);
    N->List.n = ListEnd;
    N->BuffText = B;
    this->Last->n = &N->List;
    this->Last = &N->List;
    return this;
}

/*________________________________________________
 |
 | EoxFeedback
 |________________________________________________
*/

static void EFbNullEndOfFile(EoxFeedback *this) {}
static void EFbNullEndOfLine(EoxFeedback *this,int NewLineNumber,int Pos){}
static void EFbNullPosInFile(EoxFeedback *this,int NewPos) {}
static struct EoxFeedback EFbNullStatic = {
	EFbNullEndOfFile,EFbNullEndOfLine,EFbNullPosInFile
};
EoxFeedback EoxFeedbackNull = {&EFbNullStatic};

static EoxFeedback *EFbConstrNullInstantiate(EoxFeedbackConstr *this) {return &EoxFeedbackNull;}
static struct EoxFeedbackConstr EFbConstrNullStatic = {EFbConstrNullInstantiate};
EoxFeedbackConstr EoxFeedbackConstrNull = {&EFbConstrNullStatic};

/*_________________________________________________
 |
 | EoxFeedbackQuote
 |_________________________________________________
*/

typedef struct {
	EoxFeedbackConstr EoxFeedbackConstr;
	EoxFeedback *Const;
} EFCConst;
static EoxFeedback *EFCConstInstantiate(EoxFeedbackConstr *this) {
	ThisToThat(EFCConst,EoxFeedbackConstr);
	return that->Const;
}
EoxFeedbackConstr *EoxFeedbackQuote(EoxFeedback *Const) {
	EFCConst *R;
	static struct EoxFeedbackConstr Static = {EFCConstInstantiate};
	rPush(R);
	R->EoxFeedbackConstr.Static = &Static;
	R->Const = Const;
	return &R->EoxFeedbackConstr;
}


/*________________________________________________
 |
 | String construction, for arbitrary long string
 |________________________________________________
*/

struct _TokenConstruct_ {
    List List;
    char *Begin,*End;
};

char *StringConstructGetEnd(StringConstruct *SC) {
    return SC->e;
}

TokenConstruct *StgCnsNewToken(StringConstruct *this) {
    TokenConstruct *R;
    mnPush(this->Mem,R,4);
    R->List.n = ListEnd;
    R->Begin = (char *)(R+1);
    R->End = (char *)(R+4);
    R->End--; *R->End = 0;
    return R;
}

char *StringConstructNext(StringConstruct *this,char *e) {
    if (e) {
	this->p = e;
    }
    if (this->p>=this->End->End) {
        TokenConstruct *Token;
        this->Length += (this->End->End-this->End->Begin);
        if (this->End->List.n==ListEnd)  {
            Token = StgCnsNewToken(this);
            this->End->List.n = &Token->List;
	} else {
            Token = CastBack(TokenConstruct,List,this->End->List.n);
	}
        this->End = Token;
        this->p = Token->Begin;
        this->e = Token->End;
    }
    *this->p = 0;
    return this->p;
}

char *StringConstructLast(StringConstruct *this,char *e) {
    char *r;
    if (e>=this->End->Begin && e<this->e) {
	r = this->p = e;
    } else {
	int Length;
	List *l;
	TokenConstruct *L;
	l = this->Begin;
	Length = 0;
	do {
	    L = CastBack(TokenConstruct,List,l);
	    Length += L->End - L->Begin;
	    l = l->n;
	} while (l!=ListEnd && (e<L->Begin || e>=L->End));
	if (e>=L->Begin && e<L->End) {
	    this->End = L;
	    this->p = e;
	    this->e = L->End;
	    Length += e - L->End;
	    this->Length = Length;
	    *this->p = 0;
	    r = this->p;
	} else {
	    r = StringConstructNext(this,e);
	}
    }
    return r;
}

void StringConstructClose(StringConstruct *this) {
    MemStackClose(this->Mem);
}

int StringConstructCurrentLength(StringConstruct *this) {
    return this->Length+(this->p - this->End->Begin);
}

char *StringConstructGetString(StringConstruct *this) {
    int l;
    char *r,*d;
    List *p;
    l = (StringConstructCurrentLength(this)+sizeof(void *)) & (-sizeof(void *));
    rnPush(r,l);
    d = r;
    p = this->Begin;
    while (p->n!=ListEnd) {
	TokenConstruct *L;
	char *s,*e;
	L = CastBack(TokenConstruct,List,p);
	s = L->Begin; e=L->End;
	while (s<e) *d++=*s++;
	p = p->n;
    }
    { 
	char *s,*e;
	s = this->End->Begin; e=this->p;
	while (s<e) *d++=*s++;
    }
    *d = 0;
    return r;
}

StringConstruct *StringConstructNew(void) {
    StringConstruct *R;
    TokenConstruct *Data;
    MemStack *Mem;
    Mem = MemStackFork(Env.r,sizeof(StringConstruct)+4*sizeof(TokenConstruct));
    mPush(Mem,R);
    R->Mem = Mem;
    Data = StgCnsNewToken(R);
    R->End = Data;
    R->Begin = &Data->List;
    R->p = Data->Begin;
    R->e = Data->End;
    R->Length = 0;
    return R;
}

StringConstruct *StringConstructStringNew(char *p) {
    StringConstruct *R;
    char *q,*e;
    R = StringConstructNew();
    q = StringConstructNext(R,0); e = R->e;
    while (*p) {
	do {
	    *q++ = *p++;
	} while (*p && q<e);
	q = StringConstructNext(R,q); e = R->e;
    }
    return R;
}


/*________________________________________________
 |
 |________________________________________________
*/

typedef struct {
    BuffText BuffText;
    StringConstruct *String;
    TokenConstruct *P;
} BTStringConstruct;

char *BTStringConstructCheck0(BuffText *this,char *p) {
    ThisToThat(BTStringConstruct,BuffText);
    this->p = p;
    if (p<that->P->Begin || p>=that->P->End) {
	if (that->P->List.n != ListEnd) {
	    TokenConstruct *N;
	    that->P = CastBack(TokenConstruct,List,that->P->List.n);
        this->p = that->P->Begin;
	} else {
        this->p = that->P->End;
	}
    }
    return this->p;
}

long BTStringConstructSeek(BuffText *this,long dp) { return 0; }
void BTStringConstructClose(BuffText *this) {
	ThisToThat(BTStringConstruct,BuffText);
	that->P = CastBack(TokenConstruct,List,that->String);
	this->p = that->P->Begin; 
}

BuffText *BTStringConstructNew(StringConstruct *S) {
    BTStringConstruct *R;
    static struct BuffText Static = {BTStringConstructCheck0,BTStringConstructSeek,BTStringConstructClose};
    rPush(R);
    R->String = S;
    R->P = CastBack(TokenConstruct,List,S->Begin);
    R->BuffText.Static = &Static;
    R->BuffText.p = R->P->Begin;
    return &R->BuffText;
}

/*___________________________________________________________
 |
 |  Preprocessor Buff Text
 |  Do the 2 primary pass :
 |  1)
 |       three character sequences have to be replace [ISO 644-1983]
 |       ??= #   ??( [   ??< {
 |       ??/ \   ??) ]   ??> }
 |       ??' ^   ??! |   ??- ~
 |  2)
 |       remove \\n 
 |  From the pointer, you always get at least n characters
 |  in the buffer, so p[0],p[1],...,p[n-1] make sense past
 |  the beginning of buffer, those characters are padded with 0.
 |  It is useful for a forwrd lexical analysis
 |___________________________________________________________
*/

typedef struct _BTCPreproc_ BTCPreproc;
struct _BTCPreproc_ {
    BuffText BuffText;
    BuffText *Child;
    char *b,*e; /* *e must be set to 0, it belongs to the buffer */
    struct {
        char *bottom,*top;
	char *b,*e;
    } Fwd;
    char *IsBlank;
    struct {
	    char *No;
	    char (*Handle)(BTCPreproc *this);
	    int EndString;
    } Except;
    EoLinePosRecord *Position;
};

static char *BTCPreprocReadCircle(BTCPreproc *this,char *q) {
    char *b,*e,*r,*p;
    b = this->Fwd.b; r = e = this->Fwd.e;
    if (e<b) e += (this->Fwd.top-this->Fwd.bottom);
    if (q<b) q += (this->Fwd.top-this->Fwd.bottom);
    p = this->Child->p;
    while (q>=e && *p) {
	    *r = *p;
        if (*r) {
			if (*r=='\n') {EoLinePosRecordWriteLine(this->Position,0);}
	        p++;
	        if (!*p) p = Call(this->Child,Check0,1(p));
	    }
	    r++; if (r>=this->Fwd.top) r += (this->Fwd.bottom-this->Fwd.top);
	    e++;
    }
    this->Fwd.e = r;
    Call(this->Child,Check0,1(p));
    while (q>=this->Fwd.top) q+=(this->Fwd.bottom-this->Fwd.top);
    return q;
}

static char BTCPreprocIGetChar(BTCPreproc *this) {
    int r;
    char *q[4];
    do {
        q[0] = BTCPreprocReadCircle(this,this->Fwd.b);
        if (*(q[0])=='?') {
	        q[1] = BTCPreprocReadCircle(this,(q[0])+1);
	        if (*(q[1])=='?') {
	            q[2] = BTCPreprocReadCircle(this,(q[1])+1);
	            if (*(q[2])=='/') {
		            q[3] = BTCPreprocReadCircle(this,(q[2])+1);
		               if (*(q[3])=='\n') {
		                   this->Fwd.b = q[3]+1;
		                   if (this->Fwd.b>=this->Fwd.top) this->Fwd.b = this->Fwd.bottom;
		                   r = 256;
		               } else {
		                   r = '\\';
		                   this->Fwd.b = q[3];
		               }
	             } else {
	                 switch(*(q[2])) {
		             case '<': r='{'; break;
		             case '>': r='}'; break;
	                 case '(': r='['; break;
		             case ')': r=']'; break;
		             case '!': r='|'; break;
		             case '-': r='~';  break;
		             case '=': r='#';  break;
		             case '\'': r='^';  break;
		             default : r ='?'; break; 
	                 }
	                 if (r=='?') {
		                 this->Fwd.b = q[1];
	                 } else {
		                 this->Fwd.b = q[2]+1;
		                 if (this->Fwd.b>=this->Fwd.top) this->Fwd.b = this->Fwd.bottom;
	                 }
	            }
	        } else {
	            r = '?';
	            this->Fwd.b = q[1];
	        }
        } else {
            if (*(q[0])=='\\') {
	            q[1] = BTCPreprocReadCircle(this,(q[0])+1);
	            if (*(q[1])=='\n') {
		            this->Fwd.b = q[1]+1;
		            if (this->Fwd.b>=this->Fwd.top) this->Fwd.b = this->Fwd.bottom;
		            r = 256;
	            } else {
		            r = '\\';
		            this->Fwd.b = q[1];
	            }
	        } else {
	            r = *(q[0]);
	            this->Fwd.b++; 
	            if (this->Fwd.b>=this->Fwd.top) this->Fwd.b=this->Fwd.bottom;
	        }
        }
    } while (r==256);
    return r;
}

char BTCPreprocIIGetChar(BTCPreproc *this);

void BTCPreprocIPutBackChar(BTCPreproc *this,char c) {
    this->Fwd.b--;
    if (this->Fwd.b<this->Fwd.bottom) this->Fwd.b+=this->Fwd.top-this->Fwd.bottom;
    *this->Fwd.b=c;
}

char BTCPreprocISkipBlank(BTCPreproc *this) {
    char *p,*IsBlank,r;
    IsBlank = this->IsBlank;
    r = ' ';
    do {
	while (IsBlank[r] && this->Fwd.b!=this->Fwd.e) {
	    char *p;
	    *this->Fwd.e = 0;
	    p = this->Fwd.b;
	    while (IsBlank[*p]) {
		    p++; if (p>=this->Fwd.top) p = this->Fwd.bottom;
	    }
	    this->Fwd.b = p;
	    if (p == this->Fwd.e) {
		    r = ' ';
	    } else {
	        r = *p;
	        if (r=='?' || r=='\\' || r=='/') {
                r = BTCPreprocIIGetChar(this);
		        if (!IsBlank[r] && r) BTCPreprocIPutBackChar(this,r); 
	        }
	    }
	}
	while (IsBlank[r] && this->Fwd.b==this->Fwd.e) {
        p = this->Child->p;
	    do {
	        while (IsBlank[*p]) p++;
	        p = Call(this->Child,Check0,1(p));
	    } while (IsBlank[*p]);
	    r = *p;
	    if (r=='?'|| r=='\\' || r=='/') {
		    r = BTCPreprocIIGetChar(this);
		    if (!IsBlank[r] && r) BTCPreprocIPutBackChar(this,r); 
	    }
	} 
    } while(IsBlank[r] && r);
    return r;
}

char BTCPreprocISkipCComment(BTCPreproc *this) {
    int In;
    In = (0==0);
    do {
	char r;
	while (In && this->Fwd.b!=this->Fwd.e) {
	    char *p;
	    *this->Fwd.e = '*';
	    p = this->Fwd.b;
	    while (*p!='*') {p++; if(p>=this->Fwd.top) p = this->Fwd.bottom;}
	    this->Fwd.b = p;
	    if (p!=this->Fwd.e) {
		    this->Fwd.b++; 
		    if(this->Fwd.b>=this->Fwd.top) this->Fwd.b = this->Fwd.bottom;
		    r = BTCPreprocIGetChar(this);
		    In = (r!='/') && r;
	    }
	}
	if (In) {
	    char *p;
	    p = this->Child->p;
	    do {
		    while (*p!='*' && *p) {
				if (*p=='\n') { EoLinePosRecordWriteLine(this->Position,0);}
				p++;
			}
		    p = Call(this->Child,Check0,1(p));
		    if (*p=='*') {
                p++; if (!*p) p = Call(this->Child,Check0,1(p));
		        In = (*p && *p!='/');
		        if (*p=='/') {
			        p++; p=Call(this->Child,Check0,1(p));
		        } else {
			        if (*p=='?'||*p=='\\') {
			            r = BTCPreprocIGetChar(this);
			            In = (r!='/') && r;
			        }
		        }
		    }
	    } while(In && this->Fwd.b==this->Fwd.e);
	}
    } while(In);
    return ' ';
}

char BTCPreprocISkipCPPComment(BTCPreproc *this) {
    int In;
    In = (0==0);
    do {
	while(In && this->Fwd.b!=this->Fwd.e) {
	    char *p;
	    p = this->Fwd.b;
	    *this->Fwd.e = '\n';
	    while (*p!='?' && *p!='/' && *p!='\n') {
	        p++; if(p>=this->Fwd.top) p = this->Fwd.bottom;
	    }
	    this->Fwd.b = p;
	    if (p!=this->Fwd.e) {
		    In = (*p!='\n');
		    if (*p=='?' || *p=='/') {
		        char r;
		        r=BTCPreprocIGetChar(this);
		        In = (r!='\n') && r;
		    }
	    }
	}
	while(In && this->Fwd.b==this->Fwd.e) {
	    char *p;
	    p = this->Child->p;
	    while (*p && *p!='/' && *p!='?' && *p!='\n') p++;
	    p = Call(this->Child,Check0,1(p));
	    In = (*p) && (*p!='\n');
	    if (In) {
		    char r;
		    r = BTCPreprocIGetChar(this);
		    In = (r!='\n');
	    } else {
		    if (*p=='\n') {
				p++;
                EoLinePosRecordWriteLine(this->Position,0);
			}
		    Call(this->Child,Check0,1(p));
	    }
	}
    } while(In);
    return '\n';
}

char *BTCPreprocIIsBlank(void) {
    static char IsBlank[256];
    static int Init=(0!=0);
    if (!Init) {
	static char Blanks[] = " \t\r\v\b\f\a";
	char *p,*e;
        p = IsBlank; e = p+256;
	while (p<e) *p++=(0!=0);
	p = Blanks;
	while (*p) IsBlank[*p++]=(0==0);
	Init = (0==0);
    }
    return IsBlank;
}

char *BTCPreprocIExceptNo(void) {
    static int Init = (0!=0);
    static char No[256];
    if (!Init) {
	    char *p,*e;
	    Init = (0==0);
	    p = No; e = p+256;
	    while (p<e) *p++ = (0==0);
	    No['?'] = No['\\']= (0!=0);
    }
    return No;
}

char *BTCPreprocIINoneExceptNo(void) {
    static char Init=(0!=0);
    static char No[256];
    if (!Init) {
	char *p,*e;
	Init = (0==0);
	p = No; e = p+256;
	while (p<e) *p++=(0!=0);
    }
    return No;
}

char *BTCPreprocIIStringExceptNo(void) {
    static char Init=(0!=0);
    static char No[256];
    if (!Init) {
	    char *p,*e;
	    Init = (0==0);
	    p = No; e = p+256;
	    while (p<e) *p++=(0!=0);
	    p = No;
	    p['?'] = p['\\'] = p['\"'] = p['\''] = (0!=0);
    }
    return No;
}

char *BTCPreprocIIExceptNo(void) {
    static char Init=(0!=0);
    static char No[256];
    if (!Init) {
	char *p,*e;
	Init = (0==0);
	p = No; e = p+256;
	while (p<e) *p++=(0==0);
	    p = No;
	    p['?'] = p['/'] = p['\\'] = p['\n'] = p['\''] = p['\"'] = (0!=0);
    }
    return No;
}

char BTCPreprocIIGetChar(BTCPreproc *this);
char BTCPreprocIISkipExceptHandle(BTCPreproc *this);
char BTCPreprocIIStringExceptHandle(BTCPreproc *this);

char BTCPreprocIISkipExceptHandle(BTCPreproc *this) {
    int r;
    r = BTCPreprocIGetChar(this);
    this->Except.Handle = BTCPreprocIIStringExceptHandle;
    this->Except.No = BTCPreprocIIStringExceptNo();
    return r;
}

char BTCPreprocIIStringExceptHandle(BTCPreproc *this) {
    int r;
    r = BTCPreprocIGetChar(this);
    if (r==this->Except.EndString) {
	this->Except.Handle = BTCPreprocIIGetChar;
	this->Except.No = BTCPreprocIIExceptNo();
    }
    if (r=='\\') {
	this->Except.Handle = BTCPreprocIISkipExceptHandle;
	this->Except.No = BTCPreprocIINoneExceptNo();
    }
    return r;
}

char BTCPreprocIIGetChar(BTCPreproc *this) {
    int r;
    r = BTCPreprocIGetChar(this);
    switch (r) {
    case '/': {
        char *q[4];
        q[0] = BTCPreprocReadCircle(this,this->Fwd.b);
	    if (*q[0]=='*' || *q[0]=='/') {
	        this->Fwd.b = q[0]+1; 
	        if(this->Fwd.b>=this->Fwd.top) this->Fwd.b = this->Fwd.bottom;
	        if (*q[0]=='*') {
	            BTCPreprocISkipCComment(this);
	            r = ' ';
	        } else {
		       BTCPreprocISkipCPPComment(this);
		       r = '\n';
	        }
	    }
    } break;
    case '\n':
        BTCPreprocISkipBlank(this);
    break;
    case '\'':
    case '\"':
	    this->Except.Handle = BTCPreprocIIStringExceptHandle;
        this->Except.No = BTCPreprocIIStringExceptNo();
	    this->Except.EndString = r;
    break;
    } 
    return r;
}

static char *BTCPreprocIICheck0(BuffText *this,char *p) {
    ThisToThat(BTCPreproc,BuffText);
    if (p>=that->e) { /* >= takes into account the end 0 */
	    char *q,*e;
	    char *NoExcept;
		EoLinePosRecordReadAt(that->Position,that->b);
	    q = that->b;
	    e = that->e;
	    NoExcept = that->Except.No;
        do {
	        while ((that->Fwd.b!=that->Fwd.e)&&(q<e)) {
		        p = that->Fwd.b;
		        while (NoExcept[*p] && p!=that->Fwd.e && q<e) {
		            *q++ = *p++;
		            if (p>=that->Fwd.top) p = that->Fwd.bottom;
		        }
		        that->Fwd.b = p;
		        if (q<e && !NoExcept[*p] && p!=that->Fwd.e) {
					EoLinePosRecordWriteAt(that->Position,q);
		            *q++ = that->Except.Handle(that);
	                NoExcept = that->Except.No;
		        }
	        }
	        do {
	            p = that->Child->p;
		        while (*p && NoExcept[*p] && q<e) {
		            do {
			            *q++ = *p++;
		            } while (*p && NoExcept[*p] && q<e);
		            p = Call(that->Child,Check0,1(p));
		        }
		        if (q<e && *p) {
					EoLinePosRecordWriteAt(that->Position,q);
		            *q++ = that->Except.Handle(that);
	                NoExcept = that->Except.No;
		            if (that->Fwd.b!=that->Fwd.e) {
			            p = that->Fwd.b;
		            } else {
			            p = that->Child->p;
		            }
		        }
	    	} while (q<e && *p && (that->Fwd.b==that->Fwd.e));
		} while(q<e && that->Fwd.b!=that->Fwd.e && *p);
		*q = 0;
		this->p = that->b;
    } else {
		EoLinePosRecordReadAt(that->Position,p);
        this->p=p;
    }
	if (!*this->p) {
		Call(that->Position->Feedback,EndOfFile,0);
	}
    return this->p;
}

static long BTCPreprocSeek(BuffText *this,long dp) {
    long r;
    /* This is supposed to be strict FIFO, so we ignore this */
    ThisToThat(BTCPreproc,BuffText);
    r = Call(that->Child,Seek,1(0));
    return r;
}

static void BTCPreprocIIClose(BuffText *this) {
    ThisToThat(BTCPreproc,BuffText);
	Call(that->Child,Close,0);
	this->p = that->e;
	that->Fwd.b = that->Fwd.e = that->Fwd.bottom;
	*that->Fwd.e = '\n'; that->Fwd.e++;
}

BuffText *BTCPreproc1New(BuffText *Child,EoxFeedback *Feedback) {
    static struct BuffText Static = {BTCPreprocIICheck0,BTCPreprocSeek,BTCPreprocIIClose};
    BTCPreproc *R;
    rPush(R);
    rnPush(R->b,128);
    R->e = R->b+127;
    *R->e = 0;
    R->Child = Child;
    R->BuffText.Static = &Static;
    R->BuffText.p = R->e;
    rnPush(R->Fwd.bottom,16);
    R->Fwd.top = R->Fwd.bottom+16;
    R->Fwd.b = R->Fwd.e = R->Fwd.bottom;
    *R->Fwd.e = '\n'; R->Fwd.e++;
    R->IsBlank = BTCPreprocIIsBlank();
    R->Except.No = BTCPreprocIIExceptNo();
    R->Except.Handle = BTCPreprocIIGetChar;
	R->Position = EoLinePosRecordNew(R->b,R->e,Feedback);
    R->BuffText.p = Call(&R->BuffText,Check0,1(R->BuffText.p));
    return &R->BuffText;
}


/*___________________________________________________________________
 |
 |___________________________________________________________________
*/

BuffText *BTConstructorNullInstantiate(BTConstructor *this) {
    return BuffTextNull();
}

BTConstructor *BTConstructorNull(void) {
    static BTConstructorStatic Static = {BTConstructorNullInstantiate};
    static BTConstructor R = {&Static};
    return &R;
}

typedef struct {
    BTConstructor BTConstructor;
    char *FileName;
	EoxFeedbackConstr *Feedback;
} BTConstrCPreIIFile;

static BuffText *BTConstrCPreIIFileInstantiate(BTConstructor *this) {
    BuffText *R;
	EoxFeedback *Feedback;
    ThisToThat(BTConstrCPreIIFile,BTConstructor);
	Feedback = Call(that->Feedback,Instantiate,0);
    R = BTCPreproc1New(BTFileAutoCloseNew(that->FileName),Feedback);
    return R;
}

BTConstructor *BTConstrCPIIFileNew(char *Name,EoxFeedbackConstr *Feedback) {
    BTConstrCPreIIFile *R;
    static BTConstructorStatic Static = {BTConstrCPreIIFileInstantiate};
    rPush(R);
    R->BTConstructor.Static = &Static;
	R->FileName = StringDuplicate(Name);
	R->Feedback = Feedback;
    return &R->BTConstructor;
}

/*__________________________________________________________
 |
 |__________________________________________________________
*/

typedef struct {
    BuffText BuffText;
    BuffText *Src;
    char *EscapedChar;
    char *b,*e;
    int HalfChar;
    int LastChar;
} BTCPQuoteString;

static char *BTCPQuoteStringCheck0(BuffText *this,char *p) {
    ThisToThat(BTCPQuoteString,BuffText);
    if (p<that->b || p>=that->e) {
	char *q,*e;
	q = this->p = that->b;
	e = that->e;
	p = that->Src->p;
	do {
	    if (that->HalfChar) { 
		*q++ = that->EscapedChar[*p++]; 
		that->HalfChar=(0!=0);
	    }
            while (*p && *p == that->EscapedChar[*p] && q<e) {
		*q++ = *p++;
	    }
	    if (!*p) p = Call(that->Src,Check0,1(p));
            if (q<e && *p) {
		that->HalfChar = (*p!=that->EscapedChar[*p]);
		if (that->HalfChar) {
                    *q++ = '\\';
		}
	    }
	} while (*p && q<e);
	Call(that->Src,Check0,1(p));
	if (!*p && q<e && !that->LastChar) {
	    *q++ = '\"';
	    that->LastChar = (0==0);
        }
	*q = 0;
    } else {
	this->p = p;
    } 
    return this->p;
}

static void BTCQuoteClose(BuffText *this) {
	ThisToThat(BTCPQuoteString,BuffText);
	Call(that->Src,Close,0);
	this->p = that->e-1; *(this->p) = '\"';
	that->HalfChar = that->LastChar = (0!=0);
}

BuffText *BTCQuoteStringNew(BuffText *Src,char *EscapedChar) {
    BTCPQuoteString *R;
    static struct BuffText Static = {BTCPQuoteStringCheck0,BuffTextNullSeek,BTCQuoteClose};
    rPush(R);
    R->BuffText.Static = &Static;
    rnPush(R->b,32);
    R->e = R->b+31;
    *R->e = 0;
    R->e[-1] = '\"';
    R->BuffText.p = R->e-1;
    R->Src = Src;
    R->EscapedChar = EscapedChar;
    R->HalfChar = R->LastChar = (0!=0);
    return &R->BuffText;
}

/*__________________________________________________
 |
 |__________________________________________________
*/

typedef struct {
    CPMacro CPMacro;
    StringConstruct *s;
} CPMacroConstString;

BuffText *CPMacroConstStringInstantiate(CPMacro *this,CPMacroParam *m) {
    ThisToThat(CPMacroConstString,CPMacro);
    return BTStringConstructNew(that->s);
}

CPMacro *CPMacroConstStringNew(StringConstruct *s) {
    static CPMacroStatic Static = {CPMacroConstStringInstantiate};
    CPMacroConstString *R;
    rPush(R);
    R->CPMacro.Static = &Static;
    R->s = s;
    return &R->CPMacro;
}


typedef struct {
    CPMacro CPMacro;
    int n;
    char *EscapedChar;
} CPMacroQuoteVariable;

BuffText *CPMacroQuoteVariableInstantiate(CPMacro *this,CPMacroParam *Param) {
    BuffText *R;
    ThisToThat(CPMacroQuoteVariable,CPMacro);
    if (that->n>=Param->Arity) {
	R = BuffTextNull();
    } else {
	R = BTStringConstructNew(Param->Params[that->n]);
	R = BTCQuoteStringNew(R,that->EscapedChar);
    }
    return R;
}

CPMacro *CPMacroQuoteVariableNew(int n,char *EscapedChar) {
    CPMacroQuoteVariable *R;
    static CPMacroStatic Static = {CPMacroQuoteVariableInstantiate};
    rPush(R);
    R->CPMacro.Static = &Static;
    R->n = n;
    R->EscapedChar = EscapedChar;
    return &R->CPMacro;
}

typedef struct {
    CPMacro *Macro;
    List List;
} CPMacroList;

struct _CPMacroArg_ {
    CPMacro CPMacro;
    List Begin,*Last;
};

CPMacro *CPMacroArgGetMacro(CPMacroArg *this) {
    return &this->CPMacro;
}

BuffText *CPMacroArgInstantiate(CPMacro *this,CPMacroParam *Param) {
    List *l;
    BuffText *r;
    ThisToThat(CPMacroArg,CPMacro);
    l = that->Begin.n;
    if (l!=ListEnd) {
        CPMacroList *L;
	BuffText *B;
        BTList *R;
	R = BTListNew(0);
	do {
	    L = CastBack(CPMacroList,List,l);
	    B = Call(L->Macro,Instantiate,1(Param));
	    R = BTListCatBuffer(R,B);
	    l = l->n;
	} while (l!=ListEnd);
	r = &R->BuffText;
    } else {
	r = BuffTextNull();
    }
    return r;
}

CPMacroArg *CPMacroArgNew(CPMacro *M,...) {
    CPMacroArg *R;
    static CPMacroStatic Static = {CPMacroArgInstantiate};
    rPush(R);
    R->CPMacro.Static = &Static;
    R->Begin.n = ListEnd;
    R->Last = &R->Begin;
    {
	va_list c;
	List *l;
        va_start(c,M);
	l = R->Last;
	while (M) {
	    CPMacroList *L;
	    rPush(L);
            L->List.n = l->n;
	    L->Macro = M;
	    l->n = &L->List;
	    l = &L->List;
	    M = va_arg(c,CPMacro *);
	}
	R->Last = l;
	va_end(c);
    }
    return R;
}

CPMacroArg *CPMacroArgCat(CPMacroArg *this,CPMacro *M) {
    CPMacroList *L;
    rPush(L);
    L->List.n = this->Last->n;
    L->Macro = M;
    this->Last->n = &L->List;
    this->Last = &L->List;
    return this;
}



