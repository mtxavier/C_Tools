/*  BigIntExpression offers primitives for creating Literal Big Integer Expression
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

#ifndef _BigIntExpression_h_
#define _BigIntExpression_h_

#include <BigValue.h>

/*_________________________________________________
 |
 | BigIntExpression
 |_________________________________________________
*/

typedef struct BigIntExpression BigIntExpression;
typedef struct BigIntExpressionStatic BigIntExpressionStatic;

struct BigIntExpressionStatic {
	BigInt *(*Int)(BigIntExpression *this);
};
struct BigIntExpression{
	BigIntExpressionStatic *Static;
};

BigInt *BigIntExpressionGetBigInt(BigIntExpression *this);
int BigIntExpressionGetBool(BigIntExpression *this);

/*________________________________________
 |
 | C operators Translated into functions
 |________________________________________
*/

int O2FbAnd(int A,int B);
int O2FbOr (int A,int B);
int O2FbNot(int A);

/*________________________________________
 |
 | Constructors
 |________________________________________
*/

BigIntExpression *BigIntExpressionIntNew(BigInt *Val);
BigIntExpression *BigIntExpressionValNew(char *Val);
BigIntExpression *BigIntUnaryExpressionNew(BigInt *(*Operator)(BigInt *A),BigIntExpression *A);
BigIntExpression *BigIntBinaryExpressionNew(BigInt *(*Operator)(BigInt *A,BigInt *B),BigIntExpression *A,BigIntExpression *B);
BigIntExpression *BigIntInt2BoolExpressionNew(int (*Operator)(BigInt *A,BigInt *B),BigIntExpression *A,BigIntExpression *B);
BigIntExpression *BigIntBinaryBoolExpressionNew(int (*Operator)(int A,int B),BigIntExpression *A,BigIntExpression *B);
BigIntExpression *BigIntUnaryBoolExpressionNew(int (*Operator)(int A),BigIntExpression *A);
BigIntExpression *BigIntCondExpressionNew(BigIntExpression *Cond,BigIntExpression *True,BigIntExpression *False);

#endif
