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

/*_____________________________________________________________________________
 |
 | UTF-8 encoding / decoding:
 |  * 0xxxxxxx <-> 1 character encoding (7 bits data).
 |  * 10xxxxxx <-> follow up byte: 1-5 of them must follow a lead in byte.
 |  * 110xxxxx <-> leading byte with 1 follow up (11 bits data).
 |  * 1110xxxx <-> leading byte with 2 follow up (16 bits data).
 |  * 11110xxx <-> leading byte with 3 follow up (21 bits data).
 |  * 111110xx <-> leading byte with 4 follow up (26 bits data).
 |  * 1111110x <-> leading byte with 5 follow up (31 bits data).
 |
 | - bytes 1111111x are forbidden (could have been 6 and 7 follow up).
 | - With multiple bytes, the information data are simply packed most 
 |   significant data sequence first.
 | - Some codes can have multiple encoding: only the most packed one is allowed
 | - follow up bytes outside a designated sequence are forbidden.
 | - also codes above +10ffff are forbidden by RFC 3629
 |_____________________________________________________________________________
*/


int TlsUTF8CodeLength(char *s) {
	char *p;
	int l;
	l = 0;
	p = s;
	while (*p) { 
		l++;
		if ((*p)&0x80) {
			do { p++; } while ((*p)&0x80);
		} else {
			p++;
		}
	}
	return l;
}
int TlsUTF8CodeSeek(char *s,int pos) {
	char *p;
	p = s;
	while ((*p)&&(pos>0)) {
		pos--;
		if ((*p)&0x80) {
			do { p++; } while ((*p)&0x80);
		} else {
			p++;
		}
	}
	return (pos==0)? p-s : ((pos>0)?-1:-2); 
}
int TlsUTF8CodingLength(int *s) {
	int *p,c;
	int l;
    // value limits : 0x80, 0x800, 0x10000, 0x200000, 0x4000000, 0x80000000
	// RFC Limit : 0x10ffff, we replace the codes above by 0xfffd
	p = s; l = 0;
	while (*p) {
		c = *p++;
		if (c<0x800) {
			if (c<0x80) { l++; } else { l+=2; }
		} else {
			if (c<0x10000) { l+=3; } else { if (c<0x110000 ) { l+=4;} else { l+=3; } }
		}
	}
	return l;
}

char *TlsIntToUTF8(char *d,int *s) {
	int *p;
	char *q;
	p = s; q = d;
	while (*p) {
		int c;
		c = *p++;
        if ((c>0x10ffff)||(c<0)) { c = 0xfffd; }
		if (c<0x80) {
			*q++ = c;
		} else {
			int max,shft,frst;
		    max = 0x800; shft = 6; frst = 0xffc0;
			while (c>=max) {
                max = max<<5; shft+=6; frst = frst>>1;
			}
			*q++ = (c>>shft)|frst;
            while (shft) {
				shft -= 6;
                *q++ = 0x80|((c>>shft)&0x3f);
			}
		}
	}
    *q++ = 0;
	return q;
}

int *TlsUTF8ToInt(int *d,char *s){
	int *q;
	char *p;
	p = s; q = d;
	while (*p) {
		int c;
		c = *p++;
		if (c&0x80) {
            if (c&0x40) {
                unsigned int msk,nb,v,ok,min,max;
				msk = 0x20; nb=1; v=0; min = 0x80; max = 0x800;
				while (c&msk) { nb++; msk=msk>>1; min = max; max = max<<5; }
				ok = msk>1;
				while (msk) { v = v|(c&msk); msk=msk>>1; }
				while (ok && nb) {
					int v0;
					v0 = *p++;
					ok = ((v0&0xc0)==0xc0);
				    v = (v<<6)|(v0&0x3f);
					nb --;
				}
				ok = ok && (v>=min) && (v<=0x10ffff);
				*q++ = v;
			} else {
				*q++ = -1; // code error
			}
		} else {
			*q++ = c;
		}
	}
	*q++ = 0;
	return q;
}

