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

#include <StackEnv.h>
#include <Classes.h>
#include <List.h>
#include <Patricia.h>
#include <C_Util.h>
#include <CPrepMacro.h>
#include <stdarg.h>
#include <Debug.h>

#define StackGrowth 2048

/*-------------------------*/

typedef struct {
    BuffText *Src;
    StringConstruct *SC;
    char *p,*q;
    char *IsBlank;
    char EolBlank; // is either \n eiher ' '
} CPTransfert;

typedef struct {
    CSynLexem CSynLexem;
    PatriciaNod PatriciaNod;
} CSynLexicId;

static char *CSynConstLexemInstance(CSynLexem *this) {
    ThisToThat(CSynLexicId,CSynLexem);
    return that->PatriciaNod.Key;
}

static CSynLexem *CSynConstLexemDuplicate(CSynLexem *this) {
    return this;
}

static CSynLexem *CSynVarLexemDuplicate(CSynLexem *this);

CSynLexicId *CSynLexicConstIdNew(PatriciaNod *C,char *Spelling,int Id) {
    static struct CSynLexem Static = {CSynConstLexemInstance,CSynConstLexemDuplicate};
    CSynLexicId *R;
    rPush(R);
    R->CSynLexem.Static = &Static;
    R->CSynLexem.Type = Id;
    PatriciaSeekOrInsert(C,Spelling,&R->PatriciaNod);
    return R;
}

/*________________________________________________
 |
 | Preprocessor browse
 |________________________________________________
*/

typedef struct _BTCPreprocParse_ BTCPreprocParse;
typedef struct _CPreprocException_ CPreprocException;

typedef CPreprocLexem *(*CPreprocFork)(CPreprocEnv *Evr);

typedef struct {
    char *IsBlank,*IsChar,*EscapedChar,*EscapedCharBack;
    char *Separator;
    PatriciaNod *Operators;
    PatriciaNod Keywords;
    CPreprocFork *Fork;
} CPreprocLexSetting;

struct _CPreprocEnv_ {
    BTStack *mSrc;
    MemStack *mMacro,*mIdentifier;
    CPreprocLexSetting *Lexic;
    /**/
    PatriciaNod Macros;
    PatriciaNod Symbols;
    char **Path;
    BuffText *Stream;
    /**/
    CPreprocLexem Last;
    BTCPreprocParse *Gate;
	struct {
	    FilePosStack *Files;
		CtPosition *Position;
	} Position;
};

BuffText *CPreprocEnvGetStream(CPreprocEnv *t) {
    return t->Stream;
}

typedef struct {
    BuffText *(*Manage)(CPreprocException *this,CPreprocEnv *Ev);
} CPreprocExceptionStatic;
struct _CPreprocException_ {
    CPreprocExceptionStatic *Static;
    PatriciaNod PatriciaNod;
};

/*________________________________________________________
 |
 |________________________________________________________
*/

char *CPGetIdentifier(BuffText *Src,char *IsChar) {
    char *r;
    char *p;
    pOpen {
	int Size;
        char *q,*e;
	StringConstruct *SC;
	pIn(SC = StringConstructNew());
	q = StringConstructNext(SC,0);
	p = Src->p;
        do {
	    q = StringConstructNext(SC,q); e = SC->e;
	    while (q<e && IsChar[*p]) {
		*q++ = *p++;
	    }
	    if (!IsChar[*p]) p = Call(Src,Check0,1(p));
        } while(IsChar[*p]);
	q = StringConstructNext(SC,q); e = SC->e;
        r = StringConstructGetString(SC);
    } pClose
    Call(Src,Check0,1(p));
    return r;
}

char *CPreprocSkipBlank(BuffText *Src,char *IsBlank) {
    char *p;
    p = Src->p;
    do {
        while (IsBlank[*p]) p++;
	if (!*p) p = Call(Src,Check0,1(p));
    } while(IsBlank[*p]);
    p = Call(Src,Check0,1(p));
    return p;
}

char *CPreprocSkipBlankOrEol(BuffText *Src,char *IsBlank) {
    char *p;
    p = Src->p;
    do {
	    do {
	        while (*p=='\n') { p++; }
	        p = Call(Src,Check0,1(p));
	    } while(*p=='\n');
	    p = CPreprocSkipBlank(Src,IsBlank);
    } while(*p=='\n');
    return p;
}

char *CPreprocSkipEndLine(BuffText *Src) {
    char *p;
    p = Src->p;
    do {
	    while (*p && *p!='\n') { p++; }
	    p = Call(Src,Check0,1(p));
    } while(*p && *p!='\n');
    return p;
}

/*________________________________________________________
 |
 |________________________________________________________
*/

CPreprocLexem *CPreprocGetEof(CPreprocEnv *Evr) {
    Evr->Last.Type = CLTEndFile;
    Evr->Last.Token.Number = 0;
    return &Evr->Last;
}

CPreprocLexem *CPreprocGetIdentifier(CPreprocEnv *Evr) {
    Evr->Last.Type = CLTIdentifier;
    Evr->Last.Token.Identifier = CPGetIdentifier(Evr->Stream,Evr->Lexic->IsChar);
    return &Evr->Last;
}

CPreprocLexem *CPreprocGetSeparator(CPreprocEnv *Evr) {
    char *p;
    p = Evr->Stream->p;
    Evr->Last.Token.Separator = Evr->Lexic->Separator+2*(*p);
    p = Call(Evr->Stream,Check0,1(p+1));
    Evr->Last.Type = CLTSeparator;
    return &Evr->Last;
}

CPreprocLexem *CPreprocGetOperator(CPreprocEnv *Evr) {
    char Operator[8],*p,*e;
    PatriciaNod *S,*R,*Q;
    R = Evr->Lexic->Operators;
    p = Evr->Stream->p;
    e = Operator;
    S = 0;
    do {
	Q = S;
	if (!*p) p = Call(Evr->Stream,Check0,1(p));
        *e++ = *p++; *e = 0; S = PatriciaSeek(R,Operator);
    } while (S!=R && e[-1]);
    Call(Evr->Stream,Check0,1(p-1));
    if (Q) {
        Evr->Last.Token.Separator = Q->Key;
        Evr->Last.Type = CLTSeparator;
    } else {
	    Evr->Last.Type = CLTBlank;
    }
    return &Evr->Last;
}

char CPreprocGetCharEscaped(BuffText *B,char *EscapedChar) {
    char c,*p;
    p = Call(B,Check0,1(B->p+1));
    c = EscapedChar[*p++];
    if (!*p) p = Call(B,Check0,1(p));
    if (c=='x' || c=='X') {
	    int i;
	    i=2; c=0; p++; if (!*p) p = Call(B,Check0,1(p));
	    while ((i>0)&&((*p>='0'&&*p<='9')||(*p>='a'&&*p<='f')||(*p>='A'&&*p<='F'))) {
	        i--;
	        if (*p>='0'&&*p<='9') c = (c<<4)+(*p-'0');
	        if (*p>='A'&&*p<='F') c = (c<<4)+(*p-'A');
	        if (*p>='a'&&*p<='f') c = (c<<4)+(*p-'a');
	        p++; if (!*p) p = Call(B,Check0,1(p));
	    }
    } else {
        if (c>='0' && c<='7') {
	        int i;
	        i=3; c = 0;
	        while ((i>0)&&(*p>='0')&&(*p<='7')) {
		        i--;
		        c = c<<3+(*p-'0');
		        p++; if (!*p) p = Call(B,Check0,1(p));
	        }
	    }
    }
    Call(B,Check0,1(p));
    return c;
}

CPreprocLexem *CPreprocGetChar(CPreprocEnv *Evr) {
    char *p;
    int c;
    p = Call(Evr->Stream,Check0,1(Evr->Stream->p+1));
    if (*p=='\\') {
	c = CPreprocGetCharEscaped(Evr->Stream,Evr->Lexic->EscapedChar);
	p = Evr->Stream->p;
    } else {
	c = *p;
    }
    do {
	while(*p && *p!='\'') p++;
	if (!*p) p = Call(Evr->Stream,Check0,1(p));
    } while (*p && *p!='\'');
    if (*p) p++;
    Call(Evr->Stream,Check0,1(p));
    if (c<0) c+=128;
    Evr->Last.Token.Separator = Evr->Lexic->Separator+2*c;
    Evr->Last.Type = CLTChar;
    return &Evr->Last;
}

StringConstruct *SCGetExpFloat(StringConstruct *SC,BuffText *Src) {
    char *p,*q;
    p = Src->p;
    q = StringConstructNext(SC,0);
    if (*p=='e'||*p=='E') {
	*q++ ='E'; p++; 
	if (q>=SC->e) q=StringConstructNext(SC,q);
	if (!*p) p = Call(Src,Check0,1(p));
	if (*p=='+' || *p=='-') { 
            *q++ = *p++; 
	    if (q>=SC->e) q=StringConstructNext(SC,q);
	    if (!*p) p = Call(Src,Check0,1(p));
	}
	do {
            while (*p>='0' && *p<='9' && q<SC->e) {
	        *q++ = *p++;
	    }
	    if (q>=SC->e) q = StringConstructNext(SC,q);
	    if (!*p) p = Call(Src,Check0,1(p));
	} while (*p>='0' && *p<='9');
    }
    if (*p=='l'||*p=='L'||*p=='f'||*p=='F') {
	 *q++ = (*p=='l' || *p=='L') ? 'L':'F';
	 p++;
    }
    Call(Src,Check0,1(p));
    StringConstructNext(SC,q);
    return SC;
}

StringConstruct *SCGetFractFloat(StringConstruct *SC,BuffText *Src) {
    char *p,*q;
    p = Src->p; q=StringConstructNext(SC,0);
    do {
	while (*p>='0' && *p<='9' && q<SC->e) {
	    *q++ = *p++; 
	}
        p = Call(Src,Check0,1(p));
	q = StringConstructNext(SC,q);
    } while (*p>='0' && *p<='9');
    return SCGetExpFloat(SC,Src);
}

CPreprocLexem *CPreprocGetPointLexem(CPreprocEnv *Evr) {
    char *p;
    p = Call(Evr->Stream,Check0,1(Evr->Stream->p+1));
    if (*p>='0' && *p<='9') {
	StringConstruct *S;
	char *q;
	pOpen
	pIn(S = StringConstructNew());
        q = StringConstructNext(S,0);
	*q++='0'; *q++='.';
	q = StringConstructNext(S,q);
        Evr->Last.Type = CLTFloat;
        Evr->Last.Token.Float = StringConstructGetString(SCGetFractFloat(S,Evr->Stream));
	pClose
    } else {
	if (*p=='.') {
            static char etc[] = "...";
            p = Call(Evr->Stream,Check0,1(p+1));
	    if (*p=='.') p = Call(Evr->Stream,Check0,1(p+1));
	    Evr->Last.Type = CLTSeparator;
	    Evr->Last.Token.Separator = etc;
	} else {
	    Evr->Last.Type = CLTSeparator;
            Evr->Last.Token.Separator = Evr->Lexic->Separator+2*('.');
	}
    }
    return &Evr->Last;
}

int SCGetNumber(StringConstruct *SC,BuffText *Src) {
    char *p,*q;
    int IsFloat;
    p = Src->p; 
    q = StringConstructNext(SC,0);
    IsFloat = (*p!='0');
    if (!IsFloat) {
	*q++ = *p++;
	if (!*p) p = Call(Src,Check0,1(p));
	if (q>=SC->e) q = StringConstructNext(SC,q);
	if (*p=='x'||*p=='X') {
	    *q++ = *p++;
	    do { 
		while (q<SC->e && ((*p>='0' && *p<='9')||(*p>='a'&&*p<='f')||(*p>='A'&&*p<='F'))) {
		    *q++=*p++;
		}
		if (!*p) p = Call(Src,Check0,1(p));
	        if (q>=SC->e) q = StringConstructNext(SC,q);
	    } while ((*p>='0' && *p<='9')||(*p>='a'&&*p<='f')||(*p>='A'&&*p<='F'));
	} else {
	    do {
		while (q<SC->e && (*p>='0'&&*p<='7')) {
		    *q++ = *p++;
		}
		if (!*p) p = Call(Src,Check0,1(p));
	        if (q>=SC->e) q = StringConstructNext(SC,q);
	    } while (*p>='0'&&*p<='7');
	}
	p = Call(Src,Check0,1(p));
	q = StringConstructNext(SC,q);
    } else {
	do {
	    while (q<SC->e && (*p>='0'&&*p<='9')) { 
		*q++ = *p++;
	    }
	    if (!*p) p = Call(Src,Check0,1(p));
	    if (q>=SC->e) q = StringConstructNext(SC,q);
	} while (*p>='0'&&*p<='9');
	p = Call(Src,Check0,1(p));
	q = StringConstructNext(SC,q);
	IsFloat = (*p=='.' || *p=='e' || *p=='E');
    }
    if (IsFloat) {
	if (*p=='.') {
	    *q++ = *p++;
	    p = Call(Src,Check0,1(p));
	    q = StringConstructNext(SC,q);
	    SCGetFractFloat(SC,Src);
	} else {
	    SCGetExpFloat(SC,Src);
	}
    } else {
	if (*p=='u'||*p=='U'||*p=='l'||*p=='L') {
	    int u,l;
	    u = (*p=='u' || *p=='U');
	    *q++ = u ? 'U':'L'; p++;
	    if (!*p) p = Call(Src,Check0,1(p));
	    if (q>=SC->e) q = StringConstructNext(SC,q);
	    l = 0;
	    if (u && (*p=='L'||*p=='l')) l = 'L';
	    if (!u && (*p=='U'||*p=='u')) l = 'U';
	    if (l) {
		*q++ = l; p++;
	    }
	    p = Call(Src,Check0,1(p));
	    q = StringConstructNext(SC,q);
	} 
    }
    return IsFloat;
}

CPreprocLexem *CPreprocGetNumber(CPreprocEnv *Evr) {
    StringConstruct *SC;
    int IsFloat;
    pOpen
    pIn( SC=StringConstructNew());
    IsFloat = SCGetNumber(SC,Evr->Stream);
    Evr->Last.Type = IsFloat? CLTFloat:CLTNumber;
    Evr->Last.Token.Number = StringConstructGetString(SC);
    pClose
    return &Evr->Last;
}

CPreprocLexem *CPreprocLastLexem(CPreprocEnv *Ev);

CPreprocLexem *CPreprocGetString(CPreprocEnv *Evr) {
    char *p;
    p = Evr->Stream->p;
    p++; // Skip first "
    {
	char *q;
	int End;
	StringConstruct *SC;
        SC = StringConstructNew();
	End = (0!=0);
	q = StringConstructNext(SC,0);
	do {
	    char *e;
	    q = StringConstructNext(SC,q); e = SC->e;
	    while (q<e && *p && *p!='\\' && *p!='\"' && *p!='\n') {
		 *q++ = *p++;
	    }
	    p = Call(Evr->Stream,Check0,1(p));
	    End = (*p=='\0');
	    if (q<e) {
		if (*p=='\"') {
		    p++;
		    p = Call(Evr->Stream,Check0,1(p));
		    p = CPreprocSkipBlank(Evr->Stream,Evr->Lexic->IsBlank);
		    End = ((*p)!='\"');
		    if (!End) p++;
		} else {
	            if (*p=='\\') {
			*q++ = CPreprocGetCharEscaped(Evr->Stream,Evr->Lexic->EscapedChar);
			p = Evr->Stream->p;
		    } else {
		       while (*p=='\n') p++;
		    }
		}
	    }
        } while (!End);
	q = StringConstructNext(SC,q);
	Evr->Last.Type = CLTString;
	Evr->Last.Token.String = SC;
    }
    p = Call(Evr->Stream,Check0,1(p));
    return &Evr->Last;
}

/*__________________________________________________
 |
 |__________________________________________________
*/

char *CPreprocSkipMacroString(BuffText *Src) {
    char *p,EndChar;
    p = Src->p;
    EndChar = *p++; 
    do {
	while (*p!=EndChar && *p && *p!='\\') {
	    p++;
	} 
	if (*p=='\\') {
	    p++; p = Call(Src,Check0,1(p));
	    if (*p) p++;
	}
	p = Call(Src,Check0,1(p));
    } while (*p!=EndChar && *p);
    if (*p==EndChar) {
       p++; 
    } 
    p = Call(Src,Check0,1(p));
    return p;
}

char CPGetnChar(CPTransfert *T,int n) {
    int r;
    char *p,*e,*q;
    q = T->q; p = T->p; e = T->SC->e;
    do {
	while (n>0 && *p && q<e) {
            r = *q++ = *p++; n--;
	}
	if ((!*p)||(n<=0)) T->p = p = Call(T->Src,Check0,1(p));
	if ((q>=e)||(n<=0)||(!*p)) {
	    T->q = q = StringConstructNext(T->SC,q);
	    e = T->SC->e;
	}
    } while (n>0 && *p);
    return r;
}

char CPSkipBlank(CPTransfert *T) {
    char *p,*IsBlank;
    p = T->p;
    IsBlank = T->IsBlank;
    do {
	while(IsBlank[*p]||*p==T->EolBlank) p++;
	p = T->p = Call(T->Src,Check0,1(p));
    } while (IsBlank[*p]||*p==T->EolBlank);
    return *p;
}

char CPGetBlank(CPTransfert *T) {
    char *p,*q,*e,*IsBlank;
    p = T->p; q = T->q; e = T->SC->e;
    IsBlank = T->IsBlank;
    do {
	while(q<e && (IsBlank[*p]||*p==T->EolBlank)) *q++ = *p++;
	q = T->q = StringConstructNext(T->SC,q);
	e = T->SC->e;
	p = T->p = Call(T->Src,Check0,1(p));
    } while (IsBlank[*p]||*p==T->EolBlank);
    return *p;
}

char CPGetMacroString(CPTransfert *T) {
    char *p,*e,*q;
    char End;
    p = T->p; q = T->q; e = T->SC->e;
    End = *p;
    *q++ = *p++;
    do {
	while (*p && *p!=End && *p!='\\' && q<e) {
	    *q++ = *p++;
	} 
        p = T->p = Call(T->Src,Check0,1(p));
        q = T->q = StringConstructNext(T->SC,q); 
	e = T->SC->e; 
	if (*p=='\\') {
	    CPGetnChar(T,2);
	    p=T->p; q=T->q; e=T->SC->e;
	}
    } while (*p!=End && *p);
    if (*p) *p++;
    T->p = Call(T->Src,Check0,1(p));
    *q++=End;
    T->q = StringConstructNext(T->SC,q);
    return *p;
}

char CPGetMacroParenthesis(CPTransfert *T,char End0, char End1) {
    char R,*p,*q,*e;
    char *IsBlank,*LastBlank;
    IsBlank = T->IsBlank;
    CPSkipBlank(T);
    do {
        p = T->p; q = T->q; e = T->SC->e;
	LastBlank = 0;
	do {
	    while (q<e && *p!=End0 && *p!=End1 && *p && *p!='(' && *p!='{' && *p!='[' && *p!='\"' && *p!='\'' && *p!='\n'
	      && *p!='\\' && !IsBlank[*p]) {
	        *q++ = *p++;
	    }
	    q = T->q = StringConstructNext(T->SC,q); e = T->SC->e;
	    p = T->p = Call(T->Src,Check0,1(p));
	    if (IsBlank[*p]||*p==T->EolBlank) {
		LastBlank = q;
                CPGetBlank(T);
		p = T->p; q = T->q; e = T->SC->e;
	    }
	    R = *p;
	} while(R!=End0 && R!=End1 && R && R!='\n' && R!='\"' && R!='\'' && R!='(' && R!='[' && R!='{' && R!='\\');
	switch (R) {
	    case '(': CPGetnChar(T,1); if (CPGetMacroParenthesis(T,')',')')==')') CPGetnChar(T,1); break;
	    case '[': CPGetnChar(T,1); if (CPGetMacroParenthesis(T,']',']')==']') CPGetnChar(T,1); break;
	    case '{': CPGetnChar(T,1); if (CPGetMacroParenthesis(T,'}','}')=='}') CPGetnChar(T,1); break;
	    case '\"': CPGetMacroString(T); break;
            case '\'': CPGetMacroString(T); break;
	    case '\\': CPGetnChar(T,2); break;
	}
    } while(R!=End0 && R!=End1 && R && R!='\n');
    if (LastBlank) T->q = StringConstructLast(T->SC,LastBlank);
    T->p = Call(T->Src,Check0,1(T->p)); 
    T->q = StringConstructNext(T->SC,T->q);
    return R;
}

typedef struct {
    List List;
    StringConstruct *Param;
} CLMacroParam;

List *CPAddMacroParams(List *L,StringConstruct *SC) {
    CLMacroParam *N;
    rPush(N);
    N->List.n = ListEnd;
    N->Param = SC;
    L->n = &N->List;
    return &N->List;
}

List *CPListMacroParams(BuffText *Src,char *IsBlank,char EolIsBlank) {
    List *R,*L;
    int End;
    CPTransfert Got;
    rPush(R);
    R->n = ListEnd;
    L = R;
    Got.Src = Src;
    Got.IsBlank = IsBlank;
    Got.EolBlank = EolIsBlank;
    do {
	    Got.SC = StringConstructNew();
	    Got.q = StringConstructNext(Got.SC,0);
        Got.p = Call(Src,Check0,1(Src->p+1));
	    End = CPGetMacroParenthesis(&Got,',', ')');
	    L = CPAddMacroParams(L,Got.SC);
    } while(End!=')' && End!=0);
    if (End) Call(Src,Check0,1(Src->p+1));
    return R;
}

int ListCount(List *L) {
   int R;
   R = 0;
   while (L->n!=ListEnd) {R++; L=L->n;}
   return R;
}

CPMacroParam *CPreprocMacroParamSumary(List *Params) {
    CPMacroParam *R;
    StringConstruct **p;
    List *l;
    rPush(R);
    R->Arity = ListCount(Params);
    rnPush(R->Params,R->Arity+1);
    p = R->Params;
    l = Params->n;
    while (l!=ListEnd) {
        CLMacroParam *L;
        L = CastBack(CLMacroParam,List,l);
        l = l->n;
        *p++ = L->Param;
    }
    *p = 0;
    return R;
}

CPMacroParam *CPreprocGetMacrosParams(BuffText *Src,char *IsBlank) {
    /* Used at call time */
    CPreprocSkipBlankOrEol(Src,IsBlank);
    /* The next character Should be '(' though we do not test this */
    return CPreprocMacroParamSumary(CPListMacroParams(Src,IsBlank,'\n'));
}

/*__________________________________________________
 |
 |__________________________________________________
*/

typedef struct {
    BTConstructor BTConstructor;
    CPMacro *Macro;
    CPMacroParam *P;
} BTConstructorMacro;

BuffText *BTConstructorMacroInstantiate(BTConstructor *this) {
    BuffText *R;
    ThisToThat(BTConstructorMacro,BTConstructor);
    R = Call(that->Macro,Instantiate,1(that->P));
    return R;
}

BTConstructor *BTConstructorMacroNew(CPMacro *Macro,CPMacroParam *P) {
    BTConstructorMacro *R;
    static BTConstructorStatic Static = {BTConstructorMacroInstantiate};
    rPush(R);
    R->BTConstructor.Static = &Static;
    R->Macro = Macro;
    R->P = P;
    return &R->BTConstructor;
}

/*_____________________________________________________________
 |
 |_____________________________________________________________
*/

typedef struct _CPMacroDefinition_ CPMacroDefinition;
typedef struct {
    CPreprocExceptionStatic CPreprocException;
    CPMacroParam *(*GetParams)(CPMacroDefinition *this,BuffText *Src);
} CPMacroDefinitionStatic;

struct _CPMacroDefinition_ {
    CPreprocException CPreprocException;
    CPMacroDefinitionStatic *Static;
    CPMacro *Macro;
};

char *StringConstructInsertBuffText(StringConstruct *SC,BuffText *B) {
    char *p,*q;
    p = B->p; 
    q = StringConstructNext(SC,0);
    while (*p) {
	    char *e;
	    e = SC->e;
        do { *q++ = *p++; } while (*p && q<e);
	    if (!*p) p = Call(B,Check0,1(p));
	    if ((q>=e)||(!*p)) q = StringConstructNext(SC,q);
    }
    return q;
}

char *StringConstructInsertString(StringConstruct *SC,char *p) {
    char *q;
    q = StringConstructNext(SC,0);
    while (*p) {
	    char *e;
	    e = SC->e;
	    do {*q++ = *p++;} while (*p && q<e);
	    if ((q>=e)||(!*p)) q = StringConstructNext(SC,q);
    }
    return q;
}

BuffText *BTExpandedMacroParamNew(CPreprocEnv *Ev,BuffText *Src) {
    CPTransfert T;
    char *IsChar;
    IsChar = Ev->Lexic->IsChar;
    T.IsBlank = Ev->Lexic->IsBlank;
    T.EolBlank = '\n';
    T.Src = Src;
    T.p = Src->p;
    T.SC = StringConstructNew();
    T.q = StringConstructNext(T.SC,0);
    do {
	if (!*T.p) T.p = Call(T.Src,Check0,1(T.p));
	while (!IsChar[*T.p] && *T.p && *T.p!='\'' && *T.p!='\"') {
	    char *e;
	    e = T.SC->e;
	    do {
		    *T.q++ = *T.p++;
	    } while(T.q<e && *T.p && !IsChar[*T.p] && *T.p!='\'' && *T.p!='\"');
	    T.p = Call(T.Src,Check0,1(T.p));
	    T.q = StringConstructNext(T.SC,T.q);
	}
	if (IsChar[*T.p]) {
        char *Identifier;
	    PatriciaNod *f;
	    pOpen
	    pIn(Identifier = CPGetIdentifier(T.Src,IsChar));
	    T.p = T.Src->p;
        f = PatriciaSeek(&Ev->Macros,Identifier);
	    if (f != &Ev->Macros) {
		    CPMacroDefinition *F;
		    CPMacroParam *Param;
		    BuffText *Inst;
		    F = CastBack(CPMacroDefinition,CPreprocException.PatriciaNod,f);
		    EnvSwap()
		    Param = Call(F,GetParams,1(T.Src)); T.p = T.Src->p;
		    Inst = Call(F->Macro,Instantiate,1(Param));
		    T.q = StringConstructInsertBuffText(T.SC,Inst);
		    EnvSwap()
	    } else {
            T.q = StringConstructInsertString(T.SC,Identifier);
	    }
	    pClose
	} else {
	    if (*T.p) CPGetMacroString(&T);
	}
    } while(*T.p);
    return BTStringConstructNew(T.SC);
}

typedef struct _CPMacroVariable_ CPMacroVariable;

struct _CPMacroVariable_ {
    CPMacro CPMacro;
    CPreprocEnv *Ev;
    int n,Expanded;
};

BuffText *CPMacroDirectVariableInstantiate(CPMacro *this,CPMacroParam *Param) {
    BuffText *R;
    ThisToThat(CPMacroVariable,CPMacro);
    if (that->n>=Param->Arity) {
	    R = BuffTextNull();
    } else {
	/* First phase should parse the param to expand contained macro prior
	  to doing the insertion */
	    if (that->Expanded) {
	        R = BTExpandedMacroParamNew(that->Ev,BTStringConstructNew(Param->Params[that->n]));
	    } else {
	        R = BTStringConstructNew(Param->Params[that->n]);
	    }
    }
    return R;
}


CPMacroVariable *CPMacroDirectVariableNew(int n,CPreprocEnv *Ev,int Expanded) {
    CPMacroVariable *R;
    static CPMacroStatic Static = {CPMacroDirectVariableInstantiate};
    rPush(R);
    R->CPMacro.Static = &Static;
    R->n = n; R->Ev = Ev;
    R->Expanded = Expanded;
    return R;
}

CPMacro *CPMacroVariable2Macro(CPMacroVariable *this) {return &this->CPMacro;}

void CPMacroVariableSetExpansion(CPMacroVariable *this,int Expansion) { this->Expanded = Expansion; }

/*_____________________________________________________________
 |
 |_____________________________________________________________
*/

typedef struct {
    PatriciaNod PatriciaNod;
    int n;
} CPMacroParamNumber;

int CPRecordParamsId(PatriciaNod *R,List *P) {
    int n;
    n=0;
    while (P->n!=ListEnd) {
	    CPMacroParamNumber *N;
	    CLMacroParam *MC;
	    char *I;
	    P=P->n;
	    MC = CastBack(CLMacroParam,List,P);
	    I = StringConstructGetString(MC->Param);
	    rPush(N);
	    N->n = n++;
	    PatriciaSeekOrInsert(R,I,&N->PatriciaNod);
    }
    return n;
}

CPMacro *CPreprocGetMacroParamBody(CPreprocEnv *Ev) {
    CPMacroArg *R;
    char *IsChar;
    R = CPMacroArgNew(0);
    IsChar = Ev->Lexic->IsChar;
    pOpen {
        CPTransfert T;
	    PatriciaNod ParamIdentifier;
	    int Arity;
	    PatriciaInit(&ParamIdentifier);
	    T.IsBlank = Ev->Lexic->IsBlank;
	    T.EolBlank = ' ';
        T.Src = Ev->Stream;
	    T.SC = StringConstructNew();
	    /* Record Identifier */ {
	        List *Params;
            rOpen
	        Params = CPListMacroParams(Ev->Stream,Ev->Lexic->IsBlank,T.EolBlank);
	        pIn(Arity = CPRecordParamsId(&ParamIdentifier,Params));
	        rClose
        }
	/* Get Macro body */ {
	    char *IsChar;
	    CPMacroVariable *LastVar;
	    IsChar = Ev->Lexic->IsChar;
	    T.q = StringConstructNext(T.SC,0);
	    T.p = T.Src->p;
            CPSkipBlank(&T);
	    LastVar = 0;
	    while (*T.p && *T.p!='\n') {
	        CPMacro *Elt;
		    char *LastBlank,*LastString;
		    int NextExpansion;
		    LastString = 0;
		    LastBlank = 0;
		    NextExpansion = (0==0);
		    while ((T.IsBlank[*T.p])||(*T.p=='#')) {
		        LastBlank = T.q;
		        CPGetBlank(&T);
		        NextExpansion = (0==0);
		        if (*T.p=='#') {
			        LastString = T.q;
			        T.p = Call(T.Src,Check0,1(T.p+1));
			        if (*T.p=='#') {
			            T.p = Call(T.Src,Check0,1(T.p+1));
			            CPSkipBlank(&T);
			            T.q = StringConstructLast(T.SC,LastBlank);
			            LastString = 0;
			            NextExpansion = (0!=0);
			            if (LastVar) CPMacroVariableSetExpansion(LastVar,NextExpansion);
		        	} else {
			            if (T.q>=T.SC->e) T.q = StringConstructNext(T.SC,T.q);
			            *T.q++ = '#';
			            if (T.q>=T.SC->e) T.q = StringConstructNext(T.SC,T.q);
			            LastBlank = 0;
			        }
		        }
		        LastVar = 0;
		    }
		    if (IsChar[*T.p]) {
		        char *Id;
		        PatriciaNod *f;
		        pOpen
		        pIn(Id = CPGetIdentifier(T.Src,Ev->Lexic->IsChar));
		        T.p = T.Src->p;
                f = PatriciaSeek(&ParamIdentifier,Id);
		        if (f==&ParamIdentifier) {
                    char *p;
			        p = Id;
			        while (*p) {
			            char *e;
			            e = T.SC->e;
			            do { *T.q++ = *p++; } while (*p && T.q<e);
                        T.q = StringConstructNext(T.SC,T.q);
			        }
		         } else { 
			        CPMacroParamNumber *F;
			        CPMacro *M,*P;
			        F = CastBack(CPMacroParamNumber,PatriciaNod,f);
                    if (LastString) {
			            T.q = StringConstructLast(T.SC,LastString);
                        *T.q = 0;
			            P = CPMacroQuoteVariableNew(F->n,Ev->Lexic->EscapedCharBack);
			        } else {
			            // NextExpansion = (0!=0);
			            LastVar = CPMacroDirectVariableNew(F->n,Ev,NextExpansion);
			            P = CPMacroVariable2Macro(LastVar);
			        }
                    M = CPMacroConstStringNew(T.SC);
			        T.SC = StringConstructNew();
			        T.q = StringConstructNext(T.SC,0);
                    R = CPMacroArgCat(R,M);
			        R = CPMacroArgCat(R,P);
		        }
		        pClose
		    } else {
		        while (!T.IsBlank[*T.p] && !IsChar[*T.p] && *T.p && *T.p!='\n' && *T.p!='#') {
			        char *e;
		            while (*T.p == '\'' || *T.p == '\"') {
			        CPGetMacroString(&T);
			    }
			    e = T.SC->e;
			    while(!T.IsBlank[*T.p] && !IsChar[*T.p] && *T.p!='#' && *T.p && *T.p!='\n' 
                            && *T.p!='\'' && *T.p!='\"' && T.q<e) {
			        *T.q++ = *T.p++;
			    }
			    T.p = Call(T.Src,Check0,1(T.p));
			    T.q = StringConstructNext(T.SC,T.q);
		        }
		    }
	    } 
	}
	R = CPMacroArgCat(R,CPMacroConstStringNew(T.SC));
        // DbgMacroArgCheck(CPMacroArgGetMacro(R),Arity);
    } pClose;
    return CPMacroArgGetMacro(R);
}

CPreprocEnv *CPreprocEnvInsertText(CPreprocEnv *Ev,BTConstructor *T);

typedef struct {
    CPMacroDefinition CPMacroDefinition;
    BTConstructor BTConstructor;
    CPreprocEnv *Ev;
} CPMacroParamBody;

CPMacroParam *CPMacroParamBodyGetParams(CPMacroDefinition *this,BuffText *Src) {
     ThisToThat(CPMacroParamBody,CPMacroDefinition);
     return CPreprocGetMacrosParams(Src,that->Ev->Lexic->IsBlank);
}

BuffText *CPMacroParamBodyInstantiate(BTConstructor *this) {
    CPMacroParam *P;
    BuffText *R;
    ThisToThat(CPMacroParamBody,BTConstructor);
    P = Call(&that->CPMacroDefinition,GetParams,1(that->Ev->Stream)); 
    R = Call(that->CPMacroDefinition.Macro,Instantiate,1(P));
    return R;
}

BuffText *CPMacroParamBodyManage(CPreprocException *this,CPreprocEnv *Ev){
    ThisToThat(CPMacroParamBody,CPMacroDefinition.CPreprocException);
    CPreprocEnvInsertText(Ev,&that->BTConstructor);
    return Ev->Stream;
}

CPreprocException *CPMacroParamBodyDefNew(CPreprocEnv *Ev) {
    CPMacroParamBody *R;
    static CPMacroDefinitionStatic Static = {{CPMacroParamBodyManage},CPMacroParamBodyGetParams};
    static BTConstructorStatic CnstrStatic = {CPMacroParamBodyInstantiate};
    rPush(R);
    R->CPMacroDefinition.CPreprocException.Static = &Static.CPreprocException;
    R->CPMacroDefinition.Static = &Static;
    PatriciaInit(&R->CPMacroDefinition.CPreprocException.PatriciaNod);
    R->CPMacroDefinition.Macro = CPreprocGetMacroParamBody(Ev);
    R->BTConstructor.Static = &CnstrStatic;
    R->Ev = Ev;
    return &R->CPMacroDefinition.CPreprocException;
}

/*------------------------*/

CPMacro *CPrepGetMacroBody(CPreprocEnv *Ev) {
    /* Skip blank then get all till end of line */
    CPMacro *R;
    StringConstruct *SC;
    char *p,*q,*e,*IsBlank;
    IsBlank = Ev->Lexic->IsBlank;
    SC = StringConstructNew();
    q = StringConstructNext(SC,0); e = SC->e;
    p = CPreprocSkipBlank(Ev->Stream,Ev->Lexic->IsBlank);
    while (*p && *p!='\n') {
	    do {
	        *q++ = *p++;
	    } while (*p && *p!='\n' && q<e);
	    q = StringConstructNext(SC,q); e = SC->e;
	    p = Call(Ev->Stream,Check0,1(p));
    }
    R = CPMacroConstStringNew(SC);
    return R;
}

CPMacroParam *CPMacroBodyGetParams(CPMacroDefinition *this,BuffText *Src) {
    static StringConstruct *SC[]={0};
    static CPMacroParam R = {0,SC};
    return &R;
}

CPreprocException *CPMacroBodyDefNew(CPreprocEnv *Ev) {
    CPMacroParamBody *R;
    static CPMacroDefinitionStatic Static = {{CPMacroParamBodyManage},CPMacroBodyGetParams};
    static BTConstructorStatic CnstrStatic = {CPMacroParamBodyInstantiate} ;
    rPush(R);
    R->CPMacroDefinition.CPreprocException.Static = &Static.CPreprocException;
    R->CPMacroDefinition.Static = &Static;
    R->CPMacroDefinition.Macro = CPrepGetMacroBody(Ev);
    R->BTConstructor.Static = &CnstrStatic;
    R->Ev = Ev;
    PatriciaInit(&R->CPMacroDefinition.CPreprocException.PatriciaNod);
    return &R->CPMacroDefinition.CPreprocException;
}

/*___________________________________________________________________
 |
 |___________________________________________________________________
*/

BuffText *CPExceptMacroDefineManage(CPreprocException *this,CPreprocEnv *Ev) {
    char *p;
    p = Ev->Stream->p;
    p  = CPreprocSkipBlank(Ev->Stream,Ev->Lexic->IsBlank);
    if (*p && *p!='\n') {
	char *Identifier;
	CPreprocException *R;
	mIn(Ev->mMacro,CPreprocGetIdentifier(Ev));
	Identifier = Ev->Last.Token.Identifier;
	p = Ev->Stream->p;
	if (*p=='(') {
        mIn(Ev->mMacro,R = CPMacroParamBodyDefNew(Ev));
	} else {
	    mIn(Ev->mMacro,R = CPMacroBodyDefNew(Ev));
	}
	PatriciaSeekOrInsert(&Ev->Macros,Identifier,&R->PatriciaNod);
    }
    return Ev->Stream;
}

CPreprocException *CPExceptMacroDefineNew(void) {
    CPreprocException *R;
    static CPreprocExceptionStatic Static = {CPExceptMacroDefineManage};
    rPush(R);
    PatriciaInit(&R->PatriciaNod);
    R->Static = &Static;
    return R;
}

/*___________________________________________________________________
 |
 |___________________________________________________________________
*/

BuffText *CPExceptMacroUndefManage(CPreprocException *this,CPreprocEnv *Ev) {
    char *p;
    CPreprocSkipBlank(Ev->Stream,Ev->Lexic->IsBlank);
    p = Ev->Stream->p;
    if (Ev->Lexic->IsChar[*p]) {
	    rOpen
	    CPreprocGetIdentifier(Ev);
	    PatriciaRemove(&Ev->Macros,Ev->Last.Token.Identifier);
	    rClose
    }
    return Ev->Stream;
}

CPreprocException *CPExceptMacroUndefNew(void) {
    CPreprocException *R;
    static CPreprocExceptionStatic Static = {CPExceptMacroUndefManage};
    rPush(R);
    R->Static = &Static;
    return R;
}

/*___________________________________________________________________
 |
 |___________________________________________________________________
*/

BuffText *CPExceptSkipLineManage(CPreprocException *this,CPreprocEnv *Ev) {
    CPreprocSkipEndLine(Ev->Stream);
    return Ev->Stream;
}


CPreprocException *CPExceptSkipLineNew(void) {
    CPreprocException *R;
    static CPreprocExceptionStatic Static = {CPExceptSkipLineManage};
    rPush(R);
    R->Static = &Static;
    return R;
}

/*___________________________________________________________________
 |
 |___________________________________________________________________
*/

typedef enum {
    CPEE_Endif,
    CPEE_Elif,
    CPEE_Else
} CPExceptEndType;

int LclStrCmp(char *a,char *b) {
    int r;
    while (*a==*b && *a) { a++; b++; }
    if (*a==*b)  r=0;  else { if (*a<*b) r=-1; else r=1; }
    return r;
}

CPExceptEndType CPrepExceptionSkipBlock(CPreprocEnv *Ev) {
    char *p;
    int End;
    CPExceptEndType R;
    R = CPEE_Endif;
    do {
        p = Ev->Stream->p;
	    do {
	        do {
	            while (*p!='\n' && *p && *p!='\'' && *p!='\"') {
	                p++;
	            }
	            p = Call(Ev->Stream,Check0,1(p));
	        } while (*p!='\n' && *p && *p!='\'' && *p!='\"');
	        while (*p=='\n') { 
		        do {p++;} while (*p=='\n');
	            Call(Ev->Stream,Check0,1(p)); 
		        p = CPreprocSkipBlank(Ev->Stream,Ev->Lexic->IsBlank);
	        }
	    } while (*p!='#' && *p && *p!='\'' && *p!='\"');
	    End = (*p=='#');
	    if (End) {
	        p++; Call(Ev->Stream,Check0,1(p));
	        p = CPreprocSkipBlank(Ev->Stream,Ev->Lexic->IsBlank);
	        End = Ev->Lexic->IsChar[*p];
	        if (End) {
		        char Word[8],*q,*e;
		        q = Word; e = Word+7;
		        do {
		            while (q<e && *p && Ev->Lexic->IsChar[*p]) *q++ = *p++;
		            p = Call(Ev->Stream,Check0,1(p));
		        } while (q<e && *p && Ev->Lexic->IsChar[*p]);
		        *q = 0;
		        { R = CPEE_Elif; End = (LclStrCmp(Word,"elif")==0); }
		        if (!End) { R = CPEE_Else; End = (LclStrCmp(Word,"else")==0); }
		        if (!End) { R = CPEE_Endif; End = (LclStrCmp(Word,"endif")==0); }
		        if (!End) {
		            int OpenBlock;
		            OpenBlock = LclStrCmp(Word,"if")==0;
		            if (!OpenBlock) OpenBlock = LclStrCmp(Word,"ifdef")==0;
		            if (!OpenBlock) OpenBlock = LclStrCmp(Word,"ifndef")==0;
		            if (OpenBlock) {
			            CPExceptEndType E;
			            do {
			                E = CPrepExceptionSkipBlock(Ev);
			            } while (E!=CPEE_Endif);
			            p = Ev->Stream->p;
		            }
		        }
	        }
	    } else {
	        if (*p=='\'' || *p=='\"') {
                p = CPreprocSkipMacroString(Ev->Stream);
	        }
	    }
    } while(!End && *p);
    return R;
}

char *CPrepExceptionSkipBlocks(CPreprocEnv *Ev) {
    CPExceptEndType E;
    do {
	    E = CPrepExceptionSkipBlock(Ev);
    } while (E!=CPEE_Endif);
    return Ev->Stream->p;
}

/*___________________________________________________________________
 |
 |___________________________________________________________________
*/

typedef struct {
	BTConstructor BTConstructor;
	BuffText *Src;
} BTConstructorEndLine;

BuffText *BTConstructorEndLineInstantiate(BTConstructor *this) {
	StringConstruct *LineEnd;
	BuffText *Src;
	char *p,*q,*e;
	ThisToThat(BTConstructorEndLine,BTConstructor);
	Src = that->Src;
	p = Src->p;
	LineEnd = StringConstructNew();
	q = StringConstructNext(LineEnd,0);
	e = LineEnd->e;
	do {
		while ((*p)&&(*p!='\n')&&(q<e)) { *q++ = *p++;}
		p = Call(Src,Check0,1(p));
		q = StringConstructNext(LineEnd,q); e = LineEnd->e;
	} while ((*p)&&(*p!='\n'));
	if (*p=='\n') {
		Call(Src,Check0,1(Src->p+1));
	}
	return BTStringConstructNew(LineEnd);
}

BTConstructor *BTConstructorEndLineNew(CPreprocEnv *Ev) {
	static BTConstructorStatic Static = {BTConstructorEndLineInstantiate};
	BTConstructorEndLine *R;
    rPush(R);
	R->BTConstructor.Static = &Static;
	R->Src = Ev->Stream;
	return &R->BTConstructor;
}

int CPrepGetBoolExpression(CPreprocEnv *Ev) { 
	int R;
    rOpen {
		CPreprocEnvEnterLocalText(Ev,BTConstructorEndLineNew(Ev));
		R = MacroExpressionParseBool(Ev);
		CPreprocEnvLeaveLocalText(Ev);
	} rClose
	return R;
}


/*___________________________________________________________________
 |
 |___________________________________________________________________
*/

char *CPExceptGetIncludeFilename(CPreprocEnv *Ev) {
    char *r,*p,*q,*e;
    StringConstruct *SC;
    p = Ev->Stream->p;
    if (*p) p = Call(Ev->Stream,Check0,1(p+1));
    pOpen
    EnvSwap()
    SC = StringConstructNew();
    q = StringConstructNext(SC,0); e = SC->e;
    while (*p && *p!='>') {
	do {
	    *q++ = *p++;
	} while (q<e && *p && *p!='>');
	p = Call(Ev->Stream,Check0,1(p));
	q = StringConstructNext(SC,q); e = SC->e;
    }
    if (*p=='>') { p++; Call(Ev->Stream,Check0,1(p)); }
    EnvSwap()
        r = StringConstructGetString(SC);
    pClose
    return r;
}

/*___________________________________________________________________
 |
 |___________________________________________________________________
*/

static int StringLength(char *S) {
    int R;
    char *p;
    p = S; R = 0;
    while (*p) {p++; R++;}
    return R;
}

static char *CatName(char *Path,char *FileName) {
    char *r,*q,*p,*lp;
    int L;
    L = StringLength(Path)+StringLength(FileName)+2;
    L = (L+(sizeof(void *)))&(-sizeof(void *));
    rnPush(r,L);
    q = r;
    lp = p = Path;
    while (*p) {lp = p; *q++=*p++;}
	if ((*lp!='/')&&(*Path!=0)) {*q++='/';}  // !!!! Unix System !!!!
    p = FileName;
    while (*p) (*q++=*p++);
    *q = 0;
    return r;
}

/*___________________________________________________________________
 |
 |___________________________________________________________________
*/

#include <stdio.h>
BTConstructor *CPExceptFindIncludeFile(CPreprocEnv *Ev,char *Filename) {
    BTConstructor *R;
    char **Path;
    Path = Ev->Path;
    R = 0;
    while(!R && *Path) {
	    char *Name;
        FILE *F;
	    pOpen
	    pIn(Name = CatName(*Path,Filename));
	    F = fopen(Name,"r");
	    if (F) {
			EoxFeedback *LineCount;
	        fclose(F);
			LineCount = FilePosStackPush(Ev->Position.Files,Name);
			R = BTConstrCPIIFileNew(Name,EoxFeedbackQuote(LineCount));
	    }
	    pClose
	    Path++;
    }
    if (!R) {
		/* !!! Should issue an error here !!! */
		R=BTConstructorNull();
	}
    return R;
}

BuffText *CPExceptIncludeManage(CPreprocException *this,CPreprocEnv *Ev) {
    char *p;
    char *Filename;
    BTConstructor *N;
    p = CPreprocSkipBlank(Ev->Stream,Ev->Lexic->IsBlank);
    Filename = "";
    pOpen
    EnvSwap()
    if (*p=='<') {
       Filename = CPExceptGetIncludeFilename(Ev);
    } else {
        CPreprocLexem *L;
	    L = CPreprocGetNextLexem(Ev);
	    if (L->Type==CLTString) {
	        Filename = StringConstructGetString(L->Token.String);
	    }
    }
    N = CPExceptFindIncludeFile(Ev,Filename);
    EnvSwap()
    CPreprocEnvInsertText(Ev,N);
    pClose
    return Ev->Stream;
}

CPreprocException *CPExceptMacroIncludeNew(void) {
    CPreprocException *R;
    static CPreprocExceptionStatic Static = {CPExceptIncludeManage};
    rPush(R);
    R->Static = &Static;
    return R;
}

BuffText *CPExceptElseManage(CPreprocException *this,CPreprocEnv *Ev) {
    int End;
    CPrepExceptionSkipBlock(Ev);
    return Ev->Stream;
}

CPreprocException *CPExceptMacroElseNew(void) {
    CPreprocException *R;
    static CPreprocExceptionStatic Static = {CPExceptElseManage};
    rPush(R);
    R->Static = &Static;
    return R;
}

BuffText *CPExceptElifManage(CPreprocException *this,CPreprocEnv *Ev) {
	/* Case normally encountered after a successful if or elif has been encountered...
	   skip to next endif */
    int End;
    do {
	    CPExceptEndType E;
	    E = CPrepExceptionSkipBlock(Ev);
	    End = (E==CPEE_Endif);
    } while (!End);
    return Ev->Stream;
}

CPreprocException *CPExceptMacroElifNew(void) {
    CPreprocException *R;
    static CPreprocExceptionStatic Static = {CPExceptElifManage};
    rPush(R);
    R->Static = &Static;
    return R;
}

BuffText *CPExceptEndifManage(CPreprocException *this,CPreprocEnv *Ev) {
    return Ev->Stream;
}

CPreprocException *CPExceptMacroEndifNew(void) {
    CPreprocException *R;
    static CPreprocExceptionStatic Static = {CPExceptEndifManage};
    rPush(R);
    R->Static = &Static;
    return R;
}

BuffText *CPExceptCondManage(CPreprocEnv *Ev,int End) {
    while (!End) {
	    CPExceptEndType R;
	    R = CPrepExceptionSkipBlock(Ev);
	    End = (R==CPEE_Endif) || (R==CPEE_Else);
	    if (!End) {
            End = CPrepGetBoolExpression(Ev);
	    }
    }
    return Ev->Stream;
}

BuffText *CPExceptIfManage(CPreprocException *this,CPreprocEnv *Ev) {
    return CPExceptCondManage(Ev,CPrepGetBoolExpression(Ev));
}

CPreprocException *CPExceptMacroIfNew(void) {
    CPreprocException *R;
    static CPreprocExceptionStatic Static = {CPExceptIfManage};
    rPush(R);
    R->Static = &Static;
    return R;
}

int CPreprocIsDefined(CPreprocEnv *Ev) {
    int Defined;
    char *p;
    p = CPreprocSkipBlank(Ev->Stream,Ev->Lexic->IsBlank);
    Defined = Ev->Lexic->IsChar[*p];
    if (Defined) {
	    PatriciaNod *f;
	    rOpen
	    CPreprocGetIdentifier(Ev);
        f = PatriciaSeek(&Ev->Macros,Ev->Last.Token.Identifier);
	    Defined = (f!=&Ev->Macros);
	    rClose
    } 
    return Defined;
}

int CPreprocNextIdIsDefined(CPreprocEnv *Ev) {
	int Defined,Parenthesis;
	char *p;
	p = CPreprocSkipBlank(Ev->Stream,Ev->Lexic->IsBlank);
	Parenthesis = (*p=='(');
	if (Parenthesis) {
		Call(Ev->Stream,Check0,1(p+1));
	}
	Defined = CPreprocIsDefined(Ev);
	if (Parenthesis) {
		p = CPreprocSkipBlank(Ev->Stream,Ev->Lexic->IsBlank);
		if (*p==')') {
			Call(Ev->Stream,Check0,1(p+1));
		}
	}
	return Defined;
}

BuffText *CPExceptIfdefManage(CPreprocException *this,CPreprocEnv *Ev) {
    return CPExceptCondManage(Ev,CPreprocIsDefined(Ev));
}

CPreprocException *CPExceptMacroIfdefNew(void) {
    CPreprocException *R;
    static CPreprocExceptionStatic Static = {CPExceptIfdefManage};
    rPush(R);
    R->Static = &Static;
    return R;
}

BuffText *CPExceptIfndefManage(CPreprocException *this,CPreprocEnv *Ev) {
    return CPExceptCondManage(Ev,!CPreprocIsDefined(Ev));
}

CPreprocException *CPExceptMacroIfndefNew(void) {
    CPreprocException *R;
    static CPreprocExceptionStatic Static = {CPExceptIfndefManage};
    rPush(R);
    R->Static = &Static;
    return R;
}

/*___________________________________________________________________
 |
 |___________________________________________________________________
*/

CPreprocLexSetting *CPreprocLexClearSettingNew(void) {
    CPreprocLexSetting *R;
    rPush(R);
    rnPush(R->IsBlank,256);
    {int i; for (i=0;i<256;i++) R->IsBlank[i] = (0!=0); }
    rnPush(R->IsChar,256);
    {int i; for (i=0;i<256;i++) R->IsChar[i] = (0==0); }
    R->IsChar[0] = R->IsChar['\n'] = (0!=0);
    rnPush(R->EscapedChar,256);
    rnPush(R->EscapedCharBack,256);
    {int i; for (i=0;i<256;i++) R->EscapedChar[i] = R->EscapedCharBack[i] = i; }
    rnPush(R->Separator,512);
    {int i; char *p; p = R->Separator; for (i=0;i<256;i++) {*p++=i; *p++=0;}}
    rPush(R->Operators);
    PatriciaInit(R->Operators);
    PatriciaInit(&R->Keywords);
    rnPush(R->Fork,256);
    {int i; for (i=0;i<256;i++) R->Fork[i] = CPreprocGetIdentifier;}
    return R;
}

CPreprocLexem *CPreprocGetBlank(CPreprocEnv *Ev) {
    CPreprocSkipBlank(Ev->Stream,Ev->Lexic->IsBlank);
    Ev->Last.Type = CLTBlank;
    return &Ev->Last;
}

CPreprocLexSetting *CPreprocLexSetBlanks(CPreprocLexSetting *this,char *Blanks) {
    char *p;
    p = Blanks;
    while (*p) {
	    this->IsBlank[*p] = (0==0);
	    this->IsChar[*p] = (0!=0);
	    this->Fork[*p] = CPreprocGetBlank;
	    p++;
    }
    return this;
}

CPreprocLexSetting *CPreprocLexSetEscapedChar(CPreprocLexSetting *this,char *Escaped) {
    char *p;
    p = Escaped;
    this->EscapedCharBack[0] = '0';
    this->EscapedChar['0'] = 0;
    while (*p) {
	    this->IsChar[p[1]] = (0!=0);
	    this->EscapedChar[*p] = p[1];
	    this->EscapedCharBack[p[1]] = p[0];
	    p+=2;
    }
    return this;
}

CPreprocLexSetting *CPreprocSetSeparators(CPreprocLexSetting *this,char **Operator) {
    char **q;
    q = Operator;
    while (*q) {
	    char *p;
	    PatriciaNod *N;
	    p = *q++;
	    if (p[1]==0 && ((this->Fork[*p]==CPreprocGetIdentifier)||(this->Fork[*p]==CPreprocGetSeparator))) {
	        this->Fork[*p]=CPreprocGetSeparator;
	        this->IsChar[*p]=(0!=0);
	    } else {
	        this->Fork[*p]=CPreprocGetOperator;
	    }
        rPush(N);
	    PatriciaSeekOrInsert(this->Operators,p,N);
    }
    return this;
}

typedef struct {
    char *Spelling;
    int Key;
} CSynKeywordTranslate;

int CPreprocSetKeywords(CPreprocLexSetting *this,CSynKeywordTranslate *Keywords) {
    int Nb;
    CSynKeywordTranslate *p;
    p = Keywords; Nb = 0;
    while (p->Spelling) {
	    CSynLexicConstIdNew(&this->Keywords,p->Spelling,p->Key);
	    p++; Nb++;
    }
    return Nb;
}

CPreprocLexSetting *CPreprocSetOperators(CPreprocLexSetting *this,CSynKeywordTranslate *Operators) {
    int Nb;
    Nb = CPreprocSetKeywords(this,Operators);
    pOpen {
	    char **Separators,**q;
        CSynKeywordTranslate *p;
	    pnPush(Separators,Nb+1);
        p = Operators; q = Separators;
	    while (p->Spelling) {
            *q = p->Spelling;
	        q++; p++;
	    }
	    *q = 0;
	    this = CPreprocSetSeparators(this,Separators);
    } pClose
    return this;
}

/*__________________________________________________________
 |
 |__________________________________________________________
*/

CPreprocLexem *CPreprocEndLine(CPreprocEnv *Evr);

struct _BTCPreprocParse_ {
    MemStack *mLast;
    CPreprocEnv *Env;
    CPreprocLexem *Last;
};
 
CPreprocEnv *CPreprocEnvNew(char **Path) {
    CPreprocEnv *R;
    MemStack *M;
    M = MemStackFork(Env.r,StackGrowth);
    mPush(M,R);
    R->mIdentifier = MemStackFork(M,StackGrowth);
    R->mMacro = MemStackFork(M,StackGrowth);
    mIn(M,R->mSrc = BTStackNew());
    R->Stream = BTStack2BuffText(R->mSrc);
    PatriciaInit(&R->Macros);
    {
        int i;
	PatriciaNod *P;
	static char *Separator[] = { "\"","\'","#",0 };
	static CSynKeywordTranslate Operators[] = {
	    {"&",'&'},{"&&",CS_bAnd},{"&=",CS_AndSet},{"~",'~'},{"{",'{'},{"}",'}'},{"(",'('},{")",')'},
	    {"[",'['},{"]",']'},{"-",'-'},{"--",CS_Dec},{"-=",CS_SubSet},{"->",CS_Idrct},
	    {"|",'|'},{"||",CS_bOr},{"|=",CS_OrSet},{"^",'^'},{"^=",CS_XorSet},{"+",'+'},
	    {"++",CS_Inc},{"+=",CS_AddSet},{"=",'='},{"==",CS_Eq},{"%",'%'},{"%=",CS_ModSet},
	    {"*",'*'},{"*=",CS_MulSet},{"<",'<'},{">",'>'},{"<<",CS_SAL},{"<<=",CS_SALSet},
	    {"<=",CS_InfEq},{">>",CS_SAR},{">>=",CS_SARSet},{">=",CS_SupEq},{"?",'?'},{",",','},
	    {".",'.'},{"...",CS_Etc},{"..",'.'},{";",';'},{"/",'/'},{"/=",CS_DivSet},{":",':'},{"!",'!'},
	    {"!=",CS_NotSet},
	    {0,255}
	};	
	static CSynKeywordTranslate Keywords[] = {
	    {"auto",CS_Auto},{"int",CS_Int},{"double",CS_Double},{"struct",CS_Struct},
	    {"break",CS_Break},{"else",CS_Else},{"long",CS_Long},{"switch",CS_Switch},
	    {"case",CS_Case},{"enum",CS_Enum},{"register",CS_Register},{"typedef",CS_Typedef},
	    {"char",CS_Char},{"extern",CS_Extern},{"return",CS_Return},{"union",CS_Union},
        {"const",CS_Const},{"float",CS_Float},{"short",CS_Short},{"unsigned",CS_Union},
	    {"continue",CS_Continue},{"for",CS_For},{"signed",CS_Signed},{"void",CS_Void},
	    {"default",CS_Default},{"goto",CS_Goto},{"sizeof",CS_Sizeof},{"volatile",CS_Volatile},
	    {"do",CS_Do},{"if",CS_If},{"static",CS_Static},{"while",CS_While},
	    {0,255}
	};	
        R->Lexic = CPreprocLexClearSettingNew();
        CPreprocLexSetBlanks(R->Lexic," \r\t\v\b\f\a");
        CPreprocLexSetEscapedChar(R->Lexic,"t\tr\rn\n0\0v\vb\bf\fa\a");
        CPreprocSetSeparators(R->Lexic,Separator);
	    CPreprocSetOperators(R->Lexic,Operators);
	    CPreprocSetKeywords(R->Lexic,Keywords);
	    for (i='0';i<='9';i++) {
	        R->Lexic->Fork[i] = CPreprocGetNumber;
	    }
	    R->Lexic->Fork[0] = CPreprocGetEof;
	    R->Lexic->Fork['.'] = CPreprocGetPointLexem;
	    R->Lexic->Fork['\''] = CPreprocGetChar;
	    R->Lexic->Fork['\"'] = CPreprocGetString;
	    R->Lexic->Fork['\n'] = CPreprocEndLine;
        PatriciaInit(&R->Symbols);
        PatriciaSeekOrInsert(&R->Symbols,"include",&CPExceptMacroIncludeNew()->PatriciaNod);
	    PatriciaSeekOrInsert(&R->Symbols,"define",&CPExceptMacroDefineNew()->PatriciaNod);
	    PatriciaSeekOrInsert(&R->Symbols,"undef",&CPExceptMacroUndefNew()->PatriciaNod);
	    PatriciaSeekOrInsert(&R->Symbols,"error",&CPExceptSkipLineNew()->PatriciaNod);
	    PatriciaSeekOrInsert(&R->Symbols,"pragma",&CPExceptSkipLineNew()->PatriciaNod);
	    PatriciaSeekOrInsert(&R->Symbols,"ifdef",&CPExceptMacroIfdefNew()->PatriciaNod);
	    PatriciaSeekOrInsert(&R->Symbols,"ifndef",&CPExceptMacroIfndefNew()->PatriciaNod);
	    PatriciaSeekOrInsert(&R->Symbols,"if",&CPExceptMacroIfNew()->PatriciaNod);
	    PatriciaSeekOrInsert(&R->Symbols,"elif",&CPExceptMacroElifNew()->PatriciaNod);
	    PatriciaSeekOrInsert(&R->Symbols,"else",&CPExceptMacroElseNew()->PatriciaNod);
	    PatriciaSeekOrInsert(&R->Symbols,"endif",&CPExceptMacroEndifNew()->PatriciaNod);
    }
    R->Path=Path;
    mIn(M,R->Position.Files = FilePosStackNew());
	R->Position.Position = FilePosStackGetPosition(R->Position.Files);
    R->Last.Type = CLTBlank;
    rPush(R->Gate);
    R->Gate->Env = R;
    R->Gate->Last = 0;
    R->Gate->mLast = MemStackFork(M,StackGrowth);
    return R;
}

/*__________________________________________________________
 |
 |__________________________________________________________
*/

typedef struct  {
    List List;
    BuffText *Child;
	int Local;
} BTStacked;

struct _BTStack_ {
    BuffText BuffText;
    MemStack *M;
    List Current,Top;
    int Lock;
};

BuffText *BTStack2BuffText(BTStack *this) {
    return &this->BuffText;
}

BTStack *BTStackFreeTop(BTStack *that) {
    if (!that->Lock) {
	    while (that->Top.n!=that->Current.n) {
	        that->Top.n = that->Top.n->n;
	        mLeave(that->M);
	    }
    }
	return that;
}

static char *BTStackCheck0(BuffText *this,char *p) {
    static char StringNull[] = "";
    ThisToThat(BTStack,BuffText);
    if (that->Current.n != ListEnd) {
	    BTStacked *B;
	    B = CastBack(BTStacked,List,that->Current.n);
	    p = this->p = Call(B->Child,Check0,1(p));
	    while ((!*p)&&(that->Current.n->n!=ListEnd)&&(!B->Local)){
	        that->Current.n = that->Current.n->n;
	        B = CastBack(BTStacked,List,that->Current.n);
	        p = this->p = Call(B->Child,Check0,1(B->Child->p));
	    }
	    if ((!*p)&&(!B->Local)) that->Current.n = that->Current.n->n;
    }
    if (that->Current.n == ListEnd) {
	    this->p = p = StringNull;
    }
	BTStackFreeTop(that);
    return p;
}
static long BTStackSeek(BuffText *this,long dp) {return 0;}
static void BTStackClose(BuffText *this) {
	ThisToThat(BTStack,BuffText);
    if (that->Current.n != ListEnd) {
		BTStacked *B;
		B = CastBack(BTStacked,List,that->Current.n);
		Call(B->Child,Close,0);
		while (that->Current.n->n!=ListEnd) {
			that->Current.n = that->Current.n->n;
			B = CastBack(BTStacked,List,that->Current.n);
			Call(B->Child,Close,0);
		}
	}
	this->p = "";
	that->Lock = (0!=0);
	BTStackFreeTop(that);
}

static BuffText *BTMoveToEnd(BuffText *B) {
	char *e;
	e = B->p;
	do {
		while (*e) e++;
		e = Call(B,Check0,1(e));
	} while (*e);
	return B;
}

BTStack *BTStackRemoveLocalBT(BTStack *R) {
	List /*<BTStacked>*/ *p;
	p = R->Current.n;
	if (p!=ListEnd) {
	    BTStacked *B;
		B = CastBack(BTStacked,List,p);
		while (p->n!=ListEnd && !B->Local) {
			BTMoveToEnd(B->Child);
			p = p->n;
			B = CastBack(BTStacked,List,p);
		}
		if (B->Local && p->n!=ListEnd) {
			BTMoveToEnd(B->Child);
			p = p->n;
			B = CastBack(BTStacked,List,p);
		}
		R->Current.n = p;
        R->BuffText.p = Call(B->Child,Check0,1(B->Child->p));
	}
	return BTStackFreeTop(R);
}

BTStack *BTStackNew(void) {
    BTStack *R;
    MemStack *M;
    static struct {
		struct BuffText BuffText;
	} Static = {{BTStackCheck0,BTStackSeek,BTStackClose}};
    M = MemStackFork(Env.r,StackGrowth);
    mPush(M,R);
    R->M = M;
    R->Current.n = R->Top.n = ListEnd;
    R->BuffText.Static = &Static.BuffText;
    R->BuffText.p = "";
    R->Lock = (0!=0);
    return R;
}

BTStack *BTStackInsertABT(BTStack *R,BTConstructor *B,int Local) {
    BTStacked *N;
    R->Lock = (0==0);
    mEnter(R->M);
    mPush(R->M,N);
	N->Local = Local;
    N->List.n = R->Top.n;
    mIn(R->M,N->Child = Call(B,Instantiate,0));
    R->Top.n = R->Current.n = &N->List;
    R->BuffText.p = N->Child->p;
    R->Lock = (0!=0);
    return R;
}

BTStack *BTStackInsertLocalBT(BTStack *R,BTConstructor *B) {
	/* A local BT ends with an EOF; you have to use BTStackRemoveLocalBT to get Back to the regular stack. */
	return BTStackInsertABT(R,B,(0==0));
}

BTStack *BTStackInsertBT(BTStack *R,BTConstructor *B) {
	/* A regular insert is fully integrated into the stream; the end won't be noticeable. */ 
	return BTStackInsertABT(R,B,(0!=0));
}

CPreprocEnv *CPreprocEnvInsertText(CPreprocEnv *Ev,BTConstructor *T) {
    BTStackInsertBT(Ev->mSrc,T);
    return Ev;
}

CPreprocEnv *CPreprocEnvEnterLocalText(CPreprocEnv *Ev,BTConstructor *T) {
	BTStackInsertLocalBT(Ev->mSrc,T);
	return Ev;
}

CPreprocEnv *CPreprocEnvLeaveLocalText(CPreprocEnv *Ev) {
	BTStackRemoveLocalBT(Ev->mSrc);
	return Ev;
}

/*__________________________________________________________
 |
 |__________________________________________________________
*/

CPreprocLexem *CPreprocEndLine(CPreprocEnv *Evr) {
    int End;
    do {
        char c;
	    c = *(CPreprocSkipBlank(Evr->Stream,Evr->Lexic->IsBlank));
        while (c=='\n') {
	        do {
	            Call(Evr->Stream,Check0,1(Evr->Stream->p+1));
		        c = *Evr->Stream->p;
	        } while(c=='\n');
	        if (c) c = *(CPreprocSkipBlank(Evr->Stream,Evr->Lexic->IsBlank));
        }
	    End = (c!='#');
        if (!End) {
	        Evr->Last.Type = CLTBlank;
	        Call(Evr->Stream,Check0,1(Evr->Stream->p+1));
	        c = *(CPreprocSkipBlank(Evr->Stream,Evr->Lexic->IsBlank));
	        if (!Evr->Lexic->IsChar[c]) {
                CPreprocSkipEndLine(Evr->Stream);
	        } else {
		        char *I;
		        PatriciaNod *f;
		        rOpen
		           I = CPreprocGetIdentifier(Evr)->Token.Identifier;
		           f = PatriciaSeek(&Evr->Symbols,I);
                rClose
		        if (f==&Evr->Symbols) {
		            CPreprocSkipEndLine(Evr->Stream);
		        } else {
		            CPreprocException *F;
		            F = CastBack(CPreprocException,PatriciaNod,f);
		            Call(F,Manage,1(Evr));
		        }
	        }
	    }
    } while (!End);
    return  CPreprocGetNextLexem(Evr);
}

CPreprocLexem *CPreprocIdentifierManage(CPreprocEnv *Ev) {
    char *I;
    CPreprocLexem *R;
    PatriciaNod *f;
    R = CPreprocGetIdentifier(Ev);
    I = R->Token.Identifier;
    f = PatriciaSeek(&Ev->Macros,I);
    if (f!=&Ev->Macros) {
	CPreprocException *F;
	F = CastBack(CPreprocException,PatriciaNod,f);
	Call(F,Manage,1(Ev));
	R = CPreprocGetNextLexem(Ev);
    }
    return R;
}

StringConstruct *StringConstructDuplicate(StringConstruct *SC) {
    pOpen {
        BuffText *B;
	char *p,*q,*e;
	pIn(B = BTStringConstructNew(SC));
        SC = StringConstructNew();
	q = StringConstructNext(SC,0); e = SC->e;
	do {
	    while (q<e && *p) {
		*q++ = *p++;
	    }
	    p = Call(B,Check0,1(p));
	    q = StringConstructNext(SC,q);
	    e = SC->e;
	} while (*p);
    } pClose
    return SC;
}

char *StringDuplicate(char *St) {
    int L;
    char *p,*q;
    p = St; L = 0;
    while (*p) {p++; L++;}
    L = (L+sizeof(void *))&(-sizeof(void *));
    rnPush(q,L);
    p = St; St = q;
    while (*p) {*q++=*p++;}
    *q = 0;
    return St;
}

CPreprocLexem *CPreprocLexemDuplicate(CPreprocLexem *Last) {
    switch(Last->Type) {
    case CLTString:
	    Last->Token.String = StringConstructDuplicate(Last->Token.String);
    break;
    case CLTIdentifier:
        Last->Token.Identifier = StringDuplicate(Last->Token.Identifier);
    break;
    case CLTNumber:
        Last->Token.Number = StringDuplicate(Last->Token.Number);
    break;
    case CLTFloat:
        Last->Token.Float = StringDuplicate(Last->Token.Float);
    break;
    }
    return Last;
}

CPreprocLexem *CPreproc2GetLexem(BTCPreprocParse *Ev) {
    CPreprocLexem *r;
    if (Ev->Last) {
        r = CPreprocLexemDuplicate(Ev->Last);
	    mLeave(Ev->mLast);
        Ev->Last = 0;
    } else {
		FilePosStackNextLexem(Ev->Env->Position.Files);
        do {
	        char *p;
            p = CPreprocSkipBlank(Ev->Env->Stream,Ev->Env->Lexic->IsBlank);
	        r = Ev->Env->Lexic->Fork[*p](Ev->Env);
	        if (r->Type == CLTIdentifier) {
	            PatriciaNod *f;
                f = PatriciaSeek(&Ev->Env->Macros,r->Token.Identifier);
	            if (f!=&Ev->Env->Macros) {
		            CPreprocException *F;
		            F = CastBack(CPreprocException,PatriciaNod,f);
		            Call(F,Manage,1(Ev->Env));
		            r = 0;
	            }
	        }
        } while (!r);
    }
    return r;
}

CPreprocLexem *CPreprocLastLexem(CPreprocEnv *Ev) {
    CPreprocLexem *R;
    if (Ev->Gate->Last) {
	    R = Ev->Gate->Last;
    } else {
	    mEnter(Ev->Gate->mLast);
        mIn(Ev->Gate->mLast,Ev->Gate->Last = R = CPreproc2GetLexem(Ev->Gate));
    }
    return R;
}

CPreprocLexem *CPreprocGetNextLexem(CPreprocEnv *Ev) {
    return CPreproc2GetLexem(Ev->Gate);
}

/*__________________________________________________________
 |
 |__________________________________________________________
*/

typedef struct {
    List List;
    StringConstruct *S;
} StringConstructList;

typedef struct {
    CSynLexem CSynLexem;
    List Strings;
} CSynSCLexem;

char *SCCatToString(char *d,StringConstruct *SC) {
    pOpen
    BuffText *B;
    char *p;
    pIn(B = BTStringConstructNew(SC));
    p = B->p;
    do {
	while (*p) { *d++ = *p++; }
	p = Call(B,Check0,1(p));
    } while (*p);
    *d = 0;
    pClose
    return d;
}

static char *CSynSCLexemInstance(CSynLexem *this) {
    char *R,*q;
    List *p;
    int L;
    ThisToThat(CSynSCLexem,CSynLexem);
    L = 0;
    p = that->Strings.n;
    while (p!=ListEnd) {
	    StringConstructList *P;
	    P = CastBack(StringConstructList,List,p);
	    p = p->n;
	    L+=StringConstructCurrentLength(P->S);
    }
    L = (L+sizeof(void *)) & (-sizeof(void *));
    rnPush(R,L);
    p = that->Strings.n; 
    q = R;
    while (p!=ListEnd) {
	    StringConstructList *P;
	    P = CastBack(StringConstructList,List,p);
	    p = p->n;
	    q = SCCatToString(q,P->S);
    }
    return R;
}

CSynLexem *CSynStringLexemNew(StringConstructList *L){
    CSynSCLexem *R;
    static struct CSynLexem Static = {CSynSCLexemInstance,CSynVarLexemDuplicate};
    rPush(R);
    R->CSynLexem.Static = &Static;
    R->CSynLexem.Type = CS_ConstString;
    R->Strings.n = &L->List;
    return &R->CSynLexem;
}

CSynLexem *CSynGetStringLexem(CPreprocEnv *Ev,CPreprocLexem *L) {
    StringConstructList *R;
    List *l;
    rPush(R);
    R->List.n = ListEnd;
    R->S = L->Token.String;
    L = CPreprocLastLexem(Ev);
    l = &R->List;
    /* Successives string must be concatenated. */
    while (L->Type == CLTString) {
	StringConstructList *N;
	L = CPreprocGetNextLexem(Ev);
	rPush(N);
	N->List.n = l->n;
	N->S = L->Token.String;
	l->n = &N->List;
        l = l->n;
	L = CPreprocLastLexem(Ev);
    }
    return CSynStringLexemNew(R);
}

/*------------------------------------------*/

typedef struct {
    CSynLexem CSynLexem;
    char *Val;
} CSynVarLexem;

static char *CSynVarInstance(CSynLexem *this) {
    ThisToThat(CSynVarLexem,CSynLexem);
    return that->Val;
}

static CSynLexem *CSynVarDuplicate(CSynLexem *this) {
     CSynVarLexem *R;
     char *p,*q;
     int L;
     static struct CSynLexem Static = {CSynVarInstance,CSynVarDuplicate};
     ThisToThat(CSynVarLexem,CSynLexem);
     rPush(R);
     L = 0;
     p = that->Val;
     while (*p) {L++; p++;}
     L = (L+sizeof(void *)) & (- sizeof(void *));
     rnPush(R->Val,L);
     q = R->Val; p = that->Val;
     while(*p) {*q++=*p++;}
     *q = 0;
     R->CSynLexem.Static = &Static;
     R->CSynLexem.Type = this->Type;
     return &R->CSynLexem;
}

CSynLexem *CSynVarLexemNew(CSynLexemType Type,char *Val) {
     CSynVarLexem *R;
     static struct CSynLexem Static = {CSynVarInstance,CSynVarDuplicate};
     rPush(R);
     R->CSynLexem.Static = &Static;
     R->CSynLexem.Type = Type;
     R->Val = Val;
     return &R->CSynLexem;
}

static CSynLexem *CSynVarLexemDuplicate(CSynLexem *this) {
     char *Val;
     Val  = Call(this,Instance,0);
     return CSynVarLexemNew(this->Type,Val);
}

CSynLexem *CSynIdentifierLexemNew(CSynLexemType T,char *Val) {
     return CSynVarLexemNew(T,Val);
}

CSynLexem *CSynCharLexemNew(char *Val) {
     return CSynVarLexemNew(CS_ConstChar,Val);
}

CSynLexem *CSynFloatLexemNew(char *Val) {
     return CSynVarLexemNew(CS_ConstFloat,Val);
}

CSynLexem *CSynIntLexemNew(char *Val) {
     return CSynVarLexemNew(CS_ConstInt,Val);
}

CSynLexem *CSynEofLexemNew(void) {
    static char Str[] = "";
    return CSynVarLexemNew(CS_Eof,Str);
}

/*------------------------------------------*/

CPreprocEnv *CPreprocEnvOpen(char **Path,char *File) {
	CPreprocEnv *new;
	new = CPreprocEnvNew(Path);
	pOpen
	    BTConstructor *F;
	    pIn(F = CPExceptFindIncludeFile(new,File));
        new = CPreprocEnvInsertText(new,F);
	pClose
	return new;
}


typedef struct {
	BTConstructor BTConstructor;
	char *data;
} BTConstrString;
static BuffText *BTConstrStringInstantiate(BTConstructor *this) {
	ThisToThat(BTConstrString,BTConstructor);
	return BuffTextNew(that->data);
}
static BTConstructor *BTConstructorString(char *Data) {
	BTConstrString *R;
	static BTConstructorStatic Static = {BTConstrStringInstantiate};
	rPush(R);
	R->BTConstructor.Static = &Static;
	R->data = Data;
	return &R->BTConstructor;
}
CPreprocEnv *CPreprocEnvStringOpen(char **Path,char *Data) {
	CPreprocEnv *new;
	new = CPreprocEnvNew(Path);
	pOpen
		BTConstructor *F;
	    pIn(F = BTConstructorString(Data));
	    new = CPreprocEnvInsertText(new,F);
	pClose
	return new;
}

CSynLexem *CSynPreprocGetLexem(CPreprocEnv *Ev) {
    CSynLexem *R;
    CPreprocLexem *P;
    P = CPreprocGetNextLexem(Ev);
    switch(P->Type) {
    case CLTString:
	    R = CSynGetStringLexem(Ev,P);
    break;
    case CLTChar:
        R = CSynCharLexemNew(P->Token.Separator);
    break;
    case CLTFloat:
        R = CSynFloatLexemNew(P->Token.Float);
    break;
    case CLTNumber:
        R = CSynIntLexemNew(P->Token.Number);
    break;
    case CLTIdentifier:
    case CLTSeparator: {
        PatriciaNod *f;
        f = PatriciaSeek(&Ev->Lexic->Keywords,P->Token.Identifier);
        if (f != &Ev->Lexic->Keywords) {
	        CSynLexicId  *F;
	        F = CastBack(CSynLexicId,PatriciaNod,f);
	        R = &F->CSynLexem;
	    } else {
	        R = CSynIdentifierLexemNew(CS_ConstIdentifier,P->Token.Identifier);
	    }
    } break;
    default:
        R = CSynEofLexemNew();
    break;
    }
    return R;
}

BuffText *CPreprocIssueError(CPreprocEnv *Ev,BuffText *Msg) {
	return Call(Ev->Position.Position,Print,1(Msg));
}


