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
#include <C_Preproc.h>
#include <Debug.h>
#include <CPrepMacro.h>
#include <string.h>
#include <stdio.h>

/*________________________________________________*/

void ListFile(BuffText *B) {
    char *p;
    do {
        StringConstruct *SC;
	char *q,*e;
	int Count;
    rOpen 
	Count = 500;
	SC = StringConstructNew();
	p = B->p;
	q = StringConstructNext(SC,0); e = StringConstructGetEnd(SC);
	do {
	    while (q<e && *p && Count) {
	        *q++=*p++;
	    }
	    q = StringConstructNext(SC,q); e = StringConstructGetEnd(SC);
	    p = Call(B,Check0,1(p));
	} while (*p && Count);
	p = Call(B,Check0,1(p));
	{
	    char *Buff;
	    Buff = StringConstructGetString(SC);
	}
    rClose
    } while (*p);
}

void DbgMacroArgCheck(CPMacro *A,int Arity) {
    BuffText *B;
    CPMacroParam P;
    StringConstruct **p,**e;
    rOpen 
    P.Arity = Arity;
    rnPush(P.Params,Arity+1);
    p = P.Params;
    e = P.Params+Arity; *e = 0; 
    while (p<e) {
	    char *S,*q;
	    rnPush(S,8);
	    q = S;
	    *q++ = '('; *q++='A'; *q++='r'; *q++='g'; *q++='0'+(p-P.Params); *q++=')'; *q++=0;
	    *p = StringConstructStringNew(S);
	    p++;
    }
    B = DbgBuffTextSpy(Call(A,Instantiate,1(&P)));
    printf("\n");
    ListFile(B);
    printf("\n");
    rClose
}

char *GetEscapedChar(void) {
    static char EscapedChar[256];
    static int Inited = (0!=0);
    if (!Inited) {
	static char Escaped[] = "a\at\tb\bn\n";
	char *p,*e,*q;
	e = EscapedChar+256;
	p = EscapedChar;
	while (p<e) {*p = p-EscapedChar; p++;}
	p = Escaped;
	while (*p) { EscapedChar[p[1]] = p[0]; p=p+2;}
	    EscapedChar[0] = '0';
	    Inited = (0==0);
    }
    return EscapedChar;
}

void CPLexemWrite(CPreprocLexem *L) {
    switch(L->Type) {
	case CLTIdentifier:
	    printf(" %s",L->Token.Identifier);
	break;
	case CLTSeparator:{
	    char c;
	    c = L->Token.Separator[0];
	    printf("%s",L->Token.Separator);
	    if (c=='{'||c=='}'||c==';') {
		printf("\n");
	    }
    } break;
	case CLTChar:
	   printf("\'%s\'",L->Token.Separator);
    break;
	case CLTNumber:
	    printf("%s",L->Token.Number);
	break;
	case CLTFloat:
	    printf("%s",L->Token.Float);
	break;
	case CLTString: {
	    BuffText *B;
	    char *p,*q,String[32];
	    B = BTStringConstructNew(L->Token.String);
            String[31] = 0;
	    q = String;
	    p = B->p;
	    printf("\"");
	    do {
		    while (*p && q<String+31) {
		        *q++ = *p++;
		    }
		    Call(B,Check0,1(p));
		    *q = 0; q=String;
		    printf("%s",q);
	    } while (*p);
	    printf("\"");
    }
	break;
    }

}

void CSLexemWrite(CPreprocEnv *Ev,CSynLexem *L) {
    char *p;
    p = Call(L,Instance,0);
    switch(L->Type) {
    case CS_ConstString:
	printf("\"%s\"",p);
    break;
    case CS_ConstChar:
        printf("\'%s\'",p);
    break;
    case '{':
    case ';':
        printf("%s\n",p);
    break;
    case CS_ConstIdentifier:
        printf(" %s",p);
    break;
	case CS_Typedef:{
		rOpen
		BuffText *Err;
		char *p;
		Err = BuffTextNew(" define type -----\n");
		Err = CPreprocIssueError(Ev,Err);
        do {
		    p = Err->p;
			printf(p);
			while (*p) p++;
			p = Call(Err,Check0,1(p));
		} while (p);
		rClose
	}
    default:
        printf(p);
    break;
    }
}

main() {
    BuffText *B;
    EnvOpen(2048,2048);
#if 1
{
    CPreprocEnv *PEnv;
    BTConstructor *BC;
    CPreprocLexem *L;
    CSynLexemType SLT;
    static char *IncPath[] = {
	""
	,"inc/"
	,"../inc/"
	,0
    };
	PEnv = CPreprocEnvOpen(IncPath,"src/C_Lexic.c");
#if 0
    B = DbgBuffTextSpy(CPreprocEnvGetStream(PEnv));
    ListFile(B);
#else
    do {
	rOpen
	    /* L = CPreprocGetNextLexem(PEnv);
	       CPLexemWrite(L); */
	    CSynLexem *SL;
	    SL = CSynPreprocGetLexem(PEnv);
	    SLT = SL->Type;
	    CSLexemWrite(PEnv,SL);
	rClose
    } while (SLT!=CS_Eof);
#endif
}
#else
{
     CPMacroParam P1,P2;
     StringConstruct *p1[4],*p2[4];
     BTList *L;
     CPMacroArg *CM;
     CPMacro *M0,*pM;
     char *EscapedChar;
     EscapedChar = GetEscapedChar();
     p1[0] = StringConstructStringNew("Un");
     p1[1] = StringConstructStringNew("chat\n");
     p1[2] = StringConstructStringNew("ivre");
     p1[3] = 0;
     P1.Arity = 3;
     P1.Params = p1;
     p2[0] = StringConstructStringNew("Les");
     p2[1] = StringConstructStringNew("bruits");
     p2[2] = StringConstructStringNew("distants");
     p2[3] = 0;
     P2.Arity = 3;
     P2.Params = p2;
     CM = CPMacroArgNew(
	 CPMacroConstStringNew(StringConstructStringNew("Debut "))
	,CPMacroQuoteVariableNew(0,EscapedChar)
	,CPMacroConstStringNew(StringConstructStringNew(" ensuite "))
	,CPMacroQuoteVariableNew(1,EscapedChar)
	,CPMacroConstStringNew(StringConstructStringNew(" encore "))
	,CPMacroQuoteVariableNew(1,EscapedChar)
	,CPMacroConstStringNew(StringConstructStringNew(" puis "))
	,CPMacroQuoteVariableNew(2,EscapedChar)
	,CPMacroConstStringNew(StringConstructStringNew(" retour "))
	,0
     );
     CM = CPMacroArgCat(CM, CPMacroQuoteVariableNew(0,EscapedChar));
     CM = CPMacroArgCat(CM, CPMacroConstStringNew(StringConstructStringNew(" Fin.\n")));

     M0 = CPMacroArgGetMacro(CM);
     DbgMacroArgCheck(M0,3);
     B = Call(M0,Instantiate,1(&P1));
     ListFile(DbgBuffTextSpy(B));
     B = Call(M0,Instantiate,1(&P2));
     ListFile(DbgBuffTextSpy(B));
     B = BTStringConstructNew(StringConstructStringNew("\n voila \n"));
     ListFile(DbgBuffTextSpy(B));
     L = BTListNew(BuffTextNew("Un,deux,trois,"),BuffTextNew("\nquatre,cinq,six,"),BuffTextNew("sept,huit,neuf,"),0);
     L = BTListCatBuffer(L,BuffTextNew("\ndix,onze,douze,"));
     L = BTListCatBuffer(L,BuffTextNew("\ntreize,quatorze,quinze.\n"));
     B = DbgBuffTextSpy(BTListGetBT(L));
     ListFile(B);

}
#endif
    EnvClose();
}

