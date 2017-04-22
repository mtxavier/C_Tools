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
#include <BigValue.h>
#include <C_MacroParse.h>
#include <Classes.h>

struct MacroParseEnv {
	CPreprocEnv *Src;
    CSynLexem *Last;
};

CSynLexem *MacroParseSkipCheck(MacroParseEnv *Env,CSynLexemType T) {
	int R;
	R = (Env->Last->Type == T);
	if (R) {
		Env->Last = CSynPreprocGetLexem(Env->Src);
	}
	return Env->Last;
}

BigIntExpression *MacroParseConstExpression(MacroParseEnv *Ev);
BigIntExpression *MacroParseCondExpression(MacroParseEnv *Ev);

BigIntExpression *MacroParseSetExpression(MacroParseEnv *Ev) {
	BigIntExpression *R;
	R = MacroParseCondExpression(Ev);
	while (Ev->Last->Type == ',') {
        MacroParseSkipCheck(Ev,',');
        R = MacroParseCondExpression(Ev);
	}
	return R;
}

BigIntExpression *MacroParsePrimaryExpression(MacroParseEnv *Ev) {
	BigIntExpression *R;
	char *Value;
    switch (Ev->Last->Type) {
	case '(':
		MacroParseSkipCheck(Ev,'(');
		R = MacroParseConstExpression(Ev);
		MacroParseSkipCheck(Ev,')');
	break;
	case CS_ConstIdentifier: {
		int Parenth;
		Value = Call(Ev->Last,Instance,0);
		/* Normally: if (strcmp("defined",Value)==0) { */
		R = BigIntExpressionIntNew(BigIntNew(CPreprocNextIdIsDefined(Ev->Src)));
		Ev->Last = CSynPreprocGetLexem(Ev->Src);
    } break;
	case CS_ConstChar:
	    Value = Call(Ev->Last,Instance,0);
	    R = BigIntExpressionIntNew(BigIntNew(Value[0]));
		MacroParseSkipCheck(Ev,CS_ConstChar);
	break;
	case CS_ConstInt:
	    Value = Call(Ev->Last,Instance,0);
	    R = BigIntExpressionValNew(Value);
		MacroParseSkipCheck(Ev,CS_ConstInt);
	break;
	case CS_ConstFloat:
	    Value = Call(Ev->Last,Instance,0);
	    R = BigIntExpressionValNew(Value);
	    MacroParseSkipCheck(Ev,CS_ConstFloat);
	break;
	}
	return R;
}

BigIntExpression *MacroParseUnaryExpression(MacroParseEnv *Ev) {
	BigIntExpression *R;
	switch (Ev->Last->Type) {
	case '+':
		MacroParseSkipCheck(Ev,'+');
		R = MacroParseUnaryExpression(Ev);
	break;
	case '-':
	    MacroParseSkipCheck(Ev,'-');
		R = BigIntUnaryExpressionNew(BigIntMinus,MacroParseUnaryExpression(Ev));
	break;
	case '~':
	    MacroParseSkipCheck(Ev,'~');
		R = BigIntUnaryExpressionNew(BigIntNot,MacroParseUnaryExpression(Ev));
	break;
	case '!':
	    MacroParseSkipCheck(Ev,'!');
		R = BigIntUnaryBoolExpressionNew(O2FbNot,MacroParseUnaryExpression(Ev));
	break;
	default:
	    R = MacroParsePrimaryExpression(Ev);
	break;
	}
	return R;
}

BigIntExpression *MacroParseProdExpression(MacroParseEnv *Ev) {
	BigIntExpression *R;
	R = MacroParseUnaryExpression(Ev);
	while((Ev->Last->Type=='*')||(Ev->Last->Type=='/')||(Ev->Last->Type=='%')) {
		BigIntExpression *B;
		switch (Ev->Last->Type) {
		case '*':
			MacroParseSkipCheck(Ev,'*');
            B = MacroParseUnaryExpression(Ev);
			R = BigIntBinaryExpressionNew(BigIntMul,R,B);
		break;
		case '/':
			MacroParseSkipCheck(Ev,'/');
            B = MacroParseUnaryExpression(Ev);
			R = BigIntBinaryExpressionNew(BigIntDiv,R,B);
		break;
		case '%':
			MacroParseSkipCheck(Ev,'%');
            B = MacroParseUnaryExpression(Ev);
			R = BigIntBinaryExpressionNew(BigIntMod,R,B);
		break;
		}
	}
	return R;
}

BigIntExpression *MacroParseSumExpression(MacroParseEnv *Ev) {
	BigIntExpression *R;
	R = MacroParseProdExpression(Ev);
	while((Ev->Last->Type=='+')||(Ev->Last->Type=='-')) {
		BigIntExpression *B;
		if (Ev->Last->Type == '+') {
			MacroParseSkipCheck(Ev,'+');
			B = MacroParseProdExpression(Ev);
			R = BigIntBinaryExpressionNew(BigIntSum,R,B);
		} else {
			MacroParseSkipCheck(Ev,'-');
			B = MacroParseProdExpression(Ev);
			R = BigIntBinaryExpressionNew(BigIntDiff,R,B);
		}
	}
	return R;
}

BigIntExpression *MacroParseShiftExpression(MacroParseEnv *Ev) {
	BigIntExpression *R;
	R = MacroParseSumExpression(Ev);
	while((Ev->Last->Type==CS_SAL)||(Ev->Last->Type==CS_SAR)) {
		BigIntExpression *B;
        if (Ev->Last->Type == CS_SAL) {
			MacroParseSkipCheck(Ev,CS_SAL);
            B = MacroParseSumExpression(Ev);
            R = BigIntBinaryExpressionNew(BigIntBSAL,R,B);
		} else {
			MacroParseSkipCheck(Ev,CS_SAR);
			B = MacroParseSumExpression(Ev);
			R = BigIntBinaryExpressionNew(BigIntBSAR,R,B);
		}
	}
	return R;
}

BigIntExpression *MacroParseCompExpression(MacroParseEnv *Ev) {
	BigIntExpression *R;
	R = MacroParseShiftExpression(Ev);
	while ((Ev->Last->Type == CS_InfEq)||(Ev->Last->Type == '<')||(Ev->Last->Type == CS_SupEq)||(Ev->Last->Type == '>')) {
		BigIntExpression *B;
		switch (Ev->Last->Type) {
		case CS_InfEq:
			MacroParseSkipCheck(Ev,CS_InfEq);
			B = MacroParseShiftExpression(Ev);
			R = BigIntInt2BoolExpressionNew(BigIntInfEq,R,B);
		break;
		case CS_SupEq:
			MacroParseSkipCheck(Ev,CS_SupEq);
			B = MacroParseShiftExpression(Ev);
			R = BigIntInt2BoolExpressionNew(BigIntInfEq,B,R);
		break;
		case '<':
			MacroParseSkipCheck(Ev,'<');
			B = MacroParseShiftExpression(Ev);
			R = BigIntInt2BoolExpressionNew(BigIntInf,R,B);
		break;
		case '>':
			MacroParseSkipCheck(Ev,'>');
			B = MacroParseShiftExpression(Ev);
			R = BigIntInt2BoolExpressionNew(BigIntInf,B,R);
		break;
		}
	}
	return R;
}

BigIntExpression *MacroParseEqExpression(MacroParseEnv *Ev) {
	BigIntExpression *R;
	R = MacroParseCompExpression(Ev);
	while ((Ev->Last->Type == CS_Eq)||(Ev->Last->Type == CS_NotSet)) {
		BigIntExpression *B;
		if (Ev->Last->Type == CS_Eq) {
		    MacroParseSkipCheck(Ev,CS_Eq);
		    B = MacroParseCompExpression(Ev);
		    R = BigIntInt2BoolExpressionNew(BigIntEq,R,B);
		} else {
		    MacroParseSkipCheck(Ev,CS_NotSet);
		    B = MacroParseCompExpression(Ev);
		    R = BigIntInt2BoolExpressionNew(BigIntNotEq,R,B);
		}
	}
	return R;
}

BigIntExpression *MacroParseAndExpression(MacroParseEnv *Ev) {
	BigIntExpression *R;
	R = MacroParseEqExpression(Ev);
	while (Ev->Last->Type == CS_And) {
		BigIntExpression *B;
		MacroParseSkipCheck(Ev,CS_And);
		B = MacroParseEqExpression(Ev);
		R = BigIntBinaryExpressionNew(BigIntAnd,R,B);
	}
	return R;
}

BigIntExpression *MacroParseXorExpression(MacroParseEnv *Ev) {
	BigIntExpression *R;
	R = MacroParseAndExpression(Ev);
	while (Ev->Last->Type == CS_Xor) {
		BigIntExpression *B;
		MacroParseSkipCheck(Ev,CS_Xor);
		B = MacroParseAndExpression(Ev);
		R = BigIntBinaryExpressionNew(BigIntXor,R,B);
	}
	return R;
}

BigIntExpression *MacroParseOrExpression(MacroParseEnv *Ev) {
	BigIntExpression *R;
	R = MacroParseXorExpression(Ev);
	while (Ev->Last->Type == CS_Or) {
		BigIntExpression *B;
		MacroParseSkipCheck(Ev,CS_Or);
		B = MacroParseXorExpression(Ev);
		R = BigIntBinaryExpressionNew(BigIntOr,R,B);
	}
	return R;
}

BigIntExpression *MacroParseBoolAndExpression(MacroParseEnv *Ev) {
	BigIntExpression *R;
	R = MacroParseOrExpression(Ev);
	while (Ev->Last->Type == CS_bAnd) {
		BigIntExpression *B;
		MacroParseSkipCheck(Ev,CS_bAnd);
		B = MacroParseOrExpression(Ev);
		R = BigIntBinaryBoolExpressionNew(O2FbAnd,R,B);
	}
	return R;
}

BigIntExpression *MacroParseBoolOrExpression(MacroParseEnv *Ev) {
	BigIntExpression *R;
	R = MacroParseBoolAndExpression(Ev);
	while (Ev->Last->Type == CS_bOr) {
		BigIntExpression *B;
		MacroParseSkipCheck(Ev,CS_bOr);
		B = MacroParseBoolAndExpression(Ev);
		R = BigIntBinaryBoolExpressionNew(O2FbOr,R,B);
	}
	return R;
}

BigIntExpression *MacroParseCondExpression(MacroParseEnv *Ev) {
	BigIntExpression *R;
	CSynLexem *L;
	R = MacroParseBoolOrExpression(Ev);
	L = Ev->Last;
	if (L->Type == '?') {
		BigIntExpression *True,*False;
		L = Ev->Last = CSynPreprocGetLexem(Ev->Src);
		True = MacroParseSetExpression(Ev);
		MacroParseSkipCheck(Ev,':');
		False = MacroParseCondExpression(Ev);
		R = BigIntCondExpressionNew(R,True,False);
	}	
	return R;
}

BigIntExpression *MacroParseConstExpression(MacroParseEnv *Ev) {
	BigIntExpression *R;
	R = MacroParseCondExpression(Ev);
	return R;
}

MacroParseEnv *MacroParseEnvNew(CPreprocEnv *Ev) {
	MacroParseEnv *R;
    rPush(R);
	R->Src = Ev;
	R->Last = CSynPreprocGetLexem(R->Src);
	return R;
}

CSynLexem *MacroParseEnvLastLexem(MacroParseEnv *Ev) {
	return Ev->Last;
}

int MacroExpressionParseBool(CPreprocEnv *Ev) {
	int R;
	R = (0!=0);
	rOpen {
	    MacroParseEnv Env;
	    BigIntExpression *M;
	    Env.Src = Ev;
	    Env.Last = CSynPreprocGetLexem(Env.Src);
        M = MacroParseConstExpression(&Env);
	    R = BigIntExpressionGetBool(M);
	} rClose
	return R;
}

