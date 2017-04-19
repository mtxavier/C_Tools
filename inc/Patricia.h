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

/*------------------------------------------------------------------------------
	Usage is exactly the same than BinTree. See this header for more 
	informations.
	A little difference: Size for SizedString may be calculated with strlen;
	that is: the 0 termination doesn't count against the size. The string "" 
	has size 0.
------------------------------------------------------------------------------*/
#ifndef _Patricia_h_
#define _Patricia_h_

typedef struct _PatriciaNod PatriciaNod;
struct _PatriciaNod {
	long BitToTest;
	char *Key;
	PatriciaNod *Next[2];
};

void PatriciaInit(PatriciaNod *Root); 
/* returns Root if not found */
PatriciaNod *PatriciaSeek(PatriciaNod *Root,char *s);
PatriciaNod *PatriciaSeekOrInsert(
	PatriciaNod *Root,char *s,PatriciaNod *NewNod
);
PatriciaNod *PatriciaRemove(PatriciaNod *Root,char *s);
PatriciaNod *PatriciaForEach(
	PatriciaNod *Root,
	PatriciaNod *(*fn)(PatriciaNod *,void *closure),void *Closure
);

/*---------------------*/

typedef struct PatriciaCtx {
	const char *Key,*c;
	int BitVal;
	int (*GetBitVal)(const char *Key,int n);
	int (*CheckRadixNext)(
		struct PatriciaCtx *ctx,const char *nodKey,int ToTestbit
	);
} PatriciaCtx;

PatriciaNod *cPatriciaSeek(PatriciaCtx *Ctx,PatriciaNod *Root,char *s);
PatriciaNod *cPatriciaSeekOrInsert(
	PatriciaCtx *Ctx,PatriciaNod *Root,char *s,PatriciaNod *NewNod
);
PatriciaNod *cPatriciaRemove(PatriciaCtx *Ctx,PatriciaNod *Root,char *s);

#endif

