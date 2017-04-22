/*  Test application for BigValue
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

#include <stdio.h>
#include <StackEnv.h>
#include <BigValue.h>


void IntShow(char *Before,BigInt *S) {
	char *s;
	rOpen {
        s = BigIntPut(S);
		printf(Before);
		printf(s);
	} rClose
}

main() {
	BigInt *A0,*A5,*A223,*A5000;
	BigInt *B0,*B1,*B2,*B3;
    EnvOpen(4000,4000);
	A0 = BigIntNew(0);
	A5 = BigIntNew(-5);
    A223 = BigIntNew(547);
	A5000 = BigIntNew(-22917);
	BigIntShow("\n-5985::",A5000); printf(" Sign(%d)",BigIntGetSign(A5000));
	BigIntShow("\n223::",A223); printf(" Sign(%d)",BigIntGetSign(A223));
    BigIntShow("\n0::",A0); printf(" Sign(%d)",BigIntGetSign(A0));
	BigIntShow("\n-5::",A5); printf(" Sign(%d)",BigIntGetSign(A5));
	A5 = BigIntMinus(A5);
	B0 = BigIntSum(A5000,A223); BigIntShow("\n-5985+223::",B0); 
	B0 = BigIntMinus(A5000); B1 = BigIntMinus(B0); BigIntShow("\n Minus -5985: ",B0); BigIntShow(" :: ",B1);
	B2 = BigIntSum(B0,B1); BigIntShow(" (x)+(-x):",B2);
	B0 = BigIntNot(B2); B1 = BigIntMinus(B2); BigIntShow("\n~0: ",B0); BigIntShow("   -0: ",B1);
	B0 = BigIntSAL(A5000,4);
	B1 = BigIntSAL(A5000,24);
	B2 = BigIntSAL(A5000,0);
	BigIntShow("\nSAL -5985 <<0: ",B2); BigIntShow(" <<4: ",B0); BigIntShow(" <<24: ",B1);
	B0 = BigIntMul(A5,B0);
	B1 = BigIntMul(A5,B1);
	B2 = BigIntMul(A5,B2);
	BigIntShow("\n5xSAL -5985 >>0: ",B2); BigIntShow(" >>4: ",B0); BigIntShow(" >>56: ",B1);
	B0 = BigIntMul(B0,A5);
	B1 = BigIntMul(B1,A5);
	B2 = BigIntMul(B2,A5);
	BigIntShow("\n5xSALx5 -5985 >>0: ",B2); BigIntShow(" >>4: ",B0); BigIntShow(" >>56: ",B1);
	B0 = BigIntSAL(A223,4);
	B1 = BigIntSAL(A223,56);
	B2 = BigIntSAL(A223,0);
	BigIntShow("\nSAL 223 <<0: ",B2); BigIntShow(" <<4: ",B0); BigIntShow(" <<56: ",B1);
	B0 = BigIntMul(A5,B0);
	B1 = BigIntMul(A5,B1);
	B2 = BigIntMul(A5,B2);
	BigIntShow("\n5xSAR 223 >>0: ",B2); BigIntShow(" >>4: ",B0); BigIntShow(" >>56: ",B1);
	B0 = BigIntMul(B0,A5);
	B1 = BigIntMul(B1,A5);
	B2 = BigIntMul(B2,A5);
	BigIntShow("\n5xSAR 223x5 >>0: ",B2); BigIntShow(" >>4: ",B0); BigIntShow(" >>56: ",B1);
	B0 = BigIntSAR(A5000,4);
	B1 = BigIntSAR(A5000,24);
	B2 = BigIntSAR(A5000,0);
	BigIntShow("\nSAR -5985 >>0: ",B2); BigIntShow(" >>4: ",B0); BigIntShow(" >>24: ",B1);
	B0 = BigIntSAR(A223,4);
	B1 = BigIntSAR(A223,56);
	B2 = BigIntSAR(A223,0);
	BigIntShow("\nSAR 223 >> 0: ",B2); BigIntShow(" >>4: ",B0); BigIntShow(" >>56: ",B1);
	B0 = BigIntSAR(A0,4);
	B1 = BigIntSAR(A0,56);
	B2 = BigIntSAR(A0,0);
	BigIntShow("\nSAR 0 >>0: ",B2); BigIntShow(" >>4: ",B0); BigIntShow(" >>56: ",B1);
	B0 = BigIntSAL(A0,4);
	B1 = BigIntSAL(A0,56);
	B2 = BigIntSAL(A0,0);
	BigIntShow("\nSAL 0 << 0: ",B2); BigIntShow(" <<4: ",B0); BigIntShow(" <<56: ",B1);
	B1 = BigIntMul(A5,A223);
	BigIntShow("\n 5x223 = ",B1); IntShow(" [Decimal]5x223 = ",B1);
    B2 = BigIntNew(1000000);
	B1 = BigIntMul(B1,B2);
	IntShow("\n x1000000: ",B1);
	B1 = BigIntMul(B1,B2);
	IntShow("   x1000000: ",B1); BigIntShow(" Hexa: ",B1);
	{
		BigIntDivResult R;
		B1 = BigIntNew(0x223);
		BigIntDivMod(&R,B1,A5);
        BigIntShow("\n 223/5= ",R.Q); BigIntShow("[",R.R); printf("]");
		BigIntDivMod(&R,A5,B1);
        BigIntShow("  5/223= ",R.Q); BigIntShow("[",R.R); printf("]");
		A5 = BigIntMinus(A5);
		BigIntDivMod(&R,B1,A5);
        BigIntShow("  223/(-5)= ",R.Q); BigIntShow("[",R.R); printf("]");
		B1 = BigIntMinus(B1);
		BigIntDivMod(&R,B1,A5);
        BigIntShow("\n(-223)/(-5)= ",R.Q); BigIntShow("[",R.R); printf("]");
		A5 = BigIntMinus(A5);
		BigIntDivMod(&R,B1,A5);
        BigIntShow("  (-223)/5= ",R.Q); BigIntShow("[",R.R); printf("]");
		BigIntDivMod(&R,A5,B1);
        BigIntShow("  5/(-223)= ",R.Q); BigIntShow("[",R.R); printf("]");
	}
	IntShow("\n0<",A0); IntShow("> 5<",A5); IntShow("> 223<",A223); IntShow("> 5985<",A5000); printf(">");
	printf("\n");
	EnvClose();
}

