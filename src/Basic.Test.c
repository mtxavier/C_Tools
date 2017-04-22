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
#include <Tools.h>

#define IntFirst1(r,v,sz) { \
	unsigned int _val,_i,_msk;\
	_i = (sz<<2); _val = v; r = 0; _msk = ((unsigned int)-1)<<(sz<<2);\
	while (_i>0) {\
		if (_val&_msk) {\
			_val=_val>>_i;\
		} else {\
			r+=_i;\
		}\
		_i = _i>>1; _msk=_msk>>_i; \
	}\
}

#define uCharFirst1(r,vl) { \
	unsigned int _v;\
	_v = vl;\
	if (_v&0xf0){\
		r=(_v&0xc0)?((_v&0x80)?0:1):((_v&0x20)?2:3);\
	} else {\
		r=(_v&0x0c)?((_v&0x08)?4:5):((_v&0x02)?6:(_v?7:8));\
	}\
}

main() {
	printf("\n uInt range:[%u,%u], size:%d",0,TlsMaxVal(int),TlsAtomSize(int));
	printf("\n Int range:[%d,%d]",TlsMinSignedVal(int),TlsMaxSignedVal(int));
	printf("\n ushort range:[%u,%u], size:%d",0,TlsMaxVal(short),TlsAtomSize(short));
	printf("\n short range:[%d,%d]",TlsMinSignedVal(short),TlsMaxSignedVal(short));
	printf("\n uchar range:[%u,%u], size:%d",0,TlsMaxVal(char),TlsAtomSize(char));
	printf("\n char range:[%d,%d]",TlsMinSignedVal(char),TlsMaxSignedVal(char));
	{ 
		int j;
		int error,err1;
		error = err1 = 0;
		for (j=1;j<0x100;j++) {
			int frst,scnd;
            // IntFirst1(frst,j,1);
            TlsILg2(frst,j); frst = 7-frst;
			if  (((j>>(7-frst))&1)==0) {
				error++;
			} else {
				if ((j>>(8-frst))!=0) {
					error++;
				}
			}
			uCharFirst1(scnd,j);
			if  (((j>>(7-scnd))&1)==0) {
				err1++;
			} else {
				if ((j>>(8-scnd))!=0) {
					err1++;
				}
			}
		}
		printf("\nInt first1 errorCount:%d",error);
		printf("\nChar first1 errorCount:%d",err1);
	}
	printf("\nTab:%d,Enter:%d\n",'\t','\n');
}

