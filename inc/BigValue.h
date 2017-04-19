/*  Big Value offers various primitives for creating and managing Big Integer
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

#ifndef _BigValue_h_
#define _BigValue_h_

/*_____________________________________
 |
 | BigInt
 |_____________________________________
*/

typedef struct BigInt BigInt;
extern BigInt BigIntZero;
extern BigInt BigIntOne;
extern BigInt BigIntTwo;
extern BigInt BigIntMinusOne;

BigInt *BigIntNew(int n);
BigInt *BigIntGet(char *Src);
char *BigIntPut(BigInt *Src);

int BigIntGetSign(BigInt *A);
BigInt *BigIntBoolify(int B);

BigInt *BigIntNot(BigInt *A);
BigInt *BigIntAnd(BigInt *A,BigInt *B);
BigInt *BigIntOr(BigInt *A,BigInt *B);
BigInt *BigIntXor(BigInt *A,BigInt *B);

BigInt *BigIntSum(BigInt *A,BigInt *B);
BigInt *BigIntMinus(BigInt *A);
BigInt *BigIntDiff(BigInt *A,BigInt *B);

int BigIntSup(BigInt *A,BigInt *B);
int BigIntSupEq(BigInt *A,BigInt *B);
int BigIntInfEq(BigInt *A,BigInt *B);
int BigIntInf(BigInt *A,BigInt *B);
int BigIntEq(BigInt *A,BigInt *B);
int BigIntNotEq(BigInt *A,BigInt *B);

typedef struct {
	BigInt *Q,*R;
} BigIntDivResult;

BigInt *BigIntSAR(BigInt *B,int S);
BigInt *BigIntSAL(BigInt *B,int S);
BigInt *BigIntBSAR(BigInt *B,BigInt *S);
BigInt *BigIntBSAL(BigInt *B,BigInt *S);
BigInt *BigIntSHR(BigInt *B,int S,int Window);
BigInt *BigIntMul(BigInt *A,BigInt *B);
BigIntDivResult *BigIntDivMod(BigIntDivResult *Res,BigInt *A,BigInt *B);
BigInt *BigIntDiv(BigInt *A,BigInt *B);
BigInt *BigIntMod(BigInt *A,BigInt *B);


#endif

