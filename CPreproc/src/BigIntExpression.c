/*  The code for BigIntExpression
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
#include <BigIntExpression.h>


/*_____________________________________
 |
 | GetBool;
 |_____________________________________
*/

int BigIntExpressionGetBool(BigIntExpression *this) {
	int R;
	BigInt *V;
    pOpen {
		V = Call(this,Int,0);
		R = BigIntGetSign(V)!=0;
	} pClose
	return R;
}

BigInt *BigIntExpressionGetBigInt(BigIntExpression *this) {
	return Call(this,Int,0);
}

/*_____________________________________
 |
 | Raw value
 |_____________________________________
*/

typedef struct {
	BigIntExpression BigIntExpression;
	BigInt *Val;
} BigIntExpressionInt;
static BigInt *BigIntExpressionIntInt(BigIntExpression *this) {
	ThisToThat(BigIntExpressionInt,BigIntExpression);
	return that->Val;
}
BigIntExpression *BigIntExpressionIntNew(BigInt *Val) {
	static BigIntExpressionStatic Static = {BigIntExpressionIntInt};
	BigIntExpressionInt *Result;
	rPush(Result);
	Result->BigIntExpression.Static = &Static;
	Result->Val = Val;
	return &Result->BigIntExpression;
}
BigIntExpression *BigIntExpressionValNew(char *Val) {
	return BigIntExpressionIntNew(BigIntGet(Val));
}

/*_____________________________________
 |
 | Unary Expression
 |_____________________________________
*/

typedef struct {
	BigIntExpression BigIntExpression;
	BigIntExpression *Children;
	BigInt *(*Operator)(BigInt *A);
} BigIntUnaryExpression;

static BigInt *BigIntUnaryExpressionInt(BigIntExpression *this) {
	BigInt *R,*A;
	ThisToThat(BigIntUnaryExpression,BigIntExpression);
    pOpen {
		EnvSwap();
		A = Call(that->Children,Int,0);
		EnvSwap();
		R = that->Operator(A);
	} pClose
	return R;
}
BigIntExpression *BigIntUnaryExpressionNew(BigInt *(*Operator)(BigInt *A),BigIntExpression *A) {
	static BigIntExpressionStatic Static = {BigIntUnaryExpressionInt};
	BigIntUnaryExpression *Result;
	rPush(Result);
	Result->BigIntExpression.Static = &Static;
	Result->Operator = Operator;
	Result->Children = A;
	return &Result->BigIntExpression;
}


/*_____________________________________
 |
 | Binary Expression
 |_____________________________________
*/

typedef struct {
	BigIntExpression BigIntExpression;
	BigIntExpression *Children[2];
	BigInt *(*Operator)(BigInt *A,BigInt *B);
} BigIntBinaryExpression;

static BigInt *BigIntBinaryExpressionInt(BigIntExpression *this) {
	BigInt *R,*A,*B;
	ThisToThat(BigIntBinaryExpression,BigIntExpression);
	pOpen { 
		EnvSwap();
		A = Call(that->Children[0],Int,0);
		B = Call(that->Children[1],Int,0);
        EnvSwap();
		R = that->Operator(A,B);
	} pClose
	return R;
}

BigIntExpression *BigIntBinaryExpressionNew(BigInt *(*Operator)(BigInt *A,BigInt *B),BigIntExpression *A,BigIntExpression *B) {
	static BigIntExpressionStatic Static = {BigIntBinaryExpressionInt};
	BigIntBinaryExpression *R;
	rPush(R);
	R->BigIntExpression.Static = &Static;
	R->Operator = Operator;
	R->Children[0] = A;
	R->Children[1] = B;
	return &R->BigIntExpression;
}

/*_____________________________________
 |
 | Int To Bool
 |_____________________________________
*/

typedef struct {
	BigIntExpression BigIntExpression;
	BigIntExpression *Children[2];
	int (*Operator)(BigInt *A,BigInt *B);
} BigIntInt2BoolExpression;

static BigInt *BigIntInt2BoolExpressionInt(BigIntExpression *this) {
	BigInt *R;
	ThisToThat(BigIntInt2BoolExpression,BigIntExpression);
	pOpen {
		BigInt *A,*B;
		EnvSwap()
		A = Call(that->Children[0],Int,0);
		B = Call(that->Children[1],Int,0);
		EnvSwap()
	    R = BigIntBoolify(that->Operator(A,B));
	} pClose
    return R;
}
BigIntExpression *BigIntInt2BoolExpressionNew(int (*Operator)(BigInt *A,BigInt *B),BigIntExpression *A,BigIntExpression *B) {
	static BigIntExpressionStatic Static = {BigIntInt2BoolExpressionInt};
	BigIntInt2BoolExpression *Result;
	rPush(Result);
	Result->BigIntExpression.Static = &Static;
	Result->Children[0] = A;
	Result->Children[1] = B;
	Result->Operator = Operator;
	return &Result->BigIntExpression;
}

/*_____________________________________
 |
 | Binary Bool
 |_____________________________________
*/

int O2FbAnd(int A,int B) { return A && B;}
int O2FbOr (int A,int B) { return A || B;}
int O2FbNot(int A) { return !A; }

typedef struct {
	BigIntExpression BigIntExpression;
	BigIntExpression *Children[2];
	int (*Operator)(int A,int B);
} BigIntExpressionBinaryBool;
static BigInt *BigIntExpressionBinaryBoolInt(BigIntExpression *this) {
	BigInt *R;
	ThisToThat(BigIntExpressionBinaryBool,BigIntExpression);
	pOpen {
		BigInt *A,*B;
		EnvSwap()
		A = Call(that->Children[0],Int,0);
		B = Call(that->Children[1],Int,0);
		EnvSwap()
		R = BigIntBoolify(that->Operator(BigIntGetSign(A)!=0,BigIntGetSign(B)!=0));
	} pClose
	return R;
}
BigIntExpression *BigIntBinaryBoolExpressionNew(int (*Operator)(int A,int B),BigIntExpression *A,BigIntExpression *B) {
	static BigIntExpressionStatic Static = {BigIntExpressionBinaryBoolInt};
	BigIntExpressionBinaryBool *Result;
	rPush(Result);
	Result->BigIntExpression.Static = &Static;
	Result->Children[0] = A;
	Result->Children[1] = B;
	Result->Operator = Operator;
	return &Result->BigIntExpression;
}

typedef struct {
	BigIntExpression BigIntExpression;
	BigIntExpression *Child;
	int (*Operator)(int A);
} BigIntExpressionUnaryBool;
static BigInt *BigIntExpressionUnaryBoolInt(BigIntExpression *this) {
	BigInt *R;
	ThisToThat(BigIntExpressionUnaryBool,BigIntExpression);
	pOpen {
		BigInt *A;
		EnvSwap()
		A = Call(that->Child,Int,0);
		EnvSwap()
		R = BigIntBoolify(that->Operator(BigIntGetSign(A)!=0));
	} pClose
	return R;
}
BigIntExpression *BigIntUnaryBoolExpressionNew(int (*Operator)(int A),BigIntExpression *A) {
	static BigIntExpressionStatic Static = {BigIntExpressionUnaryBoolInt};
	BigIntExpressionUnaryBool *Result;
	rPush(Result);
	Result->BigIntExpression.Static = &Static;
	Result->Child = A;
	Result->Operator = Operator;
	return &Result->BigIntExpression;
}

/*_____________________________________
 |
 | Cond Expression
 |_____________________________________
*/

typedef struct {
	BigIntExpression BigIntExpression;
	BigIntExpression *Children[3];
} BigIntCondExpression;
static BigInt *BigIntCondExpressionInt(BigIntExpression *this) {
	BigInt *R;
	ThisToThat(BigIntCondExpression,BigIntExpression);
    pOpen {
		BigInt *Cond;
		EnvSwap();
		Cond = Call(that->Children[0],Int,0);
		EnvSwap();
		if (BigIntGetSign(Cond)!=0) {
			R = Call(that->Children[1],Int,0);
		} else {
			R = Call(that->Children[2],Int,0);
		}
	} pClose
	return R;
}
BigIntExpression *BigIntCondExpressionNew(BigIntExpression *Cond,BigIntExpression *True,BigIntExpression *False) {
	 static BigIntExpressionStatic Static = {BigIntCondExpressionInt};
	 BigIntCondExpression *Result;
	 rPush(Result);
	 Result->BigIntExpression.Static = &Static;
	 Result->Children[0] = Cond;
	 Result->Children[1] = True;
	 Result->Children[2] = False;
	 return &Result->BigIntExpression;
}

/*_____________________________________
 |
 | Raw value
 |_____________________________________
*/

typedef struct {
	BigIntExpression BigIntExpression;
	char *Label;
} BigIntExpressionVar;

