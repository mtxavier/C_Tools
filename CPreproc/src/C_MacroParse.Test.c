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
#include <C_Lexic.h>
#include <C_MacroParse.h>
#include <stdio.h>


main() {
	static char *Path[] = {"./",0};
	BigIntExpression *R;
	CPreprocEnv *PEnv;
	MacroParseEnv *PE;
	EnvOpen(4000,4000);
  
	rOpen {
	printf("\n%s\n",BigIntPut(BigIntNew(5769)));
	R = BigIntExpressionIntNew(BigIntNew(4924));
    printf("%s\n",BigIntPut(BigIntExpressionGetBigInt(R)));
    {
		BigIntExpression *A,*B,*C;
		A = BigIntExpressionIntNew(BigIntNew(4924));
		B = BigIntExpressionIntNew(BigIntNew(5769));
		C = BigIntBinaryExpressionNew(BigIntSum,A,B);
        printf("%s\n",BigIntPut(BigIntExpressionGetBigInt(C)));
		C = BigIntBinaryExpressionNew(BigIntSum,C,A);
        printf("%s\n",BigIntPut(BigIntExpressionGetBigInt(C)));
	}
	} rClose

	PEnv = CPreprocEnvOpen(Path,"Test.Macro");
	PE = MacroParseEnvNew(PEnv);
	while (MacroParseEnvLastLexem(PE)->Type!=CS_Eof) {
		printf("%s\n",BigIntPut(BigIntExpressionGetBigInt(MacroParseConstExpression(PE))));
	}
	EnvClose();
}

