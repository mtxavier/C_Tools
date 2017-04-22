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

#ifndef _BTTools_h_
#define _BTTools_h_

#include <BuffText.h>

char *StringDuplicate(char *St);

/*_____________________________________________________
 |
 |_____________________________________________________
*/


typedef struct _BTList_ BTList;
BuffText *BTListGetBT(BTList *L);
BTList *BTListNew(BuffText *B,...); /* End with 0 */
BTList *BTListCatBuffer(BTList *this,BuffText *B); /* Cat at end */

BuffText *BTIntDecimal(int x);
BuffText *BTCatenate(BuffText *A,...);

#endif

