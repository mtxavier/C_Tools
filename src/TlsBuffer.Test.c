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

#include <stdio.h>
#include <StackEnv.h>
#include <Classes.h>
#include <Tools.h>




main() {
	TlsCircleBuffer *b;
	int i,j;
	char c,bin[64],bout[96],binsert[64],*p,*e,*ee,*q;
	EnvOpen(4096,4096);
	b = TlsCBfNew(20);
	for (i=0;i<64;i++) { bin[i] = '0'+(i%10); binsert[i] = 'a'+(i%26); }
	e = bin+64;
	bout[64]=0;
	TlsCBfWrite(b,bin,bin+20);
	TlsCBfRead(bout,bout+10,b);
	TlsCBfWrite(b,bin,bin+10);
	TlsCBfGet(bout,bout+10,b,5);
	bout[10]=0;
	printf("Sample: %s\n",bout);
	TlsCBfClear(b);
	printf("Writing n by n, reading 1 by 1.\n");
    for (j=1;j<17;j++) {
		bout[j] = 0; p = bin; q = bout;
		printf("  n = %d : ",j);
		while (p<e) {
            ee = p+j; if (ee>e) { ee=e; }
			TlsCBfWrite(b,p,ee);
			while (p<ee) { 
				TlsCBfRead(q,q+1,b); 
				p++; q++;
			}
		}
		printf(bout); printf("\n");
	}
	printf("Insert at head, (6 elements in buffer).\n");
	for (j=0;j<15;j++) {
         printf(" %d added: ",j);
		 TlsCBfWrite(b,bin+16,bin+26);
		 TlsCBfRead(bout,bout+4,b);
		 TlsCBfInsert(b,0,j);
		 TlsCBfOverwrite(b,0,binsert,binsert+j);
		 TlsCBfRead(bout,bout+6+j,b);
		 bout[6+j] = 0;
		 printf(bout); printf("\n");
	}
	printf("Insert at queue, (6 elements in buffer).\n");
	for (j=0;j<15;j++) {
         printf(" %d added: ",j);
		 TlsCBfWrite(b,bin+16,bin+26);
		 TlsCBfRead(bout,bout+4,b);
		 TlsCBfInsert(b,6,j);
		 TlsCBfOverwrite(b,6,binsert,binsert+j);
		 TlsCBfRead(bout,bout+6+j,b);
		 bout[6+j] = 0;
		 printf(bout); printf("\n");
	}
	printf("Insert at random, (6 elements in buffer).\n");
	for (j=0;j<=10;j++) {
         printf("At pos %d: ",j);
		 TlsCBfWrite(b,bin+10,bin+20);
		 TlsCBfInsert(b,j,6);
		 TlsCBfOverwrite(b,j,binsert,binsert+6);
		 TlsCBfRead(bout,bout+16,b);
		 bout[16] = 0;
		 printf(bout); printf("\n");
	}
	printf("Removing n elements, (16 elements in buffer).\n");
	for (j=0;j<=16;j++) {
		bout[16-j]=0;
		TlsCBfWrite(b,bin,bin+16);
		TlsCBfRemove(b,0,j);
		TlsCBfRead(bout,bout+16-j,b);
		printf("At start: %s\n",bout);
		TlsCBfWrite(b,bin,bin+16);
		TlsCBfRemove(b,16-j,j);
		TlsCBfRead(bout,bout+16-j,b);
		printf("At end:   %s\n",bout);
		TlsCBfWrite(b,bin,bin+16);
		TlsCBfRemove(b,(16-j)>>2,j);
		TlsCBfRead(bout,bout+16-j,b);
		printf("First 1/2:%s\n",bout);
		TlsCBfWrite(b,bin,bin+16);
		TlsCBfRemove(b,16-(j+((16-j)>>2)),j);
		TlsCBfRead(bout,bout+16-j,b);
		printf("Last 1/2: %s\n",bout);
	}
	printf("Insertion on empty buffer:\n");
	for (i=0;i<=20;i++) {
		int ok,j;
		printf(" %d Elts:",i);
	    TlsCBfClear(b);
		TlsCBfInsert(b,0,i);
		TlsCBfOverwrite(b,0,binsert+i,binsert+(i<<1));
		TlsCBfGet(bout,bout+i,b,0);
		ok = (0==0);
		for (j=0;j<i;j++) { ok = ok && (binsert[i+j]==bout[j]); }
		if (ok) { printf("Ok; ");} else { printf("Failed; "); }
	}
	printf("\n");
	EnvClose();
}



