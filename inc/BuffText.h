/*  BuffText offers an interface to read text file in a fast way.
 *  It also offers file abstraction so other object might be substituted for files
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

#ifndef _BuffText_h_
#define _BuffText_h_

/*______________________________________________________
 |
 | Classe BuffText : provides quick access to filelike 
 | structures.
 |
 | char *p : 
 |     aim at the current location in the buffer.
 |     Only forward increment (p++) is guaranteed.
 |     Moreover, whenever you step on a 0 value, you
 |     must make a call to Check0 that will tell you
 |     if it is an actual value or an end of file.
 |     When writing back p, you must ensure it is a
 |     valid pointer.
 |
 | char *Check0() :
 |     Updates p so preceding values become 
 |     unavailable. It returns a pointer on the new
 |     location or NULL on end of file. On NULL value,
 |     p is made to point on the end of buffer.
 | int *Seek(dp) :
 |     Move p of dp char. dp can be positive, negative 
 |     or null.
 |     Returns indice of the new position from the 
 |     begining of buffer. For the purpose of seek,
 |     the buffer is considered as a ring so:
 |       - Move to begining-1 moves to the end of buffer.
 |       - Move to begining-2 moves to the last char
 |         and so on...
 |     Note that Seek(0) gives the current position and 
 |     Seek(-(Seek(0)+1)) gives the length of buffer.
 |______________________________________________________
*/
/*______________________________________________________
 |
 |  Example:
 |
 |  GetString(char *dst,BuffText *B){
 |     char *p,*q;
 |     q=dst;
 |     p=B->p;   // Normally, no need to check for Null pointer here.
 |     do {
 |         while(*p) {
 |             *q++=*p++;
 |         }
 |         p = Call(B,Check0,1(p));
 |     } while (p!=NULL&&*p);
 |     if (p!=NULL) {
 |         B->p=p;  // Write the non Null value back.
 |     }
 |     *q=0;
 |  }
 |______________________________________________________
*/


typedef struct {
	struct BuffText *Static;
	char *p;
} BuffText;
struct BuffText {
    char *(*Check0)(BuffText *this,char *p);
	long (*Seek)(BuffText *this,long dp);
	void (*Close)(BuffText *this);  // Any operation on the BuffText will reopen it.
};

BuffText *BuffTextNew(char *s);
BuffText *BTFileNew(char *Name,long BuffSize);

#endif
