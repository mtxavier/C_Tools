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

#ifndef _Debug_h_
#define _Debug_h_

char *DbgPutBaseInt(char *p,int b,int d);
char *DbgPutBaseUint(char *p,int b,int d);
char *DbgPutString(char *p,char *s);
char *DbgPutDecInt(char *p,int d);

#define DbgPutDecInt(p,d) DbgPutBaseInt(p,10,d);
#define DbgPutDecUint(p,d) DbgPutBaseUint(p,10,d);
#define DbgPutBinUint(p,d) DbgPutBaseUint(p,2,d);
#define DbgPutHexUint(p,d) DbgPutBaseUint(p,16,d);

#include <BuffText.h>

BuffText *DbgBuffTextSpy(BuffText *BT);

#endif
