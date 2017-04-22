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

#ifndef _Rpg_Core_h_
#define _Rpg_Core_h_

#include <Geo.h>

/*______________________________________________
 |
 |
 |______________________________________________
*/

typedef struct { struct RpgCategory *Static; } RpgCategory;
struct RpgCategory {
	int (*IsA)(RpgCategory *this,RpgCategory *Super);
	RpgCategory *(*Core)(RpgCategory *this);
	int (*GetDepth)(RpgCategory *this);
	RpgCategory *(*IsTiedTo)(RpgCategory *this,RpgCategory *Other); // returns the nature of the relation
};

/*______________________________________________
 |
 |
 |______________________________________________
*/

typedef struct { struct RpgPhysObjType *Static;} RpgPhysObjType;
typedef struct { struct RpgPhysObj *Static;} RpgPhysObj;
typedef struct { struct RpgPhysRel *Static;} RpgPhyRel;
struct RpgPhysObjType {
	RpgCategory *(*Id)(RpgPhysObjType *this);
	int (*Attribut)(RpgPhysObjType *this,RpgCategory *Id,int *Cotes);
	RpgPhysObjType *(*Parts)(RpgPhysObjType *this,int (*Record)(RpgCategory *id,RpgPhysObjType *part,void *Closure),void *Closure);
};
struct RpgPhysObj {
	RpgCategory *(*Id)(RpgPhysObj *this);
	int (*Attribut)(RpgPhysObject *this,RpgCategory *Id);
	RpgPhysObj *(*Parts)(RpgPhysObjType *this,int (*Record)(RpgCategory *id,RpgPhysObj *part,void *Closure),void *Closure);
	void (*Connectivity)(RpgPhysObjType *this,int (*Record)(RpgPhysObj *P0,RpgPhysObj *P1,RpgPhysRel *Rel,void *Clos),void *clos);
};

#endif
