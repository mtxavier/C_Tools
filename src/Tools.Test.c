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

#include <Tools.h>
#include <stdio.h>


void BitFieldTest(int num,int f1,int f2,int f3,int f4,int f5) {
	{
	    char Buffer[8];
	    int i,j,k,l,m,n;
		int Failed,errors;
		struct {
			int min,max,range;
		} Bornes[7];
		for (i=0;i<6;i++) Buffer[i] = 0;
		Bornes[0].min = 0; 
		Bornes[1].min = f1; Bornes[2].min = f2; Bornes[3].min=f3; Bornes[4].min=f4; Bornes[5].min=f5;
		Bornes[6].min = 32;
		for (i=0;i<6;i++) {
			Bornes[i].max = Bornes[i+1].min;
			Bornes[i].range = 1<<(Bornes[i].max-Bornes[i].min);
		}
		errors = 0;
		for (i=0;i<Bornes[1].range;i++) {
            TlsBitFieldSetField(Buffer,Bornes[1].min,Bornes[1].max,i);
			for(j=0;j<Bornes[2].range;j++) {
				TlsBitFieldSetField(Buffer,Bornes[2].min,Bornes[2].max,j);
				for(k=0;k<Bornes[3].range;k++) { 
					TlsBitFieldSetField(Buffer,Bornes[3].min,Bornes[3].max,k);
					for(l=0;l<Bornes[4].range;l++) {
						TlsBitFieldSetField(Buffer,Bornes[4].min,Bornes[4].max,l);
						if (l!=TlsBitFieldGetField(Buffer,Bornes[4].min,Bornes[4].max)) {
							errors++;
						}
		            } 
					if (k!=TlsBitFieldGetField(Buffer,Bornes[3].min,Bornes[3].max)) {
						errors++;
					}
				}
                if (j!=TlsBitFieldGetField(Buffer,Bornes[2].min,Bornes[2].max)) {
					errors++;
				}
			}
			if (i!=TlsBitFieldGetField(Buffer,Bornes[1].min,Bornes[1].max)) {
				errors++;
			}
		}
		for (i=0;i<48;i+=8) {
			if ((i+8<=Bornes[0].max)||(i>=Bornes[5].min)) {
				if (Buffer[i>>3]!=0) {
					errors++;
				}
			}
		}
		Failed = (errors!=0);
		printf("BitField Test %d: [%d,%d[ [%d,%d[ [%d,%d[ [%d,%d[\n     ",num,f1,f2,f2,f3,f3,f4,f4,f5);
		if (Failed) {
			printf("Failed (%d errors found).\n",errors);
		} else {
			printf("Success\n");
		}
	}
}

main() {
	printf("\nchar size:%d",TlsAtomSize(char)*8);
	printf("\nshort size:%d",TlsAtomSize(short)*8);
	printf("\nint size:%d",TlsAtomSize(int)*8);
	printf("\nlong size:%d",TlsAtomSize(long)*8);
	printf("\nfloat size:%d",TlsAtomSize(float)*8);
	printf("\ndouble size:%d",TlsAtomSize(double)*8);
	printf("\n");
	printf("\nMax unsigned short:%u int:%u long:%u",TlsMaxVal(short),TlsMaxVal(int),TlsMaxVal(long));
	printf("\nMax signed short:%d int:%d long:%d",TlsMaxSignedVal(short),TlsMaxSignedVal(int),TlsMaxSignedVal(long));
	printf("\nMin signed short:%d int:%d long:%d",TlsMinSignedVal(short),TlsMinSignedVal(int),TlsMinSignedVal(long));
	printf("\n");
	BitFieldTest(1,8,9,12,17,24);
	BitFieldTest(2,8,15,16,24,24);
	BitFieldTest(3,8,11,19,22,22);
	BitFieldTest(4,8,11,20,22,22);
	BitFieldTest(5,8,10,20,22,22);
	BitFieldTest(6,8,18,20,22,22);
	BitFieldTest(7,8,16,24,24,24);
	BitFieldTest(8,8,10,24,28,28);
	BitFieldTest(9,8,24,24,24,24);
	BitFieldTest(10,8,9,25,28,28);
	BitFieldTest(11,8,11,26,28,28);
	printf("\n");
}

